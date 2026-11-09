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
    protected ref ServerSentryDeathReportBridge m_DeathReportBridge;
    protected ref ServerSentryTicketLinkBridge m_TicketLinkBridge;
    protected ref ServerSentryTicketCreationBridge m_TicketCreationBridge;
    protected ref ServerSentryTicketConversationBridge m_TicketConversationBridge;
    protected ref ServerSentryTicketNotificationBridge m_TicketNotifications;

    /**
    	\brief Handles the DayZ Init callback.
    */
    override void OnInit()
    {
        super.OnInit();

        EnableMissionFinish();
        if (!GetGame().IsServer()) return;
        if (ServerSentryConfigManager.Get().IsDeathReportingEnabled())
        {
            m_DeathReportBridge = new ServerSentryDeathReportBridge;
            GetRPCManager().AddRPC("ServerSentryBot", "GetDeathReportContext", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("ServerSentryBot", "SubmitDeathReport", this, SingleplayerExecutionType.Server);
        }
        if (ServerSentryConfigManager.Get().IsTicketsEnabled())
        {
            m_TicketLinkBridge = new ServerSentryTicketLinkBridge;
            m_TicketCreationBridge = new ServerSentryTicketCreationBridge;
            m_TicketConversationBridge = new ServerSentryTicketConversationBridge;
            m_TicketNotifications = new ServerSentryTicketNotificationBridge;
            m_TicketNotifications.Start();
            GetRPCManager().AddRPC("ServerSentryBot", "TicketConversation", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("ServerSentryBot", "CancelTicketConversation", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("ServerSentryBot", "GetTicketLinkStatus", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("ServerSentryBot", "CreateTicket", this, SingleplayerExecutionType.Server);
        }
        if (ServerSentryConfigManager.Get().IsSupportCasesEnabled())
        {
            GetRPCManager().AddRPC("ServerSentryBot", "GetSupportState", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("ServerSentryBot", "SpawnSupportCase", this, SingleplayerExecutionType.Server);
        }
    }

    /**
        \brief Returns the current support case with the request token.
    	\param type \p type name or value type used by the operation.
    	\param ctx \p action context containing request, target, and input data.
    	\param sender \p identity of the RPC sender.
    	\param target \p target object associated with the operation.
    */
    void GetSupportState(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !ServerSentryConfigManager.Get().IsSupportCasesEnabled())
            return;

        Param1<int> request;
        if (!ctx.Read(request) || !request)
            return;

        ServerSentrySupportCase data = ServerSentrySupportCaseLoader.Load(sender.GetPlainId());
        GetRPCManager().SendRPC("ServerSentryBot", "SupportStateResponse", new Param2<int, ServerSentrySupportCase>(request.param1, data), true, sender);
    }

    override void OnMissionFinish(Class sender, CF_EventArgs args)
    {
        super.OnMissionFinish(sender, args);
        ServerSentryDeathReportStore.Clear();
        if (m_DeathReportBridge) m_DeathReportBridge.Stop();
        if (m_TicketLinkBridge) m_TicketLinkBridge.Stop();
        if (m_TicketCreationBridge) m_TicketCreationBridge.Stop();
        if (m_TicketConversationBridge) m_TicketConversationBridge.Stop();
        if (m_TicketNotifications) m_TicketNotifications.Stop();
    }

    void GetDeathReportContext(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !m_DeathReportBridge || !ServerSentryConfigManager.Get().IsDeathReportingEnabled()) return;
        Param1<int> request;
        if (!ctx.Read(request) || !request) return;
        string eventId = "";
        ServerSentryDeathReportCommand command = ServerSentryDeathReportStore.Current(sender.GetPlainId());
        if (command) eventId = command.killEventId;
        GetRPCManager().SendRPC("ServerSentryBot", "DeathReportContext", new Param2<int, string>(request.param1, eventId), true, sender);
    }

    void SubmitDeathReport(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !m_DeathReportBridge || !ServerSentryConfigManager.Get().IsDeathReportingEnabled()) return;
        Param1<string> request;
        if (ctx.Read(request) && request) m_DeathReportBridge.Request(sender, request.param1);
    }

    void GetTicketLinkStatus(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !m_TicketLinkBridge || !ServerSentryConfigManager.Get().IsTicketsEnabled())
            return;

        Param1<ServerSentryTicketLinkQuery> request;
        if (ctx.Read(request) && request)
        {
            if (request.param1) m_TicketLinkBridge.Request(sender, request.param1);
        }
    }

    void CreateTicket(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !m_TicketCreationBridge || !ServerSentryConfigManager.Get().IsTicketsEnabled())
            return;
        Param1<ServerSentryTicketSubmission> request;
        if (ctx.Read(request) && request && request.param1)
            m_TicketCreationBridge.Request(sender, request.param1);
    }

    void TicketConversation(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !m_TicketConversationBridge || !ServerSentryConfigManager.Get().IsTicketsEnabled()) return;
        Param1<ServerSentryTicketConversationRequest> request;
        if (ctx.Read(request) && request && request.param1)
            m_TicketConversationBridge.Request(sender, request.param1);
    }

    void CancelTicketConversation(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !m_TicketConversationBridge) return;
        Param1<string> request;
        if (ctx.Read(request) && request && ServerSentryTicketSubmission.IsValidRequestId(request.param1))
            m_TicketConversationBridge.CancelRead(sender, request.param1);
    }

    void ForgetPlayerTicketConversations(PlayerIdentity identity)
    {
        if (identity && m_TicketConversationBridge) m_TicketConversationBridge.ForgetPlayer(identity);
    }

    void SpawnSupportCase(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server || !sender || !ServerSentryConfigManager.Get().IsSupportCasesEnabled())
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
