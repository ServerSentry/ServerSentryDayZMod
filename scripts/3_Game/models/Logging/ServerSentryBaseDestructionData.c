/**
    \file ServerSentryBaseDestructionData.c
    \brief Defines a construction-part destruction observation.
*/

/**
    \brief Stores the health-level callback and observed construction-state transition.
    \note The callback supplies no damage source and does not establish raid success.
*/
class ServerSentryBaseDestructionData
{
    /** \brief Construction callback that supplied the observation. */
    string sourceHook;
    /** \brief Target object ID scoped to the event's server and boot session. */
    int targetId;
    /** \brief Construction-part name used to resolve the observed built state. */
    string partName;
    /** \brief Original damage-zone name supplied by the health-level callback. */
    string damageZone;
    /** \brief Previous health level supplied by the callback. */
    int oldLevel;
    /** \brief New health level supplied by the callback. */
    int newLevel;
    /** \brief Indicates that the part was built when the observation began. */
    bool wasBuiltBefore;
    /** \brief Built state confirmed after inherited handling; true until checked. */
    bool isBuiltAfter;
    /** \brief Explains why the callback cannot identify an attacker. */
    string attackerResolution;

    /** \brief Initializes a pending observation that still requires the postcondition. */
    void ServerSentryBaseDestructionData()
    {
        sourceHook = "Construction.OnHealthLevelChanged";
        targetId = 0;
        partName = "";
        damageZone = "";
        oldLevel = -1;
        newLevel = -1;
        wasBuiltBefore = true;
        isBuiltAfter = true;
        attackerResolution = "unavailable_in_health_callback";
    }
}
