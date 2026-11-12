/**
    \file ServerSentryMapMarker.c
    \brief Provides a small marker handle without exposing mutable transport fields.
*/

/**
    \brief Updates one registered marker; removal or replacement invalidates this handle.
*/
class ServerSentryMapMarker
{
    /** \brief Transport data shared only with the registry and snapshot bridge. */
    protected ref ServerSentryLiveWorldEventDto m_Data;

    /**
        \brief Wraps registered data; integrations obtain handles from ServerSentryMapMarkers.
        \param data \p marker data created and owned by the registry.
    */
    void ServerSentryMapMarker(ServerSentryLiveWorldEventDto data)
    {
        m_Data = data;
    }

    /**
        \brief Returns the stable ID even after this handle is removed or replaced.
        \return \p Marker ID, or an empty string for an uninitialized handle.
    */
    string GetId()
    {
        if (!m_Data)
            return "";

        return m_Data.objectId;
    }

    /**
        \brief Checks whether this exact registration still exists on the server.
        \return \p false after removal, replacement, or mission cleanup.
    */
    bool IsActive()
    {
        return ServerSentryMapMarkers.IsRegistered(m_Data);
    }

    /**
        \brief Moves a point or circle without changing its radius.
        \param position \p new world position.
        \return \p true when the active point/circle was updated.
    */
    bool SetPosition(vector position)
    {
        if (!IsActive() || !ServerSentryMapMarkers.IsPositionValid(position))
            return false;

        if (m_Data.shape != "point" && m_Data.shape != "circle")
            return false;

        m_Data.position.FromVector(position);
        return true;
    }

    /**
        \brief Sets a circle radius, or changes the marker to a point when radius is zero.
        \param radius \p finite radius from zero through MAX_RADIUS.
        \return \p true when the active point/circle was updated.
    */
    bool SetRadius(float radius)
    {
        if (!IsActive() || !ServerSentryMapMarkers.IsRadiusValid(radius))
            return false;

        if (m_Data.shape != "point" && m_Data.shape != "circle")
            return false;

        m_Data.radius = radius;
        m_Data.shape = "point";
        if (radius > 0)
            m_Data.shape = "circle";

        return true;
    }

    /**
        \brief Replaces an open line after validating every position.
        \param positions \p two through MAX_LINE_POINTS ordered positions, copied on success.
        \return \p true when updated; invalid input preserves the entire previous line.
    */
    bool SetPoints(TVectorArray positions)
    {
        if (!IsActive() || m_Data.shape != "polyline" || !ServerSentryMapMarkers.ArePointsValid(positions))
            return false;

        ServerSentryMapMarkers.ReplaceLinePoints(m_Data, positions);
        return true;
    }

    /**
        \brief Appends a trail sample, retaining only the newest MAX_LINE_POINTS positions.
        \param position \p observed position; an identical adjacent sample is ignored.
        \return \p true for an accepted update to an active line.
    */
    bool AppendPoint(vector position)
    {
        if (!IsActive() || m_Data.shape != "polyline" || !ServerSentryMapMarkers.IsPositionValid(position))
            return false;

        int count = m_Data.points.Count();
        if (count > 0)
        {
            ServerSentryVectorDto last = m_Data.points.Get(count - 1);
            if (last.x == position[0] && last.y == position[1] && last.z == position[2])
                return true;
        }

        while (m_Data.points.Count() >= ServerSentryMapMarkers.MAX_LINE_POINTS)
            m_Data.points.Remove(0);

        ServerSentryVectorDto point = new ServerSentryVectorDto();
        point.FromVector(position);
        m_Data.points.Insert(point);
        m_Data.position.FromVector(position);
        return true;
    }

    /**
        \brief Removes this registration without affecting a newer marker with the same ID.
        \return \p true when removed; false for an already removed or replaced handle.
    */
    bool Remove()
    {
        if (!IsActive())
            return false;

        return ServerSentryMapMarkers.Remove(m_Data.objectId);
    }
}
