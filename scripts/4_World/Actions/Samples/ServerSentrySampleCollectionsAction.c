/**
	\file ServerSentrySampleCollectionsAction.c
	\brief Implements live-action support for Sample collections action.
*/

/* class ServerSentrySampleCollectionsSettingsInput
{
    bool enabled;
    float multiplier;
    string note;

    void ServerSentrySampleCollectionsSettingsInput()
    {
        enabled = true;
        multiplier = 1.0;
        note = "";
    }
}

class ServerSentrySampleCollectionsInput
{
    ref array<string> tags;
    ref map<string, int> itemPrices;
    ref map<int, string> tierRewards;
    ref ServerSentrySampleCollectionsSettingsInput settings;

    void ServerSentrySampleCollectionsInput()
    {
        tags = new array<string>();
        itemPrices = new map<string, int>();
        tierRewards = new map<int, string>();
        settings = new ServerSentrySampleCollectionsSettingsInput();
    }
}

class ServerSentrySampleCollectionsAction: ServerSentryModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "server.sample.collections";
        d.targetType = "server";
        d.label = "Sample: Arrays + Map";
        d.icon = "ListTree";
        d.description = "Demonstrates typed string arrays, map<string, int> and a nested settings object.";
        d.scopes.Insert("live");

        ServerSentryLiveModActionInputDto tagItem = ServerSentryLiveModActionInputFactory.Text("tag", "Tag", true, "e.g. starter");
        ServerSentryLiveModActionInputDto tags = ServerSentryLiveModActionInputFactory.List("tags", "Tags", tagItem, true, 1, 50, "[\"starter\",\"featured\"]");
        tags.helpText = "A dynamically sized array<string>.";
        d.inputs.Insert(tags);

        ServerSentryLiveModActionMapKeyDto priceKey = ServerSentryLiveModActionInputFactory.StringMapKey("Item classname", "e.g. Apple");
        priceKey.maxLength = 160;
        ServerSentryLiveModActionInputDto priceValue = ServerSentryLiveModActionInputFactory.Integer("price", "Price", true, 0, 10000000, 1, "100");
        ServerSentryLiveModActionInputDto prices = ServerSentryLiveModActionInputFactory.MapInput("itemPrices", "Item prices", priceKey, priceValue, true, 1, 500, "{\"Apple\":25,\"BandageDressing\":100}");
        prices.helpText = "A map<string, int>. JSON map keys are object property names.";
        d.inputs.Insert(prices);

        ServerSentryLiveModActionMapKeyDto tierKey = ServerSentryLiveModActionInputFactory.IntegerMapKey("Tier number");
        ServerSentryLiveModActionInputDto tierValue = ServerSentryLiveModActionInputFactory.Text("reward", "Reward classname", true, "e.g. Apple");
        ServerSentryLiveModActionInputDto tiers = ServerSentryLiveModActionInputFactory.MapInput("tierRewards", "Tier rewards", tierKey, tierValue, false, 0, 20, "{\"1\":\"Apple\",\"2\":\"Mlock91\"}");
        tiers.helpText = "A map<int, string>. JSON still transports integer keys as quoted object property names.";
        d.inputs.Insert(tiers);

        array<ref ServerSentryLiveModActionInputDto> settingsFields = new array<ref ServerSentryLiveModActionInputDto>();
        settingsFields.Insert(ServerSentryLiveModActionInputFactory.Boolean("enabled", "Enabled", true));
        settingsFields.Insert(ServerSentryLiveModActionInputFactory.Number("multiplier", "Multiplier", true, 0, 100, 0.1, "1"));
        settingsFields.Insert(ServerSentryLiveModActionInputFactory.TextArea("note", "Note", false, "Optional note"));

        ServerSentryLiveModActionInputDto settings = ServerSentryLiveModActionInputFactory.Object("settings", "Settings", settingsFields, true, "{\"enabled\":true,\"multiplier\":1,\"note\":\"Sample payload\"}");
        settings.helpText = "A fixed object with typed nested fields.";
        d.inputs.Insert(settings);

        return d;
    }

    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        ServerSentrySampleCollectionsInput input;
        string error;
        if (!JsonFileLoader<ServerSentrySampleCollectionsInput>.LoadData(ctx.inputJson, input, error))
        {
            message = "Invalid typed sample input: " + error;
            return false;
        }

        if (!input || !input.tags || input.tags.Count() == 0)
        {
            message = "At least one tag is required.";
            return false;
        }

        if (!input.itemPrices || input.itemPrices.Count() == 0)
        {
            message = "At least one item price is required.";
            return false;
        }

        float multiplier = 1.0;
        bool enabled = false;
        if (input.settings)
        {
            multiplier = input.settings.multiplier;
            enabled = input.settings.enabled;
        }

        string enabledText = "false";
        if (enabled)
        {
            enabledText = "true";
        }

        int tierCount = 0;
        if (input.tierRewards)
        {
            tierCount = input.tierRewards.Count();
        }

        message = "Parsed array items=" + input.tags.Count().ToString() + ", string-key map entries=" + input.itemPrices.Count().ToString() + ", integer-key map entries=" + tierCount.ToString() + ", enabled=" + enabledText + ", multiplier=" + multiplier.ToString() + ".";
        return true;
    }
}
*/
