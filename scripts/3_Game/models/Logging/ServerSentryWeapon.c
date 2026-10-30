/**
	\file ServerSentryWeapon.c
	\brief Defines the Weapon logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Weapon.
*/
class ServerSentryWeapon
{
    /**
    	\brief Stores the class name.
    */
    string className;
    /**
    	\brief Stores the display name.
    */
    string displayName;
    /** \brief Canonical DayZ item PID of the verified source; never a runtime or explosive UUID. */
    string persistentId;
    /**
    	\brief Stores the ammo type.
    */
    string ammoType;

    /**
    	\brief Stores the distance.
    */
    float distance;

    /**
    	\brief Initializes a new ServerSentryWeapon instance.
    */
    void ServerSentryWeapon()
    {
        className = "";
        displayName = "";
        persistentId = "";
        ammoType = "";
        distance = 0;
    }
}
