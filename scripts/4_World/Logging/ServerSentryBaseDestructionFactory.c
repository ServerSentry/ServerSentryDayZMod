/**
    \file ServerSentryBaseDestructionFactory.c
    \brief Copies base-part metadata before inherited construction code changes it.
*/

/** \brief Creates pending health-triggered destruction observations. */
class ServerSentryBaseDestructionFactory
{
    /**
        \brief Checks the server and the independent telemetry/destruction switches.
        \return \p true when base-part destruction observations are enabled.
    */
    static bool IsEnabled()
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        ServerSentryConfigManager config = ServerSentryConfigManager.Get();
        return config.IsAllowedToStartTelemetryLogging() && config.IsAllowedToLogBaseDestruction();
    }

    /**
        \brief Copies a built part's metadata for confirmation after the inherited callback.
        \param target \p base-building entity owning the part.
        \param part \p currently built script part; its reference is not stored in the event.
        \param damageZone \p original zone name supplied by the health callback.
        \param oldLevel \p previous engine health level, not a health-point value.
        \param newLevel \p resulting engine health level.
        \return \p pending snapshot, or null for a disabled or ineligible observation.
        \note The caller must confirm that this same part became unbuilt before emitting.
    */
    static ServerSentryBaseDestructionEvent CreateRuinedSnapshot(BaseBuildingBase target, ConstructionPart part, string damageZone, int oldLevel, int newLevel)
    {
        if (!IsEnabled() || !target || !part || !part.IsBuilt() || damageZone == "")
        {
            return null;
        }
        if (oldLevel == GameConstants.STATE_RUINED || newLevel != GameConstants.STATE_RUINED)
        {
            return null;
        }

        ServerSentryBaseDestructionEvent ev = new ServerSentryBaseDestructionEvent();
        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.occurredAtUtc = ServerSentryTime.NowUtc();
        ev.eventType = "basebuilding_part_destroyed";
        ev.category = "basebuilding";
        ev.severity = "info";
        ev.mod = "ServerSentry";
        ev.action.name = "basebuilding_part_destroyed";
        ev.action.displayName = "Base part destroyed";
        ev.action.result = "observed";
        ev.dataJson = "";

        // This callback supplies no source, attacker, ammunition or damage amount.
        ev.actor.type = "unknown";
        ev.actor.entityKind = "unknown";
        ev.actor.name = "Unknown source";

        vector targetPosition = target.GetPosition();
        ServerSentryEventFactory.FillTargetFromEntity(ev.target, target);
        ev.target.entityKind = "basebuilding";
        ev.position.FromVector(targetPosition);
        ev.targetPosition = new ServerSentryVectorDto();
        ev.targetPosition.FromVector(targetPosition);

        ServerSentryBaseDestructionData data = ev.data;
        data.targetId = target.GetID();
        data.partName = part.GetPartName();
        data.damageZone = damageZone;
        data.oldLevel = oldLevel;
        data.newLevel = newLevel;
        data.wasBuiltBefore = true;
        data.isBuiltAfter = true;

        ev.message = "Base part destroyed: " + ev.target.className + " / " + data.partName;
        return ev;
    }
}
