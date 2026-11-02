class NotificationStateReview extends ServerSentryTicketNotifications
{
    int Count() { return m_OwnNotifications.Count(); }
    void Expire() { m_Deadline = GetGame().GetTime(); }
}

class NotificationQueueReview extends NotificationSystem
{
    static int VisibleTickets()
    {
        int count;
        foreach (NotificationRuntimeData data : m_Instance.m_TimeArray)
        {
            if (ServerSentryTicketNotificationRuntime.Cast(data)) count++;
        }
        return count;
    }
    static int VisibleCount() { return m_Instance.m_TimeArray.Count(); }
    static int DeferredCount() { return m_Instance.m_DeferredArray.Count(); }
    static void Reset()
    {
        foreach (NotificationRuntimeData data : m_Instance.m_TimeArray)
            m_Instance.m_OnNotificationRemoved.Invoke(data);
        m_Instance.m_TimeArray.Clear();
        m_Instance.m_DeferredArray.Clear();
    }
    static void Fill()
    {
        for (int i = 0; i < 5; i++) AddNotificationExtended(1, "Queue fixture");
    }
    static float TicketDuration()
    {
        foreach (NotificationRuntimeData data : m_Instance.m_TimeArray)
        {
            if (ServerSentryTicketNotificationRuntime.Cast(data)) return data.GetTime();
        }
        return 0;
    }
}

class NotificationInboxReview extends ServerSentryTicketNotificationBridge
{
    ref array<ref ServerSentryTicketNotification> delivered = {};
    override protected void Deliver(ServerSentryTicketNotification notification, array<Man> players) { delivered.Insert(notification); }
    void RunPoll() { Poll(); }
}

class TicketNotificationsReviewMission extends MissionGameplay
{
    protected int m_Failures;
    protected ref NotificationStateReview m_State;
    protected int m_Removed;

    void Removed(NotificationRuntimeData data)
    {
        if (ServerSentryTicketNotificationRuntime.Cast(data)) m_Removed++;
    }

    override void OnInit()
    {
        super.OnInit();
        vector position = "7500 0 7500";
        position[1] = GetGame().SurfaceY(position[0], position[2]) + 0.2;
        PlayerBase player = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Mirek", position, 0, "NONE"));
        if (player) GetGame().SelectPlayer(null, player);
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Run, 2000, false);
    }

    void Check(bool passed, string name)
    {
        if (!passed) m_Failures++;
        Print("[TicketNotificationsReview] " + passed + " " + name);
    }

    ServerSentryTicketNotification Hint(string messageId)
    {
        ServerSentryTicketNotification hint = new ServerSentryTicketNotification;
        hint.messageId = messageId;
        hint.steamId64 = "offline-fixture";
        hint.expiresAtUtc = "2099-01-01T00:00:00Z";
        hint.ticket = new ServerSentryTicketSummary;
        hint.ticket.ticketId = "offline-support-ticket";
        hint.ticket.ticketNumber = "12";
        hint.ticket.ticketTypeName = "Notification review";
        return hint;
    }

    void Run()
    {
        ServerSentryModFeatures features = new ServerSentryModFeatures;
        features.tickets = true;
        features.supportCases = true;
        ServerSentryClientManager.Get().ApplyModFeatures(features);
        NotificationQueueReview.Reset();
        m_State = new NotificationStateReview;
        ServerSentryTicketNotification hint = Hint("101");
        m_State.Receive(hint, false);
        Check(m_State.GetTicket() == hint.ticket, "visible notification targets the exact ticket");
        Check(NotificationQueueReview.TicketDuration() == 10, "native notification display duration is ten seconds");
        Check(InputUtils.GetButtonNameFromInput("UAOpenSupportNotification", EUAINPUT_DEVICE_KEYBOARDMOUSE) != "", "shortcut has a configurable keyboard binding");
        m_State.Receive(hint, false);
        Check(m_State.Count() == 1, "duplicate message does not create another popup");
        NotificationSystem.AddNotificationExtended(60, "Unrelated notification");
        NotificationSystem.GetInstance().m_OnNotificationRemoved.Insert(Removed);
        m_State.Consume();
        Check(NotificationQueueReview.VisibleTickets() == 0 && m_State.Count() == 0, "opening removes its popup from the native queue and owned state");
        Check(m_Removed == 1 && NotificationQueueReview.VisibleCount() == 1, "opening emits one UI removal without removing unrelated notifications");
        m_State.Consume();
        Check(m_Removed == 1, "repeated consume does not emit duplicate UI removals");
        m_State.Receive(Hint("expiry"), false);
        m_State.Expire();
        Check(!m_State.GetTicket(), "shortcut expires at its deadline");
        m_State.Stop();
        Check(NotificationQueueReview.VisibleTickets() == 0, "stopping removes owned visible notifications");
        NotificationQueueReview.Reset();

        NotificationQueueReview.Fill();
        m_State.Receive(Hint("stopped-queue"), false);
        m_State.Stop();
        Check(NotificationQueueReview.DeferredCount() == 0 && NotificationQueueReview.VisibleCount() == 5, "stopping removes owned queued notifications and preserves unrelated popups");
        m_State.Receive(Hint("102"), false);
        Check(!m_State.GetTicket(), "queued notification has no active shortcut yet");
        NotificationSystem.Update(1);
        NotificationSystem.Update(1);
        Check(m_State.GetTicket() != null, "shortcut activates when the native queue shows the notification");
        for (int extra = 0; extra < 4; extra++) NotificationSystem.AddNotificationExtended(60, "Unrelated queued notification");
        NotificationSystem.AddNotificationExtended(60, "Deferred unrelated notification");
        Check(NotificationQueueReview.DeferredCount() == 1, "native queue is full before opening the ticket");
        m_State.Consume();
        Check(!m_State.GetTicket(), "opening consumes the notification shortcut");
        Check(NotificationQueueReview.VisibleTickets() == 0 && NotificationQueueReview.VisibleCount() == 5 && NotificationQueueReview.DeferredCount() == 0, "dismissal frees a native slot and displays the deferred notification");
        m_State.Stop();
        NotificationQueueReview.Reset();
        m_State.Receive(Hint("103"), true);
        Check(m_State.Count() == 0 && !m_State.GetTicket(), "already-open ticket is not announced again");
        m_State.Stop();
        CheckInbox();

        NotificationQueueReview.Reset();
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        manager.ReceiveTicketNotification(Hint("104"));
        Check(manager.GetTicketChat().IsReading(), "notification starts preloading the conversation before pressing the shortcut");
        manager.OpenNotifiedTicket();
        ServerSentryTicketConversationMenu conversation = ServerSentryTicketConversationMenu.Cast(GetGame().GetUIManager().GetMenu());
        Check(conversation && conversation.IsTicket("offline-support-ticket"), "actual shortcut handler opens the complete notified conversation");
        Check(conversation && ServerSentryTicketsMenu.Cast(conversation.GetParentMenu()), "direct opening retains the Tickets sidebar root");
        Check(!manager.IsCheckingTicketLink(), "direct shortcut does not request the hidden ticket list");
        Check(NotificationQueueReview.VisibleTickets() == 0, "actual shortcut handler removes the ticket popup");
        CheckAssets();
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Finish, 200, false);
    }

    void CheckAssets()
    {
        Check(ServerSentryEmojiCatalog.GetChoices().Count() == 3953, "packed emoji catalog contains all standard picker entries");
        Widget root = GetGame().GetWorkspace().CreateWidgets("gui/layouts/new_ui/notifications/notification_element.layout");
        ImageWidget image = ImageWidget.Cast(root.FindAnyWidget("Image"));
        TStringArray icons = {"serversentry_logo", "back", "refresh", "discord", "close", "ticket", "reimbursement", "smile", "switch_thumb", "support"};
        foreach (string icon : icons)
        {
            string path = "ServerSentryDayZMod/gui/skin/" + icon + ".edds";
            Check(image.LoadImageFile(0, path, true), "packed UI texture loads: " + icon);
            int width, height;
            image.GetImageSize(0, width, height);
            Check(width > 0 && height > 0, "packed UI texture has dimensions: " + icon);
            Print("[TicketNotificationsReview] Texture " + icon + " " + width + "x" + height);
        }
        for (int atlas = 0; atlas < 6; atlas++)
        {
            Check(LoadWidgetImageSet("ServerSentryDayZMod/gui/emoji/ServerSentryEmoji" + atlas.ToString() + ".imageset"), "packed emoji imageset loads: " + atlas);
            Check(image.LoadImageFile(0, "ServerSentryDayZMod/gui/emoji/emoji" + atlas.ToString() + "_ca.paa", true), "packed emoji atlas loads: " + atlas);
        }
        root.Unlink();
    }

    void CheckInbox()
    {
        MakeDirectory("$profile:ServerSentry");
        MakeDirectory("$profile:ServerSentry/tickets");
        MakeDirectory("$profile:ServerSentry/tickets/notifications");
        ServerSentryTicketNotification fresh = Hint("201");
        ServerSentryTicketNotification expired = Hint("202");
        expired.expiresAtUtc = "2000-01-01T00:00:00Z";
        Check(fresh.IsFresh() && !expired.IsFresh(), "inbox rejects expired timestamps");
        string error;
        JsonFileLoader<ServerSentryTicketNotification>.SaveFile("$profile:ServerSentry/tickets/notifications/201.json", fresh, error);
        JsonFileLoader<ServerSentryTicketNotification>.SaveFile("$profile:ServerSentry/tickets/notifications/202.json", expired, error);
        NotificationInboxReview inbox = new NotificationInboxReview;
        inbox.RunPoll();
        Check(inbox.delivered.Count() == 1 && inbox.delivered[0].messageId == "201", "inbox loads only complete fresh notification files");
        Check(!FileExist("$profile:ServerSentry/tickets/notifications/201.json") && !FileExist("$profile:ServerSentry/tickets/notifications/202.json"), "inbox consumes delivered and expired files");
        inbox.RunPoll();
        Check(inbox.delivered.Count() == 1, "consumed notification is not sent again");
    }

    void Finish()
    {
        Check(!ServerSentryClientManager.Get().IsCheckingTicketLink(), "hidden ticket list stays unloaded after normal UI updates");
        UIScriptedMenu conversation = GetGame().GetUIManager().GetMenu();
        if (conversation) conversation.Close();
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(CheckVisibleList, 200, false);
    }

    void CheckVisibleList()
    {
        Check(ServerSentryClientManager.Get().IsCheckingTicketLink(), "ticket list loads normally after returning to its visible screen");
        Print("[TicketNotificationsReview] Finished failures=" + m_Failures);
        Print("[TicketNotificationsReview] Offline fixture only; no Discord messages or real ticket writes");
    }
}

void main() {}
Mission CreateCustomMission(string path) { return new TicketNotificationsReviewMission; }
