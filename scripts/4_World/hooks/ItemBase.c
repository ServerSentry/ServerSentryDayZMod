/**
	\file ItemBase.c
	\brief Connects ServerSentry to DayZ or third-party hooks for Item base.
*/

/**
	\brief Extends the DayZ Item base class with ServerSentry integration.
*/
modded class ItemBase
{
    /**
    	\brief Processes the ee init operation.
    */
    override void EEInit()
    {
        super.EEInit();
        ServerSentryLiveApi.TryRegisterEntity(this);
    }

    /**
    	\brief Processes the ee delete operation.
    	\param parent \p parent used by the operation.
    */
    override void EEDelete(EntityAI parent)
    {
        ServerSentryLiveApi.UnregisterEntity(this);
        super.EEDelete(parent);
    }

    /**
    	\brief Processes the ee item location changed operation.
    	\param oldLoc \p old loc used by the operation.
    	\param newLoc \p new loc used by the operation.
    */
    override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        int ssMoveId = ServerSentryItemMoveScriptLog.NextMoveId();
        ServerSentryItemMoveSnapshot snapshot = null;

        if (ServerSentryConfigManager.Get().IsAllowedToLogItemMovement())
        {
            // Crash fix: capture only primitive data before super. Some inventory
            // transitions can invalidate the native item after super, especially
            // in hand/animation driven moves from other mods.
            snapshot = ServerSentryItemMoveScriptLog.CaptureSnapshot(ssMoveId, this, oldLoc, newLoc);
        }

        super.EEItemLocationChanged(oldLoc, newLoc);

        if (snapshot)
        {
            ServerSentryItemMoveScriptLog.EmitSnapshot(snapshot);
        }
    }
};
