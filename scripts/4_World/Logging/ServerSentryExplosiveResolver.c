/**
    \file ServerSentryExplosiveResolver.c
    \brief Resolves verified explosive actors for hits, kills and ruined base zones.
*/

/** \brief Provides direct source lookup and explicit context transfer without world scans. */
class ServerSentryExplosiveResolver
{
    /**
        \brief Identifies supported devices, including non-exploding tripwire controllers.
        \param item \p physical item to inspect.
        \return \p true for vanilla explosive devices and their trap controllers.
    */
    static bool Supports(ItemBase item)
    {
        return item && (item.IsExplosive() || LandMineTrap.Cast(item) || TripwireTrap.Cast(item));
    }

    /**
        \brief Checks that attribution is being captured on the authoritative server.
        \return \p true on a running server.
    */
    static bool IsServer()
    {
        return GetGame() && GetGame().IsServer();
    }

    /**
        \brief Creates an identity only from a currently authenticated player.
        \param man \p actor supplied by a server action or a verified direct source.
        \param action \p operation that established the relationship.
        \return \p owned identity, or null for missing identities and AI.
    */
    static ServerSentryExplosiveIdentity Capture(Man man, string action)
    {
        PlayerBase player = PlayerBase.Cast(man);
        if (!IsServer() || !player)
            return null;

        PlayerIdentity identity = player.GetIdentity();
        if (!identity || identity.GetPlainId() == "")
            return null;

        ServerSentryExplosiveIdentity result = new ServerSentryExplosiveIdentity();
        ServerSentryEventFactory.FillActor(result, player, identity);
        result.action = action;
        result.occurredAtUtc = ServerSentryTime.NowUtc();
        return result;
    }

    /**
        \brief Allocates a UUID only for a tracked device or actual explosion.
        \return \p fresh UUID.
    */
    static string NewId()
    {
        int uuid[4];
        UUIDApi.Generate(uuid);
        return UUIDApi.FormatString(uuid);
    }

    /**
        \brief Resolves an explosion's actor identically for combat and base damage.
        \param actor \p receives the actor or an explicit unknown identity.
        \param source \p actual damage source supplied by DayZ.
        \param attribution \p receives an owned explosive provenance snapshot.
        \return \p attribution method; unresolved sources never use a current carrier.
    */
    static string FillActor(ServerSentryActor actor, EntityAI source, out ServerSentryExplosiveAttribution attribution)
    {
        attribution = null;
        actor.type = "unknown";
        actor.entityKind = "unknown";
        actor.className = "";
        actor.steamId = "";
        actor.bohemiaId = "";
        actor.name = "Unknown attacker";
        if (!source)
            return "unresolved";

        ItemBase item = ItemBase.Cast(source);
        if (Supports(item))
        {
            attribution = item.ServerSentryExplosionSnapshot();
            if (!attribution)
                return "unresolved";

            ServerSentryExplosiveIdentity responsible = attribution.ResponsibleActor();
            if (responsible)
                ServerSentryEventFactory.CopyActor(responsible, actor);

            return attribution.resolution;
        }

        Man man = Man.Cast(source);
        string resolution = "source_player";
        // Grenade-launcher damage can use the firing weapon as its source.
        // This exception applies to weapons, never to an explosive's carrier.
        if (!man && Weapon_Base.Cast(source))
        {
            man = source.GetHierarchyRootPlayer();
            resolution = "source_weapon_player";
        }
        ServerSentryExplosiveIdentity direct = Capture(man, resolution);
        if (!direct)
            return "unresolved";

        ServerSentryEventFactory.CopyActor(direct, actor);
        return resolution;
    }

    /**
        \brief Records a verified remote trigger before the inherited code detonates the device.
        \param device \p armed device accepting this trigger.
        \param trigger \p supplied activation item.
        \return \p true when the native pairing accepts this remote activation.
    */
    static bool RecordRemote(ExplosivesBase device, ItemBase trigger)
    {
        if (!IsServer() || !device || !trigger || !device.GetArmed() || device.IsRuined())
            return false;

        if (device.GetPairDevice() != trigger)
            return false;

        // Even an unidentified accepted trigger replaces an earlier actor claim.
        ServerSentryExplosiveAttribution state = device.ServerSentryExplosiveState(true);
        state.detonatedBy = null;
        RemoteDetonatorTrigger remote = RemoteDetonatorTrigger.Cast(trigger);
        if (remote && remote.GetControlledDevice() == device)
            state.detonatedBy = remote.ServerSentryTriggerOperator();

        state.activation = "remote";
        device.ServerSentryResetExplosionSnapshot();
        return true;
    }

    /**
        \brief Transfers controller provenance before charges detach or activate asynchronously.
        \param controller \p IED or tripwire controller.
        \param activation \p controller activation path.
    */
    static void TransferCharges(ItemBase controller, string activation)
    {
        if (!IsServer() || !Supports(controller))
            return;

        ServerSentryExplosiveAttribution parent = controller.ServerSentryExplosiveState(true);
        for (int i = 0; i < controller.GetInventory().AttachmentCount(); i++)
        {
            ItemBase charge = ItemBase.Cast(controller.GetInventory().GetAttachmentFromIndex(i));
            if (!Supports(charge))
                continue;

            ServerSentryExplosiveAttribution inherited = parent.Copy();
            ServerSentryExplosiveAttribution child = charge.ServerSentryExplosiveState(true);
            inherited.parentDeviceId = parent.deviceId;
            inherited.deviceId = child.deviceId;
            inherited.detonationId = "";
            inherited.sourcePosition = null;
            inherited.sourceClassName = charge.GetType();
            inherited.activation = activation;
            if (parent.activation == "damage" || parent.activation == "remote")
                inherited.activation = parent.activation;

            charge.ServerSentrySetExplosiveState(inherited);
        }
    }
}
