/**
    \file ServerSentryExplosiveStorage.c
    \brief Persists explosive roles in the isolated ServerSentry CF storage stream.
*/

/** \brief Keeps attribution out of vanilla persistence bytes and transient object IDs. */
class ServerSentryExplosiveStorage
{
    /**
        \brief Writes the persistent roles, omitting runtime detonation snapshots.
        \param storage \p ServerSentry version-one CF stream.
        \param state \p device attribution to save.
    */
    static void Save(CF_ModStorage storage, ServerSentryExplosiveAttribution state)
    {
        storage.Write(state.deviceId);
        storage.Write(state.parentDeviceId);
        storage.Write(state.activation);
        storage.Write(state.releaseObserved);
        WriteIdentity(storage, state.placedBy);
        WriteIdentity(storage, state.armedBy);
        WriteIdentity(storage, state.releasedBy);
        WriteIdentity(storage, state.detonatedBy);
        WriteIdentity(storage, state.triggeredBy);
    }

    /**
        \brief Reads an entire attribution before publishing it to the live item.
        \param storage \p isolated CF stream at its initial position.
        \param state \p receives validated attribution, or null on failure.
        \return \p true for a supported complete record.
    */
    static bool Load(CF_ModStorage storage, out ServerSentryExplosiveAttribution state)
    {
        state = null;
        if (storage.GetVersion() != 1)
            return false;

        ServerSentryExplosiveAttribution loaded = new ServerSentryExplosiveAttribution();
        if (!storage.Read(loaded.deviceId) || loaded.deviceId == "")
            return false;
        if (!storage.Read(loaded.parentDeviceId) || !storage.Read(loaded.activation))
            return false;
        if (!storage.Read(loaded.releaseObserved))
            return false;
        if (!ReadIdentity(storage, loaded.placedBy) || !ReadIdentity(storage, loaded.armedBy))
            return false;
        if (!ReadIdentity(storage, loaded.releasedBy) || !ReadIdentity(storage, loaded.detonatedBy))
            return false;
        if (!ReadIdentity(storage, loaded.triggeredBy))
            return false;

        state = loaded;
        return true;
    }

    /**
        \brief Stores one optional identity as small typed CF entries.
        \param storage \p target CF stream.
        \param identity \p copied identity, or null.
    */
    protected static void WriteIdentity(CF_ModStorage storage, ServerSentryExplosiveIdentity identity)
    {
        storage.Write(identity != null);
        if (!identity)
            return;

        storage.Write(identity.steamId);
        storage.Write(identity.bohemiaId);
        storage.Write(identity.name);
        storage.Write(identity.className);
        storage.Write(identity.action);
        storage.Write(identity.occurredAtUtc);
    }

    /**
        \brief Reconstructs an optional verified player identity without a live player lookup.
        \param storage \p source CF stream.
        \param identity \p receives the identity, or null when absent.
        \return \p true for a complete identity record.
    */
    protected static bool ReadIdentity(CF_ModStorage storage, out ServerSentryExplosiveIdentity identity)
    {
        identity = null;
        bool present;
        if (!storage.Read(present))
            return false;
        if (!present)
            return true;

        ServerSentryExplosiveIdentity loaded = new ServerSentryExplosiveIdentity();
        if (!storage.Read(loaded.steamId) || loaded.steamId == "")
            return false;
        if (!storage.Read(loaded.bohemiaId) || !storage.Read(loaded.name))
            return false;
        if (!storage.Read(loaded.className) || !storage.Read(loaded.action))
            return false;
        if (!storage.Read(loaded.occurredAtUtc))
            return false;

        loaded.type = "player";
        loaded.entityKind = "human_player";
        identity = loaded;
        return true;
    }
}
