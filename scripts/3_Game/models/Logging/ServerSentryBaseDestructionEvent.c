/**
    \file ServerSentryBaseDestructionEvent.c
    \brief Defines the typed event for an observed construction-part destruction.
*/

/** \brief Represents a part-destruction observation independently of damage events. */
class ServerSentryBaseDestructionEvent extends ServerSentryEvent
{
    /** \brief Callback metadata and confirmed construction-state transition. */
    ref ServerSentryBaseDestructionData data;

    /** \brief Initializes the event-specific part-destruction payload. */
    void ServerSentryBaseDestructionEvent()
    {
        data = new ServerSentryBaseDestructionData();
    }
}
