/**
	\file ServerSentryLivePlayerDto.c
	\brief Defines the Live player DTO live-data transfer model.
*/

/**
	\brief Data transfer object for Live player.
*/
class ServerSentryLivePlayerDto
{
    /**
    	\brief Stores the name.
    */
    string name;
    /** \brief CMS marker icon name: bundled SVG first, then Lucide. */
    string icon;
    /**
    	\brief Steam ID of the target player.
    */
    string steamId;
    /**
    	\brief Stores the bohemia ID.
    */
    string bohemiaId;
    /**
    	\brief Network or persistent ID of the target object.
    */
    string objectId;
    /**
    	\brief Stores the ping avg.
    */
    int pingAvg;
    /**
    	\brief Stores the position.
    */
    ref ServerSentryVectorDto position;
    /**
    	\brief Stores the health.
    */
    float health;
    /**
    	\brief Stores the blood.
    */
    float blood;
    /**
    	\brief Stores the shock.
    */
    float shock;
    /**
    	\brief Indicates whether is alive.
    */
    bool isAlive;

    /**
    	\brief Initializes a new ServerSentryLivePlayerDto instance.
    */
    void ServerSentryLivePlayerDto()
    {
        name = "";
        icon = "UserRound";
        steamId = "";
        bohemiaId = "";
        objectId = "";
        pingAvg = 0;
        position = new ServerSentryVectorDto();
        health = 0;
        blood = 0;
        shock = 0;
        isAlive = false;
    }
}
