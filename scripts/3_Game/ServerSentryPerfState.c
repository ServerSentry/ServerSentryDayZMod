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

    /**
    	\brief Instance state for update counter.
    */
    private static int m_UpdateCounter = 0;
    /**
    	\brief Instance state for last sample time.
    */
    private static float m_LastSampleTime = 0.0;

    /**
    	\brief Updates the requested operation.
    */
    static void Tick()
    {
        if (!GetGame() || !GetGame().IsServer())
            return;

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

        m_UpdateCounter = 0;
        m_LastSampleTime = now;
    }
}
