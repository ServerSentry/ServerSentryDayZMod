/**
	\file ServerSentry_Exporter.c
	\brief Implements item export support for Exporter.
*/

/**
	\brief Provides ServerSentry behavior for Exporter.
*/
class ServerSentryExporter
{
    /**
    	\brief Constant value for dir.
    */
    static const string OUT_DIR = "$profile:ServerSentry";
    /**
    	\brief Constant value for file.
    */
    static const string OUT_FILE = "$profile:ServerSentry/items.json";
    /**
    	\brief Constant value for file.
    */
    static const string COUNT_FILE = "$profile:ServerSentry/items_count.json";
    /**
    	\brief Constant value for file.
    */
    static const string FORCE_FILE = "$profile:ServerSentry/force_item_export";

    /**
    	\brief Shared direct base cache state used by all instances.
    */
    protected static ref map<string, string> s_DirectBaseCache;

    /**
    	\brief Processes the export operation.
    */
    static void Export()
    {
        Print("[Server Sentry] Lightweight item slot export check started");

        if (!FileExist(OUT_DIR))
        {
            MakeDirectory(OUT_DIR);
            Print("[Server Sentry] Created directory: " + OUT_DIR);
        }

        int cfgVehiclesChildren = GetGame().ConfigGetChildrenCount("CfgVehicles");
        int cfgWeaponsChildren = GetGame().ConfigGetChildrenCount("CfgWeapons");
        int cfgMagazinesChildren = GetGame().ConfigGetChildrenCount("CfgMagazines");
        int cfgSlotsChildren = GetGame().ConfigGetChildrenCount("CfgSlots");
        int cfgPatchesChildren = GetGame().ConfigGetChildrenCount("CfgPatches");

        if (CanSkipExportByConfigState(cfgVehiclesChildren, cfgWeaponsChildren, cfgMagazinesChildren, cfgSlotsChildren, cfgPatchesChildren))
        {
            Print("[Server Sentry] Item config unchanged. Skipping slot export.");
            return;
        }

        float exportStartedAt = GetGame().GetTickTime();

        ResetLookupCaches();

        ServerSentryCatalog catalog = new ServerSentryCatalog;

        CollectRoot("CfgVehicles", catalog.items);
        CollectRoot("CfgWeapons", catalog.items);
        CollectRoot("CfgMagazines", catalog.items);

        catalog.itemCount = catalog.items.Count();

        float writeStartedAt = GetGame().GetTickTime();

        JsonFileLoader<ServerSentryCatalog>.JsonSaveFile(OUT_FILE, catalog);
        SaveItemCountState(catalog.itemCount, catalog.schemaVersion, cfgVehiclesChildren, cfgWeaponsChildren, cfgMagazinesChildren, cfgSlotsChildren, cfgPatchesChildren);

        if (FileExist(FORCE_FILE))
            DeleteFile(FORCE_FILE);

        float writeSeconds = GetGame().GetTickTime() - writeStartedAt;
        float totalSeconds = GetGame().GetTickTime() - exportStartedAt;

        ClearLookupCaches();

        Print("[Server Sentry] Lightweight slot export finished. Items: " + catalog.itemCount.ToString());
        Print("[Server Sentry] JSON write time: " + writeSeconds.ToString() + " seconds.");
        Print("[Server Sentry] Total export time: " + totalSeconds.ToString() + " seconds.");
        Print("[Server Sentry] Written to: " + OUT_FILE);
    }

    /**
    	\brief Collects root.
    	\param root \p root used by the operation.
    	\param items \p collection of items values.
    */
    static void CollectRoot(string root, notnull array<ref ServerSentryModItem> items)
    {
        int count = GetGame().ConfigGetChildrenCount(root);

        for (int i = 0; i < count; i++)
        {
            string className = "";
            GetGame().ConfigGetChildName(root, i, className);

            if (className == "")
                continue;

            string basePath = root + " " + className;

            if (!GetGame().ConfigIsExisting(basePath + " scope"))
                continue;

            // Only public/spawnable config entries requested by the web catalog.
            if (GetGame().ConfigGetInt(basePath + " scope") != 2)
                continue;

            ServerSentryModItem item = new ServerSentryModItem;
            item.itemClassName = className;

            ReadTextOrArray(basePath + " inventorySlot", item.inventorySlots);
            ReadTextOrArray(basePath + " attachments", item.attachmentSlots);

            if (!IsRelevantItem(root, className, item.inventorySlots, item.attachmentSlots))
                continue;

            if (root == "CfgWeapons")
            {
                // DayZ weapon-magazine compatibility is defined by magazines[].
                // Read it from the concrete class and fall back through its base chain
                // so definitions inherited from classes such as M4A1_Base are exported.
                ReadInheritedTextOrArray("CfgWeapons", className, "magazines", item.magazines);
            }

            if (GetGame().ConfigIsExisting(basePath + " displayName"))
                GetGame().ConfigGetText(basePath + " displayName", item.itemDisplayName);

            if (item.itemDisplayName == "")
                item.itemDisplayName = className;

            items.Insert(item);
        }
    }

    /**
    	\brief Returns whether relevant item.
    	\param root \p root used by the operation.
    	\param className \p class name used by the operation.
    	\param inventorySlots \p inventory slots used by the operation.
    	\param attachmentSlots \p attachment slots used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsRelevantItem(string root, string className, notnull TStringArray inventorySlots, notnull TStringArray attachmentSlots)
    {
        bool hasSlots = inventorySlots.Count() > 0 || attachmentSlots.Count() > 0;

        // Keep actual weapons and magazines in the item search even when they have no slots.
        if (root == "CfgWeapons")
            return hasSlots || IsWeaponConfigItem(className);

        if (root == "CfgMagazines")
            return hasSlots || IsMagazineConfigItem(className);

        if (root == "CfgVehicles")
            return IsCfgVehiclesCatalogItem(className, hasSlots);

        return false;
    }

    /**
    	\brief Returns whether weapon config item.
    	\param className \p class name used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsWeaponConfigItem(string className)
    {
        string currentClass = className;

        for (int depth = 0; depth < 64; depth++)
        {
            if (currentClass == "Weapon_Base" || currentClass == "Rifle_Base" || currentClass == "Pistol_Base" || currentClass == "Launcher_Base")
                return true;

            string baseName = "";

            if (!GetDirectBaseName("CfgWeapons", currentClass, baseName))
                break;

            if (baseName == "" || baseName == currentClass)
                break;

            currentClass = baseName;
        }

        return false;
    }

    /**
    	\brief Returns whether magazine config item.
    	\param className \p class name used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsMagazineConfigItem(string className)
    {
        string currentClass = className;

        for (int depth = 0; depth < 64; depth++)
        {
            if (currentClass == "Ammunition_Base" || currentClass == "Magazine_Base")
                return true;

            string baseName = "";

            if (!GetDirectBaseName("CfgMagazines", currentClass, baseName))
                break;

            if (baseName == "" || baseName == currentClass)
                break;

            currentClass = baseName;
        }

        return false;
    }

    /**
    	\brief Returns whether cfg vehicles catalog item.
    	\param className \p class name used by the operation.
    	\param hasSlots \p true when has slots.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsCfgVehiclesCatalogItem(string className, bool hasSlots)
    {
        string currentClass = className;

        for (int depth = 0; depth < 64; depth++)
        {
            // Never export world objects, players, infected or animals as catalog items.
            if (currentClass == "Man" || currentClass == "SurvivorBase" || currentClass == "ZombieBase" || currentClass == "AnimalBase" || currentClass == "House" || currentClass == "HouseNoDestruct" || currentClass == "Building")
                return false;

            // Inventory items and vehicles are relevant for the web catalog.
            if (currentClass == "Inventory_Base" || currentClass == "ItemBase" || currentClass == "Clothing" || currentClass == "Clothing_Base" || currentClass == "Container_Base" || currentClass == "KitBase" || currentClass == "CarScript" || currentClass == "BoatScript" || currentClass == "VehicleBase" || currentClass == "Transport" || currentClass == "LandVehicle")
                return true;

            string baseName = "";

            if (!GetDirectBaseName("CfgVehicles", currentClass, baseName))
                break;

            if (baseName == "" || baseName == currentClass)
                break;

            currentClass = baseName;
        }

        // Unknown modded classes with real slot declarations are still useful.
        return hasSlots;
    }

    /**
    	\brief Reads inherited text or array.
    	\param root \p root used by the operation.
    	\param className \p class name used by the operation.
    	\param propertyName \p property name used by the operation.
    	\param output \p output used by the operation.
    */
    static void ReadInheritedTextOrArray(string root, string className, string propertyName, notnull TStringArray output)
    {
        string currentClass = className;

        for (int depth = 0; depth < 64; depth++)
        {
            string propertyPath = root + " " + currentClass + " " + propertyName;

            if (GetGame().ConfigIsExisting(propertyPath))
            {
                ReadTextOrArray(propertyPath, output);
                return;
            }

            string baseName = "";

            if (!GetDirectBaseName(root, currentClass, baseName))
                return;

            if (baseName == "" || baseName == currentClass)
                return;

            currentClass = baseName;
        }
    }

    /**
    	\brief Reads text or array.
    	\param path \p file-system or API path used by the operation.
    	\param output \p output used by the operation.
    */
    static void ReadTextOrArray(string path, notnull TStringArray output)
    {
        if (!GetGame().ConfigIsExisting(path))
            return;

        string singleValue = "";
        GetGame().ConfigGetText(path, singleValue);

        if (singleValue != "")
            AddUniqueCI(output, singleValue);

        ref TStringArray values = new TStringArray;
        GetGame().ConfigGetTextArray(path, values);

        for (int i = 0; i < values.Count(); i++)
        {
            AddUniqueCI(output, values.Get(i));
        }
    }

    /**
    	\brief Adds unique ci.
    	\param values \p values used by the operation.
    	\param value \p value to process or assign.
    */
    static void AddUniqueCI(notnull TStringArray values, string value)
    {
        string wanted = NormalizeKey(value);

        if (wanted == "")
            return;

        for (int i = 0; i < values.Count(); i++)
        {
            if (NormalizeKey(values.Get(i)) == wanted)
                return;
        }

        values.Insert(value);
    }

    /**
    	\brief Normalizes key.
    	\param value \p value to process or assign.
    	\return \p The resulting string value.
    */
    static string NormalizeKey(string value)
    {
        string normalized = value;
        normalized.ToLower();
        return normalized;
    }

    /**
    	\brief Returns whether skip export by config state.
    	\param cfgVehiclesChildren \p cfg vehicles children used by the operation.
    	\param cfgWeaponsChildren \p cfg weapons children used by the operation.
    	\param cfgMagazinesChildren \p cfg magazines children used by the operation.
    	\param cfgSlotsChildren \p cfg slots children used by the operation.
    	\param cfgPatchesChildren \p cfg patches children used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool CanSkipExportByConfigState(int cfgVehiclesChildren, int cfgWeaponsChildren, int cfgMagazinesChildren, int cfgSlotsChildren, int cfgPatchesChildren)
    {
        if (FileExist(FORCE_FILE))
        {
            Print("[Server Sentry] Force export marker found: " + FORCE_FILE);
            return false;
        }

        if (!FileExist(OUT_FILE) || !FileExist(COUNT_FILE))
            return false;

        ServerSentryItemCountState oldState = new ServerSentryItemCountState;
        JsonFileLoader<ServerSentryItemCountState>.JsonLoadFile(COUNT_FILE, oldState);

        ServerSentryItemCountState currentState = new ServerSentryItemCountState;
        ServerSentryCatalog currentCatalog = new ServerSentryCatalog;

        if (oldState.m_SchemaVersion != currentState.m_SchemaVersion)
            return false;

        if (oldState.m_CatalogSchemaVersion != currentCatalog.schemaVersion)
            return false;

        if (oldState.m_CfgVehiclesChildren != cfgVehiclesChildren)
            return false;

        if (oldState.m_CfgWeaponsChildren != cfgWeaponsChildren)
            return false;

        if (oldState.m_CfgMagazinesChildren != cfgMagazinesChildren)
            return false;

        if (oldState.m_CfgSlotsChildren != cfgSlotsChildren)
            return false;

        if (oldState.m_CfgPatchesChildren != cfgPatchesChildren)
            return false;

        return true;
    }

    /**
    	\brief Saves item count state.
    	\param itemCount \p item count used by the operation.
    	\param catalogSchemaVersion \p catalog schema version used by the operation.
    	\param cfgVehiclesChildren \p cfg vehicles children used by the operation.
    	\param cfgWeaponsChildren \p cfg weapons children used by the operation.
    	\param cfgMagazinesChildren \p cfg magazines children used by the operation.
    	\param cfgSlotsChildren \p cfg slots children used by the operation.
    	\param cfgPatchesChildren \p cfg patches children used by the operation.
    */
    static void SaveItemCountState(int itemCount, int catalogSchemaVersion, int cfgVehiclesChildren, int cfgWeaponsChildren, int cfgMagazinesChildren, int cfgSlotsChildren, int cfgPatchesChildren)
    {
        ServerSentryItemCountState state = new ServerSentryItemCountState;

        state.m_TotalServerItems = itemCount;
        state.m_CatalogSchemaVersion = catalogSchemaVersion;
        state.m_CfgVehiclesChildren = cfgVehiclesChildren;
        state.m_CfgWeaponsChildren = cfgWeaponsChildren;
        state.m_CfgMagazinesChildren = cfgMagazinesChildren;
        state.m_CfgSlotsChildren = cfgSlotsChildren;
        state.m_CfgPatchesChildren = cfgPatchesChildren;

        JsonFileLoader<ServerSentryItemCountState>.JsonSaveFile(COUNT_FILE, state);
    }

    /**
    	\brief Resets lookup caches.
    */
    static void ResetLookupCaches()
    {
        s_DirectBaseCache = new map<string, string>;
    }

    /**
    	\brief Clears lookup caches.
    */
    static void ClearLookupCaches()
    {
        s_DirectBaseCache = null;
    }

    /**
    	\brief Returns direct base name.
    	\param root \p root used by the operation.
    	\param className \p class name used by the operation.
    	\param baseName \p receives the base name used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    static bool GetDirectBaseName(string root, string className, out string baseName)
    {
        baseName = "";

        if (root == "" || className == "")
            return false;

        string cacheKey = root + "|" + className;

        if (s_DirectBaseCache && s_DirectBaseCache.Contains(cacheKey))
        {
            baseName = s_DirectBaseCache.Get(cacheKey);
            return baseName != "";
        }

        GetGame().ConfigGetBaseName(root + " " + className, baseName);

        if (s_DirectBaseCache)
            s_DirectBaseCache.Insert(cacheKey, baseName);

        return baseName != "";
    }
};
