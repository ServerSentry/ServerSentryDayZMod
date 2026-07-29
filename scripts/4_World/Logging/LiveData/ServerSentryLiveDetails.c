/**
    \file ServerSentryLiveDetails.c
    \brief Provides key-based detail helpers for Live Map objects.
*/

/**
    \brief Adds, replaces, and removes Live Map details by stable key.
*/
class ServerSentryLiveDetails
{
    /**
        \brief Adds or replaces a base detail while preserving its display position.
        \param dto \p dto receiving the detail.
        \param key \p stable detail key. Matching is case-insensitive.
        \param label \p human-readable label shown on the Live Map.
        \param value \p human-readable value shown on the Live Map.
    */
    static void UpsertBaseDetail(ServerSentryLiveBaseDto dto, string key, string label, string value)
    {
        if (!dto || key == "")
        {
            return;
        }

        if (!dto.details)
        {
            dto.details = new array<ref ServerSentryLiveDetailDto>();
        }

        UpsertDetail(dto.details, key, label, value);
    }

    /**
        \brief Removes all base details with the supplied stable key.
        \param dto \p dto containing the detail.
        \param key \p stable detail key. Matching is case-insensitive.
    */
    static void RemoveBaseDetail(ServerSentryLiveBaseDto dto, string key)
    {
        if (!dto || !dto.details || key == "")
        {
            return;
        }

        RemoveDetail(dto.details, key);
    }

    /**
        \brief Adds or replaces a world-marker detail while preserving its display position.
        \param dto \p dto receiving the detail.
        \param key \p stable detail key. Matching is case-insensitive.
        \param label \p human-readable label shown on the Live Map.
        \param value \p human-readable value shown on the Live Map.
    */
    static void UpsertMarkerDetail(ServerSentryLiveWorldEventDto dto, string key, string label, string value)
    {
        if (!dto || key == "")
        {
            return;
        }

        if (!dto.details)
        {
            dto.details = new array<ref ServerSentryLiveDetailDto>();
        }

        UpsertDetail(dto.details, key, label, value);
    }

    /**
        \brief Removes all world-marker details with the supplied stable key.
        \param dto \p dto containing the detail.
        \param key \p stable detail key. Matching is case-insensitive.
    */
    static void RemoveMarkerDetail(ServerSentryLiveWorldEventDto dto, string key)
    {
        if (!dto || !dto.details || key == "")
        {
            return;
        }

        RemoveDetail(dto.details, key);
    }

    /**
        \brief Adds or replaces a detail in a detail collection.
    */
    protected static void UpsertDetail(array<ref ServerSentryLiveDetailDto> details, string key, string label, string value)
    {
        if (!details || key == "")
        {
            return;
        }

        for (int i = 0; i < details.Count(); i++)
        {
            ServerSentryLiveDetailDto existing = details.Get(i);
            if (existing && KeysEqual(existing.key, key))
            {
                existing.key = key;
                existing.label = ResolveLabel(key, label);
                existing.value = value;
                return;
            }
        }

        ServerSentryLiveDetailDto detail = new ServerSentryLiveDetailDto();
        detail.key = key;
        detail.label = ResolveLabel(key, label);
        detail.value = value;
        details.Insert(detail);
    }

    /**
        \brief Removes all matching details from a detail collection.
    */
    protected static void RemoveDetail(array<ref ServerSentryLiveDetailDto> details, string key)
    {
        for (int i = details.Count() - 1; i >= 0; i--)
        {
            ServerSentryLiveDetailDto detail = details.Get(i);
            if (detail && KeysEqual(detail.key, key))
            {
                details.Remove(i);
            }
        }
    }

    /**
        \brief Compares stable keys without case sensitivity.
    */
    protected static bool KeysEqual(string left, string right)
    {
        left.TrimInPlace();
        right.TrimInPlace();
        left.ToLower();
        right.ToLower();
        return left == right;
    }

    /**
        \brief Resolves a non-empty display label.
    */
    protected static string ResolveLabel(string key, string label)
    {
        label.TrimInPlace();
        if (label != "")
        {
            return label;
        }

        return key;
    }
}
