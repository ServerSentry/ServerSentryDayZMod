/**
    \file ServerSentryCharBackupConfig.c
    \brief Defines automatic character backup settings.
*/

/** \brief Stores the per-player snapshot policy. */
class ServerSentryCharBackupConfig
{
    /** \brief Enables death-only captures and server-side restores. */
    bool EnableCharacterBackup = true;
    /** \brief Minimum character lifetime at death. */
    int MinCharacterLiveTimeSeconds = 3600;
    /** \brief Minimum number of inventory entities, excluding the player. */
    int MinItemsInInventory = 1;
    /** \brief Maximum committed snapshots per Steam ID; zero disables captures and clears snapshots. */
    int MaxBackupFiles = 5;
    /** \brief Legacy configuration field; ignored. No periodic backups are scheduled. */
    int BackupIntervalSeconds = 0;
    /** \brief Minimum time between restore attempts, clamped to at least five seconds. */
    int RestoreCooldownSeconds = 60;
}
