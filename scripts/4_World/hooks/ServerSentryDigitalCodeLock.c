/**
    \file ServerSentryDigitalCodeLock.c
    \brief Audits vanilla digital lock interactions and completed code requests.
*/
/**
    \brief Records keypad request outcomes after the vanilla component processes them.
*/
modded class CodeLockComponent
{
    /**
        \brief Logs code setup and successful or failed unlock attempts without logging PINs.
        \param pin \p code passed unchanged to the vanilla handler.
        \param identity \p requesting player's server-provided identity.
    */
    override protected void CheckCodeRequest(string pin, PlayerIdentity identity)
    {
        DigitalCodeLock codeLock = DigitalCodeLock.Cast(m_Parent);
        ServerSentryCodeLockEventData data = ServerSentryDigitalCodeLockLog.Capture(codeLock, identity, "unlock", "CodeLockComponent.CheckCodeRequest");
        bool hadPIN = HasPIN();
        bool wasLocked = IsLocked();

        super.CheckCodeRequest(pin, identity);

        if (!data)
            return;

        data.codeLockAccessType = "keypad";
        if (!hadPIN)
        {
            data.codeLockAction = "set_code";
            if (HasPIN())
                ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "success");
            else
                ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "failed", "Code was not accepted by the lock.");
        }
        else if (wasLocked)
        {
            if (!IsLocked())
                ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "success");
            else
                ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "failed", "Lock remained locked after the code request.");
        }
        else
        {
            data.codeLockAction = "access";
            ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "ignored", "Lock was already unlocked.");
        }
    }
}

/**
    \brief Records keypad access, locking, and physical release from the inside.
*/
modded class ActionInteractCodeLockItem
{
    /**
        \brief Captures the interaction and reads the result after the vanilla handler.
        \param action_data \p server-side action and target data.
    */
    override void OnExecuteServer(ActionData action_data)
    {
        DigitalCodeLock codeLock;
        ServerSentryCodeLockEventData data;
        bool wasLocked;
        bool hadPIN;
        bool validInteraction;
        bool backside;

        if (action_data && action_data.m_Target && action_data.m_Player)
        {
            codeLock = DigitalCodeLock.Cast(action_data.m_Target.GetObject());
            PlayerIdentity identity = action_data.m_Player.GetIdentity();
            data = ServerSentryDigitalCodeLockLog.Capture(codeLock, identity, "access", "ActionInteractCodeLockItem.OnExecuteServer");
            if (data)
            {
                data.actorPosition.FromVector(action_data.m_Player.GetPosition());
                wasLocked = codeLock.IsLocked();
                hadPIN = codeLock.HasPIN();
                backside = codeLock.IsPlayerOnBackside(action_data.m_Player);
                InteractCodeLockActionData interaction = InteractCodeLockActionData.Cast(action_data);
                if (interaction && interaction.m_CodeLockPosition != vector.Zero)
                    validInteraction = !interaction.m_IsFacingBackside || backside;
            }
        }

        super.OnExecuteServer(action_data);

        if (!data)
            return;

        data.codeLockAccessType = "keypad";
        if (backside && wasLocked)
            data.codeLockAccessType = "physical_release";

        if (!validInteraction)
        {
            ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "denied", "Server rejected the interaction data or lock side.");
        }
        else if (wasLocked != codeLock.IsLocked())
        {
            data.codeLockAction = "unlock";
            if (codeLock.IsLocked())
                data.codeLockAction = "lock";
            ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "success");
        }
        else if (!backside && (!hadPIN || wasLocked))
        {
            ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "success", "Keypad interaction requested; no code has been checked yet.");
        }
        else
        {
            ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "ignored", "Interaction did not change the lock state.");
        }
    }
}

/**
    \brief Records player-triggered resets of a digital lock code.
*/
modded class ActionResetCodeOnCodeLock
{
    /**
        \brief Logs whether the vanilla reset removed the configured code.
        \param action_data \p player and target for the reset operation.
    */
    override void OnExecuteServer(ActionData action_data)
    {
        DigitalCodeLock codeLock;
        ServerSentryCodeLockEventData data;
        if (action_data && action_data.m_Target && action_data.m_Player)
        {
            codeLock = DigitalCodeLock.Cast(action_data.m_Target.GetObject());
            PlayerIdentity identity = action_data.m_Player.GetIdentity();
            data = ServerSentryDigitalCodeLockLog.Capture(codeLock, identity, "reset_code", "ActionResetCodeOnCodeLock.OnExecuteServer");
            if (data)
            {
                data.actorPosition.FromVector(action_data.m_Player.GetPosition());
                data.codeLockAccessType = "reset";
            }
        }

        super.OnExecuteServer(action_data);

        if (data)
        {
            if (!codeLock.HasPIN())
                ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "success");
            else
                ServerSentryDigitalCodeLockLog.Emit(data, codeLock, "failed", "Code remained configured after reset.");
        }
    }
}
