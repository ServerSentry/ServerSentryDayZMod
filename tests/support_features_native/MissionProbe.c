/** Copy only into the isolated test PBO's 5_Mission scripts. */
modded class ServerSentryClientManager
{
    void ReviewApply(ServerSentryModFeatures features) { ApplyModFeatures(features); }
    bool ReviewHasTicketRuntime() { return m_TicketChat || m_TicketNotifications; }
    int ReviewBindingCount()
    {
        int count;
        CF_InputBinding binding = m_CF_Bindings.m_Head;
        while (binding)
        {
            count++;
            binding = binding.m_Next;
        }
        return count;
    }
}
