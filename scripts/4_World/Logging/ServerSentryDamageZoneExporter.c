/** \file ServerSentryDamageZoneExporter.c
    \brief Exports effective server-side damage zones once per encountered entity class.
*/

/** \brief Describes one effective damage zone and its configured model selections. */
class ServerSentryDamageZoneDto
{
    /** \brief Native damage-zone name. */
    string name;
    /** \brief Bounded configured model-component names for this zone. */
    ref array<string> components = new array<string>();
}

/** \brief Carries class-level diagnostics without player identities or positions. */
class ServerSentryDamageZoneExportDto
{
    /** \brief Diagnostic format version. */
    int version = 1;
    /** \brief Entity configuration class inspected in the running game. */
    string className;
    /** \brief Server game build string. */
    string gameVersion;
    /** \brief UTC capture time. */
    string capturedAtUtc;
    /** \brief Bounded Mod names and versions reported by the game; not a load-order guarantee. */
    ref array<string> reportedMods;
    /** \brief Whether the bounded export omitted zones or components. */
    bool truncated;
    /** \brief Effective native damage zones with configured selections. */
    ref array<ref ServerSentryDamageZoneDto> zones = new array<ref ServerSentryDamageZoneDto>();
}

/** \brief Writes bounded local diagnostic files without spawning or damaging entities. */
class ServerSentryDamageZoneExporter
{
    /** \brief Classes already attempted in this mission, including failed writes. */
    protected static ref set<string> s_Seen;
    /** \brief Cached game-reported Mod metadata for this mission. */
    protected static ref array<string> s_Mods;
    /** \brief Local diagnostics destination under the configured server profile. */
    static const string DIRECTORY = "$profile:ServerSentry/diagnostics/damage-zones";

    /** \brief Clears the per-mission observation bound. */
    static void Reset()
    {
        s_Seen = new set<string>();
        s_Mods = null;
    }

    /** \brief Validates native class identifiers before using them as filenames.
        \param name \p configuration classname to check.
        \return \p True for a bounded identifier without path separators.
    */
    protected static bool ValidClassName(string name)
    {
        if (name == "" || name.Length() > 160)
            return false;
        string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
        for (int i = 0; i < name.Length(); i++)
        {
            string character = name.Substring(i, 1);
            if (!allowed.Contains(character))
                return false;
        }
        return true;
    }

    /** \brief Captures at most 64 native zones and 64 configured components per zone.
        \param entity \p existing server entity to inspect without modification.
        \return \p Effective class-level diagnostic data.
    */
    static ServerSentryDamageZoneExportDto Capture(EntityAI entity)
    {
        ServerSentryDamageZoneExportDto data = new ServerSentryDamageZoneExportDto();
        data.className = entity.GetType();
        data.capturedAtUtc = ServerSentryTime.NowUtc();
        GetGame().GetVersion(data.gameVersion);
        if (!s_Mods)
        {
            s_Mods = new array<string>();
            array<ref ModInfo> infos = new array<ref ModInfo>();
            GetGame().GetModInfos(infos);
            for (int modIndex = 0; modIndex < infos.Count() && modIndex < 256; modIndex++)
            {
                ModInfo info = infos.Get(modIndex);
                if (info)
                {
                    string modDescription = info.GetName() + " @ " + info.GetVersion();
                    s_Mods.Insert(modDescription);
                }
            }
        }
        data.reportedMods = s_Mods;
        array<string> names = new array<string>();
        entity.GetDamageZones(names);
        for (int i = 0; i < names.Count(); i++)
        {
            if (data.zones.Count() >= 64)
            {
                data.truncated = true;
                break;
            }
            ServerSentryDamageZoneDto zone = new ServerSentryDamageZoneDto();
            zone.name = names.Get(i);
            string configPath = "CfgVehicles " + data.className + " DamageSystem DamageZones " + zone.name + " componentNames";
            array<string> components = new array<string>();
            if (GetGame().ConfigIsExisting(configPath))
                GetGame().ConfigGetTextArray(configPath, components);
            for (int j = 0; j < components.Count(); j++)
            {
                if (zone.components.Count() >= 64)
                {
                    data.truncated = true;
                    break;
                }
                zone.components.Insert(components.Get(j));
            }
            data.zones.Insert(zone);
        }
        return data;
    }

    /** \brief Exports one existing entity class at most once, bounded to 128 classes per mission.
        \param entity \p connected player or observed combat target; never spawned by this exporter.
    */
    static void Observe(EntityAI entity)
    {
        if (!GetGame() || !GetGame().IsServer() || !entity)
            return;
        if (!s_Seen)
            Reset();
        string className = entity.GetType();
        if (!ValidClassName(className) || s_Seen.Find(className) >= 0 || s_Seen.Count() >= 128)
            return;
        s_Seen.Insert(className);
        ServerSentryDamageZoneExportDto data = Capture(entity);
        MakeDirectory("$profile:ServerSentry");
        MakeDirectory("$profile:ServerSentry/diagnostics");
        MakeDirectory(DIRECTORY);
        string filename = DIRECTORY + "/" + className + ".json";
        string error;
        if (!JsonFileLoader<ServerSentryDamageZoneExportDto>.SaveFile(filename, data, error))
            Print("[ServerSentry] Damage-zone export could not be saved for " + className);
    }
}
