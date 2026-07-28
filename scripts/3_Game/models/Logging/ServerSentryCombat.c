/**
	\file ServerSentryCombat.c
	\brief Defines the Combat logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Combat.
*/
class ServerSentryCombat
{
    /**
    	\brief Stores the hit ID.
    */
    string hitId;
    /**
    	\brief Stores the hit sequence.
    */
    int hitSequence;

    /**
    	\brief Stores the damage.
    */
    float damage;
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
    	\brief Stores the damage type.
    */
    int damageType;
    /**
    	\brief Stores the component.
    */
    int component;

    /**
    	\brief Stores the hit zone.
    */
    string hitZone;

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

    // Derived low-cardinality dimensions for frontend/admin aggregation.
    /**
    	\brief Stores the relation.
    */
    string relation;
    /**
    	\brief Stores the actor group.
    */
    string actorGroup;
    /**
    	\brief Stores the actor kind.
    */
    string actorKind;
    /**
    	\brief Stores the target group.
    */
    string targetGroup;
    /**
    	\brief Stores the target kind.
    */
    string targetKind;

    /**
    	\brief Indicates whether counts for player stats.
    */
    bool countsForPlayerStats;
    /**
    	\brief Indicates whether counts for player hit stats.
    */
    bool countsForPlayerHitStats;
    /**
    	\brief Indicates whether counts for player kill stats.
    */
    bool countsForPlayerKillStats;

    /**
    	\brief Initializes a new ServerSentryCombat instance.
    */
    void ServerSentryCombat()
    {
        hitId = "";
        hitSequence = 0;
        damage = 0;
        bloodDamage = 0;
        shockDamage = 0;
        targetHealthBefore = 0;
        targetHealthAfter = 0;
        damageType = 0;
        component = 0;
        hitZone = "";
        targetAliveAfter = true;
        isHeadshot = false;
        isMelee = false;
        isExplosion = false;
        isLethal = false;
        relation = "";
        actorGroup = "";
        actorKind = "";
        targetGroup = "";
        targetKind = "";
        countsForPlayerStats = false;
        countsForPlayerHitStats = false;
        countsForPlayerKillStats = false;
    }
}
