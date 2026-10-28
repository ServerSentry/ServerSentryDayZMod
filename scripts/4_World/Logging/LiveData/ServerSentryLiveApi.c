/**
	\file ServerSentryLiveApi.c
	\brief Provides logging support for Live API.
*/

/**
	\brief Provides the public API for Live.
*/
class ServerSentryLiveApi
{
    /**
    	\brief Shared started state used by all instances.
    */
    protected static bool s_Started;
    /**
    	\brief Shared vehicles state used by all instances.
    */
    protected static ref array<Transport> s_Vehicles;
    /**
    	\brief Shared world events state used by all instances.
    */
    protected static ref array<EntityAI> s_WorldEvents;
    /**
    	\brief Shared bases state used by all instances.
    */
    protected static ref array<EntityAI> s_Bases;
    /** \brief Small delay to combine marker updates made in one callback. */
    protected static const int SNAPSHOT_COALESCE_MS = 250;
    /** \brief Bounds repeated refresh requests to at most one write per second. */
    protected static const int SNAPSHOT_MIN_INTERVAL_MS = 1000;
    /** \brief Whether one snapshot flush is already queued. */
    protected static bool s_SnapshotQueued;
    /** \brief Prevents callbacks from scheduling exports after mission shutdown. */
    protected static bool s_Stopped;
    /** \brief Distinguishes an initial write from a write at game time zero. */
    protected static bool s_HasWrittenSnapshot;
    /** \brief Game time at completion of the most recent successful export. */
    protected static int s_LastSnapshotWriteMs;
    /** \brief Bounded cache containing only currently registered base objects. */
    protected static ref map<int, ref ServerSentryLiveBaseCacheEntry> s_BaseCache;
    /** \brief Version of the current set of built base DTOs. */
    protected static int s_BaseGeneration;
    /** \brief Owns recoverable manifest and changed-section file publication. */
    protected static ref ServerSentryLiveFilePublisher s_FilePublisher;

    /**
    	\brief Processes the start operation.
    */
    static void Start()
    {
        if (s_Started)
        {
            return;
        }

        if (!GetGame() || !GetGame().IsServer())
        {
            return;
        }

        if (!ServerSentryConfigManager.Get().IsAllowedToStartLiveLogging())
        {
            return;
        }

        EnsureState();
        s_Stopped = false;
        s_Started = true;
        WriteSnapshot();

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(WriteSnapshot, ServerSentryConfigManager.Get().GetLiveUpdatePollingDelay(), true);
    }

    /** \brief Cancels pending exports and releases mission-owned world references. */
    static void Stop()
    {
        s_Stopped = true;
        if (GetGame())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(WriteSnapshot);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(FlushSnapshot);
        }
        s_Started = false;
        s_SnapshotQueued = false;
        s_HasWrittenSnapshot = false;
        s_LastSnapshotWriteMs = 0;
        s_Vehicles = null;
        s_WorldEvents = null;
        s_Bases = null;
        s_BaseCache = null;
        s_BaseGeneration = 0;
        s_FilePublisher = null;
    }

    /**
        \brief Invalidates cached base details after a known world or provider change.
        \param entity \p affected base; null invalidates all base details.
    */
    static void InvalidateBaseDetails(EntityAI entity = null)
    {
        if (!s_BaseCache)
            return;

        if (!entity)
        {
            s_BaseCache.Clear();
            return;
        }
        s_BaseCache.Remove(entity.GetID());
    }

    /**
    	\brief Registers vehicle.
    	\param vehicle \p vehicle used by the operation.
    */
    static void RegisterVehicle(Transport vehicle)
    {
        if (!vehicle)
        {
            return;
        }

        EnsureState();

        if (s_Vehicles.Find(vehicle) == -1)
        {
            s_Vehicles.Insert(vehicle);
        }
    }

    /**
    	\brief Processes the unregister vehicle operation.
    	\param vehicle \p vehicle used by the operation.
    */
    static void UnregisterVehicle(Transport vehicle)
    {
        if (!vehicle || !s_Vehicles)
        {
            return;
        }

        int index = s_Vehicles.Find(vehicle);
        if (index >= 0)
        {
            s_Vehicles.Remove(index);
        }
    }

    /**
    	\brief Attempts to register entity.
    	\param entity \p entity processed by the operation.
    */
    static void TryRegisterEntity(EntityAI entity)
    {
        if (!entity)
        {
            return;
        }

        Transport vehicle = Transport.Cast(entity);
        if (vehicle)
        {
            RegisterVehicle(vehicle);
        }

        if (IsWorldEventObject(entity))
        {
            RegisterWorldEvent(entity);
        }

        if (IsBaseFlagObject(entity))
        {
            RegisterBase(entity);
        }
    }

    /**
    	\brief Processes the unregister entity operation.
    	\param entity \p entity processed by the operation.
    */
    static void UnregisterEntity(EntityAI entity)
    {
        if (!entity)
        {
            return;
        }

        Transport vehicle = Transport.Cast(entity);
        if (vehicle)
        {
            UnregisterVehicle(vehicle);
        }

        UnregisterWorldEvent(entity);
        UnregisterBase(entity);
    }

    /**
    	\brief Registers world event.
    	\param entity \p entity processed by the operation.
    */
    static void RegisterWorldEvent(EntityAI entity)
    {
        if (!entity)
        {
            return;
        }

        EnsureState();

        if (s_WorldEvents.Find(entity) == -1)
        {
            s_WorldEvents.Insert(entity);
        }
    }

    /**
    	\brief Processes the unregister world event operation.
    	\param entity \p entity processed by the operation.
    */
    static void UnregisterWorldEvent(EntityAI entity)
    {
        if (!entity || !s_WorldEvents)
        {
            return;
        }

        int index = s_WorldEvents.Find(entity);
        if (index >= 0)
        {
            s_WorldEvents.Remove(index);
        }
    }

    /**
    	\brief Registers base.
    	\param entity \p entity processed by the operation.
    */
    static void RegisterBase(EntityAI entity)
    {
        if (!entity)
        {
            return;
        }

        EnsureState();

        if (s_Bases.Find(entity) == -1)
        {
            s_Bases.Insert(entity);
            //Print("[ServerSentry][LiveMap] Registered base object class=" + entity.GetType() + " objectId=" + entity.ToString());
        }
    }

    /**
    	\brief Processes the unregister base operation.
    	\param entity \p entity processed by the operation.
    */
    static void UnregisterBase(EntityAI entity)
    {
        if (!entity || !s_Bases)
        {
            return;
        }

        InvalidateBaseDetails(entity);
        int index = s_Bases.Find(entity);
        if (index >= 0)
        {
            s_Bases.Remove(index);
            Print("[ServerSentry][LiveMap] Unregistered base object class=" + entity.GetType() + " objectId=" + entity.ToString());
        }
    }



    /**
    	\brief Creates marker.
    	\param id \p identifier used by the operation.
    	\param displayName \p display name used by the operation.
    	\param position \p world position used by the operation.
    	\param radius \p radius used by the operation.
    	\param icon \p icon used by the operation.
    	\return \p The resulting live world event DTO instance, or null when unavailable.
    */
    static ServerSentryLiveWorldEventDto CreateMarker(string id, string displayName, vector position, float radius = 0, string icon = "MapPin")
    {
        ServerSentryLiveWorldEventDto marker = new ServerSentryLiveWorldEventDto();
        marker.eventType = "custom_marker";
        marker.className = "ServerSentryLiveMarker";
        marker.displayName = displayName;
        marker.objectId = id;
        marker.position.FromVector(position);
        marker.orientation.FromVector(Vector(0, 0, 0));
        marker.health = -1;
        marker.radius = radius;
        marker.icon = icon;

        if (radius > 0)
        {
            marker.shape = "circle";
        }
        else
        {
            marker.shape = "point";
        }

        return marker;
    }

    /**
    	\brief Creates polygon marker.
    	\param id \p identifier used by the operation.
    	\param displayName \p display name used by the operation.
    	\param positions \p positions used by the operation.
    	\param icon \p icon used by the operation.
    	\return \p The resulting live world event DTO instance, or null when unavailable.
    */
    static ServerSentryLiveWorldEventDto CreatePolygonMarker(string id, string displayName, TVectorArray positions, string icon = "MapPin")
    {
        ServerSentryLiveWorldEventDto marker = new ServerSentryLiveWorldEventDto();
        marker.eventType = "custom_marker";
        marker.className = "ServerSentryLivePolygonMarker";
        marker.displayName = displayName;
        marker.objectId = id;
        marker.orientation.FromVector(Vector(0, 0, 0));
        marker.health = -1;
        marker.icon = icon;
        SetMarkerPolygon(marker, positions);
        return marker;
    }

    /**
    	\brief Sets marker polygon.
    	\param marker \p marker used by the operation.
    	\param positions \p positions used by the operation.
    */
    static void SetMarkerPolygon(ServerSentryLiveWorldEventDto marker, TVectorArray positions)
    {
        if (!marker || !positions || positions.Count() < 3)
        {
            return;
        }

        marker.shape = "polygon";
        marker.points.Clear();

        vector center = GetMarkerPolygonCenter(positions);
        marker.position.FromVector(center);
        marker.radius = GetMarkerPolygonRadius(positions, center);

        for (int i = 0; i < positions.Count(); i++)
        {
            ServerSentryVectorDto point = new ServerSentryVectorDto();
            point.FromVector(positions.Get(i));
            marker.points.Insert(point);
        }
    }

    /**
    	\brief Adds marker point.
    	\param marker \p marker used by the operation.
    	\param position \p world position used by the operation.
    */
    static void AddMarkerPoint(ServerSentryLiveWorldEventDto marker, vector position)
    {
        if (!marker)
        {
            return;
        }

        ServerSentryVectorDto point = new ServerSentryVectorDto();
        point.FromVector(position);
        marker.points.Insert(point);
    }

    /**
    	\brief Processes the upsert marker operation.
    	\param marker \p marker used by the operation.
    */
    static void UpsertMarker(ServerSentryLiveWorldEventDto marker)
    {
        StoreMarker(marker, true);
    }

    /**
        \brief Registers a marker for export on the existing snapshot cadence.
        \param marker \p marker to register or replace.
    */
    static void UpsertMarkerDeferred(ServerSentryLiveWorldEventDto marker)
    {
        StoreMarker(marker, false);
    }

    /**
        \brief Stores a marker with the requested export behavior.
        \param marker \p marker to register or replace.
        \param requestExport \p true to request an early coalesced export.
    */
    protected static void StoreMarker(ServerSentryLiveWorldEventDto marker, bool requestExport)
    {
        if (!ServerSentryMapMarkers.Register(marker))
            return;

        if (requestExport)
        {
            WriteSnapshot();
        }
    }

    /**
        \brief Finds a registered marker without writing a snapshot.
        \param id \p marker ID to look up.
        \return \p Registered handle, or null if absent.
    */
    static ServerSentryLiveWorldEventDto FindMarker(string id)
    {
        return ServerSentryMapMarkers.Find(id);
    }

    /**
        \brief Returns the number of currently registered custom markers.
        \return \p Registry size used by the bounded simple API.
    */
    static int GetMarkerCount()
    {
        return ServerSentryMapMarkers.Count();
    }

    /**
    	\brief Removes marker.
    	\param id \p identifier used by the operation.
    */
    static void RemoveMarker(string id)
    {
        DeleteMarker(id, true);
    }

    /**
        \brief Removes a marker on the existing export cadence.
        \param id \p stable marker ID.
    */
    static void RemoveMarkerDeferred(string id)
    {
        DeleteMarker(id, false);
    }

    /**
        \brief Removes the registry entry with the requested export behavior.
        \param id \p stable marker ID.
        \param requestExport \p true to request an early coalesced export.
    */
    protected static void DeleteMarker(string id, bool requestExport)
    {
        if (id == "")
        {
            return;
        }
        ServerSentryMapMarkers.Remove(id);
        if (requestExport)
        {
            WriteSnapshot();
        }
    }

    /**
    	\brief Clears markers by prefix.
    	\param prefix \p prefix used by the operation.
    */
    static void ClearMarkersByPrefix(string prefix)
    {
        if (prefix == "")
        {
            return;
        }

        ServerSentryMapMarkers.ClearByPrefix(prefix);
        WriteSnapshot();
    }

    /**
    	\brief Adds marker detail.
    	\param marker \p marker used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param value \p value to process or assign.
    */
    static void AddMarkerDetail(ServerSentryLiveWorldEventDto marker, string key, string label, string value)
    {
        ServerSentryLiveDetails.UpsertMarkerDetail(marker, key, label, value);
    }

    /**
    	\brief Adds marker action.
    	\param marker \p marker used by the operation.
    	\param actionId \p stable ID of the action.
    */
    static void AddMarkerAction(ServerSentryLiveWorldEventDto marker, string actionId)
    {
        if (!marker || actionId == "")
        {
            return;
        }

        marker.actions.Insert(actionId);
    }

    /**
    	\brief Finds live base object by ID.
    	\param objectId \p network or persistent ID of the target object.
    	\return \p The resulting entity ai instance, or null when unavailable.
    */
    static EntityAI FindLiveBaseObjectById(string objectId)
    {
        if (objectId == "" || !s_Bases)
        {
            return null;
        }

        for (int i = s_Bases.Count() - 1; i >= 0; i--)
        {
            EntityAI entity = s_Bases.Get(i);
            if (!entity)
            {
                s_Bases.Remove(i);
                continue;
            }

            if (entity.ToString() == objectId)
            {
                return entity;
            }
        }

        return null;
    }

    /**
    	\brief Finds live vehicle by ID.
    	\param objectId \p network or persistent ID of the target object.
    	\return \p The resulting transport instance, or null when unavailable.
    */
    static Transport FindLiveVehicleById(string objectId)
    {
        if (objectId == "" || !s_Vehicles)
        {
            return null;
        }

        for (int i = s_Vehicles.Count() - 1; i >= 0; i--)
        {
            Transport vehicle = s_Vehicles.Get(i);
            if (!vehicle)
            {
                s_Vehicles.Remove(i);
                continue;
            }

            if (vehicle.ToString() == objectId)
            {
                return vehicle;
            }
        }

        return null;
    }

    /**
        \brief Requests a coalesced snapshot without building or serializing on this call stack.
    */
    static void WriteSnapshot()
    {
        if (!CanWriteSnapshot() || s_SnapshotQueued)
            return;

        int delayMs = SNAPSHOT_COALESCE_MS;
        int elapsedMs = GetGame().GetTime() - s_LastSnapshotWriteMs;
        if (s_HasWrittenSnapshot && elapsedMs >= 0)
            delayMs = Math.Max(delayMs, SNAPSHOT_MIN_INTERVAL_MS - elapsedMs);

        s_SnapshotQueued = true;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(FlushSnapshot, delayMs, false);
    }

    /**
        \brief Checks execution context and the live export configuration.
        \return \p true when live snapshots may be built and written.
    */
    protected static bool CanWriteSnapshot()
    {
        if (s_Stopped || !GetGame() || !GetGame().IsServer())
            return false;

        if (!ServerSentryDefaults.LiveDataEnabled())
            return false;

        return ServerSentryConfigManager.Get().IsAllowedToStartLiveLogging();
    }

    /** \brief Builds and writes the latest state once for all pending requests. */
    protected static void FlushSnapshot()
    {
        s_SnapshotQueued = false;
        if (!CanWriteSnapshot())
            return;

        int elapsedMs = GetGame().GetTime() - s_LastSnapshotWriteMs;
        if (s_HasWrittenSnapshot && elapsedMs >= 0 && elapsedMs < SNAPSHOT_MIN_INTERVAL_MS)
        {
            WriteSnapshot();
            return;
        }

        ServerSentryLiveSnapshot snapshot = BuildSnapshot();
        snapshot.generatedAtUtc = ServerSentryTime.NowUtc();
        if (!WriteSnapshotFile(snapshot))
            return;

        s_HasWrittenSnapshot = true;
        s_LastSnapshotWriteMs = GetGame().GetTime();
        LogSnapshot(snapshot);
    }

    /**
        \brief Publishes changed slow sections and the current hot-data manifest.
        \param snapshot \p complete latest state.
        \return \p true when serialization and opening the output file succeeded.
    */
    protected static bool WriteSnapshotFile(ServerSentryLiveSnapshot snapshot)
    {
        if (!EnsureDirectories())
            return false;

        if (!s_FilePublisher)
            s_FilePublisher = new ServerSentryLiveFilePublisher();

        return s_FilePublisher.Publish(snapshot, s_BaseGeneration);
    }

    /**
        \brief Logs export counts without constructing a second full-state string.
        \param snapshot \p snapshot that was just written.
    */
    protected static void LogSnapshot(ServerSentryLiveSnapshot snapshot)
    {
        string summary = "[ServerSentry][LiveMap] Wrote snapshot";
        summary += " players=" + snapshot.players.Count().ToString();
        summary += " vehicles=" + snapshot.vehicles.Count().ToString();
        summary += " events=" + snapshot.events.Count().ToString();
        summary += " bases=" + snapshot.bases.Count().ToString();
        summary += " actions=" + snapshot.actions.Count().ToString();
        Print(summary);
    }

    /**
    	\brief Builds snapshot.
    	\return \p The resulting live snapshot instance, or null when unavailable.
    */
    static ServerSentryLiveSnapshot BuildSnapshot()
    {
        EnsureState();

        ServerSentryLiveSnapshot snapshot = new ServerSentryLiveSnapshot();

        string worldName;
        GetGame().GetWorldName(worldName);
        snapshot.mapName = worldName;

        snapshot.currentFPS = ServerSentryPerformance.CurrentServerFps;
        snapshot.minFPS = ServerSentryPerformance.MinServerFps;
        snapshot.maxFPS = ServerSentryPerformance.MaxServerFps;
        snapshot.performance.averageTickTimeMs = ServerSentryPerformance.AverageTickTimeMs;
        snapshot.performance.maxTickTimeMs = ServerSentryPerformance.MaxTickTimeMs;
        snapshot.performance.tickSampleCount = ServerSentryPerformance.TickSampleCount;
        snapshot.performance.groundItemCount = ServerSentryWorldItemCounts.Ground;
        snapshot.performance.loadedItemCount = ServerSentryWorldItemCounts.Loaded;

        FillPlayers(snapshot);
        FillVehicles(snapshot);
        FillWorldEvents(snapshot);
        FillCustomMarkers(snapshot);
        FillBases(snapshot);
        FillActionDescriptors(snapshot);

        return snapshot;
    }

    /**
    	\brief Processes the fill players operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillPlayers(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot)
        {
            return;
        }

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        foreach (Man man: players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (!player)
            {
                continue;
            }

            ServerSentryLivePlayerDto dto = new ServerSentryLivePlayerDto();
            dto.objectId = player.ToString();
            dto.position.FromVector(player.GetPosition());
            dto.health = player.GetHealth("", "Health");
            dto.blood = player.GetHealth("", "Blood");
            dto.shock = player.GetHealth("", "Shock");
            dto.isAlive = player.IsAlive();

            PlayerIdentity identity = player.GetIdentity();
            if (identity)
            {
                dto.name = identity.GetName();
                dto.steamId = identity.GetPlainId();
                dto.bohemiaId = identity.GetId();
                dto.pingAvg = identity.GetPingAvg();
            }

            snapshot.players.Insert(dto);
        }
    }

    /**
    	\brief Processes the fill vehicles operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillVehicles(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot || !s_Vehicles)
        {
            return;
        }

        for (int i = s_Vehicles.Count() - 1; i >= 0; i--)
        {
            Transport vehicle = s_Vehicles.Get(i);
            if (!vehicle)
            {
                s_Vehicles.Remove(i);
                continue;
            }

            ServerSentryLiveVehicleDto dto = new ServerSentryLiveVehicleDto();
            dto.className = vehicle.GetType();
            dto.displayName = vehicle.GetDisplayName();
            dto.objectId = vehicle.ToString();
            dto.position.FromVector(vehicle.GetPosition());
            dto.orientation.FromVector(vehicle.GetOrientation());
            dto.health = vehicle.GetHealth("", "Health");
            dto.crewSize = CountCrew(vehicle);

            CarScript car = CarScript.Cast(vehicle);
            if (car)
            {
                dto.engineStateKnown = true;
                dto.engineOn = car.EngineIsOn();
            }

            if (vehicle.IsInherited(HelicopterScript))
            {
                dto.icon = "helicopter";
            }
            else if (vehicle.IsInherited(Truck_01_Base))
            {
                dto.icon = "truck";
            }
            else if (vehicle.IsInherited(Boat))
            {
                dto.icon = "boat";
            }
            else if (vehicle.IsInherited(MotorbikeScript))
            {
                dto.icon = "motorcycle";
            }

            snapshot.vehicles.Insert(dto);
        }
    }

    /**
    	\brief Processes the fill world events operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillWorldEvents(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot || !s_WorldEvents)
        {
            return;
        }

        for (int i = s_WorldEvents.Count() - 1; i >= 0; i--)
        {
            EntityAI entity = s_WorldEvents.Get(i);
            if (!entity)
            {
                s_WorldEvents.Remove(i);
                continue;
            }

            string eventType = ResolveWorldEventType(entity.GetType());
            if (eventType == "")
            {
                s_WorldEvents.Remove(i);
                continue;
            }

            ServerSentryLiveWorldEventDto dto = new ServerSentryLiveWorldEventDto();
            dto.eventType = eventType;
            dto.className = entity.GetType();
            dto.displayName = entity.GetDisplayName();
            dto.objectId = entity.ToString();
            dto.position.FromVector(entity.GetPosition());
            dto.orientation.FromVector(entity.GetOrientation());
            
            if (eventType == "contaminated_area")
            {
                dto.icon = "toxic";
            }
            else if (eventType == "heli_crash")
            {
                dto.icon = "event";
            }

            // Many vanilla event marker/static objects do not have DamageSystemData
            // (for example Wreck_UH1Y, Land_Wreck_C130J, ContaminatedArea_Static).
            // Calling GetHealth/GetMaxHealth on those classes throws a VM exception and
            // can crash the server during snapshot generation. Use -1 as "unknown".
            dto.health = -1;

            snapshot.events.Insert(dto);
        }
    }



    /**
    	\brief Processes the fill custom markers operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillCustomMarkers(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot)
        {
            return;
        }

        ServerSentryMapMarkers.AppendTo(snapshot.events);
    }

    /**
    	\brief Processes the fill bases operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillBases(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot || !s_Bases)
            return;

        int nowMs = GetGame().GetTime();
        int lifetimeMs = ServerSentryConfigManager.Get().GetLiveBaseDetailsCacheMs();
        PruneBaseCache();
        for (int i = s_Bases.Count() - 1; i >= 0; i--)
        {
            EntityAI entity = s_Bases.Get(i);
            if (!entity || !IsBaseFlagObject(entity))
            {
                if (entity)
                    InvalidateBaseDetails(entity);
                s_Bases.Remove(i);
                continue;
            }
            snapshot.bases.Insert(GetBaseSnapshot(entity, nowMs, lifetimeMs));
        }
    }

    /**
        \brief Reuses unchanged base details for the configured interval.
        \param entity \p registered live base.
        \param nowMs \p current game time.
        \param lifetimeMs \p maximum permitted cache age.
        \return \p completed base DTO, including provider output.
    */
    protected static ServerSentryLiveBaseDto GetBaseSnapshot(EntityAI entity, int nowMs, int lifetimeMs)
    {
        int id = entity.GetID();
        ServerSentryLiveBaseCacheEntry entry = s_BaseCache.Get(id);
        if (entry && entry.IsFresh(entity, nowMs, lifetimeMs))
            return entry.dto;

        entry = new ServerSentryLiveBaseCacheEntry();
        entry.entity = entity;
        entry.position = entity.GetPosition();
        entry.orientation = entity.GetOrientation();
        entry.capturedAtMs = nowMs;
        entry.dto = BuildBaseSnapshot(entity);
        s_BaseCache.Set(id, entry);
        s_BaseGeneration++;
        return entry.dto;
    }

    /** \brief Releases entries whose weak native base reference has expired. */
    protected static void PruneBaseCache()
    {
        for (int i = s_BaseCache.Count() - 1; i >= 0; i--)
        {
            ServerSentryLiveBaseCacheEntry entry = s_BaseCache.GetElement(i);
            if (!entry || !entry.entity)
                s_BaseCache.Remove(s_BaseCache.GetKey(i));
        }
    }

    /**
        \brief Builds Vanilla rows once before applying integration overrides.
        \param entity \p live base to capture.
        \return \p completed base DTO.
    */
    protected static ServerSentryLiveBaseDto BuildBaseSnapshot(EntityAI entity)
    {
        ServerSentryLiveBaseDto dto = new ServerSentryLiveBaseDto();
        dto.type = "base_flagpole";
        dto.className = entity.GetType();
        dto.displayName = ResolveBaseDisplayName(entity);
        dto.objectId = entity.ToString();
        dto.position.FromVector(entity.GetPosition());
        dto.orientation.FromVector(entity.GetOrientation());
        dto.health = SafeGetEntityHealth(entity);
        dto.radius = 60;

        AddDetail(dto, "type", "Type", dto.type);
        AddDetail(dto, "className", "Class", dto.className);
        AddDetail(dto, "objectId", "Object", dto.objectId);
        string positionLabel = "X " + Math.Round(dto.position.x).ToString();
        positionLabel += " · Z " + Math.Round(dto.position.z).ToString();
        AddDetail(dto, "position", "Position", positionLabel);
        AddDetail(dto, "health", "Health", FormatHealthValue(dto.health));
        AddDetail(dto, "radius", "Radius", Math.Round(dto.radius).ToString() + " m");
        AddBaseFlagDetails(dto, entity);
        AddBaseDespawnDetails(dto, entity);

        ServerSentryLiveBaseDetailRegistry.Apply(entity, dto);
        AddRegisteredActionsForTarget(dto.actions, dto.type);
        return dto;
    }

    /**
    	\brief Adds all registered action IDs that apply to a live-map target subtype.
    */
    protected static void AddRegisteredActionsForTarget(array<string> actionIds, string requestedTargetType)
    {
        if (!actionIds)
        {
            return;
        }

        array<ref ServerSentryLiveModActionDescriptorDto> descriptors = ServerSentryModActionRegistry.GetDescriptors();
        foreach (ServerSentryLiveModActionDescriptorDto descriptor: descriptors)
        {
            if (!descriptor || descriptor.id == "")
            {
                continue;
            }

            if (ActionTargetTypeMatches(descriptor.targetType, requestedTargetType) && actionIds.Find(descriptor.id) == -1)
            {
                actionIds.Insert(descriptor.id);
            }
        }
    }

    /**
    	\brief Matches generic and subtype-specific live action target types.
    */
    protected static bool ActionTargetTypeMatches(string registeredTargetType, string requestedTargetType)
    {
        registeredTargetType.TrimInPlace();
        requestedTargetType.TrimInPlace();
        registeredTargetType.ToLower();
        requestedTargetType.ToLower();

        if (registeredTargetType == "" || registeredTargetType == "any")
        {
            return true;
        }

        if (registeredTargetType == "object")
        {
            if (requestedTargetType == "player")
            {
                return false;
            }

            if (requestedTargetType == "eventzone")
            {
                return false;
            }

            if (requestedTargetType == "server")
            {
                return false;
            }

            if (requestedTargetType == "world")
            {
                return false;
            }

            if (requestedTargetType == "position")
            {
                return false;
            }

            if (requestedTargetType == "contextmenu")
            {
                return false;
            }

            return true;
        }

        if (registeredTargetType == "base")
        {
            return requestedTargetType == "base" || requestedTargetType.IndexOf("base_") == 0;
        }

        return registeredTargetType == requestedTargetType;
    }

    /**
    	\brief Processes the fill action descriptors operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillActionDescriptors(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot)
        {
            return;
        }

        array<ref ServerSentryLiveModActionDescriptorDto> descriptors = ServerSentryModActionRegistry.GetDescriptors();
        foreach (ServerSentryLiveModActionDescriptorDto descriptor: descriptors)
        {
            snapshot.actions.Insert(descriptor);
        }
    }

    /**
    	\brief Adds detail.
    	\param dto \p dto used by the operation.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param value \p value to process or assign.
    */
    protected static void AddDetail(ServerSentryLiveBaseDto dto, string key, string label, string value)
    {
        ServerSentryLiveDetails.UpsertBaseDetail(dto, key, label, value);
    }

    /**
    	\brief Adds Vanilla flag-height details when the base is a TerritoryFlag.
    	\param dto \p dto used by the operation.
    	\param entity \p entity processed by the operation.
    */
    protected static void AddBaseFlagDetails(ServerSentryLiveBaseDto dto, EntityAI entity)
    {
        if (!dto || !entity)
        {
            return;
        }

        TerritoryFlag territoryFlag = TerritoryFlag.Cast(entity);
        if (!territoryFlag)
        {
            return;
        }

        float flagHeight01 = territoryFlag.ServerSentryGetFlagHeight01();
        int flagHeightPercent = Math.Round(flagHeight01 * 100);
        AddDetail(dto, "flagHeightPercent", "Flag height", flagHeightPercent.ToString() + "%");
    }

    /**
    	\brief Formats a health value for Live Map details.
    	\param health \p normalized entity health or a negative unknown value.
    	\return \p the formatted health value.
    */
    protected static string FormatHealthValue(float health)
    {
        if (health < 0)
        {
            return "Unknown";
        }

        return Math.Round(health).ToString();
    }

    /**
    	\brief Adds base despawn details.
    	\param dto \p dto used by the operation.
    	\param entity \p entity processed by the operation.
    */
    protected static void AddBaseDespawnDetails(ServerSentryLiveBaseDto dto, EntityAI entity)
    {
        if (!dto || !entity)
        {
            return;
        }

        float lifetimeRemaining = -1;
        float lifetimeMax = -1;

        TerritoryFlag territoryFlag = TerritoryFlag.Cast(entity);
        if (territoryFlag)
        {
            lifetimeRemaining = territoryFlag.ServerSentryGetFlagLifetimeRemaining();
            lifetimeMax = territoryFlag.ServerSentryGetFlagLifetimeMax();
        }
        else
        {
            lifetimeRemaining = SafeGetEntityLifetime(entity);
            lifetimeMax = SafeGetEntityLifetimeMax(entity);
        }

        if (lifetimeRemaining >= 0)
        {
            AddDetail(dto, "lifetimeRemaining", "Despawn in", FormatDurationSeconds(lifetimeRemaining));
        }
        else
        {
            AddDetail(dto, "lifetimeRemaining", "Despawn in", "Unknown");
        }

        if (lifetimeMax >= 0)
        {
            AddDetail(dto, "lifetimeMax", "Lifetime max", FormatDurationSeconds(lifetimeMax));
        }
    }

    /**
    	\brief Resolves base display name.
    	\param entity \p entity processed by the operation.
    	\return \p The resulting string value.
    */
    protected static string ResolveBaseDisplayName(EntityAI entity)
    {
        if (!entity)
        {
            return "Base / Flag Pole";
        }

        string displayName = entity.GetDisplayName();
        if (displayName != "")
        {
            return displayName;
        }

        return "Base / Flag Pole";
    }

    /**
    	\brief Processes the safe get entity health operation.
    	\param entity \p entity processed by the operation.
    	\return \p The resulting floating-point value.
    */
    protected static float SafeGetEntityHealth(EntityAI entity)
    {
        if (!entity)
        {
            return -1;
        }

        // Some base/flag objects do not expose DamageSystemData safely. Use -1 as unknown.
        return -1;
    }

    /**
    	\brief Processes the safe get entity lifetime operation.
    	\param entity \p entity processed by the operation.
    	\return \p The resulting floating-point value.
    */
    protected static float SafeGetEntityLifetime(EntityAI entity)
    {
        if (!entity)
        {
            return -1;
        }

        return entity.GetLifetime();
    }

    /**
    	\brief Processes the safe get entity lifetime max operation.
    	\param entity \p entity processed by the operation.
    	\return \p The resulting floating-point value.
    */
    protected static float SafeGetEntityLifetimeMax(EntityAI entity)
    {
        if (!entity)
        {
            return -1;
        }

        return entity.GetLifetimeMax();
    }

    /**
    	\brief Formats duration seconds.
    	\param seconds \p seconds used by the operation.
    	\return \p The resulting string value.
    */
    protected static string FormatDurationSeconds(float seconds)
    {
        if (seconds < 0)
        {
            return "Unknown";
        }

        if (seconds >= 86400)
        {
            return Math.Round(seconds / 86400).ToString() + " days";
        }

        if (seconds >= 3600)
        {
            return Math.Round(seconds / 3600).ToString() + " hours";
        }

        if (seconds >= 60)
        {
            return Math.Round(seconds / 60).ToString() + " minutes";
        }

        return Math.Round(seconds).ToString() + " seconds";
    }

    /**
    	\brief Returns marker polygon center.
    	\param positions \p positions used by the operation.
    	\return \p The resulting vector value.
    */
    protected static vector GetMarkerPolygonCenter(TVectorArray positions)
    {
        vector center = vector.Zero;
        if (!positions || positions.Count() == 0)
        {
            return center;
        }

        for (int i = 0; i < positions.Count(); i++)
        {
            center = center + positions.Get(i);
        }

        center[0] = center[0] / positions.Count();
        center[1] = center[1] / positions.Count();
        center[2] = center[2] / positions.Count();
        return center;
    }

    /**
    	\brief Returns marker polygon radius.
    	\param positions \p positions used by the operation.
    	\param center \p center used by the operation.
    	\return \p The resulting floating-point value.
    */
    protected static float GetMarkerPolygonRadius(TVectorArray positions, vector center)
    {
        float radius = 0;
        if (!positions)
        {
            return radius;
        }

        for (int i = 0; i < positions.Count(); i++)
        {
            float distance = GetMarkerDistance2D(center, positions.Get(i));
            if (distance > radius)
            {
                radius = distance;
            }
        }

        return radius;
    }

    /**
    	\brief Returns marker distance 2 d.
    	\param a \p a used by the operation.
    	\param b \p b used by the operation.
    	\return \p The resulting floating-point value.
    */
    protected static float GetMarkerDistance2D(vector a, vector b)
    {
        vector delta = a - b;
        float dx = delta[0];
        float dz = delta[2];
        return Math.Sqrt((dx * dx) + (dz * dz));
    }

    /**
    	\brief Processes the count crew operation.
    	\param vehicle \p vehicle used by the operation.
    	\return \p The resulting integer value or status code.
    */
    protected static int CountCrew(Transport vehicle)
    {
        if (!vehicle)
        {
            return 0;
        }

        int count = 0;
        int size = vehicle.CrewSize();
        for (int i = 0; i < size; i++)
        {
            if (vehicle.CrewMember(i))
            {
                count++;
            }
        }

        return count;
    }

    /**
    	\brief Returns whether world event object.
    	\param entity \p entity processed by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsWorldEventObject(EntityAI entity)
    {
        if (!entity)
        {
            return false;
        }

        return ResolveWorldEventType(entity.GetType()) != "";
    }

    /**
    	\brief Returns whether base flag object.
    	\param entity \p entity processed by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsBaseFlagObject(EntityAI entity)
    {
        if (!entity)
        {
            return false;
        }

        string className = entity.GetType();
        if (IsBaseFlagKitClassName(className))
        {
            return false;
        }

        if (className == "TerritoryFlag" || className == "FlagPole")
        {
            return true;
        }

        if (ContainsCI(className, "TerritoryFlag") || ContainsCI(className, "FlagPole") || ContainsCI(className, "Flag_Pole") || ContainsCI(className, "BaseFlag"))
        {
            return true;
        }

        if (ContainsCI(className, "Flag") && ContainsCI(className, "Territory"))
        {
            return true;
        }

        return false;
    }

    /**
    	\brief Returns whether base flag kit class name.
    	\param className \p class name used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsBaseFlagKitClassName(string className)
    {
        if (className == "")
        {
            return true;
        }

        if (ContainsCI(className, "Kit") || ContainsCI(className, "Hologram") || ContainsCI(className, "Placing") || ContainsCI(className, "Blueprint") || ContainsCI(className, "Packed") || ContainsCI(className, "Projection"))
        {
            return true;
        }

        return false;
    }

    /**
    	\brief Resolves world event type.
    	\param className \p class name used by the operation.
    	\return \p The resulting string value.
    */
    protected static string ResolveWorldEventType(string className)
    {
        if (className == "")
        {
            return "";
        }

        if (ContainsCI(className, "StaticContaminatedArea") || ContainsCI(className, "ContaminatedArea"))
        {
            return "contaminated_area";
        }

        if (ContainsCI(className, "Wreck_Mi8") || ContainsCI(className, "Wreck_UH1Y") || ContainsCI(className, "Wreck_Heli") || ContainsCI(className, "HeliCrash") || ContainsCI(className, "Helicrash"))
        {
            return "heli_crash";
        }

        if (ContainsCI(className, "Police") && (ContainsCI(className, "Wreck") || ContainsCI(className, "Crash")))
        {
            return "police_event";
        }

        if ((ContainsCI(className, "Military") || ContainsCI(className, "Convoy")) && (ContainsCI(className, "Wreck") || ContainsCI(className, "Crash")))
        {
            return "military_event";
        }

        if (ContainsCI(className, "Wreck") && (ContainsCI(className, "C130") || ContainsCI(className, "Mi8") || ContainsCI(className, "UH1Y")))
        {
            return "crash_site";
        }

        return "";
    }

    /**
    	\brief Processes the contains ci operation.
    	\param text \p text used by the operation.
    	\param needle \p needle used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected static bool ContainsCI(string text, string needle)
    {
        if (text == "" || needle == "")
        {
            return false;
        }

        string t = text;
        string n = needle;
        t.ToLower();
        n.ToLower();
        return t.IndexOf(n) != -1;
    }

    /**
    	\brief Initializes the registry collections when they are first used.
    */
    protected static void EnsureState()
    {
        if (!s_Vehicles)
        {
            s_Vehicles = new array<Transport>();
        }

        if (!s_WorldEvents)
        {
            s_WorldEvents = new array<EntityAI>();
        }

        if (!s_Bases)
        {
            s_Bases = new array<EntityAI>();
        }

        if (!s_BaseCache)
            s_BaseCache = new map<int, ref ServerSentryLiveBaseCacheEntry>();
    }

    /**
    	\brief Creates the fulfillment storage directories when necessary.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected static bool EnsureDirectories()
    {
        string root = ServerSentryDefaults.RootDirectory();
        if (!FileExist(root))
        {
            if (!MakeDirectory(root))
            {
                Print("[ServerSentry] Failed to create live root directory: " + root);
                return false;
            }
        }

        string live = ServerSentryDefaults.LiveDirectory();
        if (!FileExist(live))
        {
            if (!MakeDirectory(live))
            {
                Print("[ServerSentry] Failed to create live directory: " + live);
                return false;
            }
        }

        return true;
    }
}
