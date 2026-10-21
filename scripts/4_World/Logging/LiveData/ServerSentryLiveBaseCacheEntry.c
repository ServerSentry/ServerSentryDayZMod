/**
    \file ServerSentryLiveBaseCacheEntry.c
    \brief Stores bounded base-detail snapshots without owning native world objects.
*/

/** \brief Reuses provider output until its TTL expires or the base moves. */
class ServerSentryLiveBaseCacheEntry
{
    /** \brief Weak native reference; deletion invalidates the entry. */
    EntityAI entity;
    /** \brief Completed DTO, including provider overrides and registered actions. */
    ref ServerSentryLiveBaseDto dto;
    /** \brief Position at capture, independent of provider display overrides. */
    vector position;
    /** \brief Orientation at capture. */
    vector orientation;
    /** \brief Capture time; negative ages invalidate entries after a clock reset. */
    int capturedAtMs;

    /**
        \brief Checks identity, age, position and orientation before reusing provider output.
        \param candidate \p currently registered base.
        \param nowMs \p current game time.
        \param lifetimeMs \p configured cache lifetime; zero disables reuse.
        \return \p true only for an unchanged, live base inside the cache interval.
    */
    bool IsFresh(EntityAI candidate, int nowMs, int lifetimeMs)
    {
        if (!entity || entity != candidate || !dto || lifetimeMs <= 0)
            return false;

        int ageMs = nowMs - capturedAtMs;
        if (ageMs < 0 || ageMs >= lifetimeMs)
            return false;

        return position == candidate.GetPosition() && orientation == candidate.GetOrientation();
    }
}
