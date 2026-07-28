/**
	\file ServerSentryMarketTransactionItem.c
	\brief Defines the Market transaction item logging data model.
*/

/**
	\brief Provides ServerSentry behavior for Market transaction item.
*/
class ServerSentryMarketTransactionItem
{
    /**
    	\brief Stores the class name.
    */
    string className;
    /**
    	\brief Stores the display name.
    */
    string displayName;
    /**
    	\brief Stores the market item ID.
    */
    int marketItemId;
    /**
    	\brief Stores the category ID.
    */
    int categoryId;
    /**
    	\brief Stores the amount.
    */
    int amount;
    /**
    	\brief Stores the price.
    */
    int price;
    /**
    	\brief Stores the taken amount.
    */
    int takenAmount;
    /**
    	\brief Stores the sold amount.
    */
    int soldAmount;
    /**
    	\brief Stores the add stock amount.
    */
    float addStockAmount;
    /**
    	\brief Network or persistent ID of the target object.
    */
    string objectId;
    /**
    	\brief Indicates whether is entity.
    */
    bool isEntity;
    /**
    	\brief Stores the health.
    */
    float health;
    /**
    	\brief Stores the quantity.
    */
    float quantity;
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
    	\brief Initializes a new ServerSentryMarketTransactionItem instance.
    */
    void ServerSentryMarketTransactionItem()
    {
        className = "";
        displayName = "";
        marketItemId = 0;
        categoryId = 0;
        amount = 0;
        price = 0;
        takenAmount = 0;
        soldAmount = 0;
        addStockAmount = 0;
        objectId = "";
        isEntity = false;
        health = 0;
        quantity = 0;
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
    }
}
