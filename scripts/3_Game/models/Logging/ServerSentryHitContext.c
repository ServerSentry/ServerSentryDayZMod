/**
	\file ServerSentryHitContext.c
	\brief Defines the Hit context logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Hit context.
*/
class ServerSentryHitContext
{
    /**
    	\brief Stores the attacker.
    */
    ref ServerSentryActor attacker;
    /**
    	\brief Stores the target.
    */
    ref ServerSentryTarget target;

    /**
    	\brief Stores the hit ID.
    */
    string hitId;
    /**
    	\brief Stores the occurred at utc.
    */
    string occurredAtUtc;

    /**
    	\brief Stores the weapon class name.
    */
    string weaponClassName;
    /**
    	\brief Stores the ammo type.
    */
    string ammoType;
    /**
    	\brief Stores the hit zone.
    */
    string hitZone;

    /**
    	\brief Stores the health damage.
    */
    float healthDamage;
    /**
    	\brief Stores the blood damage.
    */
    float bloodDamage;
    /**
    	\brief Stores the shock damage.
    */
    float shockDamage;

    /**
    	\brief Stores the target health before.
    */
    float targetHealthBefore;
    /**
    	\brief Stores the target health after.
    */
    float targetHealthAfter;
    /**
    	\brief Stores the distance.
    */
    float distance;

    /**
    	\brief Stores the damage type.
    */
    int damageType;
    /**
    	\brief Stores the component.
    */
    int component;
    /**
    	\brief Stores the event sequence.
    */
    int eventSequence;

    /**
    	\brief Stores the victim position.
    */
    vector victimPosition;
    /**
    	\brief Stores the attacker position when the source can be resolved.
    */
    vector attackerPosition;
    /**
    	\brief Indicates whether attacker position contains a resolved world position.
    */
    bool hasAttackerPosition;

    /**
    	\brief Indicates whether target alive after.
    */
    bool targetAliveAfter;
    /**
    	\brief Indicates whether is headshot.
    */
    bool isHeadshot;
    /**
    	\brief Indicates whether is melee.
    */
    bool isMelee;
    /**
    	\brief Indicates whether is explosion.
    */
    bool isExplosion;
    /**
    	\brief Indicates whether is lethal.
    */
    bool isLethal;

    /**
    	\brief Initializes a new ServerSentryHitContext instance.
    */
    void ServerSentryHitContext()
    {
        attacker = new ServerSentryActor();
        target = new ServerSentryTarget();

        hitId = "";
        occurredAtUtc = "";
        weaponClassName = "";
        ammoType = "";
        hitZone = "";

        healthDamage = 0;
        bloodDamage = 0;
        shockDamage = 0;
        targetHealthBefore = 0;
        targetHealthAfter = 0;
        distance = 0;

        damageType = 0;
        component = 0;
        eventSequence = 0;
        victimPosition = "0 0 0";
        attackerPosition = "0 0 0";
        hasAttackerPosition = false;

        targetAliveAfter = true;
        isHeadshot = false;
        isMelee = false;
        isExplosion = false;
        isLethal = false;
    }
};
