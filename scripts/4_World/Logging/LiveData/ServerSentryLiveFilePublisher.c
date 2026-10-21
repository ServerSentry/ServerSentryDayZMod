/**
    \file ServerSentryLiveFilePublisher.c
    \brief Publishes complete recoverable state without rewriting unchanged slow collections.
*/

/** \brief Owns one file-transport session and commits its manifest after all section files. */
class ServerSentryLiveFilePublisher
{
    /** \brief Exporter lifetime UUID; runtime entity IDs are scoped to this value. */
    protected string m_SessionId;
    /** \brief Last successfully published manifest sequence. */
    protected int m_Sequence;
    /** \brief Last base build generation examined by the native serializer. */
    protected int m_BaseGeneration = -1;
    /** \brief Last base count, including removals of deleted native entities. */
    protected int m_BaseCount = -1;
    /** \brief Immutable base section writer. */
    protected ref ServerSentryLiveSectionWriter m_Bases;
    /** \brief Immutable event section writer. */
    protected ref ServerSentryLiveSectionWriter m_Events;
    /** \brief Immutable action descriptor section writer. */
    protected ref ServerSentryLiveSectionWriter m_Actions;
    /** \brief Reused native serializer; no script-built full-state fingerprint. */
    protected ref JsonSerializer m_Serializer;

    /** \brief Initializes a fresh session without reading or enumerating existing files. */
    void ServerSentryLiveFilePublisher()
    {
        int uuid[4];
        UUIDApi.Generate(uuid);
        m_SessionId = UUIDApi.FormatString(uuid);
        m_Bases = new ServerSentryLiveSectionWriter("bases", m_SessionId);
        m_Events = new ServerSentryLiveSectionWriter("events", m_SessionId);
        m_Actions = new ServerSentryLiveSectionWriter("actions", m_SessionId);
        m_Serializer = new JsonSerializer();
    }

    /**
        \brief Publishes slow sections first, then one small complete hot-data manifest.
        \param snapshot \p complete in-memory state for this frame.
        \param baseGeneration \p changes whenever any base DTO is rebuilt.
        \return \p false when serialization or opening any required file fails.
    */
    bool Publish(ServerSentryLiveSnapshot snapshot, int baseGeneration)
    {
        if (!PrepareBases(snapshot.bases, baseGeneration))
            return false;

        string json;
        if (!m_Serializer.WriteToString(snapshot.events, false, json) || !m_Events.Prepare(json))
            return false;

        if (!m_Serializer.WriteToString(snapshot.actions, false, json) || !m_Actions.Prepare(json))
            return false;

        ServerSentryLiveManifest manifest = new ServerSentryLiveManifest(snapshot);
        manifest.sessionId = m_SessionId;
        manifest.sequence = m_Sequence + 1;
        manifest.basesRevision = m_Bases.GetRevision();
        manifest.eventsRevision = m_Events.GetRevision();
        manifest.actionsRevision = m_Actions.GetRevision();
        if (!m_Serializer.WriteToString(manifest, false, json))
            return false;

        string path = ServerSentryDefaults.LiveStateFilePath();
        if (!ServerSentryLiveSectionWriter.WriteText(path, json))
            return false;

        m_Sequence = manifest.sequence;
        m_Bases.Commit();
        m_Events.Commit();
        m_Actions.Commit();
        return true;
    }

    /**
        \brief Skips even native base serialization while every cached DTO is still current.
        \param bases \p complete current collection, including an intentionally empty world.
        \param generation \p current base DTO build generation.
        \return \p true when the referenced complete base section is ready.
    */
    protected bool PrepareBases(array<ref ServerSentryLiveBaseDto> bases, int generation)
    {
        if (m_BaseGeneration == generation && m_BaseCount == bases.Count())
            return true;

        string json;
        if (!m_Serializer.WriteToString(bases, false, json) || !m_Bases.Prepare(json))
            return false;

        m_BaseGeneration = generation;
        m_BaseCount = bases.Count();
        return true;
    }
}
