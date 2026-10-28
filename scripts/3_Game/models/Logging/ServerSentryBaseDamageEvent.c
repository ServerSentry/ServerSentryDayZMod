/**
    \file ServerSentryBaseDamageEvent.c
    \brief Defines the typed event for damage to base-building objects.
*/

/** \brief Represents a base-building damage event with a structured payload. */
class ServerSentryBaseDamageEvent extends ServerSentryEvent
{
    /** \brief Damage metadata captured during the engine callback. */
    ref ServerSentryBaseDamageData data;

    /** \brief Initializes the event-specific damage payload. */
    void ServerSentryBaseDamageEvent()
    {
        data = new ServerSentryBaseDamageData();
    }
}
