/**
	\file ServerSentryLBATMEventData.c
	\brief Defines the Lbatm event data logging data model.
*/

/**
	\brief Carries payload data for Lbatm events.
*/
class ServerSentryLBATMEventData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the atm action.
    */
    string atmAction;
    /**
    	\brief Stores the result.
    */
    string result;
    /**
    	\brief Stores the reason.
    */
    string reason;
    /**
    	\brief Stores the source hook.
    */
    string sourceHook;
    /**
    	\brief Stores the webhook type.
    */
    string webhookType;

    /**
    	\brief Stores the actor steam ID.
    */
    string actorSteamId;
    /**
    	\brief Stores the actor bohemia ID.
    */
    string actorBohemiaId;
    /**
    	\brief Stores the actor name.
    */
    string actorName;
    /**
    	\brief Stores the actor position.
    */
    ref ServerSentryVectorDto actorPosition;
    /**
    	\brief Indicates whether is admin.
    */
    bool isAdmin;

    /**
    	\brief Stores the account type.
    */
    string accountType;
    /**
    	\brief Stores the direction.
    */
    string direction;
    /**
    	\brief Stores the destination.
    */
    string destination;

    /**
    	\brief Stores the amount text.
    */
    string amountText;
    /**
    	\brief Stores the amount.
    */
    int amount;
    /**
    	\brief Stores the fee text.
    */
    string feeText;
    /**
    	\brief Stores the fee.
    */
    int fee;
    /**
    	\brief Stores the balance before text.
    */
    string balanceBeforeText;
    /**
    	\brief Stores the balance after text.
    */
    string balanceAfterText;
    /**
    	\brief Stores the balance before.
    */
    int balanceBefore;
    /**
    	\brief Stores the balance after.
    */
    int balanceAfter;
    /**
    	\brief Stores the balance delta.
    */
    int balanceDelta;
    /**
    	\brief Indicates whether amount inferred from balance delta.
    */
    bool amountInferredFromBalanceDelta;

    /**
    	\brief Stores the target steam ID.
    */
    string targetSteamId;
    /**
    	\brief Stores the target bohemia ID.
    */
    string targetBohemiaId;
    /**
    	\brief Stores the target name.
    */
    string targetName;

    /**
    	\brief Stores the recipient steam ID.
    */
    string recipientSteamId;
    /**
    	\brief Stores the recipient bohemia ID.
    */
    string recipientBohemiaId;
    /**
    	\brief Stores the recipient name.
    */
    string recipientName;

    /**
    	\brief Stores the group name.
    */
    string groupName;
    /**
    	\brief Stores the group tag.
    */
    string groupTag;

    /**
    	\brief Stores the atm name.
    */
    string atmName;
    /**
    	\brief Stores the atm class name.
    */
    string atmClassName;
    /**
    	\brief Stores the atm position.
    */
    ref ServerSentryVectorDto atmPosition;

    /**
    	\brief Stores the card UID.
    */
    string cardUid;
    /**
    	\brief Stores the card class name.
    */
    string cardClassName;
    /**
    	\brief Stores the card display name.
    */
    string cardDisplayName;
    /**
    	\brief Indicates whether pin redacted.
    */
    bool pinRedacted;
    /**
    	\brief Stores the wrong pin count.
    */
    int wrongPinCount;

    /**
    	\brief Stores the player count.
    */
    int playerCount;
    /**
    	\brief Stores the robbing time minutes.
    */
    int robbingTimeMinutes;
    /**
    	\brief Indicates whether spawn robbed money at ATM.
    */
    bool spawnRobbedMoneyAtATM;
    /**
    	\brief Stores the robbery group name.
    */
    string robberyGroupName;
    /**
    	\brief Stores the robbery group tag.
    */
    string robberyGroupTag;

    /**
    	\brief Stores the max money bonus before text.
    */
    string maxMoneyBonusBeforeText;
    /**
    	\brief Stores the max money bonus after text.
    */
    string maxMoneyBonusAfterText;
    /**
    	\brief Stores the paycheck bonus before text.
    */
    string paycheckBonusBeforeText;
    /**
    	\brief Stores the paycheck bonus after text.
    */
    string paycheckBonusAfterText;
    /**
    	\brief Stores the ignore transfer fee before.
    */
    string ignoreTransferFeeBefore;
    /**
    	\brief Stores the ignore transfer fee after.
    */
    string ignoreTransferFeeAfter;
    /**
    	\brief Stores the had account before.
    */
    string hadAccountBefore;
    /**
    	\brief Stores the has account after.
    */
    string hasAccountAfter;

    /**
    	\brief Stores the raw.
    */
    string raw;
    /**
    	\brief Stores the extra.
    */
    string extra;

    /**
    	\brief Initializes a new ServerSentryLBATMEventData instance.
    */
    void ServerSentryLBATMEventData()
    {
        sourceMod = "LBmaster_Groups_ATM";
        atmAction = "";
        result = "success";
        reason = "";
        sourceHook = "";
        webhookType = "";

        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();
        isAdmin = false;

        accountType = "";
        direction = "";
        destination = "";

        amountText = "";
        amount = 0;
        feeText = "";
        fee = 0;
        balanceBeforeText = "";
        balanceAfterText = "";
        balanceBefore = 0;
        balanceAfter = 0;
        balanceDelta = 0;
        amountInferredFromBalanceDelta = false;

        targetSteamId = "";
        targetBohemiaId = "";
        targetName = "";

        recipientSteamId = "";
        recipientBohemiaId = "";
        recipientName = "";

        groupName = "";
        groupTag = "";

        atmName = "";
        atmClassName = "";
        atmPosition = new ServerSentryVectorDto();

        cardUid = "";
        cardClassName = "";
        cardDisplayName = "";
        pinRedacted = false;
        wrongPinCount = 0;

        playerCount = 0;
        robbingTimeMinutes = 0;
        spawnRobbedMoneyAtATM = false;
        robberyGroupName = "";
        robberyGroupTag = "";

        maxMoneyBonusBeforeText = "";
        maxMoneyBonusAfterText = "";
        paycheckBonusBeforeText = "";
        paycheckBonusAfterText = "";
        ignoreTransferFeeBefore = "";
        ignoreTransferFeeAfter = "";
        hadAccountBefore = "";
        hasAccountAfter = "";

        raw = "";
        extra = "";
    }
}
