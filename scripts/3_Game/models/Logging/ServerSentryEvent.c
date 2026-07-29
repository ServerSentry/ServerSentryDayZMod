/**
	\file ServerSentryEvent.c
	\brief Defines the Event logging data model.
*/

/**
	\brief Represents ServerSentry events.
*/
class ServerSentryEvent
{
    /**
    	\brief Stores the schema version.
    */
    int schemaVersion = 1;
    /**
    	\brief Stores the event ID.
    */
    string eventId;
    /**
    	\brief Stores the sequence.
    */
    int sequence;
    /**
    	\brief Stores the occurred at utc.
    */
    string occurredAtUtc;
    /**
    	\brief Stores the server ID.
    */
    string serverId;
    /**
    	\brief Stores the boot ID.
    */
    string bootId;

    /**
    	\brief Stores the event type.
    */
    string eventType;
    /**
    	\brief Stores the category.
    */
    string category;
    /**
    	\brief Stores the severity.
    */
    string severity;

    /**
    	\brief Stores the mod.
    */
    string mod;
    /**
    	\brief Stores the action.
    */
    ref ServerSentryAction action;
    /**
    	\brief Stores the actor.
    */
    ref ServerSentryActor actor;
    /**
    	\brief Stores the target.
    */
    ref ServerSentryTarget target;
    /**
    	\brief Stores the primary event position.
    */
    ref ServerSentryVectorDto position;
    /**
    	\brief Stores the actor position for events with two spatial participants.
    */
    ref ServerSentryVectorDto actorPosition;
    /**
    	\brief Stores the target position for events with two spatial participants.
    */
    ref ServerSentryVectorDto targetPosition;
    /**
    	\brief Schema used for each list item.
    */
    ref ServerSentryItem item;
    /**
    	\brief Stores the weapon.
    */
    ref ServerSentryWeapon weapon;
    /**
    	\brief Stores the combat.
    */
    ref ServerSentryCombat combat;
    /**
    	\brief Human-readable result or error message.
    */
    string message;
    /**
    	\brief Stores the raw.
    */
    string raw;
    /**
    	\brief Stores the data JSON.
    */
    string dataJson;

    /**
    	\brief Initializes a new ServerSentryEvent instance.
    */
    void ServerSentryEvent()
    {
        mod = "Server Sentry";
        action = new ServerSentryAction();
        actor = new ServerSentryActor();
        target = new ServerSentryTarget();
        item = new ServerSentryItem();
        weapon = new ServerSentryWeapon();
        combat = new ServerSentryCombat();
        position = new ServerSentryVectorDto();
    }
}
