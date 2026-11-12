/**
    \file ServerSentryMapMarkers.c
    \brief Owns server-side map markers independently of world entities and snapshot export.
*/

/**
    \brief Creates markers in 3_Game and stores them until removed or the mission ends.
*/
class ServerSentryMapMarkers
{
    /** \brief Maximum number of registered custom markers. */
    static const int MAX_MARKERS = 512;
    /** \brief Maximum number of points retained by one line. */
    static const int MAX_LINE_POINTS = 256;
    /** \brief Maximum circle radius in metres. */
    static const float MAX_RADIUS = 100000;
    /** \brief Maximum absolute coordinate accepted by the map transport. */
    static const float MAX_COORDINATE = 1000000;
    /** \brief Shared registry for the public marker API and legacy World integrations. */
    protected static ref map<string, ref ServerSentryLiveWorldEventDto> s_Markers;
    /** \brief Process-local sequence; not reset when clearing a mission. */
    protected static int s_Sequence;

    /**
        \brief Creates and registers a point, or a circle when radius is positive.
        \param name \p display name, up to 200 characters.
        \param icon \p Lucide icon name; empty uses MapPin.
        \param position \p world position in X/Y/Z metres.
        \param radius \p radius in metres, from zero through MAX_RADIUS.
        \param id \p optional mod-prefixed ID; empty generates a unique session ID.
        \return \p Marker handle, or null for invalid input, a client call, or a full registry.
        \note Registration does not require live export to be started and performs no file IO.
    */
    static ServerSentryMapMarker Create(string name, string icon, vector position, float radius = 0, string id = "")
    {
        if (!CanUse() || !IsPositionValid(position) || !IsRadiusValid(radius))
            return null;

        ServerSentryLiveWorldEventDto data = CreateData(name, icon, position, id);
        if (!data)
            return null;

        data.radius = radius;
        if (radius > 0)
            data.shape = "circle";

        if (!Register(data))
            return null;

        return new ServerSentryMapMarker(data);
    }

    /**
        \brief Creates and registers a circle with a positive radius.
        \param name \p display name.
        \param icon \p Lucide icon name.
        \param position \p circle centre in world coordinates.
        \param radius \p positive radius in metres, at most MAX_RADIUS.
        \param id \p optional mod-prefixed ID.
        \return \p Registered marker, or null when creation fails.
    */
    static ServerSentryMapMarker CreateCircle(string name, string icon, vector position, float radius, string id = "")
    {
        if (radius <= 0)
            return null;

        return Create(name, icon, position, radius, id);
    }

    /**
        \brief Creates an open line, copying its positions and placing the icon at its endpoint.
        \param name \p route or trail display name.
        \param icon \p Lucide endpoint icon.
        \param positions \p two through MAX_LINE_POINTS ordered world positions.
        \param id \p optional mod-prefixed ID.
        \return \p Registered marker, or null when creation fails.
    */
    static ServerSentryMapMarker CreateLine(string name, string icon, TVectorArray positions, string id = "")
    {
        if (!CanUse() || !ArePointsValid(positions))
            return null;

        vector endpoint = positions.Get(positions.Count() - 1);
        ServerSentryLiveWorldEventDto data = CreateData(name, icon, endpoint, id);
        if (!data)
            return null;

        ReplaceLinePoints(data, positions);
        if (!Register(data))
            return null;

        return new ServerSentryMapMarker(data);
    }

    /**
        \brief Removes all markers in the supplied integration namespace.
        \param prefix \p non-empty mod/event ID prefix.
        \return \p Number of removed entries.
    */
    static int ClearByPrefix(string prefix)
    {
        if (!CanUse() || prefix == "" || !s_Markers)
            return 0;

        int removed;
        for (int i = s_Markers.Count() - 1; i >= 0; i--)
        {
            string id = s_Markers.GetKey(i);
            if (id.IndexOf(prefix) == 0)
            {
                s_Markers.Remove(id);
                removed++;
            }
        }
        return removed;
    }

    /**
        \brief Clears mission-owned entries and invalidates outstanding handles.
    */
    static void Clear()
    {
        if (s_Markers)
            s_Markers.Clear();
    }

    /**
        \brief Returns the current registry size without creating it.
        \return \p Number of registered custom markers.
    */
    static int Count()
    {
        if (!s_Markers)
            return 0;

        return s_Markers.Count();
    }

    /**
        \brief Checks whether marker operations are running on a server.
        \return \p true when a server game instance is available.
    */
    static bool CanUse()
    {
        return GetGame() && GetGame().IsServer();
    }

    /**
        \brief Validates finite coordinates within the supported map bounds.
        \param position \p world position.
        \return \p true when every component lies within MAX_COORDINATE.
    */
    static bool IsPositionValid(vector position)
    {
        for (int i = 0; i < 3; i++)
        {
            if (!(position[i] >= -MAX_COORDINATE && position[i] <= MAX_COORDINATE))
                return false;
        }
        return true;
    }

    /**
        \brief Validates a finite, non-negative radius.
        \param radius \p candidate radius in metres.
        \return \p true for values from zero through MAX_RADIUS.
    */
    static bool IsRadiusValid(float radius)
    {
        return radius >= 0 && radius <= MAX_RADIUS;
    }

    /**
        \brief Validates every line point before modifying any registered geometry.
        \param positions \p candidate ordered positions.
        \return \p true for two through MAX_LINE_POINTS valid positions.
    */
    static bool ArePointsValid(TVectorArray positions)
    {
        if (!positions || positions.Count() < 2 || positions.Count() > MAX_LINE_POINTS)
            return false;

        foreach (vector position: positions)
        {
            if (!IsPositionValid(position))
                return false;
        }
        return true;
    }

    /**
        \brief Copies an already validated line into its transport data.
        \param data \p marker data owned by the registry.
        \param positions \p validated ordered positions.
        \note Internal bridge for marker handles; callers validate first.
    */
    static void ReplaceLinePoints(ServerSentryLiveWorldEventDto data, TVectorArray positions)
    {
        data.shape = "polyline";
        data.radius = 0;
        data.points.Clear();
        foreach (vector position: positions)
        {
            ServerSentryVectorDto point = new ServerSentryVectorDto();
            point.FromVector(position);
            data.points.Insert(point);
        }
        vector endpoint = positions.Get(positions.Count() - 1);
        data.position.FromVector(endpoint);
    }

    /**
        \brief Registers transport data shared with the legacy World API.
        \param data \p marker to register or replace by ID.
        \return \p true when registered; replacements do not consume additional capacity.
    */
    static bool Register(ServerSentryLiveWorldEventDto data)
    {
        if (!CanUse() || !data || data.objectId == "")
            return false;

        if (!s_Markers)
            s_Markers = new map<string, ref ServerSentryLiveWorldEventDto>();

        if (!s_Markers.Contains(data.objectId) && s_Markers.Count() >= MAX_MARKERS)
            return false;

        s_Markers.Set(data.objectId, data);
        return true;
    }

    /**
        \brief Looks up transport data for legacy integrations and handle identity checks.
        \param id \p registered marker ID.
        \return \p Registered data, or null when absent.
    */
    static ServerSentryLiveWorldEventDto Find(string id)
    {
        if (!s_Markers)
            return null;

        return s_Markers.Get(id);
    }

    /**
        \brief Prevents removed or replaced handles from mutating a newer registration.
        \param data \p data held by the caller.
        \return \p true only for the exact registered instance on a server.
    */
    static bool IsRegistered(ServerSentryLiveWorldEventDto data)
    {
        return CanUse() && data && Find(data.objectId) == data;
    }

    /**
        \brief Removes a registry entry without performing file IO.
        \param id \p marker ID to remove.
        \return \p true when an entry was removed.
    */
    static bool Remove(string id)
    {
        if (!CanUse() || !s_Markers || !s_Markers.Contains(id))
            return false;

        s_Markers.Remove(id);
        return true;
    }

    /**
        \brief Appends registered transport data to the world exporter's event collection.
        \param events \p snapshot collection receiving the custom markers.
    */
    static void AppendTo(array<ref ServerSentryLiveWorldEventDto> events)
    {
        if (!events || !s_Markers)
            return;

        for (int i = 0; i < s_Markers.Count(); i++)
        {
            ServerSentryLiveWorldEventDto data = s_Markers.GetElement(i);
            if (data && data.position)
                events.Insert(data);
        }
    }

    /**
        \brief Normalizes metadata and creates point data without accessing world objects.
        \param name \p display name, trimmed before validation.
        \param icon \p icon name, trimmed and defaulted to MapPin when empty.
        \param position \p validated world position.
        \param id \p explicit identity, or empty for a collision-free session ID.
        \return \p Initialized point data, or null when metadata is invalid.
    */
    protected static ServerSentryLiveWorldEventDto CreateData(string name, string icon, vector position, string id)
    {
        name.TrimInPlace();
        icon.TrimInPlace();
        id.TrimInPlace();
        if (name == "" || name.Length() > 200 || icon.Length() > 80 || id.Length() > 240)
            return null;

        if (icon == "")
            icon = "MapPin";

        if (id == "")
        {
            while (true)
            {
                s_Sequence++;
                id = "serversentry:auto:" + s_Sequence.ToString();
                if (!Find(id))
                    break;
            }
        }
        ServerSentryLiveWorldEventDto data = new ServerSentryLiveWorldEventDto();
        data.eventType = "custom_marker";
        data.className = "ServerSentryLiveMarker";
        data.objectId = id;
        data.displayName = name;
        data.icon = icon;
        data.health = -1;
        data.position.FromVector(position);
        return data;
    }
}
