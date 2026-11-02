/** Client transport plus an optional additive overlay. Never receives the killer's identity. */
class ServerSentryDeathReports
{
    protected PlayerBase m_Player;
    protected bool m_Dead;
    protected bool m_AutomaticOverlay;
    protected bool m_Sending;
    protected bool m_Received;
    protected bool m_Confirming;
    protected int m_Sequence;
    protected int m_Attempts;
    protected int m_NextQuery;
    protected int m_Deadline;
    protected string m_KillEventId;
    protected string m_Message;
    protected ref ServerSentryDeathReportButton m_AutomaticButton;
    protected ref array<ref ServerSentryDeathReportButton> m_CustomButtons = {};

    static ServerSentryDeathReports Get()
    {
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (!manager) return null;
        return manager.GetDeathReports();
    }
    void Start(bool automaticOverlay)
    {
        m_AutomaticOverlay = automaticOverlay;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(Poll);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(Poll, 250, true);
    }
    void Stop()
    {
        if (GetGame()) GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(Poll);
        if (m_AutomaticButton) m_AutomaticButton.Detach();
        m_AutomaticButton = null;
        foreach (ServerSentryDeathReportButton custom : m_CustomButtons) custom.Detach();
        m_CustomButtons.Clear();
        Reset(null);
    }

    /** A custom death-screen owner can attach this component and detach it on close. */
    ServerSentryDeathReportButton AttachButton(Widget parent)
    {
        if (!parent) return null;
        ServerSentryDeathReportButton button = new ServerSentryDeathReportButton(this, parent);
        m_CustomButtons.Insert(button);
        return button;
    }

    bool CanShow() { return m_Dead && m_KillEventId != ""; }
    bool CanSubmit() { return CanShow() && !m_Sending && !m_Received; }
    bool IsSending() { return m_Sending; }
    bool IsReceived() { return m_Received; }
    bool IsConfirming() { return m_Confirming; }

    void AskToReport()
    {
        if (!CanSubmit()) return;
        m_Confirming = true;
        RefreshButtons();
    }
    void CancelReport()
    {
        m_Confirming = false;
        RefreshButtons();
    }
    string GetMessage() { return m_Message; }

    void Submit()
    {
        if (!CanSubmit()) return;
        m_Confirming = false;
        m_Sending = true;
        m_Message = "Submitting report...";
        m_Deadline = GetGame().GetTime() + 65000;
        GetRPCManager().SendRPC("ServerSentryBot", "SubmitDeathReport", new Param1<string>(m_KillEventId), true);
        RefreshButtons();
    }

    void ReceiveContext(int sequence, string eventId)
    {
        if (!m_Dead || sequence != m_Sequence || m_KillEventId != "") return;
        m_KillEventId = eventId;
        RefreshButtons();
    }
    void ReceiveReceipt(string eventId, bool accepted, string message)
    {
        if (!m_Dead || m_KillEventId == "" || eventId != m_KillEventId) return;
        m_Sending = false;
        m_Received = accepted;
        m_Message = message;
        RefreshButtons();
    }

    protected void Poll()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        bool dead = player && !player.IsAlive();
        if (player != m_Player || dead != m_Dead)
        {
            Reset(player);
            m_Dead = dead;
        }
        if (!m_Dead) return;
        int now = GetGame().GetTime();
        if (m_KillEventId == "" && m_Attempts < 3 && now >= m_NextQuery)
        {
            m_Attempts++;
            m_Sequence++;
            m_NextQuery = now + 2000;
            GetRPCManager().SendRPC("ServerSentryBot", "GetDeathReportContext", new Param1<int>(m_Sequence), true);
        }
        if (m_Sending && now >= m_Deadline)
        {
            m_Sending = false;
            m_Message = "Delivery is taking longer. Retry to check your report.";
        }
        if (CanShow() && m_AutomaticOverlay && !m_AutomaticButton)
            m_AutomaticButton = new ServerSentryDeathReportButton(this);
        // The native respawn/menu flow owns hiding the cursor. We never hide it.
        if (CanShow() && m_AutomaticOverlay) GetGame().GetUIManager().ShowCursor(true);
        RefreshButtons();
    }

    protected void Reset(PlayerBase player)
    {
        m_Player = player;
        m_Dead = false;
        m_Sequence++;
        m_Attempts = 0;
        m_NextQuery = 0;
        if (GetGame()) m_NextQuery = GetGame().GetTime() + 500;
        m_Sending = false;
        m_Received = false;
        m_Confirming = false;
        m_KillEventId = "";
        m_Message = "";
        if (m_AutomaticButton) m_AutomaticButton.Detach();
        m_AutomaticButton = null;
        RefreshButtons();
    }
    protected void RefreshButtons()
    {
        if (m_AutomaticButton) m_AutomaticButton.Refresh();
        for (int index = m_CustomButtons.Count() - 1; index >= 0; index--)
        {
            if (!m_CustomButtons[index].GetRoot()) m_CustomButtons.Remove(index);
            else m_CustomButtons[index].Refresh();
        }
    }
    void ~ServerSentryDeathReports() { Stop(); }
}
