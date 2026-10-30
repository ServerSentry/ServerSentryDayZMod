/**
    \file ServerSentryExplosiveAttribution.c
    \brief Defines owned explosive identities shared by combat and base-damage events.
*/

/** \brief Copies a verified player identity at the time of an explosive action. */
class ServerSentryExplosiveIdentity : ServerSentryActor
{
    /** \brief Server operation that supplied this identity. */
    string action;
    /** \brief UTC time of that operation, not the later explosion. */
    string occurredAtUtc;

    /**
        \brief Returns an independent identity without any player reference.
        \return \p copied identity.
    */
    ServerSentryExplosiveIdentity Copy()
    {
        ServerSentryExplosiveIdentity result = new ServerSentryExplosiveIdentity();
        ServerSentryEventFactory.CopyActor(this, result);
        result.action = action;
        result.occurredAtUtc = occurredAtUtc;
        return result;
    }
}

/** \brief Retains distinct player roles and physical explosion provenance. */
class ServerSentryExplosiveAttribution
{
    /** \brief Persistent UUID of this physical device. */
    string deviceId;
    /** \brief Parent IED or tripwire that activated this charge. */
    string parentDeviceId;
    /** \brief Runtime UUID shared by all damage from this detonation. */
    string detonationId;
    /** \brief Explosion that caused a chain reaction, when known. */
    string parentDetonationId;
    /** \brief Actual physical source class. */
    string sourceClassName;
    /** \brief Activation path: fuse, trap, timer, remote, damage or unknown. */
    string activation = "unknown";
    /** \brief Selected actor role, or unresolved. */
    string resolution = "unresolved";
    /** \brief Blast origin; never presented as the player's current position. */
    ref ServerSentryVectorDto sourcePosition;
    /** \brief Player who placed this device. Placement alone does not award a kill. */
    ref ServerSentryExplosiveIdentity placedBy;
    /** \brief Player who armed this activation cycle. */
    ref ServerSentryExplosiveIdentity armedBy;
    /** \brief Player who last released the live grenade from their hands. */
    ref ServerSentryExplosiveIdentity releasedBy;
    /** \brief A live release was observed even if the releasing identity was unavailable. */
    bool releaseObserved;
    /** \brief Player who deliberately detonated it remotely or through damage. */
    ref ServerSentryExplosiveIdentity detonatedBy;
    /** \brief Player who stepped on the trap; this role never awards a kill. */
    ref ServerSentryExplosiveIdentity triggeredBy;

    /**
        \brief Selects a responsible identity without guessing from inventory ownership.
        \return \p responsible identity, or null when unresolved.
    */
    ServerSentryExplosiveIdentity ResponsibleActor()
    {
        if (detonatedBy)
            return detonatedBy;

        // An unknown external cause must not silently fall back to the old owner.
        if (activation == "damage" || activation == "remote")
            return null;

        if (releasedBy)
            return releasedBy;

        if (releaseObserved)
            return null;

        return armedBy;
    }

    /**
        \brief Derives the selected actor role independently of the triggering victim.
        \return \p stable attribution method.
    */
    string ResolveRole()
    {
        if (detonatedBy)
            return "detonated_by";

        if (activation == "damage" || activation == "remote")
            return "unresolved";

        if (releasedBy)
            return "released_by";

        if (releaseObserved)
            return "unresolved";

        if (armedBy)
            return "armed_by";

        return "unresolved";
    }

    /**
        \brief Copies all roles so later device changes cannot alter an emitted event.
        \return \p independent attribution snapshot.
    */
    ServerSentryExplosiveAttribution Copy()
    {
        ServerSentryExplosiveAttribution result = new ServerSentryExplosiveAttribution();
        result.deviceId = deviceId;
        result.parentDeviceId = parentDeviceId;
        result.detonationId = detonationId;
        result.parentDetonationId = parentDetonationId;
        result.sourceClassName = sourceClassName;
        result.activation = activation;
        result.releaseObserved = releaseObserved;
        result.resolution = ResolveRole();
        if (sourcePosition)
        {
            result.sourcePosition = new ServerSentryVectorDto();
            result.sourcePosition.x = sourcePosition.x;
            result.sourcePosition.y = sourcePosition.y;
            result.sourcePosition.z = sourcePosition.z;
        }
        if (placedBy)
            result.placedBy = placedBy.Copy();
        if (armedBy)
            result.armedBy = armedBy.Copy();
        if (releasedBy)
            result.releasedBy = releasedBy.Copy();
        if (detonatedBy)
            result.detonatedBy = detonatedBy.Copy();
        if (triggeredBy)
            result.triggeredBy = triggeredBy.Copy();

        return result;
    }
}
