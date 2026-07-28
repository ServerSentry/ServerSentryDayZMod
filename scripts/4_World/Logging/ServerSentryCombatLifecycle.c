/**
    \file ServerSentryCombatLifecycle.c
    \brief Provides logging support for Combat lifecycle.
*/

/**
    \brief Tracks lifecycle events for Combat.
*/
class ServerSentryCombatLifecycle
{
    /**
        \brief Returns whether process hit.
        \param damageResult \p damage result used by the operation.
        \return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldProcessHit(TotalDamageResult damageResult)
    {
        return GetGame() && GetGame().IsDedicatedServer() && damageResult != null;
    }

    /**
        \brief Evaluates and emits one hit directly after the engine applied damage.
        \param request \p immutable callback data and logging policy.

        EEHitBy is already invoked after engine damage resolution. The target
        hook captures the final alive/health state and emits the same callback as
        combat_hit or combat_kill. A target may already report dead when the
        callback begins, especially for PlayerBase-derived AI.
    */
    static void LogHit(ServerSentryHitRequest request)
    {
        if (!request)
        {
            return;
        }

        if (!request.victim || !request.state || !ShouldProcessHit(request.damageResult))
        {
            return;
        }

        ServerSentryHitContext ctx = ServerSentryCombatFactory.CreateHitContext(request);
        if (!ctx)
        {
            return;
        }

        if (ctx.isLethal)
        {
            if (!request.logKills)
            {
                return;
            }
        }
        else if (!request.logHits)
        {
            return;
        }

        if (!ctx.isLethal && ServerSentryDefaults.SuppressZeroDamageHitEvents() && !HasResolvedDamage(ctx))
        {
            return;
        }

        if (!request.state.ShouldLogHit(ctx))
        {
            return;
        }

        ServerSentryLogger.Get().CombatHit(null, ctx);
    }

    /**
        \brief Returns whether resolved damage.
        \param ctx \p action context containing request, target, and input data.
        \return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool HasResolvedDamage(ServerSentryHitContext ctx)
    {
        return ctx.healthDamage > 0 || ctx.bloodDamage > 0 || ctx.shockDamage > 0;
    }
};
