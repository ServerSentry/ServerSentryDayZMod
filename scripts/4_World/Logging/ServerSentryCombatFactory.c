/**
	\file ServerSentryCombatFactory.c
	\brief Provides logging support for Combat factory.
*/

/**
	\brief Creates values used by Combat.
*/
class ServerSentryCombatFactory
{
    /**
    	\brief Shared hit sequence state used by all instances.
    */
    protected static int s_HitSequence;
    /**
    	\brief Creates a normalized hit context from one callback request.
    	\param request \p captured callback data and target metadata.
    	\return \p The resulting hit context instance, or null when unavailable.
    */
    static ServerSentryHitContext CreateHitContext(ServerSentryHitRequest request)
    {
        if (!request || !request.victim || !request.damageResult)
        {
            return null;
        }

        EntityAI victim = request.victim;
        // ServerSentryDamageZoneExporter.Observe(victim); // Development diagnostics only; disabled in production.
        TotalDamageResult damageResult = request.damageResult;
        int damageType = request.damageType;
        EntityAI source = request.source;
        int component = request.component;
        string dmgZone = request.damageZone;
        string ammo = request.ammoType;

        ServerSentryHitContext ctx = new ServerSentryHitContext();
        s_HitSequence++;

        int timestampMs = GetGame().GetTime();
        ctx.eventSequence = s_HitSequence;
        ctx.occurredAtUtc = ServerSentryTime.NowUtc();
        ctx.hitId = "hit-" + ctx.eventSequence.ToString() + "-" + timestampMs.ToString();
        ctx.damageType = damageType;
        ctx.sourceItem = request.sourceItem;
        ctx.component = component;
        ctx.hitZone = ResolveHitZone(victim, component, dmgZone);
        // EEHitBy component indices refer to fire geometry. Preserve every
        // named selection: the first name alone may only be a broad damage zone.
        // No inference from position or skeleton; damage/headshot logic is unchanged.
        if (component >= 0)
        {
            victim.GetActionComponentNameList(component, ctx.hitComponentNames, "fire");
        }
        ctx.ammoType = ammo;
        ctx.victimPosition = victim.GetPosition();

        ServerSentryEventFactory.FillTargetFromEntity(ctx.target, victim);
        ServerSentryEntityClassifier.ApplyTypeHint(ctx.target, victim, request.targetTypeHint);

        // Prefer the pre-hit identity snapshot for human players. On a lethal
        // callback GetIdentity() can already be null after super.EEHitBy.
        if (request.targetSteamId != "" || request.targetBohemiaId != "" || request.targetName != "")
        {
            ServerSentryEventFactory.FillPlayerActorData(ctx.target, request.targetSteamId, request.targetBohemiaId, request.targetName);
            ctx.target.className = victim.GetType();
        }

        ctx.healthDamage = ResolveDamage(damageResult, ctx.hitZone, "Health");
        ctx.bloodDamage = ResolveDamage(damageResult, ctx.hitZone, "Blood");
        ctx.shockDamage = ResolveDamage(damageResult, ctx.hitZone, "Shock");

        // EEHitBy is already a post-damage callback. Capture the final state first.
        ApplyPostHitState(request, ctx);

        // TotalDamageResult contains the resolved hit damage, so pre-hit health can
        // be reconstructed and clamped to the entity's maximum health.
        ctx.targetHealthBefore = ctx.targetHealthAfter + ctx.healthDamage;
        float maxHealth = victim.GetMaxHealth("", "Health");
        if (maxHealth > 0 && ctx.targetHealthBefore > maxHealth)
        {
            ctx.targetHealthBefore = maxHealth;
        }

        ctx.isLethal = IsLethalHit(ctx);
        ctx.isHeadshot = IsHeadshotZone(ctx.hitZone);
        ctx.isExplosion = damageType == DamageType.EXPLOSION;
        ctx.isMelee = damageType == DT_CLOSE_COMBAT;

        if (ctx.isExplosion)
        {
            ServerSentryEventFactory.CopyActor(request.explosionActor, ctx.attacker);
            ctx.explosive = request.explosive;
            ctx.isMelee = false;
            ctx.weaponClassName = request.explosionSourceClass;
            if (request.hasExplosionPosition)
                ctx.distance = vector.Distance(request.explosionPosition, ctx.victimPosition);

            return ctx;
        }

        if (!source)
        {
            ServerSentryEntityClassifier.FillEnvironment(ctx.attacker, ResolveEnvironmentDamageName(ammo));
            return ctx;
        }

        ctx.weaponClassName = source.GetType();

        if (IsSelfEnvironmentDamage(victim, source, ammo))
        {
            ServerSentryEntityClassifier.FillEnvironment(ctx.attacker, ResolveEnvironmentDamageName(ammo));
            ctx.distance = 0;
            ctx.weaponClassName = "";
            return ctx;
        }

        Transport sourceVehicle = Transport.Cast(source);
        if (sourceVehicle)
        {
            ctx.isMelee = false;
            ctx.weaponClassName = sourceVehicle.GetType();
            ctx.attackerPosition = sourceVehicle.GetPosition();
            ctx.hasAttackerPosition = true;
            ctx.distance = vector.Distance(ctx.attackerPosition, ctx.victimPosition);

            DayZPlayer vehicleDriver = ResolveVehicleDriver(sourceVehicle);
            if (vehicleDriver)
            {
                ServerSentryEventFactory.FillActor(ctx.attacker, vehicleDriver, vehicleDriver.GetIdentity());
            }
            else
            {
                ServerSentryEntityClassifier.FillEnvironment(ctx.attacker, "unoccupied_vehicle");
            }

            return ctx;
        }

        Man attackerMan = source.GetHierarchyRootPlayer();
        DayZPlayer attackerPlayer = DayZPlayer.Cast(attackerMan);
        if (attackerPlayer)
        {
            ctx.attackerPosition = attackerPlayer.GetPosition();
            ctx.hasAttackerPosition = true;
            ctx.distance = vector.Distance(ctx.attackerPosition, ctx.victimPosition);
            ctx.weaponClassName = ResolveWeaponClassName(source, attackerPlayer, ctx.isMelee);
            ServerSentryEventFactory.FillActor(ctx.attacker, attackerPlayer, attackerPlayer.GetIdentity());
            return ctx;
        }

        ctx.attackerPosition = source.GetPosition();
        ctx.hasAttackerPosition = true;
        ctx.distance = vector.Distance(ctx.attackerPosition, ctx.victimPosition);
        ctx.weaponClassName = ResolveWeaponClassName(source, null, ctx.isMelee);
        ServerSentryEventFactory.FillActorFromEntity(ctx.attacker, source);
        return ctx;
    }

    /**
        \brief Applies the latest captured alive and health state to a hit context.
        \param request \p hit request containing the target state snapshot.
        \param ctx \p hit context that receives the final state.

        Damage, bone, attacker, weapon, and the post-hit target state all belong
        to the same EEHitBy callback.
    */
    static void ApplyPostHitState(ServerSentryHitRequest request, ServerSentryHitContext ctx)
    {
        if (!request || !ctx || !request.victim)
        {
            return;
        }

        if (request.hasPostHitState)
        {
            ctx.targetHealthAfter = request.targetHealthAfter;
            ctx.targetAliveAfter = request.targetAliveAfter;
        }
        else
        {
            ctx.targetHealthAfter = request.victim.GetHealth("", "Health");
            ctx.targetAliveAfter = request.victim.IsAlive();
        }

    }

    /**
        \brief Returns whether the resolved callback represents the killing hit.
        \param ctx \p hit context containing resolved damage and final target state.
        \return \p true when positive damage ended with a dead target.

        EEHitBy runs after engine damage resolution. PlayerBase-derived AI may
        already report IsAlive() == false when the callback starts, so a
        pre-super alive snapshot must not be used as a gate. Per-target combat
        state guarantees that only the first lethal callback is emitted.
    */
    protected static bool IsLethalHit(ServerSentryHitContext ctx)
    {
        if (!ctx)
        {
            return false;
        }

        bool hasResolvedDamage = ctx.healthDamage > 0 || ctx.bloodDamage > 0 || ctx.shockDamage > 0;
        if (!hasResolvedDamage)
        {
            return false;
        }

        bool targetDeadAfter = !ctx.targetAliveAfter || ctx.targetHealthAfter <= 0;
        return targetDeadAfter && ctx.targetHealthBefore > 0;
    }

    /**
    	\brief Resolves hit zone.
    	\param victim \p victim used by the operation.
    	\param component \p component used by the operation.
    	\param damageZone \p damage zone used by the operation.
    	\return \p The resulting string value.
    */
    protected static string ResolveHitZone(EntityAI victim, int component, string damageZone)
    {
        string componentZone = "";
        if (victim && component >= 0)
        {
            componentZone = victim.GetDamageZoneNameByComponentIndex(component);
        }

        if (damageZone == "")
        {
            return componentZone;
        }

        // Some lethal callbacks provide a generic/non-head damageZone while the
        // component still resolves to Head/Brain. Prefer the more specific engine
        // component zone in that case.
        if (IsHeadshotZone(componentZone) && !IsHeadshotZone(damageZone))
        {
            return componentZone;
        }

        return damageZone;
    }

    /**
    	\brief Returns whether headshot zone.
    	\param damageZone \p damage zone used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsHeadshotZone(string damageZone)
    {
        if (damageZone == "")
        {
            return false;
        }

        string normalized = damageZone;
        normalized.ToLower();

        // DayZ damage zones use the exact names Head and Brain.
        // Compare case-insensitively, but do not guess additional zone names.
        return normalized == "head" || normalized == "brain";
    }

    /**
    	\brief Resolves vehicle driver.
    	\param vehicle \p vehicle used by the operation.
    	\return \p The resulting day z player instance, or null when unavailable.
    */
    protected static DayZPlayer ResolveVehicleDriver(Transport vehicle)
    {
        if (!vehicle)
        {
            return null;
        }

        return DayZPlayer.Cast(vehicle.CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER));
    }

    /**
    	\brief Resolves damage.
    	\param damageResult \p damage result used by the operation.
    	\param zoneName \p zone name used by the operation.
    	\param healthType \p health type used by the operation.
    	\return \p The resulting floating-point value.
    */
    protected static float ResolveDamage(TotalDamageResult damageResult, string zoneName, string healthType)
    {
        if (!damageResult)
        {
            return 0;
        }

        float damage = damageResult.GetDamage(zoneName, healthType);
        if (damage > 0)
        {
            return damage;
        }

        return damageResult.GetHighestDamage(healthType);
    }

    /**
    	\brief Resolves weapon class name.
    	\param source \p source used by the operation.
    	\param attackerPlayer \p attacker player used by the operation.
    	\param isMelee \p true when is melee.
    	\return \p The resulting string value.
    */
    protected static string ResolveWeaponClassName(EntityAI source, DayZPlayer attackerPlayer, bool isMelee)
    {
        if (source && source.IsWeapon())
        {
            return source.GetType();
        }

        if (attackerPlayer)
        {
            EntityAI itemInHands = attackerPlayer.GetHumanInventory().GetEntityInHands();
            if (itemInHands)
            {
                return itemInHands.GetType();
            }
        }

        if (isMelee)
        {
            return "melee";
        }

        if (source)
        {
            return source.GetType();
        }

        return "";
    }

    /**
    	\brief Returns whether self environment damage.
    	\param victim \p victim used by the operation.
    	\param source \p source used by the operation.
    	\param ammo \p ammo used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected static bool IsSelfEnvironmentDamage(EntityAI victim, EntityAI source, string ammo)
    {
        if (!victim || !source)
        {
            return false;
        }

        if (source != victim)
        {
            return false;
        }

        return ammo.Contains("FallDamage") || ammo.Contains("Drowning") || ammo.Contains("Temperature") || ammo.Contains("Bleeding");
    }

    /**
    	\brief Resolves environment damage name.
    	\param ammo \p ammo used by the operation.
    	\return \p The resulting string value.
    */
    protected static string ResolveEnvironmentDamageName(string ammo)
    {
        if (ammo.Contains("FallDamage"))
        {
            return "fall_damage";
        }

        if (ammo.Contains("Drowning"))
        {
            return "drowning";
        }

        if (ammo.Contains("Temperature"))
        {
            return "temperature";
        }

        if (ammo.Contains("Bleeding"))
        {
            return "bleeding";
        }

        return "environment";
    }

};
