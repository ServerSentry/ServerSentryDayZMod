/**
    \file ServerSentryConfigMigration.c
    \brief Upgrades complete server configurations without discarding administrator settings.
*/

/** \brief Owns config schema upgrades and verified persistence at startup. */
class ServerSentryConfigMigration
{
    /** \brief Current persisted schema; increment when changing configuration semantics. */
    static const int CURRENT_VERSION = 1;
    /** \brief Maximum accepted config size; oversized files are never rewritten. */
    protected static const int MAX_CONFIG_BYTES = 1048576;

    /**
        \brief Loads and migrates the same source text so edits cannot mix two config generations.
        \param path \p server configuration path.
        \param config \p receives effective defaults with existing overrides.
        \param error \p receives a load error; migration write failures are logged separately.
        \return \p true when the configuration can be used, even if migration persistence fails.
    */
    static bool LoadFile(string path, out ServerSentryLoggingConfig config, out string error)
    {
        string original;
        if (!ReadText(path, original))
        {
            error = "Cannot read a complete config within the one MiB size limit.";
            return false;
        }

        if (!JsonFileLoader<ServerSentryLoggingConfig>.LoadData(original, config, error))
            return false;

        if (!config)
        {
            error = "The config must be a JSON object.";
            return false;
        }

        if (!config.CharBackupConfig)
            config.CharBackupConfig = new ServerSentryCharBackupConfig();

        UpgradeFile(path, original, config);
        return true;
    }

    /**
        \brief Adds missing defaults and saves only a changed, supported configuration.
        \param path \p path of the successfully loaded configuration.
        \param original \p exact source text used to load the configuration.
        \param config \p effective settings; version advances after a verified save.
    */
    protected static void UpgradeFile(string path, string original, ServerSentryLoggingConfig config)
    {
        if (config.ConfigVersion > CURRENT_VERSION || config.ConfigVersion < 0)
        {
            Print("[ServerSentry][Config] Unsupported config version " + config.ConfigVersion + "; file left unchanged.");
            return;
        }

        ServerSentryLoggingConfig defaults = new ServerSentryLoggingConfig();
        defaults.ConfigVersion = CURRENT_VERSION;
        string defaultJson;
        string error;
        if (!JsonFileLoader<ServerSentryLoggingConfig>.MakeData(defaults, defaultJson, error))
        {
            Print("[ServerSentry][Config] Cannot serialize defaults: " + error);
            return;
        }

        string updated;
        if (!ServerSentryConfigJson.MergeDefaults(original, defaultJson, updated, "ConfigVersion"))
        {
            Print("[ServerSentry][Config] Cannot merge config properties; file left unchanged.");
            return;
        }

        if (updated == original)
            return;

        ServerSentryLoggingConfig validated;
        if (!JsonFileLoader<ServerSentryLoggingConfig>.LoadData(updated, validated, error) || !validated)
        {
            Print("[ServerSentry][Config] Cannot validate migrated config; file left unchanged: " + error);
            return;
        }

        int previousVersion = config.ConfigVersion;
        string backup = path + ".before-v" + previousVersion.ToString();
        if (!SaveUpgrade(path, original, updated, backup))
            return;

        config.ConfigVersion = CURRENT_VERSION;
        Print("[ServerSentry][Config] Migrated config version " + previousVersion + " -> " + CURRENT_VERSION + "; missing defaults saved. Backup: " + backup);
    }

    /**
        \brief Stages and verifies a migration before replacing the original file.
        \param path \p active configuration path.
        \param original \p exact original text to preserve.
        \param updated \p validated migrated text.
        \param backup \p recovery copy of the previous configuration.
        \return \p true after the active file matches the migration.
        \note Native CopyFile is not an atomic rename; a verified backup is retained.
    */
    protected static bool SaveUpgrade(string path, string original, string updated, string backup)
    {
        string temporary = path + ".migration.tmp";
        if (!WriteText(temporary, updated))
        {
            Print("[ServerSentry][Config] Cannot verify staged config; original file left unchanged.");
            return false;
        }

        // Refuse to overwrite an administrator's edit made after the initial read.
        if (!MatchesText(path, original))
        {
            DeleteFile(temporary);
            Print("[ServerSentry][Config] Config changed while migrating; retry on the next start.");
            return false;
        }

        if (!CopyFile(path, backup) || !MatchesText(backup, original))
        {
            DeleteFile(temporary);
            Print("[ServerSentry][Config] Cannot preserve config backup; migration skipped.");
            return false;
        }

        bool saved = CopyFile(temporary, path);
        saved = saved && MatchesText(path, updated);
        DeleteFile(temporary);
        if (saved)
            return true;

        bool restored = CopyFile(backup, path);
        restored = restored && MatchesText(path, original);
        if (restored)
            Print("[ServerSentry][Config] Migration write failed; original config restored.");
        else
            Print("[ServerSentry][Config] Migration write failed; restore the preserved config from " + backup);

        return false;
    }

    /**
        \brief Reads a bounded config without accepting truncated input.
        \param path \p file to read.
        \param text \p receives the file contents.
        \return \p true for a non-empty file within the supported size.
    */
    protected static bool ReadText(string path, out string text)
    {
        FileHandle file = OpenFile(path, FileMode.READ);
        if (!file)
            return false;

        int length = ReadFile(file, text, MAX_CONFIG_BYTES + 1);
        CloseFile(file);
        return length > 0 && length <= MAX_CONFIG_BYTES;
    }

    /**
        \brief Checks that native file contents match the expected text.
        \param path \p file to verify.
        \param expected \p complete expected contents.
        \return \p true for an exact match.
    */
    protected static bool MatchesText(string path, string expected)
    {
        string actual;
        return ReadText(path, actual) && actual == expected;
    }

    /**
        \brief Writes and verifies a temporary configuration file.
        \param path \p staging path.
        \param text \p validated migrated JSON.
        \return \p true after complete readback.
    */
    protected static bool WriteText(string path, string text)
    {
        FileHandle file = OpenFile(path, FileMode.WRITE);
        if (!file)
            return false;

        FPrint(file, text);
        CloseFile(file);
        if (MatchesText(path, text))
            return true;

        DeleteFile(path);
        return false;
    }
}
