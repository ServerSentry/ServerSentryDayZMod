/**
	\file ServerSentryPerfState.c
	\brief Defines ServerSentry support for Perf state.
*/

/**
	\brief Provides ServerSentry behavior for Performance.
*/
class ServerSentryPerformance
{
    /**
    	\brief Stores the current server fps.
    */
    static int CurrentServerFps = 0;
    /**
    	\brief Stores the min server fps.
    */
    static int MinServerFps = 0;
    /**
    	\brief Stores the max server fps.
    */
    static int MaxServerFps = 0;
    /** \brief Mean simulation frame interval in milliseconds, or -1 before sampling. */
    static float AverageTickTimeMs = -1.0;
    /** \brief Longest simulation frame interval in the latest window, or -1. */
    static float MaxTickTimeMs = -1.0;
    /** \brief Number of simulation frames represented by the published window. */
    static int TickSampleCount = 0;
    /** \brief Accumulated simulation frame intervals in seconds. */
    private static float m_TickSeconds = 0.0;
    /** \brief Longest interval in the window, in seconds. */
    private static float m_MaxTickSeconds = 0.0;
    /** \brief Number of simulation frames in the accumulating window. */
    private static int m_TickSamples = 0;

    /**
    	\brief Instance state for update counter.
    */
    private static int m_UpdateCounter = 0;
    /**
    	\brief Instance state for last sample time.
    */
    private static float m_LastSampleTime = 0.0;

    /**
        \brief Samples simulation intervals while preserving the existing FPS estimate.
        \param doSim \p true for simulation updates.
        \param timeslice \p frame interval supplied by DayZ, in seconds.
    */
    static void Tick(bool doSim = false, float timeslice = 0.0)
    {
        if (!GetGame() || !GetGame().IsServer())
            return;

        // Simulation frame intervals, not the duration of this mod's callback.
        if (doSim && timeslice > 0.0 && timeslice <= 600.0)
        {
            m_TickSeconds = m_TickSeconds + timeslice;
            m_TickSamples++;
            if (timeslice > m_MaxTickSeconds)
                m_MaxTickSeconds = timeslice;
        }
        m_UpdateCounter++;

        float now = GetGame().GetTickTime();

        if (m_LastSampleTime == 0.0)
        {
            m_LastSampleTime = now;
            return;
        }

        float elapsed = now - m_LastSampleTime;

        if (elapsed < 1.0)
            return;

        float updatesPerSecond = m_UpdateCounter / elapsed;

        // DayZ/CFTools-artige Server-FPS-Schätzung
        int estimatedServerFps = (int)updatesPerSecond;

        CurrentServerFps = estimatedServerFps;

        if (MinServerFps == 0 || estimatedServerFps < MinServerFps)
            MinServerFps = estimatedServerFps;

        if (MaxServerFps == 0 || estimatedServerFps > MaxServerFps)
            MaxServerFps = estimatedServerFps;

        TickSampleCount = m_TickSamples;
        AverageTickTimeMs = -1.0;
        MaxTickTimeMs = -1.0;
        if (m_TickSamples > 0)
        {
            AverageTickTimeMs = (m_TickSeconds / m_TickSamples) * 1000.0;
            MaxTickTimeMs = m_MaxTickSeconds * 1000.0;
        }
        m_TickSeconds = 0.0;
        m_MaxTickSeconds = 0.0;
        m_TickSamples = 0;
        m_UpdateCounter = 0;
        m_LastSampleTime = now;
    }
}

/** \brief Constant-size server-side lifecycle counters without scans or an entity registry. */
class ServerSentryWorldItemCounts
{
    /** \brief All loaded ItemBase instances, including inventory and attachments. */
    static int Loaded = 0;
    /** \brief Loaded ItemBase instances currently without an inventory parent. */
    static int Ground = 0;
}
