/**
    \file ServerSentryBaseDamageFactory.c
    \brief Converts base-building hit callbacks into self-contained telemetry.
*/

/** \brief Copies engine damage results without inferring health loss or raid success. */
class ServerSentryBaseDamageFactory
{
    /**
        \brief Checks the server and the independent telemetry/category switches.
        \return \p true when base-damage snapshots are enabled on the server.
    */
    static bool IsEnabled()
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return false;
        }

        ServerSentryConfigManager config = ServerSentryConfigManager.Get();
        return config.IsAllowedToStartTelemetryLogging() && config.IsAllowedToLogBaseDamage();
    }

    /**
        \brief Copies positive damage from a hit confirmed to have ruined the target or a zone.
        \param target \p base-building entity receiving the callback.
        \param damageResult \p engine-owned result, used only during this call.
        \param damageType \p numeric DamageType value supplied by the engine.
        \param source \p damage source; missing player attribution is allowed.
        \param component \p original hit component index.
        \param damageZone \p original damage-zone name, including an empty name.
        \param ammo \p ammunition or damage configuration name.
        \param modelPosition \p callback position in the target's model space.
        \return \p owned event, or null when disabled or no positive Health damage exists.
        \note The caller confirms the ruined transition before invoking this factory.
    */
    static ServerSentryBaseDamageEvent CreateHitSnapshot(EntityAI target, TotalDamageResult damageResult, int damageType, EntityAI source, int component, string damageZone, string ammo, vector modelPosition)
    {
        if (!IsEnabled() || !target || !damageResult)
        {
            return null;
        }

        float highestDamage = damageResult.GetHighestDamage("Health");
        float globalDamage = damageResult.GetDamage("", "Health");
        if (highestDamage <= 0 && globalDamage <= 0)
        {
            return null;
        }

        ServerSentryBaseDamageEvent ev = new ServerSentryBaseDamageEvent();
        ev.schemaVersion = ServerSentryDefaults.SchemaVersion();
        ev.occurredAtUtc = ServerSentryTime.NowUtc();
        ev.eventType = "basebuilding_damage";
        ev.category = "basebuilding";
        ev.severity = "info";
        ev.mod = "ServerSentry";
        ev.action.name = "basebuilding_damage";
        ev.action.displayName = "Base damage";
        ev.action.result = "observed";
        ev.dataJson = "";

        vector targetPosition = target.GetPosition();
        ServerSentryEventFactory.FillTargetFromEntity(ev.target, target);
        ev.target.entityKind = "basebuilding";
        ev.position.FromVector(targetPosition);
        ev.targetPosition = new ServerSentryVectorDto();
        ev.targetPosition.FromVector(targetPosition);

        ServerSentryBaseDamageData data = ev.data;
        data.targetId = target.GetID();
        data.damageType = damageType;
        data.damageKind = ResolveDamageKind(damageType);
        data.component = component;
        data.damageZone = damageZone;
        data.ammoType = ammo;
        data.modelPosition.FromVector(modelPosition);
        if (highestDamage > 0)
        {
            data.highestHealthDamage = highestDamage;
        }
        if (globalDamage > 0)
        {
            data.globalHealthDamage = globalDamage;
        }

        CaptureZones(data, target, damageResult);
        if (source)
        {
            data.sourceClassName = source.GetType();
        }
        FillAttacker(ev, source);

        ev.message = "Base damage observed: " + ev.target.className + " (" + data.damageKind + ")";
        return ev;
    }

    /**
        \brief Copies positive named-zone results, keeping global damage separate.
        \param data \p event payload receiving owned zone values.
        \param target \p entity whose configured zones are enumerated.
        \param damageResult \p engine result for this callback only.
        \note Zone values and global damage must not be summed as independent HP loss.
    */
    protected static void CaptureZones(ServerSentryBaseDamageData data, EntityAI target, TotalDamageResult damageResult)
    {
        array<string> zoneNames = new array<string>();
        target.GetDamageZones(zoneNames);

        // Preserve a named callback zone even if a mod omits it from enumeration.
        if (data.damageZone != "" && zoneNames.Find(data.damageZone) == -1)
        {
            zoneNames.Insert(data.damageZone);
        }

        foreach (string zoneName : zoneNames)
        {
            if (zoneName == "")
            {
                continue;
            }

            float healthDamage = damageResult.GetDamage(zoneName, "Health");
            if (healthDamage <= 0)
            {
                continue;
            }

            ServerSentryBaseDamageZone zone = new ServerSentryBaseDamageZone();
            zone.zoneName = zoneName;
            zone.healthDamage = healthDamage;
            data.zones.Insert(zone);
        }
    }

    /**
        \brief Uses the common explosive resolver or the direct firearm/melee source.
        \param ev \p event receiving identity and attribution method.
        \param source \p physical damage source, possibly null.
        \note Explosive provenance is shared with combat hits and kills.
    */
    protected static void FillAttacker(ServerSentryBaseDamageEvent ev, EntityAI source)
    {
        if (ev.data.damageType == DamageType.EXPLOSION)
        {
            ev.data.attackerResolution = ServerSentryExplosiveResolver.FillActor(ev.actor, source, ev.data.explosive);
            return;
        }

        ev.actor.type = "unknown";
        ev.actor.entityKind = "unknown";
        ev.actor.name = "Unknown attacker";
        if (!source)
        {
            return;
        }

        PlayerBase player = PlayerBase.Cast(source);
        string resolution = "source_player";
        if (!player)
        {
            // An explosive can detonate in somebody else's inventory. Its
            // current carrier is not sufficient evidence of the initiator.
            if (ev.data.damageType != DamageType.FIRE_ARM && ev.data.damageType != DamageType.CLOSE_COMBAT)
            {
                return;
            }

            Man rootPlayer = source.GetHierarchyRootPlayer();
            player = PlayerBase.Cast(rootPlayer);
            resolution = "hierarchy_root_player";
        }
        if (!player)
        {
            return;
        }

        vector playerPosition = player.GetPosition();
        ev.actorPosition = new ServerSentryVectorDto();
        ev.actorPosition.FromVector(playerPosition);
        ev.actor.className = player.GetType();

        PlayerIdentity identity = player.GetIdentity();
        if (!identity)
        {
            // A missing identity can be AI or a disconnect; do not fabricate IDs.
            ev.data.attackerResolution = "player_entity_without_identity";
            return;
        }

        ServerSentryEventFactory.FillActor(ev.actor, player, identity);
        ev.data.attackerResolution = resolution;
    }

    /**
        \brief Maps engine damage categories without guessing from ammunition names.
        \param damageType \p original numeric DamageType value.
        \return \p stable category for grouping base-damage observations.
    */
    protected static string ResolveDamageKind(int damageType)
    {
        switch (damageType)
        {
            case DamageType.FIRE_ARM:
                return "firearm";
            case DamageType.EXPLOSION:
                return "explosion";
            case DamageType.CLOSE_COMBAT:
                return "melee";
            case DamageType.STUN:
                return "stun";
            case DamageType.CUSTOM:
                return "custom";
        }

        return "other";
    }
}
