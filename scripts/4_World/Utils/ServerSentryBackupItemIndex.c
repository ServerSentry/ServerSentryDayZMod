/** Exact identity of one backed-up entity; no class-name-only matching. */
class ServerSentryBackupItemIdentity
{
    string Type;
    string PersistentKey;
    string RuntimeKey;

    static string Persistent(EntityAI entity)
    {
        int a, b, c, d;
        entity.GetPersistentID(a, b, c, d);
        if (a == 0 && b == 0 && c == 0 && d == 0)
            return "";
        return a.ToString() + ":" + b.ToString() + ":" + c.ToString() + ":" + d.ToString();
    }
}

/** Identity sidecar keeps the existing binary persistence layout readable. */
class ServerSentryBackupItemManifest
{
    int Version = 1;
    ref array<ref ServerSentryBackupItemIdentity> Items = new array<ref ServerSentryBackupItemIdentity>();

    static ServerSentryBackupItemManifest Capture(array<EntityAI> entities)
    {
        ServerSentryBackupItemManifest manifest = new ServerSentryBackupItemManifest();
        foreach (EntityAI entity : entities)
        {
            if (!entity)
                return null;
            ServerSentryBackupItemIdentity item = new ServerSentryBackupItemIdentity();
            item.Type = entity.GetType();
            item.PersistentKey = ServerSentryBackupItemIdentity.Persistent(entity);
            item.RuntimeKey = ServerSentryBackupItemIndex.Runtime(entity, true);
            manifest.Items.Insert(item);
        }
        return manifest;
    }

    bool Matches(array<ref ServerSentryBackupEntityState> states)
    {
        if (Version != 1 || !Items || Items.Count() != states.Count() || Items.Count() > 10000)
            return false;
        for (int i = 0; i < Items.Count(); i++)
        {
            if (!Items[i] || Items[i].Type != states[i].Type || Items[i].RuntimeKey == "")
                return false;
        }
        return true;
    }
}

/** Tracks only IDs referenced by retained backups, rather than every world item. */
class ServerSentryBackupItemIndex
{
    protected static ref map<string, int> s_Wanted = new map<string, int>();
    protected static ref map<string, EntityAI> s_Found = new map<string, EntityAI>();
    protected static ref map<string, bool> s_Ambiguous = new map<string, bool>();
    protected static ref map<string, ref ServerSentryBackupItemManifest> s_Manifests = new map<string, ref ServerSentryBackupItemManifest>();
    protected static string s_Epoch;
    protected static int s_Sequence;
    protected static bool s_Initialized;
    protected static bool s_Stopped;

    static string Runtime(EntityAI entity, bool create = false)
    {
        ItemBase item = ItemBase.Cast(entity);
        if (!item)
            return "character";
        return item.ServerSentryBackupRuntimeKey(create);
    }

    static string NewRuntimeKey()
    {
        if (s_Epoch == "")
        {
            int uuid[4];
            UUIDApi.Generate(uuid);
            s_Epoch = UUIDApi.FormatString(uuid);
        }
        s_Sequence++;
        return s_Epoch + ":" + s_Sequence.ToString();
    }

    static void Stop()
    {
        s_Stopped = true;
        s_Found.Clear();
        s_Wanted.Clear();
        s_Ambiguous.Clear();
        s_Manifests.Clear();
    }

    static void BeginMission()
    {
        s_Stopped = false;
        s_Initialized = false;
    }

    protected static array<string> Names(string pattern, bool directories)
    {
        array<string> names = new array<string>();
        string name;
        FileAttr attributes;
        FindFileHandle handle = CF.FindFileEx(pattern, name, attributes, FindFileFlags.ALL);
        if (!handle)
            return names;
        while (true)
        {
            if (name != "." && name != ".." && (attributes == FileAttr.DIRECTORY) == directories)
                names.Insert(name);
            if (!FindNextFile(handle, name, attributes))
                break;
        }
        CloseFindFile(handle);
        return names;
    }

    /** Runs once before the first persistence-load observation, never on a timer. */
    protected static void Initialize()
    {
        if (s_Initialized || s_Stopped)
            return;
        s_Initialized = true;
        string root = ServerSentryCharacterBackupService.BACKUP_DIR;
        array<string> owners = Names(root + "/*", true);
        foreach (string owner : owners)
        {
            if (owner.Length() != 17)
                continue;
            array<string> files = Names(root + "/" + owner + "/*.bin", false);
            foreach (string file : files)
            {
                if (!ServerSentryCharacterBackupService.ValidBackupId(file))
                    continue;
                string path = root + "/" + owner + "/" + file;
                if (FileExist(path + ".restored"))
                    continue;
                ServerSentryBackupItemManifest manifest = ReadManifest(path);
                if (manifest)
                    Add(path, manifest);
            }
        }
    }

    static ServerSentryBackupItemManifest ReadManifest(string path)
    {
        ServerSentryBackupItemManifest manifest;
        string error;
        if (!FileExist(path + ".items.json") || !JsonFileLoader<ServerSentryBackupItemManifest>.LoadFile(path + ".items.json", manifest, error))
            return null;
        if (!manifest || manifest.Version != 1 || !manifest.Items || manifest.Items.Count() < 1 || manifest.Items.Count() > 10000)
            return null;
        foreach (ServerSentryBackupItemIdentity item : manifest.Items)
        {
            if (!item || item.Type == "" || item.RuntimeKey == "")
                return null;
        }
        return manifest;
    }

    static bool WriteManifest(string path, ServerSentryBackupItemManifest manifest)
    {
        string error;
        string temporary = path + ".items.tmp";
        if (!JsonFileLoader<ServerSentryBackupItemManifest>.SaveFile(temporary, manifest, error))
            return false;
        bool saved = CopyFile(temporary, path + ".items.json");
        DeleteFile(temporary);
        return saved;
    }

    protected static void Reference(string key, int delta)
    {
        if (key == "")
            return;
        int count;
        s_Wanted.Find(key, count);
        count += delta;
        if (count > 0)
            s_Wanted.Set(key, count);
        else
        {
            s_Wanted.Remove(key);
            s_Found.Remove(key);
            s_Ambiguous.Remove(key);
        }
    }

    protected static void Add(string path, ServerSentryBackupItemManifest manifest)
    {
        if (s_Manifests.Contains(path))
            return;
        s_Manifests.Set(path, manifest);
        for (int i = 1; i < manifest.Items.Count(); i++)
        {
            Reference(manifest.Items[i].PersistentKey, 1);
            Reference(manifest.Items[i].RuntimeKey, 1);
        }
    }

    static void Track(string path, ServerSentryBackupItemManifest manifest, array<EntityAI> entities)
    {
        Initialize();
        Add(path, manifest);
        for (int i = 1; i < entities.Count(); i++)
            Observe(entities[i]);
    }

    static void Remove(string path, bool deleteFile = true)
    {
        ServerSentryBackupItemManifest manifest;
        if (s_Manifests.Find(path, manifest))
        {
            for (int i = 1; i < manifest.Items.Count(); i++)
            {
                Reference(manifest.Items[i].PersistentKey, -1);
                Reference(manifest.Items[i].RuntimeKey, -1);
            }
            s_Manifests.Remove(path);
        }
        if (deleteFile)
            DeleteFile(path + ".items.json");
    }

    protected static void Found(string key, EntityAI entity)
    {
        if (key == "" || !s_Wanted.Contains(key))
            return;
        EntityAI existing;
        if (s_Found.Find(key, existing) && existing && existing != entity)
            s_Ambiguous.Set(key, true);
        else
            s_Found.Set(key, entity);
    }

    /** Also observes items loaded later with reconnecting players or containers. */
    static void Observe(EntityAI entity)
    {
        if (!GetGame() || !GetGame().IsServer() || s_Stopped || !entity)
            return;

        if (!ServerSentryConfigManager.Get().IsAllowedToDoCharacterBackup())
            return;
        Initialize();
        string persistent = ServerSentryBackupItemIdentity.Persistent(entity);
        string runtime = Runtime(entity);
        if (!s_Wanted.Contains(persistent) && !s_Wanted.Contains(runtime))
            return;
        Found(persistent, entity);
        Found(runtime, entity);
        ItemBase item = ItemBase.Cast(entity);
        if (item)
            item.ServerSentryMarkBackupTracked();
    }

    static void Forget(EntityAI entity)
    {
        if (!GetGame() || !GetGame().IsServer() || !entity || !s_Initialized || s_Stopped)
            return;
        array<string> keys = {ServerSentryBackupItemIdentity.Persistent(entity), Runtime(entity)};
        foreach (string key : keys)
        {
            EntityAI found;
            if (s_Found.Find(key, found) && found == entity)
                s_Found.Remove(key);
        }
    }

    /** Returns false for an ambiguous identity, never silently chooses a duplicate. */
    static bool Resolve(ServerSentryBackupItemIdentity identity, out EntityAI entity)
    {
        entity = null;
        if (s_Ambiguous.Contains(identity.PersistentKey) || s_Ambiguous.Contains(identity.RuntimeKey))
            return false;
        EntityAI persisted;
        EntityAI runtime;
        s_Found.Find(identity.PersistentKey, persisted);
        s_Found.Find(identity.RuntimeKey, runtime);
        if (persisted && runtime && persisted != runtime)
            return false;
        entity = persisted;
        if (!entity)
            entity = runtime;
        return !entity || entity.GetType() == identity.Type;
    }
}

/** Retains the exact pre-restore location of a reclaimed original. */
class ServerSentryBackupItemMove
{
    EntityAI Item;
    ref InventoryLocation Location;
    int Depth;
    int Index;
    bool ReturnRequired;
}
