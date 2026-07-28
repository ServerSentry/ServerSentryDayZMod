/**
    \file ServerSentryHitRequest.c
    \brief Defines the input captured for one combat hit callback.
*/

/**
    \brief Bundles the data required to process one combat hit.

    Enforce Script limits methods to 16 arguments. Keeping the callback data in
    one request object also prevents positional-argument mistakes and makes
    future combat fields additive instead of changing every method signature.
*/
class ServerSentryHitRequest
{
    /**
        \brief Stores the entity receiving the hit.
    */
    EntityAI victim;
    /**
        \brief Stores the per-victim combat deduplication state.
    */
    ServerSentryCombatState state;
    /**
        \brief Stores the engine damage result for the hit.
    */
    TotalDamageResult damageResult;
    /**
        \brief Stores the entity reported as the damage source.
    */
    EntityAI source;

    /**
        \brief Stores the DayZ damage type.
    */
    int damageType;
    /**
        \brief Stores the hit component index.
    */
    int component;
    /**
        \brief Stores the damage-zone name supplied by the engine.
    */
    string damageZone;
    /**
        \brief Stores the ammunition or damage class name.
    */
    string ammoType;

    /**
        \brief Indicates whether post-hit state was captured after super.EEHitBy.
    */
    bool hasPostHitState;
    /**
        \brief Indicates whether the target is alive after the hit was applied.
    */
    bool targetAliveAfter;
    /**
        \brief Stores the target health after the hit was applied.
    */
    float targetHealthAfter;
    /**
        \brief Indicates whether non-lethal hits may be logged.
    */
    bool logHits;
    /**
        \brief Indicates whether lethal hits may be logged.
    */
    bool logKills;

    /**
        \brief Stores an optional target classifier hint.
    */
    string targetTypeHint;
    /**
        \brief Stores the target Steam ID captured before the hit is applied.
    */
    string targetSteamId;
    /**
        \brief Stores the target Bohemia ID captured before the hit is applied.
    */
    string targetBohemiaId;
    /**
        \brief Stores the target display name captured before the hit is applied.
    */
    string targetName;

    /**
        \brief Initializes the engine-owned data captured by EEHitBy.
        \param hitVictim \p entity receiving the hit.
        \param combatState \p per-victim combat state.
        \param result \p engine damage result.
        \param hitDamageType \p DayZ damage type.
        \param hitSource \p entity reported as the damage source.
        \param hitComponent \p hit component index.
        \param hitDamageZone \p engine damage-zone name.
        \param hitAmmoType \p ammunition or damage class name.
    */
    void ServerSentryHitRequest(
        EntityAI hitVictim,
        ServerSentryCombatState combatState,
        TotalDamageResult result,
        int hitDamageType,
        EntityAI hitSource,
        int hitComponent,
        string hitDamageZone,
        string hitAmmoType)
    {
        victim = hitVictim;
        state = combatState;
        damageResult = result;
        damageType = hitDamageType;
        source = hitSource;
        component = hitComponent;
        damageZone = hitDamageZone;
        ammoType = hitAmmoType;
        hasPostHitState = false;
        targetAliveAfter = true;
        targetHealthAfter = 0;

        logHits = false;
        logKills = false;
        targetTypeHint = "";
        targetSteamId = "";
        targetBohemiaId = "";
        targetName = "";
    }

    /**
        \brief Captures the target state directly after super.EEHitBy.

        EEHitBy is already a post-damage callback. This snapshot is therefore
        the authoritative final state for classifying the callback as a hit or kill.
    */
    void CapturePostHitState()
    {
        if (!victim)
        {
            return;
        }

        targetAliveAfter = victim.IsAlive();
        targetHealthAfter = victim.GetHealth("", "Health");
        hasPostHitState = true;
    }

    /**
        \brief Configures which event severities are enabled for this target.
        \param shouldLogHits \p true to allow non-lethal hit events.
        \param shouldLogKills \p true to allow lethal hit events.
    */
    void ConfigureLogging(bool shouldLogHits, bool shouldLogKills)
    {
        logHits = shouldLogHits;
        logKills = shouldLogKills;
    }

    /**
        \brief Supplies a classifier hint for non-player target hooks.
        \param typeHint \p normalized target type such as zombie or animal.
    */
    void SetTargetTypeHint(string typeHint)
    {
        targetTypeHint = typeHint;
    }

    /**
        \brief Supplies the pre-hit player identity snapshot.
        \param steamId \p target Steam ID.
        \param bohemiaId \p target Bohemia ID.
        \param playerName \p target display name.
    */
    void SetTargetIdentity(string steamId, string bohemiaId, string playerName)
    {
        targetSteamId = steamId;
        targetBohemiaId = bohemiaId;
        targetName = playerName;
    }
};
