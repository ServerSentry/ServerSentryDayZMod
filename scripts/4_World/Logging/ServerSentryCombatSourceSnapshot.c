/**
    \file ServerSentryCombatSourceSnapshot.c
    \brief Captures a physical damage instrument without retaining engine entities.
*/
class ServerSentryCombatSourceSnapshot
{
    /**
        \brief Copies a verified item while the callback source is still available.
        \param source \p actual entity supplied by the engine damage callback.
        \param damageType \p native damage type; only immediate melee may use a player's held tool.
        \return \p owned item data or null when no physical instrument is established.
        \note Ranged callbacks naming only a player do not prove which gun fired.
    */
    static ServerSentryItem Capture(EntityAI source, int damageType)
    {
        if (!GetGame() || !GetGame().IsServer() || !source)
            return null;

        ItemBase item = ItemBase.Cast(source);
        if (!item && damageType == DT_CLOSE_COMBAT)
        {
            DayZPlayer meleeActor = DayZPlayer.Cast(source);
            if (meleeActor && meleeActor.GetHumanInventory())
                item = ItemBase.Cast(meleeActor.GetHumanInventory().GetEntityInHands());
        }
        if (!item)
            return null;

        ServerSentryItem snapshot = new ServerSentryItem();
        ServerSentryItemEventFactory.FillItem(snapshot, item);
        return snapshot;
    }
}
