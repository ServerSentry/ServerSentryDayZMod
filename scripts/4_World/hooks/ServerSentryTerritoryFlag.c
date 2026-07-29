/**
    \file ServerSentryTerritoryFlag.c
    \brief Exposes read-only Vanilla TerritoryFlag state to ServerSentry.
*/

/**
    \brief Adds read-only ServerSentry accessors to the Vanilla TerritoryFlag.
*/
modded class TerritoryFlag
{
    /**
        \brief Returns the normalized Vanilla flag refresher value.
        \return \p a value from zero to one, where one represents a fully raised flag.
    */
    float ServerSentryGetFlagHeight01()
    {
        return Math.Clamp(GetRefresherTime01(), 0.0, 1.0);
    }

    /**
        \brief Returns the remaining Vanilla flag refresher lifetime in seconds.
        \return \p remaining lifetime in seconds.
    */
    float ServerSentryGetFlagLifetimeRemaining()
    {
        if (m_RefresherTimeRemaining < 0)
        {
            return 0;
        }

        return m_RefresherTimeRemaining;
    }

    /**
        \brief Returns the maximum Vanilla flag refresher lifetime in seconds.
        \return \p maximum lifetime in seconds.
    */
    float ServerSentryGetFlagLifetimeMax()
    {
        if (m_FlagRefresherMaxDuration < 0)
        {
            return 0;
        }

        return m_FlagRefresherMaxDuration;
    }
}
