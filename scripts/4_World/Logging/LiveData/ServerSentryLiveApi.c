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
    /**
    	\brief Shared custom markers state used by all instances.
    */
    protected static ref array<ref ServerSentryLiveWorldEventDto> s_CustomMarkers;
    /**
    	\brief Shared last snapshot fingerprint state used by all instances.
    */
    protected static string s_LastSnapshotFingerprint;
    /**
    	\brief Shared last forced snapshot write ms state used by all instances.
    */
    protected static int s_LastForcedSnapshotWriteMs;

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
        s_Started = true;
        WriteSnapshot();

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(WriteSnapshot, ServerSentryConfigManager.Get().GetLiveUpdatePollingDelay(), true);
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
            Print("[ServerSentry][LiveMap] Registered base object class=" + entity.GetType() + " objectId=" + entity.ToString());
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
        if (!marker || marker.objectId == "")
        {
            return;
        }

        EnsureState();

        for (int i = 0; i < s_CustomMarkers.Count(); i++)
        {
            ServerSentryLiveWorldEventDto existing = s_CustomMarkers.Get(i);
            if (existing && existing.objectId == marker.objectId)
            {
                s_CustomMarkers.Set(i, marker);
                WriteSnapshot();
                return;
            }
        }

        s_CustomMarkers.Insert(marker);
        WriteSnapshot();
    }

    /**
    	\brief Removes marker.
    	\param id \p identifier used by the operation.
    */
    static void RemoveMarker(string id)
    {
        if (id == "")
        {
            return;
        }

        EnsureState();

        for (int i = s_CustomMarkers.Count() - 1; i >= 0; i--)
        {
            ServerSentryLiveWorldEventDto marker = s_CustomMarkers.Get(i);
            if (marker && marker.objectId == id)
            {
                s_CustomMarkers.Remove(i);
            }
        }

        WriteSnapshot();
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

        EnsureState();

        for (int i = s_CustomMarkers.Count() - 1; i >= 0; i--)
        {
            ServerSentryLiveWorldEventDto marker = s_CustomMarkers.Get(i);
            if (marker && marker.objectId.IndexOf(prefix) == 0)
            {
                s_CustomMarkers.Remove(i);
            }
        }

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
        if (!marker || key == "")
        {
            return;
        }

        ServerSentryLiveDetailDto detail = new ServerSentryLiveDetailDto();
        detail.key = key;
        detail.label = label;
        detail.value = value;
        marker.details.Insert(detail);
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
    	\brief Writes snapshot.
    */
    static void WriteSnapshot()
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return;
        }

        if (!ServerSentryDefaults.LiveDataEnabled())
        {
            return;
        }

        ServerSentryLiveSnapshot snapshot = BuildSnapshot();
        string fingerprint = BuildSnapshotFingerprint(snapshot);
        int nowMs = GetGame().GetTime();
        bool changed = fingerprint != s_LastSnapshotFingerprint;
        bool keepAliveDue = (nowMs - s_LastForcedSnapshotWriteMs) >= LiveSnapshotKeepAliveMs();

        if (!changed && !keepAliveDue)
        {
            return;
        }

        snapshot.generatedAtUtc = ServerSentryTime.NowUtc();

        string json;
        string error;
        if (!JsonFileLoader<ServerSentryLiveSnapshot>.MakeData(snapshot, json, error, false) || json == "")
        {
            Print("[ServerSentry][LiveMap] Failed to serialize live map snapshot - " + error);
            return;
        }

        if (!EnsureDirectories())
        {
            return;
        }

        string filePath = ServerSentryDefaults.LiveStateFilePath();
        FileHandle file = OpenFile(filePath, FileMode.WRITE);
        if (!file)
        {
            Print("[ServerSentry][LiveMap] Failed to open live map state file for write: " + filePath);
            return;
        }

        FPrintln(file, json);
        CloseFile(file);

        s_LastSnapshotFingerprint = fingerprint;
        s_LastForcedSnapshotWriteMs = nowMs;
        string changedText = BoolLogValue(changed);
        string keepAliveText = BoolLogValue(keepAliveDue);
        Print("[ServerSentry][LiveMap] Wrote snapshot changed=" + changedText + " keepAlive=" + keepAliveText + " players=" + snapshot.players.Count().ToString() + " vehicles=" + snapshot.vehicles.Count().ToString() + " events=" + snapshot.events.Count().ToString() + " bases=" + snapshot.bases.Count().ToString() + " actions=" + snapshot.actions.Count().ToString());
    }

    /**
    	\brief Builds snapshot.
    	\return \p The resulting live snapshot instance, or null when unavailable.
    */
    static ServerSentryLiveSnapshot BuildSnapshot()
    {
        EnsureState();

        ServerSentryLiveSnapshot snapshot = new ServerSentryLiveSnapshot();

        snapshot.currentFPS = GetGame().GetLastFPS();
        string worldName;
	    g_Game.GetWorldName(worldName);
        snapshot.mapName = worldName;
        float minFps;
        float maxFps;
        float avgFps;

        GetGame().GetFPSStats(minFps, maxFps, avgFps, 64);

        snapshot.currentFPS = ServerSentryPerformance.CurrentServerFps;
        snapshot.minFPS = ServerSentryPerformance.MinServerFps;
        snapshot.maxFPS = ServerSentryPerformance.MaxServerFps;

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
        if (!snapshot || !s_CustomMarkers)
        {
            return;
        }

        foreach (ServerSentryLiveWorldEventDto marker: s_CustomMarkers)
        {
            if (marker && marker.position)
            {
                snapshot.events.Insert(marker);
            }
        }
    }

    /**
    	\brief Processes the fill bases operation.
    	\param snapshot \p snapshot used by the operation.
    */
    protected static void FillBases(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot || !s_Bases)
        {
            return;
        }

        for (int i = s_Bases.Count() - 1; i >= 0; i--)
        {
            EntityAI entity = s_Bases.Get(i);
            if (!entity)
            {
                s_Bases.Remove(i);
                continue;
            }

            if (!IsBaseFlagObject(entity))
            {
                s_Bases.Remove(i);
                continue;
            }

            ServerSentryLiveBaseDto dto = new ServerSentryLiveBaseDto();
            dto.type = "base_flagpole";
            dto.className = entity.GetType();
            dto.displayName = ResolveBaseDisplayName(entity);
            dto.objectId = entity.ToString();
            dto.position.FromVector(entity.GetPosition());
            dto.orientation.FromVector(entity.GetOrientation());
            dto.health = SafeGetEntityHealth(entity);
            dto.radius = 60;

            AddDetail(dto, "className", "Class", dto.className);
            AddDetail(dto, "objectId", "Object", dto.objectId);
            AddDetail(dto, "position", "Position", "X " + Math.Round(dto.position.x).ToString() + " · Z " + Math.Round(dto.position.z).ToString());
            AddDetail(dto, "health", "Health", dto.health.ToString());
            AddBaseDespawnDetails(dto, entity);

            dto.actions.Insert("object.inspect");
            dto.actions.Insert("object.delete");
            snapshot.bases.Insert(dto);
        }
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
        if (!dto || key == "")
        {
            return;
        }

        ServerSentryLiveDetailDto detail = new ServerSentryLiveDetailDto();
        detail.key = key;
        detail.label = label;
        detail.value = value;
        dto.details.Insert(detail);
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

        float lifetimeRemaining = SafeGetEntityLifetime(entity);
        if (lifetimeRemaining >= 0)
        {
            AddDetail(dto, "lifetimeRemaining", "Despawn in", FormatDurationSeconds(lifetimeRemaining));
        }
        else
        {
            AddDetail(dto, "lifetimeRemaining", "Despawn in", "Unknown");
        }

        float lifetimeMax = SafeGetEntityLifetimeMax(entity);
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
    	\brief Processes the live snapshot keep alive ms operation.
    	\return \p The resulting integer value or status code.
    */
    protected static int LiveSnapshotKeepAliveMs()
    {
        return 10000;
    }

    /**
    	\brief Builds snapshot fingerprint.
    	\param snapshot \p snapshot used by the operation.
    	\return \p The resulting string value.
    */
    protected static string BuildSnapshotFingerprint(ServerSentryLiveSnapshot snapshot)
    {
        if (!snapshot)
        {
            return "";
        }

        string value = "";
        value = value + "map=" + snapshot.mapName + ";";
        value = value + "p=" + snapshot.players.Count().ToString() + ";";
        foreach (ServerSentryLivePlayerDto player: snapshot.players)
        {
            string aliveValue = BoolFingerprintValue(player.isAlive);
            value = value + player.steamId + "@" + RoundedVectorKey(player.position) + ":" + Math.Round(player.health).ToString() + ":" + Math.Round(player.blood).ToString() + ":" + aliveValue + ";";
        }

        value = value + "v=" + snapshot.vehicles.Count().ToString() + ";";
        foreach (ServerSentryLiveVehicleDto vehicle: snapshot.vehicles)
        {
            string engineOnValue = BoolFingerprintValue(vehicle.engineOn);
            value = value + vehicle.objectId + "@" + RoundedVectorKey(vehicle.position) + ":" + Math.Round(vehicle.health).ToString() + ":" + vehicle.crewSize.ToString() + ":" + engineOnValue + ";";
        }

        value = value + "e=" + snapshot.events.Count().ToString() + ";";
        foreach (ServerSentryLiveWorldEventDto eventDto: snapshot.events)
        {
            value = value + eventDto.objectId + "@" + eventDto.eventType + "@" + RoundedVectorKey(eventDto.position) + ":" + Math.Round(eventDto.radius).ToString() + ":" + eventDto.shape + ":" + eventDto.icon + ":" + eventDto.points.Count().ToString() + ":" + eventDto.actions.Count().ToString() + ";";
        }

        value = value + "b=" + snapshot.bases.Count().ToString() + ";";
        foreach (ServerSentryLiveBaseDto baseDto: snapshot.bases)
        {
            value = value + baseDto.objectId + "@" + baseDto.className + "@" + RoundedVectorKey(baseDto.position) + ":" + Math.Round(baseDto.health).ToString() + ";";
        }

        value = value + "a=" + snapshot.actions.Count().ToString() + ";";
        foreach (ServerSentryLiveModActionDescriptorDto action: snapshot.actions)
        {
            value = value + action.id + ":" + action.targetType + ";";
        }

        return value;
    }

    /**
    	\brief Processes the bool fingerprint value operation.
    	\param value \p value to process or assign.
    	\return \p The resulting string value.
    */
    protected static string BoolFingerprintValue(bool value)
    {
        if (value)
        {
            return "1";
        }

        return "0";
    }

    /**
    	\brief Processes the bool log value operation.
    	\param value \p value to process or assign.
    	\return \p The resulting string value.
    */
    protected static string BoolLogValue(bool value)
    {
        if (value)
        {
            return "true";
        }

        return "false";
    }

    /**
    	\brief Processes the rounded vector key operation.
    	\param position \p world position used by the operation.
    	\return \p The resulting string value.
    */
    protected static string RoundedVectorKey(ServerSentryVectorDto position)
    {
        if (!position)
        {
            return "no-position";
        }

        return Math.Round(position.x).ToString() + "," + Math.Round(position.y).ToString() + "," + Math.Round(position.z).ToString();
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

        if (!s_CustomMarkers)
        {
            s_CustomMarkers = new array<ref ServerSentryLiveWorldEventDto>();
        }

        if (s_LastSnapshotFingerprint == "")
        {
            s_LastSnapshotFingerprint = "";
        }
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
