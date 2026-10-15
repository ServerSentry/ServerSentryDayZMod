/**
    \file ServerSentryItemLifecycle.c
    \brief Records observable runtime item deletions without changing gameplay.
*/
class ServerSentryItemLifecycle
{
    /** \brief True only between mission start and mission finish. */
    protected static bool s_Active;

    /** \brief Enables observations after startup. */
    static void Start()
    {
        s_Active = true;
    }

    /** \brief Disables observations before shutdown removes loaded entities. */
    static void Stop()
    {
        s_Active = false;
    }

    /**
        \brief Records a deletion before native entity invalidation; skips known unload cases.
        \param item \p item being removed from the runtime world.
        \param parent \p parent supplied by the deletion callback.
    */
    static void ObserveDeletion(ItemBase item, EntityAI parent)
    {
        if (!s_Active || !GetGame() || !GetGame().IsServer() || !item)
            return;
        if (!ServerSentryDefaults.LogItemMoveEvents() || !ServerSentryConfigManager.Get().IsAllowedToStartTelemetryLogging() || !ServerSentryConfigManager.Get().IsAllowedToLogItemMovement())
            return;

        PlayerBase owner = PlayerBase.Cast(item.GetHierarchyRootPlayer());
        if (owner && (!owner.GetIdentity() || owner.IsPlayerDisconnected()))
            return;
        Mission mission = GetGame().GetMission();
        if (owner && mission && mission.IsPlayerDisconnecting(owner))
            return;

        string identity = ServerSentryItemMoveScriptLog.ResolveItemPersistentId(item);
        if (identity == "")
            return;

        // Reuse the typed inventory-event envelope, which the logger already serializes fully.
        ServerSentryItemMoveEvent ev = new ServerSentryItemMoveEvent();
        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.eventType = "item_deleted";
        ev.category = "item";
        ev.severity = "info";
        ev.mod = "ServerSentry";
        ev.action.name = "item_delete";
        ev.action.result = "success";
        ServerSentryItemEventFactory.FillItem(ev.item, item);
        ServerSentryEntityClassifier.FillEnvironment(ev.actor, "world");
        ev.target.type = "item";
        ev.target.className = item.GetType();
        ev.target.name = item.GetDisplayName();
        ev.position.FromVector(item.GetPosition());
        ev.data.sourceHook = "ItemBase.EEDelete";
        ev.data.itemPersistentId = identity;
        ev.data.itemClassName = ev.item.className;
        ev.data.itemDisplayName = ev.item.displayName;
        ev.data.runtimeItemId = item.GetID();
        ev.data.relation = "deleted";
        ev.data.from.type = "ground";
        ev.data.from.position.FromVector(item.GetPosition());
        ev.data.fromLabel = "Ground";
        if (parent)
        {
            ev.data.from.type = "storage";
            ev.data.from.parentClassName = parent.GetType();
            ev.data.from.parentName = parent.GetDisplayName();
            ev.data.fromLabel = parent.GetDisplayName();
        }
        if (owner)
        {
            ev.data.from.parentOwnerType = "player";
            ev.data.from.parentOwnerName = owner.GetIdentity().GetName();
            ev.data.from.parentOwnerSteamId = owner.GetIdentity().GetPlainId();
            ev.data.fromLabel = ev.data.from.parentOwnerName + " inventory";
        }
        ev.data.to.type = "deleted";
        ev.data.toLabel = "Removed from world";
        ev.message = "Item deletion observed: " + ev.item.displayName;
        ServerSentryLogger.Get().Emit(ev);
    }
}
