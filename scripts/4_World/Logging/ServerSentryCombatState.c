/**
    \file ServerSentryCombatState.c
    \brief Provides logging support for Combat state.
*/

/**
    \brief Stores runtime state for Combat.
*/
class ServerSentryCombatState
{
    /**
        \brief Instance state for last hit log ms.
    */
    protected int m_LastHitLogMs;
    /**
        \brief Instance state for last hit key.
    */
    protected string m_LastHitKey;
    /**
        \brief Indicates whether the killing callback was already emitted.
    */
    protected bool m_KillLogged;

    /**
        \brief Initializes a new ServerSentryCombatState instance.
    */
    void ServerSentryCombatState()
    {
        m_LastHitLogMs = 0;
        m_LastHitKey = "";
        m_KillLogged = false;
    }

    /**
        \brief Returns whether log hit.
        \param ctx \p action context containing request, target, and input data.
        \return \p true when the condition is satisfied; otherwise false.

        Deduplication is limited to callbacks with the same attacker, damage
        metadata, and identical target health transition. Legitimate rapid fire
        therefore remains visible even when several bullets hit the same zone.
    */
    bool ShouldLogHit(ServerSentryHitContext ctx)
    {
        if (!ctx || !GetGame())
        {
            return false;
        }

        if (ctx.isLethal)
        {
            if (m_KillLogged)
            {
                return false;
            }

            m_KillLogged = true;
            return true;
        }

        // Once the entity is dead, ignore any additional corpse callbacks.
        if (m_KillLogged)
        {
            return false;
        }

        string hitKey = BuildHitDedupKey(ctx);
        int nowMs = GetGame().GetTime();

        if (m_LastHitKey == hitKey && (nowMs - m_LastHitLogMs) <= ServerSentryDefaults.HitDedupWindowMs())
        {
            return false;
        }

        m_LastHitKey = hitKey;
        m_LastHitLogMs = nowMs;
        return true;
    }

    /**
        \brief Builds hit dedup key.
        \param ctx \p action context containing request, target, and input data.
        \return \p The resulting string value.
    */
    protected string BuildHitDedupKey(ServerSentryHitContext ctx)
    {
        string attackerKey = ctx.attacker.steamId;

        if (attackerKey == "")
        {
            attackerKey = ctx.attacker.bohemiaId;
        }

        if (attackerKey == "")
        {
            attackerKey = ctx.attacker.className;
        }

        if (attackerKey == "")
        {
            attackerKey = ctx.attacker.name;
        }

        string hitKey = attackerKey;
        hitKey += "|";
        hitKey += ctx.ammoType;
        hitKey += "|";
        hitKey += ctx.hitZone;
        hitKey += "|";
        hitKey += ctx.component.ToString();
        hitKey += "|";
        hitKey += ctx.damageType.ToString();
        hitKey += "|";
        hitKey += ctx.healthDamage.ToString();
        hitKey += "|";
        hitKey += ctx.bloodDamage.ToString();
        hitKey += "|";
        hitKey += ctx.shockDamage.ToString();
        hitKey += "|";
        hitKey += ctx.targetHealthBefore.ToString();
        hitKey += "|";
        hitKey += ctx.targetHealthAfter.ToString();
        hitKey += "|";

        if (ctx.isLethal)
        {
            hitKey += "1";
        }
        else
        {
            hitKey += "0";
        }

        return hitKey;
    }
};
