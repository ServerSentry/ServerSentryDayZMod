/**
	\file VoiceMuteDatabase.c
	\brief Provides persistence support for Voice mute database.
*/

/**
	\brief Persists records for Voice mute.
*/
class ServerSentryVoiceMuteDb
{
    /**
    	\brief Constant value for path.
    */
    static const string VOICE_MUTE_FILE_PATH = "$profile:ServerSentry/voice-mutes/data.json";
    /**
    	\brief Instance state for muted ids.
    */
    ref TStringArray m_MutedIds;

    void ServerSentryVoiceMuteDb()
    {
        if (!FileExist("$profile:ServerSentry/voice-mutes"))
        {
            MakeDirectory("$profile:ServerSentry/voice-mutes");
        }

        m_MutedIds = new TStringArray;
        JsonFileLoader<TStringArray>.JsonLoadFile(VOICE_MUTE_FILE_PATH, m_MutedIds);
    }

    /**
    	\brief Returns whether voice muted.
    	\param steamid \p identifier of the steam.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    bool IsVoiceMuted(string steamid)
    {
        return m_MutedIds.Find(steamid) != -1;
    }

    /**
    	\brief Adds mute.
    	\param steamid \p identifier of the steam.
    */
    void AddMute(string steamid)
    {   
        if (m_MutedIds.Find(steamid) != -1) return;

        m_MutedIds.Insert(steamid);
        JsonFileLoader<TStringArray>.JsonSaveFile(VOICE_MUTE_FILE_PATH, m_MutedIds);
    }

    /**
    	\brief Removes mute.
    	\param steamid \p identifier of the steam.
    */
    void RemoveMute(string steamid)
    {
        if (m_MutedIds.Find(steamid) != -1)
        {
            m_MutedIds.RemoveItem(steamid);
            JsonFileLoader<TStringArray>.JsonSaveFile(VOICE_MUTE_FILE_PATH, m_MutedIds);
        }
    }

    static ref ServerSentryVoiceMuteDb g_Instance;
    static ref ServerSentryVoiceMuteDb Get()
    {
        if(!g_Instance)
            g_Instance = new ServerSentryVoiceMuteDb();

        return g_Instance;
    }
};
