/**
    \file ServerSentryDigitalCodeLockLog.c
    \brief Captures server-side audit data for vanilla digital code locks.
*/
class ServerSentryDigitalCodeLockLog
{
    /**
        \brief Captures the actor, lock, and attachment parent before an interaction.
        \param codeLock \p digital lock being accessed.
        \param identity \p server-provided identity of the interacting player.
        \param action \p semantic lock action.
        \param sourceHook \p callback that observed the interaction.
        \return \p Captured event data, or null when logging is disabled.
    */
    static ServerSentryCodeLockEventData Capture(DigitalCodeLock codeLock, PlayerIdentity identity, string action, string sourceHook)
    {
        if (!GetGame() || !GetGame().IsServer() || !codeLock)
            return null;

        if (!ServerSentryDefaults.LogCodeLockEvents() || !ServerSentryConfigManager.Get().IsAllowedToStartTelemetryLogging())
            return null;

        ServerSentryCodeLockEventData data = new ServerSentryCodeLockEventData();
        data.sourceMod = "DayZ";
        data.codeLockAction = action;
        data.sourceHook = sourceHook;
        data.lockClassName = codeLock.GetType();
        int lockId = codeLock.GetID();
        data.lockObjectId = lockId.ToString();
        data.lockPosition.FromVector(codeLock.GetPosition());
        data.lockHealth = codeLock.GetHealth("", "Health");
        data.lockMaxHealth = codeLock.GetMaxHealth("", "Health");
        data.isLocked = codeLock.IsLocked();
        data.lockStateKnown = true;
        data.targetType = "object";

        if (identity)
        {
            data.actorSteamId = identity.GetPlainId();
            data.actorBohemiaId = identity.GetId();
            data.actorName = identity.GetName();
            Man player = identity.GetPlayer();
            if (player)
                data.actorPosition.FromVector(player.GetPosition());
        }

        EntityAI parent = codeLock.GetHierarchyParent();
        if (parent && !PlayerBase.Cast(parent))
        {
            data.parentClassName = parent.GetType();
            data.parentDisplayName = parent.GetDisplayName();
            int parentId = parent.GetID();
            data.parentObjectId = parentId.ToString();
            data.parentPosition.FromVector(parent.GetPosition());
            data.targetClassName = data.parentClassName;
            data.targetDisplayName = data.parentDisplayName;
            data.targetPosition.FromVector(parent.GetPosition());
        }
        else
        {
            data.targetClassName = data.lockClassName;
            data.targetDisplayName = codeLock.GetDisplayName();
            data.targetPosition.FromVector(codeLock.GetPosition());
        }

        return data;
    }

    /**
        \brief Emits the captured interaction with its observed server-side outcome.
        \param data \p primitive event snapshot captured before the interaction.
        \param codeLock \p lock whose resulting state is read.
        \param result \p success, failed, denied, or ignored.
        \param reason \p explanation of the observed outcome.
    */
    static void Emit(ServerSentryCodeLockEventData data, DigitalCodeLock codeLock, string result, string reason = "")
    {
        if (!data || !codeLock)
            return;

        data.result = result;
        data.reason = reason;
        data.isLocked = codeLock.IsLocked();
        ServerSentryApi.EmitCodeLockEvent(data);
    }
}
