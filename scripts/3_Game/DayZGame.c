/**
	\file DayZGame.c
	\brief Defines ServerSentry support for Day z game.
*/

/**
	\brief Extends the DayZ Day z game class with ServerSentry integration.
*/
modded class DayZGame
{
    /**
    	\brief Handles the DayZ Update callback.
    	\param doSim \p true while game simulation is running.
    	\param timeslice \p elapsed frame time supplied by DayZ.
    */
    override void OnUpdate(bool doSim, float timeslice)
    {
        super.OnUpdate(doSim, timeslice);

        ServerSentryPerformance.Tick();
    }
}
