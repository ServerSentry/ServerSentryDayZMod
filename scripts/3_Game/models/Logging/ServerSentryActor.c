/**
	\file ServerSentryActor.c
	\brief Defines the Actor logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Actor.
*/
class ServerSentryActor
{
    // Stable analytics dimension. Keep values low-cardinality:
    // player, ai, zombie, animal, environment, object, unknown.
    /**
    	\brief Input or event type exposed to external consumers.
    */
    string type;

    // More specific low-cardinality dimension for frontend grouping, e.g.
    // human_player, human_ai, infected, wolf, bear, deer, chicken.
    /**
    	\brief Stores the entity kind.
    */
    string entityKind;

    // Exact DayZ/mod classname for drill-down/debugging. High-cardinality fields
    // should not be used as the primary stat dimension in the cloud.
    /**
    	\brief Stores the class name.
    */
    string className;

    /**
    	\brief Steam ID of the target player.
    */
    string steamId;
    /**
    	\brief Stores the bohemia ID.
    */
    string bohemiaId;
    /**
    	\brief Stores the name.
    */
    string name;

    /**
    	\brief Initializes a new ServerSentryActor instance.
    */
    void ServerSentryActor()
    {
        type = "";
        entityKind = "";
        className = "";
        steamId = "";
        bohemiaId = "";
        name = "";
    }
}
