/**
	\file ServerSentryServerManager.c
	\brief Defines ServerSentry support for Server manager.
*/

[CF_RegisterModule(ServerSentryServerManager)]
/**
	\brief Coordinates the lifecycle of Server.
*/
class ServerSentryServerManager: CF_ModuleWorld
{
    /**
    	\brief Handles the DayZ Init callback.
    */
    override void OnInit()
    {
        super.OnInit();

        GetRPCManager().AddRPC("ServerSentryBot", "GetSupportCase",     this, SingleplayerExecutionType.Server);
        GetRPCManager().AddRPC("ServerSentryBot", "SpawnSupportCase",     this, SingleplayerExecutionType.Server);
    }

    /**
    	\brief Returns support case.
    	\param type \p type name or value type used by the operation.
    	\param ctx \p action context containing request, target, and input data.
    	\param sender \p identity of the RPC sender.
    	\param target \p target object associated with the operation.
    */
    void GetSupportCase(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender)
            return;

        ServerSentrySupportCase data = ServerSentrySupportCaseLoader.Load(sender.GetPlainId());

        GetRPCManager().SendRPC("ServerSentryBot", "SupportCaseResponse", new Param1<ServerSentrySupportCase>(data), true, sender);
    }

    /**
    	\brief Spawns support case.
    	\param type \p type name or value type used by the operation.
    	\param ctx \p action context containing request, target, and input data.
    	\param sender \p identity of the RPC sender.
    	\param target \p target object associated with the operation.
    */
    void SpawnSupportCase(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender)
            return;

        Print("[ServerSentry] -> Recived: SpawnSupportCase() from=" + sender.GetPlainId());

        ServerSentrySupportCase data = ServerSentrySupportCaseLoader.Load(sender.GetPlainId());
        if(!data || !data.Items) return;

        PlayerBase player = PlayerBase.Cast(sender.GetPlayer());

        if(!player) return; // maybe dead or currently re-creating.

        foreach (ServerSentrySupportCaseItem item: data.Items)
        {
            if (!item)
            {
                continue;
            }

            
            EntityAI entity = SpawnToPlayerOrGround(player, item.ClassName);
            if (!entity)
            {
                Print("[ServerSentry] -> Error while spawning item: " + item.ClassName + " maybe item does not exist?");
                continue;
            }

            AddAttachments(entity, item.Attachments);
        }

        ServerSentrySupportCaseLoader.Delete(sender.GetPlainId());

        //TODO Notify player its spawned!
    }

    /**
    	\brief Adds attachments.
    	\param item \p item or value processed by the operation.
    	\param attachments \p collection of attachments values.
    */
    protected void AddAttachments(EntityAI item, array<ref ServerSentrySupportCaseItem> attachments)
    {
        if(!item) return;

        foreach (ServerSentrySupportCaseItem attachment: attachments)
        {
            EntityAI entity = item.GetInventory().CreateAttachment(attachment.ClassName);
            if (!entity)
            {
                Print("[ServerSentry] -> Error while attaching attachment: " + attachment.ClassName + " maybe not attachable?");
            }
        }
    }

    /**
    	\brief Spawns to player or ground.
    	\param player \p player entity associated with the operation.
    	\param typeName \p DayZ class name of the entity or item.
    	\return \p The resulting entity ai instance, or null when unavailable.
    */
    protected EntityAI SpawnToPlayerOrGround(PlayerBase player, string typeName)
	{
        if (!player || typeName == "")
			return null;

        vector pos = player.GetPosition();

        EntityAI item = EntityAI.Cast(GetGame().CreateObjectEx(typeName, pos, ECE_SETUP | ECE_PLACE_ON_SURFACE));

        if (!item)
		{
			Print("[ServerSentry] -> CreateObjectEx failed for type=" + typeName);
			return null;
		}

		SetMaxQuantity(item);

        int invFlags = FindInventoryLocationType.ANY_CARGO | FindInventoryLocationType.ATTACHMENT;

        if (TryMoveToPlayerInventory(player, item, invFlags))
		{
			Print("[ServerSentry] Spawned " + typeName + " into player inventory");
			return item;
		}

        item.SetPosition(pos);
		item.PlaceOnSurface();
        Print("[ServerSentry] Inventory full, spawned " + typeName + " on ground");
		return item;
    }

    /**
    	\brief Attempts to move to player inventory.
    	\param player \p player entity associated with the operation.
    	\param item \p item or value processed by the operation.
    	\param flags \p DayZ inventory or operation flags.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected bool TryMoveToPlayerInventory(PlayerBase player, EntityAI item, int flags)
	{
        if (!player || !item)
			return false;

        if (player.GetInventory().CanAddEntityInto(item, flags))
		{
			if (player.GetInventory().TakeEntityToInventory(InventoryMode.SERVER, flags, item))
				return true;
		}

        item.GetInventory().FlipCargo();

		if (player.GetInventory().CanAddEntityInto(item, flags))
		{
			if (player.GetInventory().TakeEntityToInventory(InventoryMode.SERVER, flags, item))
				return true;
		}

		item.GetInventory().FlipCargo();
        return false;
    }

    /**
    	\brief Sets max quantity.
    	\param item \p item or value processed by the operation.
    */
    protected void SetMaxQuantity(EntityAI item)
	{
		ItemBase itemBase = ItemBase.Cast(item);
		if (!itemBase)
        {
            Magazine mag = Magazine.Cast(item);
            if (!mag) return;
            mag.ServerSetAmmoMax();
        }

		if (!itemBase.HasQuantity())
			return;

		itemBase.SetQuantityMax();
	}
};
