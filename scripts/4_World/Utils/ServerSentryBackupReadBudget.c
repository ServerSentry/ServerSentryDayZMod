/** Bounded synchronous work; native calls themselves cannot be interrupted from script. */
class ServerSentryBackupReadBudget
{
    static const int MAX_CARTRIDGES = 8192;
    static const float MAX_MILLISECONDS = 100;
    protected float m_Started;
    protected int m_Cartridges;

    void ServerSentryBackupReadBudget() { m_Started = GetGame().GetTickTime(); }
    float ElapsedMs() { return Math.Max(0, (GetGame().GetTickTime() - m_Started) * 1000); }
    bool HasTime() { return ElapsedMs() < MAX_MILLISECONDS; }

    bool Reserve(int count)
    {
        if (count < 0 || count > MAX_CARTRIDGES - m_Cartridges || !HasTime()) return false;
        m_Cartridges += count;
        return true;
    }
}
