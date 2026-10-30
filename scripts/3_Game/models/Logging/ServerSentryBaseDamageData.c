/**
    \file ServerSentryBaseDamageData.c
    \brief Defines primitive snapshots of a base-building damage callback.
*/

/** \brief Records the health damage reported for one damage zone. */
class ServerSentryBaseDamageZone
{
    /** \brief Name of the zone in the target's damage configuration. */
    string zoneName;
    /** \brief Health damage reported by TotalDamageResult for this zone. */
    float healthDamage;

    /** \brief Initializes an empty zone damage snapshot. */
    void ServerSentryBaseDamageZone()
    {
        zoneName = "";
        healthDamage = 0;
    }
}

/**
    \brief Stores damage metadata without retaining engine-owned callback objects.
    \note Reported damage does not reconstruct exact health loss or infer raid success.
*/
class ServerSentryBaseDamageData
{
    /** \brief The same explosive provenance contract used by combat hit and kill events. */
    ref ServerSentryExplosiveAttribution explosive;
    /** \brief DayZ callback that supplied the snapshot. */
    string sourceHook;
    /** \brief Target object ID scoped to the event's server and boot session. */
    int targetId;
    /** \brief Damage type supplied by the engine. */
    int damageType;
    /** \brief Readable classification derived from the engine damage type. */
    string damageKind;
    /** \brief Fire-geometry component index supplied by the callback. */
    int component;
    /** \brief Damage zone supplied by the callback, which may be empty. */
    string damageZone;
    /** \brief Ammunition class supplied by the callback. */
    string ammoType;
    /** \brief Class of the source entity, when available. */
    string sourceClassName;
    /** \brief Method used to resolve the attacker, or unresolved when unknown. */
    string attackerResolution;
    /** \brief Health damage reported for the global damage zone. */
    float globalHealthDamage;
    /** \brief Highest health damage reported by the callback result. */
    float highestHealthDamage;
    /** \brief Hit position in the target's model coordinates. */
    ref ServerSentryVectorDto modelPosition;
    /** \brief Positive health damage entries for the target's named damage zones. */
    ref array<ref ServerSentryBaseDamageZone> zones;

    /** \brief Initializes metadata defaults and owned snapshot containers. */
    void ServerSentryBaseDamageData()
    {
        sourceHook = "BaseBuildingBase.EEHitBy";
        targetId = 0;
        damageType = 0;
        damageKind = "other";
        component = -1;
        damageZone = "";
        ammoType = "";
        sourceClassName = "";
        attackerResolution = "unresolved";
        globalHealthDamage = 0;
        highestHealthDamage = 0;
        modelPosition = new ServerSentryVectorDto();
        zones = new array<ref ServerSentryBaseDamageZone>();
    }
}
