/**
    \file ServerSentryBackupConfigMigration.c
    \brief Retains the section migration helper using the shared config merge implementation.
*/

/** \brief Builds an in-memory backup-section upgrade without a separate file-writing path. */
class ServerSentryBackupConfigMigration
{
    /**
        \brief Adds missing backup settings while preserving existing JSON members.
        \param json \p original server configuration.
        \param config \p defaults for missing backup settings.
        \param updated \p receives the migrated configuration, or the unchanged source.
        \return \p true when a validated configuration changed.
    */
    static bool Build(string json, ServerSentryCharBackupConfig config, out string updated)
    {
        updated = json;
        ServerSentryLoggingConfig validated;
        string error;
        if (!JsonFileLoader<ServerSentryLoggingConfig>.LoadData(json, validated, error) || !validated)
            return false;

        string section;
        if (!JsonFileLoader<ServerSentryCharBackupConfig>.MakeData(config, section, error))
            return false;

        string defaults = "{\"CharBackupConfig\": " + section + "}";
        if (!ServerSentryConfigJson.MergeDefaults(json, defaults, updated))
        {
            updated = json;
            return false;
        }

        if (!JsonFileLoader<ServerSentryLoggingConfig>.LoadData(updated, validated, error) || !validated)
        {
            updated = json;
            return false;
        }
        return updated != json;
    }
}
