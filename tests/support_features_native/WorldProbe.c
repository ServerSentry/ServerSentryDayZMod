/** Copy only into the isolated test PBO's 4_World scripts. */
modded class ServerSentryServerManager
{
    bool ReviewHasTicketBridges()
    {
        return m_TicketLinkBridge || m_TicketCreationBridge || m_TicketConversationBridge || m_TicketNotifications;
    }
}
