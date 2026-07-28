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
        ammoType = "";
        distance = 0;
    }
}
