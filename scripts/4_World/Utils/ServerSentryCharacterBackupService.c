/**
    \file ServerSentryCharacterBackupService.c
    \brief Saves bounded per-player snapshots and restores them on the server.
*/

/** \brief Coordinates character persistence independently of telemetry and live logging. */
class ServerSentryCharacterBackupService
{
    /** \brief Committed backup directory. */
    static const string BACKUP_DIR = "$profile:ServerSentry/CharacterBackups";
    /** \brief Terminates each script stream and the complete snapshot. */
    protected const int STREAM_END = 1397965378;
    /** \brief Defensive upper bound for one inventory. */
    protected const int MAX_ENTITIES = 10000;
    /** \brief Shared server instance. */
    protected static ref ServerSentryCharacterBackupService s_Instance;
    /** \brief Current backup configuration. */
    protected ref ServerSentryCharBackupConfig m_Config;
    /** \brief Request throttling survives disconnects for the duration of the mission. */
    protected ref map<string, int> m_LastRestore = new map<string, int>();
    /** \brief Prevents reentrant captures and restores. */
    protected bool m_Busy;
    protected string m_RestorePhase;
    protected string m_LastRestoreFailure;
    protected bool m_RollbackIncomplete;
    /** \brief Whether startup retention has run. */
    protected bool m_Started;
    /** \brief Last filename timestamp used during this mission. */
    protected string m_LastTimestamp;
    /** \brief Monotonic suffix for captures in the same second. */
    protected int m_PathSequence;

    /** \brief Resolves the server configuration. */
    void ServerSentryCharacterBackupService()
    {
        m_Config = ServerSentryConfigManager.Get().GetCharacterBackupConfig();
    }

    /**
        \brief Returns the shared service.
        \return \p server-side service instance.
    */
    static ServerSentryCharacterBackupService Get()
    {
        if (!s_Instance)
            s_Instance = new ServerSentryCharacterBackupService();
        return s_Instance;
    }

    /** \brief Enforces existing retention without scheduling any captures. */
    void Start()
    {
        if (!GetGame().IsServer() || m_Started)
            return;
        if (!FileExist(BACKUP_DIR))
            MakeDirectory(BACKUP_DIR);
        array<string> owners = ListNames(BACKUP_DIR + "/*", true);
        foreach (string owner : owners)
        {
            if (IsSteamId(owner))
                TrimBackups(BACKUP_DIR + "/" + owner, Math.Max(0, m_Config.MaxBackupFiles));
        }
        m_Started = true;
        string settings = "[ServerSentry][CharacterBackup] Settings:";
        settings += " enabled=" + m_Config.EnableCharacterBackup.ToString();
        settings += " minAge=" + m_Config.MinCharacterLiveTimeSeconds.ToString() + "s";
        settings += " minItems=" + m_Config.MinItemsInInventory.ToString();
        settings += " maxFiles=" + m_Config.MaxBackupFiles.ToString();
        Print(settings + " trigger=death");
    }

    /** \brief Clears mission-only authorization and item references. */
    void Stop()
    {
        m_Started = false;
        m_LastRestore.Clear();
        ServerSentryCharacterBackupAction.ResetPermits();
        ServerSentryBackupItemIndex.Stop();
    }

    /**
        \brief Checks identity components before using them in paths.
        \param value \p Steam ID from an authenticated identity or directory name.
        \return \p true for a non-empty numeric identifier.
    */
    protected bool IsSteamId(string value)
    {
        if (value.Length() < 1 || value.Length() > 32)
            return false;
        string digits = "0123456789";
        for (int i = 0; i < value.Length(); i++)
        {
            if (digits.IndexOf(value.Substring(i, 1)) < 0)
                return false;
        }
        return true;
    }

    /**
        \brief Enumerates and closes a directory before callers mutate its files.
        \param pattern \p filesystem search pattern.
        \param directories \p true to enumerate directories instead of files.
        \return \p sorted names, oldest timestamp first.
    */
    protected array<string> ListNames(string pattern, bool directories = false)
    {
        array<string> names = new array<string>();
        string name;
        FileAttr attributes;
        FindFileHandle handle = CF.FindFileEx(pattern, name, attributes, FindFileFlags.ALL);
        if (!handle)
            return names;
        while (true)
        {
            bool isDirectory = attributes == FileAttr.DIRECTORY;
            if (name != "" && name != "." && name != ".." && isDirectory == directories)
                names.Insert(name);
            if (!FindNextFile(handle, name, attributes))
                break;
        }
        CloseFindFile(handle);
        names.Sort();
        return names;
    }

    /**
        \brief Deletes oldest committed snapshots until the configured bound is met.
        \param directory \p one player's backup directory.
        \param keep \p number of committed snapshots to retain.
        \return \p true when retention was enforced successfully.
    */
    protected bool TrimBackups(string directory, int keep)
    {
        array<string> names = ListNames(directory + "/*.bin");
        int removeCount = names.Count() - keep;
        for (int i = 0; i < removeCount; i++)
        {
            if (!DeleteFile(directory + "/" + names[i]))
            {
                Print("[ServerSentry][CharacterBackup] Cannot prune " + directory + "/" + names[i]);
                return false;
            }
            ServerSentryBackupItemIndex.Remove(directory + "/" + names[i]);
            DeleteFile(directory + "/" + names[i] + ".restore-pending");
            DeleteFile(directory + "/" + names[i] + ".restored");
        }
        return true;
    }

    /**
        \brief Produces a chronologically sortable collision-free snapshot filename.
        \param directory \p one player's backup directory.
        \return \p full path, or an empty string if all suffixes are occupied.
    */
    protected string NewPath(string directory)
    {
        string timestamp = ServerSentryTime.NowUtc();
        timestamp.Replace("-", "");
        timestamp.Replace(":", "");
        if (m_LastTimestamp != timestamp)
        {
            m_LastTimestamp = timestamp;
            m_PathSequence = 0;
        }
        array<string> existing = ListNames(directory + "/" + timestamp + "_*.bin");
        if (existing.Count() > 0)
        {
            string newest = existing[existing.Count() - 1];
            m_PathSequence = Math.Max(m_PathSequence, newest.Substring(timestamp.Length() + 1, 4).ToInt() + 1);
        }
        for (int i = m_PathSequence; i < 10000; i++)
        {
            string path = directory + "/" + timestamp + "_" + i.ToStringLen(4) + ".bin";
            if (!FileExist(path))
            {
                m_PathSequence = i + 1;
                return path;
            }
        }
        return "";
    }

    /** Captures a death before vanilla destroys character persistence managers. */
    void BackupOnDeath(PlayerBase player)
    {
        string message;
        bool saved = TryBackupOnDeath(player, message);
        string steamId = BackupOwnerId(player);
        if (steamId != "")
            Print("[ServerSentry][CharacterBackup] Death steamId=" + steamId + " " + message);
    }

    /**
        \brief Resolves the authenticated owner used by capture operations.
        \param player \p capture candidate.
        \return \p Steam ID, or empty when no authenticated identity is present.
    */
    protected string BackupOwnerId(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return "";
        return player.GetIdentity().GetPlainId();
    }

    /**
        \brief Captures a character and reports the exact skip or write outcome.
        \param player \p capture candidate.
        \param message \p receives the capture result or unmet condition.
        \return \p true when a committed backup file was created.
    */
    bool TryBackupOnDeath(PlayerBase player, out string message)
    {
        message = "Backup skipped: service unavailable or busy.";
        if (!GetGame().IsServer() || m_Busy)
            return false;
        if (!m_Config.EnableCharacterBackup || m_Config.MaxBackupFiles <= 0)
        {
            message = "Backup disabled by EnableCharacterBackup or MaxBackupFiles.";
            return false;
        }
        if (!player || player.IsAlive())
        {
            message = "Backup skipped: capture requires a dead character.";
            return false;
        }
        string steamId = BackupOwnerId(player);
        if (!IsSteamId(steamId))
        {
            message = "Backup skipped: no valid authenticated Steam ID.";
            return false;
        }
        if (!player.ServerSentryClaimDeathBackup())
        {
            message = "Backup skipped: this death has already been processed.";
            return false;
        }
        player.StatUpdateByTime(AnalyticsManagerServer.STAT_PLAYTIME);
        if (player.StatGet(AnalyticsManagerServer.STAT_PLAYTIME) < m_Config.MinCharacterLiveTimeSeconds)
        {
            message = "Backup pending: MinCharacterLiveTimeSeconds=" + m_Config.MinCharacterLiveTimeSeconds.ToString() + " has not been reached.";
            return false;
        }
        array<EntityAI> inventory = CharacterInventory(player);
        if (inventory.Count() - 1 < m_Config.MinItemsInInventory)
        {
            message = "Backup pending: MinItemsInInventory=" + m_Config.MinItemsInInventory.ToString() + " has not been reached.";
            return false;
        }
        if (!FileExist(BACKUP_DIR) && !MakeDirectory(BACKUP_DIR))
        {
            message = "Cannot create backup directory: " + BACKUP_DIR;
            return false;
        }
        string directory = BACKUP_DIR + "/" + steamId;
        if (!FileExist(directory) && !MakeDirectory(directory))
        {
            message = "Cannot create player backup directory: " + directory;
            return false;
        }
        string path = NewPath(directory);
        if (path == "")
        {
            message = "Cannot allocate a new backup filename in " + directory;
            return false;
        }
        m_Busy = true;
        string temporary = directory + "/pending.tmp";
        bool saved = SavePlayerState(player, temporary, true);
        ServerSentryBackupItemManifest manifest = ServerSentryBackupItemManifest.Capture(inventory);
        if (saved)
            saved = manifest && ServerSentryBackupItemIndex.WriteManifest(path, manifest);
        message = "Cannot serialize complete character state to " + temporary;
        if (saved)
        {
            saved = CopyFile(temporary, path);
            message = "Cannot commit backup file to " + path;
        }
        if (saved)
            ServerSentryBackupItemIndex.Track(path, manifest, inventory);
        if (saved && !TrimBackups(directory, m_Config.MaxBackupFiles))
        {
            DeleteFile(path);
            saved = false;
            message = "Cannot enforce backup retention in " + directory;
        }
        DeleteFile(temporary);
        m_Busy = false;
        if (saved)
            message = "Backup saved: " + path;
        else
        {
            DeleteFile(path);
            ServerSentryBackupItemIndex.Remove(path);
        }
        return saved;
    }

    /**
        \brief Enumerates a deterministic parent-before-child tree with the player at index zero.
        \param player \p inventory owner.
        \return \p preorder inventory including the player exactly once.
    */
    protected array<EntityAI> CharacterInventory(PlayerBase player)
    {
        array<EntityAI> entities = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, entities);
        int index = entities.Find(player);
        if (index >= 0)
            entities.RemoveOrdered(index);
        entities.InsertAt(player, 0);
        return entities;
    }

    /**
        \brief Writes a complete versioned snapshot including script persistence for every entity.
        \param player \p living character with an authenticated identity.
        \param path \p private output path below the server profile.
        \return \p true when all explicit writes and stream terminators succeed.
    */
    bool SavePlayerState(PlayerBase player, string path, bool death = false)
    {
        if (!GetGame().IsServer() || !player || !IsSteamId(BackupOwnerId(player)))
            return false;

        bool isAlive = player.IsAlive();
        if ((!death && !isAlive) || (death && isAlive))
            return false;
        array<EntityAI> entities = CharacterInventory(player);
        if (entities.Count() > MAX_ENTITIES)
            return false;
        ServerSentryCharacterBackupHeader header = new ServerSentryCharacterBackupHeader();
        if (death)
            header.Version = 3;
        header.SaveVersion = GetGame().SaveVersion();
        header.WorldName = GetGame().GetWorldName();
        header.SteamId = BackupOwnerId(player);
        header.CreatedUtc = ServerSentryTime.NowUtc();
        header.Position = player.GetPosition();
        header.Orientation = player.GetOrientation();
        header.Stamina = player.GetStaminaHandler().GetStamina();
        header.Restrained = player.IsRestrained();
        header.Unconscious = player.IsUnconscious();
        if (death)
        {
            header.Restrained = false;
            header.Unconscious = false;
        }
        header.Count = entities.Count();
        player.ServerSentryCaptureBackupStatistics(header.Statistics);

        FileSerializer file = new FileSerializer();
        if (!file.Open(path, FileMode.WRITE))
            return false;
        bool success = file.Write(header);
        foreach (EntityAI entity : entities)
        {
            if (!success || !entity)
            {
                success = false;
                break;
            }
            ServerSentryBackupEntityState state = new ServerSentryBackupEntityState();
            success = state.Capture(entity, player, entities);
            if (success && death && entity == player)
            {
                foreach (ServerSentryBackupDamage damage : state.Damage)
                {
                    for (int pool = 0; pool < damage.Pools.Count(); pool++)
                    {
                        if (damage.Pools[pool] >= 0)
                            damage.Pools[pool] = 1;
                    }
                }
            }
            if (success)
                success = file.Write(state);
            if (success)
            {
                entity.OnStoreSave(file);
                success = file.Write(STREAM_END);
            }
        }
        if (success)
            success = file.Write(STREAM_END);
        file.Close();
        if (!success)
            DeleteFile(path);
        return success;
    }

    /**
        \brief Validates a header before reading scripted state or creating inventory.
        \param file \p open input stream.
        \param player \p authenticated restore recipient.
        \param header \p receives the validated header.
        \return \p true for a supported snapshot owned by this player.
    */
    protected bool ReadHeader(FileSerializer file, PlayerBase player, out ServerSentryCharacterBackupHeader header)
    {
        if (!file.Read(header) || !header || (header.Version != 2 && header.Version != 3) || header.SaveVersion != GetGame().SaveVersion())
            return false;
        if (header.WorldName != GetGame().GetWorldName() || header.SteamId != BackupOwnerId(player) || header.Count < 1 || header.Count > MAX_ENTITIES)
            return false;
        if (!header.Statistics || header.Statistics.Count() != 5 || !(header.Stamina >= 0 && header.Stamina <= player.GetStaminaHandler().GetStaminaMax()))
            return false;
        foreach (float statistic : header.Statistics)
        {
            if (!(statistic >= 0 && statistic < 1.0e30))
                return false;
        }
        for (int i = 0; i < 3; i++)
        {
            if (!(header.Position[i] > -1000000 && header.Position[i] < 1000000 && header.Orientation[i] >= -360 && header.Orientation[i] <= 360))
                return false;
        }
        return true;
    }

    /**
        \brief Reads one checked script-stream boundary.
        \param file \p open input stream.
        \return \p true when the stream ended at the expected position.
    */
    protected bool ReadEnd(FileSerializer file)
    {
        int end;
        return file.Read(end) && end == STREAM_END;
    }

    /**
        \brief Builds and validates an isolated character tree before touching the recipient.
        \param player \p authenticated recipient.
        \param path \p server-selected snapshot path.
        \param staged \p receives the temporary character, including on failure for cleanup.
        \param entities \p receives recreated entities in preorder.
        \param states \p receives their saved native state.
        \param header \p receives the snapshot metadata.
        \return \p true when the complete snapshot and every inventory location loaded.
    */
    protected bool Stage(PlayerBase player, string path, out PlayerBase staged, out array<EntityAI> entities, out array<ref ServerSentryBackupEntityState> states, out ServerSentryCharacterBackupHeader header)
    {
        entities = new array<EntityAI>();
        states = new array<ref ServerSentryBackupEntityState>();
        FileSerializer file = new FileSerializer();
        if (!file.Open(path, FileMode.READ))
            return false;
        bool success = ReadHeader(file, player, header);
        for (int i = 0; success && i < header.Count; i++)
        {
            ServerSentryBackupEntityState state;
            success = file.Read(state);
            if (!success || !state || !state.IsValid())
            {
                success = false;
                break;
            }
            EntityAI entity;
            if (i == 0)
            {
                // Loading a different survivor class into a live instance is unsafe.
                if (state.Type != player.GetType() || state.Parent != -1)
                {
                    Print("[ServerSentry][CharacterBackup] Survivor type mismatch or invalid root in " + path);
                    success = false;
                    break;
                }
                staged = PlayerBase.Cast(GetGame().CreatePlayer(null, state.Type, "0 -1000 0", 0, "NONE"));
                entity = staged;
                if (staged)
                    staged.SetAllowDamage(false);
            }
            else if (state.Parent >= 0 && state.Parent < i)
            {
                InventoryLocation destination = state.Destination(entities[state.Parent], null);
                if (destination)
                    entity = GameInventory.LocationCreateEntity(destination, state.Type, ECE_KEEPHEIGHT, RF_DEFAULT);
            }
            if (!entity)
            {
                Print("[ServerSentry][CharacterBackup] Cannot create " + state.Type + " at saved inventory location in " + path);
                success = false;
                break;
            }
            entities.Insert(entity);
            states.Insert(state);
            success = entity.OnStoreLoad(file, header.SaveVersion);
            if (success)
                success = ReadEnd(file);
        }
        if (success)
            success = ReadEnd(file);
        file.Close();
        if (!success)
            Print("[ServerSentry][CharacterBackup] Invalid or incompatible persistence stream in " + path);
        // Children must exist before weapon/energy/container post-load callbacks run.
        for (int index = entities.Count() - 1; success && index >= 0; index--)
        {
            success = states[index].Apply(entities[index]);
            if (success && index > 0)
                entities[index].AfterStoreLoad();
            if (!success)
                Print("[ServerSentry][CharacterBackup] Cannot restore native state for " + states[index].Type);
        }
        if (success)
            success = staged.IsAlive() && ApplyEnergy(entities, states);
        if (success)
            staged.SetAllowDamage(false);
        return success;
    }

    /**
        \brief Applies only the character portion of an already validated snapshot.
        \param player \p existing connected character.
        \param path \p snapshot or rollback file.
        \return \p true when script state, native damage and player metadata loaded.
    */
    protected bool LoadCharacter(PlayerBase player, string path)
    {
        FileSerializer file = new FileSerializer();
        if (!file.Open(path, FileMode.READ))
            return false;
        ServerSentryCharacterBackupHeader header;
        ServerSentryBackupEntityState state;
        bool success = ReadHeader(file, player, header);
        if (success)
            success = file.Read(state);
        if (success)
            success = state && state.Type == player.GetType() && state.Parent == -1;
        if (success)
        {
            player.ServerSentryPrepareBackupLoad();
            success = player.OnStoreLoad(file, header.SaveVersion);
        }
        if (success)
            success = ReadEnd(file);
        file.Close();
        if (success)
            success = state.Apply(player);
        if (success)
        {
            player.SetPosition(header.Position);
            player.SetOrientation(header.Orientation);
            player.AfterStoreLoad();
            player.ServerSentryFinishBackupLoad(header);
        }
        return success;
    }

    /**
        \brief Moves a top-level entity to its saved location under another character.
        \param entity \p existing inventory tree root.
        \param state \p saved destination metadata.
        \param player \p destination character.
        \return \p true when the synchronized inventory move succeeds.
    */
    protected bool MoveRoot(EntityAI entity, ServerSentryBackupEntityState state, PlayerBase player)
    {
        return Move(entity, state.Destination(player, entity));
    }

    protected bool AtLocation(EntityAI entity, InventoryLocation destination)
    {
        InventoryLocation source = new InventoryLocation();
        if (!destination || !entity || !entity.GetInventory().GetCurrentInventoryLocation(source) || !source.CompareLocationOnly(destination))
            return false;
        if (source.GetType() == InventoryLocationType.GROUND)
            return vector.DistanceSq(source.GetPos(), destination.GetPos()) < 0.0001;
        return true;
    }

    protected InventoryLocation GroundLocation(EntityAI entity, vector position)
    {
        InventoryLocation destination = new InventoryLocation();
        vector transform[4];
        entity.GetTransform(transform);
        transform[3] = position;
        destination.SetGround(entity, transform);
        return destination;
    }

    protected string LocationSummary(InventoryLocation location)
    {
        if (!location)
            return "invalid";
        string parent = "world";
        if (location.GetParent())
            parent = location.GetParent().GetType();
        string summary = "type=" + location.GetType().ToString();
        summary += " parent=" + parent;
        summary += " slot=" + location.GetSlot().ToString();
        summary += " row=" + location.GetRow().ToString();
        summary += " col=" + location.GetCol().ToString();
        return summary;
    }

    protected bool MoveFailed(EntityAI entity, InventoryLocation source, InventoryLocation destination, string reason)
    {
        string type = "item";
        if (entity)
            type = entity.GetType();
        if (m_LastRestoreFailure == "")
            m_LastRestoreFailure = "Could not move " + type + " during " + m_RestorePhase + ".";
        string diagnostic = "[ServerSentry][CharacterBackup] Move failed phase=" + m_RestorePhase;
        diagnostic += " item=" + type + " reason=" + reason;
        diagnostic += " source={" + LocationSummary(source) + "}";
        diagnostic += " destination={" + LocationSummary(destination) + "}";
        Print(diagnostic);
        return false;
    }

    protected bool Ground(EntityAI entity, vector position, bool template = false)
    {
        InventoryLocation source = new InventoryLocation();
        if (!entity.GetInventory().GetCurrentInventoryLocation(source))
            return MoveFailed(entity, source, null, "source unavailable");
        if (source.GetType() == InventoryLocationType.GROUND)
            return true;
        return Move(entity, GroundLocation(entity, position), false, template);
    }

    protected bool Move(EntityAI entity, InventoryLocation destination, bool rollback = false, bool template = false)
    {
        InventoryLocation source = new InventoryLocation();
        if (!entity || !destination || !entity.GetInventory().GetCurrentInventoryLocation(source))
            return MoveFailed(entity, source, destination, "location unavailable");
        if (AtLocation(entity, destination))
            return true;
        // Only temporary template items bypass gameplay release rules. In
        // particular, a template magazine need not be unloaded via weapon input.
        // Real originals still pass normal removal checks. Exact rollback uses
        // native destination checks because a corpse cannot be newly equipped.
        bool allowed;
        if (rollback || template)
            allowed = GameInventory.LocationTestAddEntity(destination, true, true, true, true, false, false);
        else
            allowed = GameInventory.LocationCanMoveEntity(source, destination);
        if (!allowed)
            return MoveFailed(entity, source, destination, "inventory check refused");
        if (!GameInventory.LocationSyncMoveEntity(source, destination))
            return MoveFailed(entity, source, destination, "native move refused");
        if (!AtLocation(entity, destination))
            return MoveFailed(entity, source, destination, "destination not confirmed");
        return true;
    }

    /** Saves original locations before any item is taken, including other players. */
    protected bool FindOriginals(ServerSentryBackupItemManifest manifest, array<EntityAI> restored, array<ref ServerSentryBackupItemMove> moves)
    {
        for (int i = 1; i < restored.Count(); i++)
        {
            EntityAI item;
            if (!ServerSentryBackupItemIndex.Resolve(manifest.Items[i], item))
                return false;
            if (!item)
                continue;
            foreach (ServerSentryBackupItemMove known : moves)
            {
                if (known.Item == item)
                    return false;
            }
            ServerSentryBackupItemMove move = new ServerSentryBackupItemMove();
            move.Item = item;
            move.Index = i;
            move.Location = new InventoryLocation();
            if (!item.GetInventory().GetCurrentInventoryLocation(move.Location))
                return false;
            EntityAI parent = item.GetHierarchyParent();
            while (parent && move.Depth < 64)
            {
                move.Depth++;
                parent = parent.GetHierarchyParent();
            }
            if (parent)
                return false;
            moves.Insert(move);
            restored[i] = item;
        }
        return true;
    }

    /** Reject gameplay-blocked moves before any live inventory is changed. */
    protected bool PreflightOriginals(array<ref ServerSentryBackupItemMove> moves, array<EntityAI> restored, array<ref ServerSentryBackupEntityState> states, vector position)
    {
        foreach (ServerSentryBackupItemMove move : moves)
        {
            ServerSentryBackupEntityState state = states[move.Index];
            if (AtLocation(move.Item, state.Destination(restored[state.Parent], move.Item)))
                continue;
            InventoryLocation source = new InventoryLocation();
            InventoryLocation ground = GroundLocation(move.Item, position);
            if (!move.Item.GetInventory().GetCurrentInventoryLocation(source) || !GameInventory.LocationCanMoveEntity(source, ground))
                return MoveFailed(move.Item, source, ground, "preflight refused");
        }
        return true;
    }

    /** Leave matching parent/slot relationships intact, including loaded weapons. */
    protected bool DetachOriginals(array<ref ServerSentryBackupItemMove> moves, array<EntityAI> restored, array<ref ServerSentryBackupEntityState> states, vector position)
    {
        for (int depth = 64; depth >= 0; depth--)
        {
            foreach (ServerSentryBackupItemMove move : moves)
            {
                if (move.Depth != depth)
                    continue;
                ServerSentryBackupEntityState state = states[move.Index];
                if (!AtLocation(move.Item, state.Destination(restored[state.Parent], move.Item)) && !Ground(move.Item, position))
                    return false;
            }
        }
        return true;
    }

    /** Only changed relationships need rollback; never disassemble an intact weapon. */
    protected bool ReturnOriginals(array<ref ServerSentryBackupItemMove> moves, vector position)
    {
        bool recovered = true;
        foreach (ServerSentryBackupItemMove candidate : moves)
            candidate.ReturnRequired = !AtLocation(candidate.Item, candidate.Location);
        for (int reverseDepth = 64; reverseDepth >= 0; reverseDepth--)
        {
            foreach (ServerSentryBackupItemMove detach : moves)
            {
                if (detach.ReturnRequired && detach.Depth == reverseDepth && !Ground(detach.Item, position))
                    recovered = false;
            }
        }
        for (int depth = 0; depth <= 64; depth++)
        {
            foreach (ServerSentryBackupItemMove move : moves)
            {
                if (move.ReturnRequired && move.Depth == depth && !Move(move.Item, move.Location, true))
                    recovered = false;
            }
        }
        return recovered;
    }

    /** Detach only boundaries between retained template and original subtrees. */
    protected bool PrepareTemplates(array<EntityAI> clones, array<EntityAI> restored, array<ref ServerSentryBackupEntityState> states, vector position)
    {
        for (int index = clones.Count() - 1; index > 0; index--)
        {
            int parent = states[index].Parent;
            if (clones[index] == restored[index] && clones[parent] != restored[parent] && !Ground(clones[index], position, true))
                return false;
        }
        // All retained descendants are now outside unused templates. No original
        // has been attached to the staged tree yet, so these are disposable copies.
        for (int unused = clones.Count() - 1; unused > 0; unused--)
        {
            if (clones[unused] != restored[unused] && clones[unused])
                GetGame().ObjectDelete(clones[unused]);
        }
        return true;
    }

    /** All clones are retained separately, so rollback never deletes an original. */
    protected bool ContainsOriginal(EntityAI container, array<ref ServerSentryBackupItemMove> moves)
    {
        foreach (ServerSentryBackupItemMove move : moves)
        {
            EntityAI parent = move.Item;
            while (parent)
            {
                if (parent == container)
                    return true;
                parent = parent.GetHierarchyParent();
            }
        }
        return false;
    }

    protected void DeleteClones(array<EntityAI> clones, array<ref ServerSentryBackupItemMove> moves)
    {
        for (int i = clones.Count() - 1; i > 0; i--)
        {
            if (clones[i] && !ContainsOriginal(clones[i], moves))
                GetGame().ObjectDelete(clones[i]);
        }
    }

    protected bool Marker(string path)
    {
        FileHandle file = OpenFile(path, FileMode.WRITE);
        if (!file)
            return false;
        FPrint(file, ServerSentryTime.NowUtc());
        CloseFile(file);
        return FileExist(path);
    }

    /**
        \brief Restores a validated snapshot while retaining original entities until commit.
        \param player \p living connected character.
        \param path \p server-selected backup path.
        \return \p true when the complete inventory and character state were restored.
    */
    bool LoadPlayerState(PlayerBase player, string path)
    {
        m_LastRestoreFailure = "";
        m_RollbackIncomplete = false;
        m_RestorePhase = "validation";
        if (!GetGame().IsServer() || !player || !player.IsAlive() || !IsSteamId(BackupOwnerId(player)))
            return false;
        if (FileExist(path + ".restored") || FileExist(path + ".restore-pending"))
            return false;
        PlayerBase staged;
        array<EntityAI> restored;
        array<ref ServerSentryBackupEntityState> states;
        ServerSentryCharacterBackupHeader header;
        bool success = Stage(player, path, staged, restored, states, header);
        if (!success)
        {
            if (staged)
                GetGame().ObjectDelete(staged);
            return false;
        }
        array<EntityAI> clones = new array<EntityAI>();
        clones.Copy(restored);
        array<ref ServerSentryBackupItemMove> moves = new array<ref ServerSentryBackupItemMove>();
        ServerSentryBackupItemManifest manifest = ServerSentryBackupItemIndex.ReadManifest(path);
        if ((header.Version == 3 && !manifest) || (manifest && (!manifest.Matches(states) || !FindOriginals(manifest, restored, moves))))
        {
            GetGame().ObjectDelete(staged);
            return false;
        }
        m_RestorePhase = "checking original items";
        if (!PreflightOriginals(moves, restored, states, player.GetPosition()))
        {
            GetGame().ObjectDelete(staged);
            return false;
        }
        string rollback = BACKUP_DIR + "/" + BackupOwnerId(player) + "/restore.rollback";
        if (FileExist(rollback))
        {
            m_LastRestoreFailure = "A previous restore needs review. Its recovery snapshot must be preserved.";
            GetGame().ObjectDelete(staged);
            return false;
        }
        if (!SavePlayerState(player, rollback))
        {
            GetGame().ObjectDelete(staged);
            return false;
        }
        array<EntityAI> original = CharacterInventory(player);
        array<ref ServerSentryBackupEntityState> originalStates = new array<ref ServerSentryBackupEntityState>();
        foreach (EntityAI originalEntity : original)
        {
            ServerSentryBackupEntityState originalState = new ServerSentryBackupEntityState();
            if (!originalState.Capture(originalEntity, player, original))
            {
                GetGame().ObjectDelete(staged);
                DeleteFile(rollback);
                return false;
            }
            originalStates.Insert(originalState);
        }
        // Persist the in-progress guard before mutation. A crash/unknown outcome
        // cannot replay this backup and spawn replacements for a second time.
        if (!Marker(path + ".restore-pending"))
        {
            GetGame().ObjectDelete(staged);
            DeleteFile(rollback);
            return false;
        }
        bool characterTouched;
        m_RestorePhase = "clearing recipient inventory";
        for (int oldIndex = 1; success && oldIndex < original.Count(); oldIndex++)
        {
            if (originalStates[oldIndex].Parent == 0)
                success = Ground(original[oldIndex], player.GetPosition());
        }
        m_RestorePhase = "collecting original items";
        if (success)
            success = DetachOriginals(moves, restored, states, player.GetPosition());
        m_RestorePhase = "preparing replacement items";
        if (success)
            success = PrepareTemplates(clones, restored, states, staged.GetPosition());
        m_RestorePhase = "assembling restored inventory";
        for (int attachIndex = 1; success && attachIndex < restored.Count(); attachIndex++)
        {
            EntityAI itemToAttach = restored[attachIndex];
            int parentIndex = states[attachIndex].Parent;
            EntityAI parent = restored[parentIndex];
            InventoryLocation destination = states[attachIndex].Destination(parent, itemToAttach);
            success = Move(itemToAttach, destination);
        }
        m_RestorePhase = "equipping restored inventory";
        for (int rootIndex = 1; success && rootIndex < restored.Count(); rootIndex++)
        {
            if (states[rootIndex].Parent == 0)
                success = MoveRoot(restored[rootIndex], states[rootIndex], player);
        }
        restored[0] = player;
        // Do not refill ammunition, quantities or energy in an existing original.
        for (int energyIndex = restored.Count() - 1; success && energyIndex > 0; energyIndex--)
        {
            if (restored[energyIndex] == clones[energyIndex])
                success = states[energyIndex].ApplyEnergy(restored[energyIndex], restored);
        }
        if (success)
        {
            m_RestorePhase = "loading character state";
            characterTouched = true;
            success = LoadCharacter(player, path);
        }
        if (success)
        {
            ApplyQuickbar(player, restored, states);
            // Dispose only the unused template copies, never reclaimed originals.
            for (int unused = clones.Count() - 1; unused > 0; unused--)
            {
                if (restored[unused] != clones[unused] && clones[unused])
                    GetGame().ObjectDelete(clones[unused]);
            }
            for (int deleteIndex = 1; deleteIndex < original.Count(); deleteIndex++)
            {
                if (original[deleteIndex] && originalStates[deleteIndex].Parent == 0 && restored.Find(original[deleteIndex]) < 0)
                    GetGame().ObjectDelete(original[deleteIndex]);
            }
            if (Marker(path + ".restored"))
                DeleteFile(path + ".restore-pending");
            else
                Print("[ServerSentry][CharacterBackup] Restore committed; pending guard retained because completion marker could not be written.");
            ServerSentryBackupItemIndex.Remove(path, false);
            DeleteFile(rollback);
            Print("[ServerSentry][CharacterBackup] Reclaimed=" + moves.Count().ToString() + " Replaced=" + (restored.Count() - 1 - moves.Count()).ToString());
        }
        else
        {
            m_RestorePhase = "rollback";
            bool recovered = ReturnOriginals(moves, player.GetPosition());
            DeleteClones(clones, moves);
            for (int recoverIndex = 1; recoverIndex < original.Count(); recoverIndex++)
            {
                if (originalStates[recoverIndex].Parent == 0 && !MoveRoot(original[recoverIndex], originalStates[recoverIndex], player))
                    recovered = false;
            }
            if (characterTouched && !LoadCharacter(player, rollback))
                recovered = false;
            if (!ApplyEnergy(original, originalStates))
                recovered = false;
            ApplyQuickbar(player, original, originalStates);
            if (recovered)
            {
                DeleteFile(rollback);
                DeleteFile(path + ".restore-pending");
            }
            else
            {
                m_RollbackIncomplete = true;
                Print("[ServerSentry][CharacterBackup] Rollback incomplete; restore locked and recovery snapshot kept at " + rollback);
            }
        }
        if (!ContainsOriginal(staged, moves))
            GetGame().ObjectDelete(staged);
        else
            Print("[ServerSentry][CharacterBackup] Recovery container retained to protect original items after failed rollback.");
        return success;
    }

    /**
        \brief Restores energy sources before consumers after an inventory transfer.
        \param entities \p complete inventory tree.
        \param states \p corresponding saved metadata.
        \return \p true when all energy state was restored.
    */
    protected bool ApplyEnergy(array<EntityAI> entities, array<ref ServerSentryBackupEntityState> states)
    {
        for (int i = entities.Count() - 1; i >= 0; i--)
        {
            if (!entities[i] || !states[i].ApplyEnergy(entities[i], entities))
                return false;
        }
        return true;
    }

    /**
        \brief Replaces shortcuts after inventory and player state have committed.
        \param player \p inventory owner.
        \param entities \p restored preorder tree.
        \param states \p corresponding saved metadata.
    */
    protected void ApplyQuickbar(PlayerBase player, array<EntityAI> entities, array<ref ServerSentryBackupEntityState> states)
    {
        for (int slot = 0; slot < player.GetQuickBarSize(); slot++)
            player.SetQuickBarEntityShortcut(null, slot, true);
        for (int i = 1; i < entities.Count(); i++)
        {
            if (states[i].Quickbar >= 0 && states[i].Quickbar < player.GetQuickBarSize())
                player.SetQuickBarEntityShortcut(entities[i], states[i].Quickbar, true);
        }
    }

    /** \brief Accepts only committed filenames generated by NewPath; never a client path. */
    static bool ValidBackupId(string id)
    {
        if (id.Length() != 25 || id.Substring(8, 1) != "T" || id.Substring(15, 1) != "Z" || id.Substring(16, 1) != "_" || id.Substring(21, 4) != ".bin")
            return false;
        string digits = "0123456789";
        for (int i = 0; i < 21; i++)
        {
            if (i != 8 && i != 15 && i != 16 && digits.IndexOf(id.Substring(i, 1)) < 0)
                return false;
        }
        return true;
    }

    /** \brief Reads ten native headers and identity sidecars per page; no inventory script streams. */
    ServerSentryCharacterBackupsDto ListBackups(string steamId, int page, PlayerBase player)
    {
        ServerSentryCharacterBackupsDto result = new ServerSentryCharacterBackupsDto();
        result.steamId = steamId;
        result.page = page;
        if (!IsSteamId(steamId))
            return result;
        string directory = BACKUP_DIR + "/" + steamId;
        array<string> names = ListNames(directory + "/*.bin");
        array<string> committed = new array<string>();
        foreach (string name : names)
        {
            if (ValidBackupId(name))
                committed.Insert(name);
        }
        result.total = committed.Count();
        if (player && player.IsAlive() && m_Config.EnableCharacterBackup && m_Config.MaxBackupFiles > 0)
            result.canRestore = 1;
        int first = result.total - 1 - (page - 1) * 10;
        for (int index = first; index >= 0 && index > first - 10; index--)
        {
            ServerSentryCharacterBackupDto item = new ServerSentryCharacterBackupDto();
            item.id = committed[index];
            string path = directory + "/" + item.id;
            FileSerializer file = new FileSerializer();
            if (file.Open(path, FileMode.READ))
            {
                ReadBackupListMetadata(file, item, steamId, player, path);
                file.Close();
            }
            result.backups.Insert(item);
        }
        return result;
    }

    /**
        \brief Reads listing metadata using guards while the caller owns the open file.
        \param file \p open backup stream.
        \param item \p listing entry receiving valid metadata.
        \param steamId \p authenticated snapshot owner.
        \param player \p optional current restore recipient.
        \param path \p server-owned backup path.
    */
    protected void ReadBackupListMetadata(FileSerializer file, ServerSentryCharacterBackupDto item, string steamId, PlayerBase player, string path)
    {
        ServerSentryCharacterBackupHeader header;
        if (!file.Read(header) || !header)
            return;

        if (header.Version != 2 && header.Version != 3)
            return;

        if (header.SteamId != steamId || header.Count < 1 || header.Count > MAX_ENTITIES)
            return;

        item.createdAtUtc = header.CreatedUtc;
        item.itemCount = header.Count - 1;
        item.worldName = header.WorldName;
        if (header.SaveVersion != GetGame().SaveVersion() || header.WorldName != GetGame().GetWorldName())
            return;

        ServerSentryBackupEntityState state;
        if (!file.Read(state) || !state || state.Parent != -1)
            return;

        if (player && state.Type != player.GetType())
            return;

        if (FileExist(path + ".restored") || FileExist(path + ".restore-pending"))
            return;

        if (header.Version == 3 && !ServerSentryBackupItemIndex.ReadManifest(path))
            return;

        item.compatible = 1;
    }

    /**
        \brief Restores an exact server-owned snapshot after the CMS action validates its one-use permit.
        \param player \p authenticated restore recipient.
        \param message \p receives a client-visible result.
        \return \p true when restoration completed.
    */
    bool RestoreSelected(PlayerBase player, string backupId, out string message)
    {
        message = "Character restore is unavailable.";
        if (!GetGame().IsServer() || m_Busy || !m_Config.EnableCharacterBackup || m_Config.MaxBackupFiles <= 0)
            return false;
        if (!player || !player.GetIdentity() || !player.IsAlive())
            return false;
        if (!CanRestoreWhileIdle(player))
        {
            message = "Stand still on foot and finish the current action before restoring.";
            return false;
        }
        string steamId = player.GetIdentity().GetPlainId();
        if (!IsSteamId(steamId))
            return false;
        int now = GetGame().GetTime();
        int last;
        if (m_LastRestore.Find(steamId, last) && now >= last && now - last < Math.Max(5, m_Config.RestoreCooldownSeconds) * 1000)
        {
            message = "Please wait before requesting another character restore.";
            return false;
        }
        m_LastRestore.Set(steamId, now);
        string directory = BACKUP_DIR + "/" + steamId;
        if (!ValidBackupId(backupId) || !FileExist(directory + "/" + backupId))
        {
            message = "This backup no longer exists. Reload the backup list.";
            return false;
        }
        ServerSentryInventoryAction.InvalidatePlayerCapture(steamId);
        m_Busy = true;
        bool success = LoadPlayerState(player, directory + "/" + backupId);
        m_Busy = false;
        if (success)
            message = "Character backup restored: inventory, position and character state.";
        else
        {
            message = m_LastRestoreFailure;
            if (message == "")
                message = "Restore failed. Check the server log and backup compatibility.";
            if (m_RollbackIncomplete)
                message += " Recovery is incomplete; this backup is locked for review.";
        }
        Print("[ServerSentry][CharacterBackup] Restore steamId=" + steamId + " backup=" + backupId + " success=" + success.ToString());
        return success;
    }

    /**
        \brief Checks movement, restraint and active commands without changing player state.
        \param player \p validated living restore recipient.
        \return \p true when no conflicting movement or action is active.
    */
    protected bool CanRestoreWhileIdle(PlayerBase player)
    {
        if (player.IsInVehicle() || player.IsClimbing() || player.IsClimbingLadder() || player.IsSwimming())
            return false;

        if (player.IsRestrained() || player.IsUnconscious() || player.IsRaised())
            return false;

        if (player.GetCommand_Melee() || player.GetWeaponManager().IsRunning())
            return false;

        return !player.GetActionManager().GetRunningAction();
    }
}
