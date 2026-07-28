/**
	\file ServerSentryItem.c
	\brief Defines ServerSentry support for Item.
*/

/**
	\brief Provides ServerSentry behavior for Mod item.
*/
class ServerSentryModItem
{
    /**
    	\brief Stores the item class name.
    */
    string itemClassName;
    /**
    	\brief Stores the item display name.
    */
    string itemDisplayName;

    // Slots this item can be mounted into (config property: inventorySlot).
    /**
    	\brief Stores the inventory slots.
    */
    ref TStringArray inventorySlots;

    // Slots this item accepts as attachments (config property: attachments).
    /**
    	\brief Stores the attachment slots.
    */
    ref TStringArray attachmentSlots;

    // Compatible magazine class names from the weapon's inherited CfgWeapons magazines[] value.
    /**
    	\brief Stores the magazines.
    */
    ref TStringArray magazines;

    /**
    	\brief Initializes a new ServerSentryModItem instance.
    */
    void ServerSentryModItem()
    {
        itemClassName = "";
        itemDisplayName = "";
        inventorySlots = new TStringArray;
        attachmentSlots = new TStringArray;
        magazines = new TStringArray;
    }
};
