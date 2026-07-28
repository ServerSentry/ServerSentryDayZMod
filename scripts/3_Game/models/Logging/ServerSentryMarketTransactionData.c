/**
	\file ServerSentryMarketTransactionData.c
	\brief Defines the Market transaction data logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Market transaction data.
*/
class ServerSentryMarketTransactionData
{
    /**
    	\brief Stores the source mod.
    */
    string sourceMod;
    /**
    	\brief Stores the transaction type.
    */
    string transactionType;
    /**
    	\brief Stores the result.
    */
    string result;
    /**
    	\brief Stores the raw expansion log.
    */
    string rawExpansionLog;
    /**
    	\brief Stores the raw log.
    */
    string rawLog;
    /**
    	\brief Stores the raw trader log.
    */
    string rawTraderLog;
    /**
    	\brief Stores the item class name.
    */
    string itemClassName;
    /**
    	\brief Stores the item display name.
    */
    string itemDisplayName;
    /**
    	\brief Stores the market item ID.
    */
    int marketItemId;
    /**
    	\brief Stores the category ID.
    */
    int categoryId;
    /**
    	\brief Stores the category file name.
    */
    string categoryFileName;
    /**
    	\brief Stores the category display name.
    */
    string categoryDisplayName;
    /**
    	\brief Stores the total amount.
    */
    int totalAmount;
    /**
    	\brief Stores the total price.
    */
    int totalPrice;
    /**
    	\brief Stores the currency.
    */
    string currency;
    /**
    	\brief Stores the trader index.
    */
    int traderIndex;
    /**
    	\brief Stores the trader ID.
    */
    int traderId;
    /**
    	\brief Stores the item index.
    */
    int itemIndex;
    /**
    	\brief Stores the configured buy price.
    */
    int configuredBuyPrice;
    /**
    	\brief Stores the configured sell price.
    */
    int configuredSellPrice;
    /**
    	\brief Stores the configured quantity.
    */
    int configuredQuantity;
    /**
    	\brief Stores the quantity mode.
    */
    string quantityMode;
    /**
    	\brief Indicates whether is vehicle.
    */
    bool isVehicle;
    /**
    	\brief Indicates whether is key service.
    */
    bool isKeyService;
    /**
    	\brief Stores the persistent ID.
    */
    string persistentId;
    /**
    	\brief Indicates whether include attachments.
    */
    bool includeAttachments;
    /**
    	\brief Indicates whether attachment not attached.
    */
    bool attachmentNotAttached;
    /**
    	\brief Stores the skin index.
    */
    int skinIndex;

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
    	\brief Stores the trader file name.
    */
    string traderFileName;
    /**
    	\brief Stores the trader display name.
    */
    string traderDisplayName;
    /**
    	\brief Stores the trader entity class name.
    */
    string traderEntityClassName;
    /**
    	\brief Stores the trader entity ID.
    */
    string traderEntityId;
    /**
    	\brief Stores the trader position.
    */
    ref ServerSentryVectorDto traderPosition;
    /**
    	\brief Stores the vehicle spawn position.
    */
    ref ServerSentryVectorDto vehicleSpawnPosition;
    /**
    	\brief Stores the vehicle spawn orientation.
    */
    ref ServerSentryVectorDto vehicleSpawnOrientation;

    /**
    	\brief Stores the zone file name.
    */
    string zoneFileName;
    /**
    	\brief Stores the zone display name.
    */
    string zoneDisplayName;
    /**
    	\brief Stores the zone position.
    */
    ref ServerSentryVectorDto zonePosition;
    /**
    	\brief Stores the zone radius.
    */
    float zoneRadius;
    /**
    	\brief Stores the zone buy price percent.
    */
    float zoneBuyPricePercent;
    /**
    	\brief Stores the zone sell price percent.
    */
    float zoneSellPricePercent;

    /**
    	\brief Stores the stock before.
    */
    int stockBefore;
    /**
    	\brief Stores the stock after.
    */
    int stockAfter;
    /**
    	\brief Stores the min price threshold.
    */
    int minPriceThreshold;
    /**
    	\brief Stores the max price threshold.
    */
    int maxPriceThreshold;
    /**
    	\brief Stores the min stock threshold.
    */
    int minStockThreshold;
    /**
    	\brief Stores the max stock threshold.
    */
    int maxStockThreshold;
    /**
    	\brief Stores the quantity percent.
    */
    int quantityPercent;
    /**
    	\brief Stores the sell price percent.
    */
    float sellPricePercent;
    /**
    	\brief Collection of spawn attachments values.
    */
    ref array<string> spawnAttachments;
    /**
    	\brief Collection of variants values.
    */
    ref array<string> variants;

    /**
    	\brief Collection of items values.
    */
    ref array<ref ServerSentryMarketTransactionItem> items;

    /**
    	\brief Initializes a new ServerSentryMarketTransactionData instance.
    */
    void ServerSentryMarketTransactionData()
    {
        sourceMod = "market";
        transactionType = "";
        result = "success";
        rawExpansionLog = "";
        rawLog = "";
        rawTraderLog = "";
        itemClassName = "";
        itemDisplayName = "";
        marketItemId = 0;
        categoryId = 0;
        categoryFileName = "";
        categoryDisplayName = "";
        totalAmount = 0;
        totalPrice = 0;
        currency = "";
        traderIndex = -1;
        traderId = -1;
        itemIndex = -1;
        configuredBuyPrice = 0;
        configuredSellPrice = 0;
        configuredQuantity = 0;
        quantityMode = "";
        isVehicle = false;
        isKeyService = false;
        persistentId = "";
        includeAttachments = false;
        attachmentNotAttached = false;
        skinIndex = -1;

        actorSteamId = "";
        actorBohemiaId = "";
        actorName = "";
        actorPosition = new ServerSentryVectorDto();

        traderFileName = "";
        traderDisplayName = "";
        traderEntityClassName = "";
        traderEntityId = "";
        traderPosition = new ServerSentryVectorDto();
        vehicleSpawnPosition = new ServerSentryVectorDto();
        vehicleSpawnOrientation = new ServerSentryVectorDto();

        zoneFileName = "";
        zoneDisplayName = "";
        zonePosition = new ServerSentryVectorDto();
        zoneRadius = 0;
        zoneBuyPricePercent = 0;
        zoneSellPricePercent = 0;

        stockBefore = 0;
        stockAfter = 0;
        minPriceThreshold = 0;
        maxPriceThreshold = 0;
        minStockThreshold = 0;
        maxStockThreshold = 0;
        quantityPercent = 0;
        sellPricePercent = 0;
        spawnAttachments = new array<string>();
        variants = new array<string>();

        items = new array<ref ServerSentryMarketTransactionItem>();
    }
}
