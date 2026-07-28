/**
	\file ServerSentrySampleObjectMapAction.c
	\brief Implements live-action support for Sample object map action.
*/

/* class ServerSentrySampleRewardConfigInput
{
    int credits;
    bool enabled;
    ref array<string> items;

    void ServerSentrySampleRewardConfigInput()
    {
        credits = 0;
        enabled = true;
        items = new array<string>();
    }
}

class ServerSentrySampleObjectMapInput
{
    ref map<string, ref ServerSentrySampleRewardConfigInput> rewards;
    ref ServerSentryShopActionMetadataInput _serverSentry;

    void ServerSentrySampleObjectMapInput()
    {
        rewards = new map<string, ref ServerSentrySampleRewardConfigInput>();
        _serverSentry = null;
    }
}

class ServerSentrySampleObjectMapAction: ServerSentryModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "shop.sample_reward_map";
        d.targetType = "player";
        d.label = "Sample: Map of Objects";
        d.icon = "Braces";
        d.description = "Demonstrates map<string, ref object> with nested arrays. It validates the payload without changing gameplay.";
        d.scopes.Insert("shop");
        d.scopes.Insert("live");

        array<ref ServerSentryLiveModActionInputDto> rewardFields = new array<ref ServerSentryLiveModActionInputDto>();
        rewardFields.Insert(ServerSentryLiveModActionInputFactory.Integer("credits", "Credits", true, 0, 100000000, 1, "100"));
        rewardFields.Insert(ServerSentryLiveModActionInputFactory.Boolean("enabled", "Enabled", true));

        ServerSentryLiveModActionInputDto rewardItem = ServerSentryLiveModActionInputFactory.Text("item", "Item classname", true, "e.g. Apple");
        rewardFields.Insert(ServerSentryLiveModActionInputFactory.List("items", "Items", rewardItem, false, 0, 100, "[]"));

        ServerSentryLiveModActionInputDto rewardObject = ServerSentryLiveModActionInputFactory.Object("reward", "Reward configuration", rewardFields, true, "{\"credits\":100,\"enabled\":true,\"items\":[\"Apple\"]}");

        ServerSentryLiveModActionMapKeyDto rewardKey = ServerSentryLiveModActionInputFactory.StringMapKey("Reward id", "e.g. starter");
        rewardKey.maxLength = 80;
        ServerSentryLiveModActionInputDto rewards = ServerSentryLiveModActionInputFactory.MapInput("rewards", "Rewards", rewardKey, rewardObject, true, 1, 100, "{\"starter\":{\"credits\":100,\"enabled\":true,\"items\":[\"Apple\",\"BandageDressing\"]}}");
        rewards.helpText = "Each dynamic map key points to the same typed object schema.";
        d.inputs.Insert(rewards);
        return d;
    }

    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        ServerSentrySampleObjectMapInput input;
        string error;
        if (!JsonFileLoader<ServerSentrySampleObjectMapInput>.LoadData(ctx.inputJson, input, error))
        {
            message = "Invalid reward map input: " + error;
            return false;
        }

        if (!input || !input.rewards || input.rewards.Count() == 0)
        {
            message = "At least one reward entry is required.";
            return false;
        }

        int enabledCount = 0;
        int itemCount = 0;
        for (int i = 0; i < input.rewards.Count(); i++)
        {
            ServerSentrySampleRewardConfigInput reward = input.rewards.GetElement(i);
            if (!reward)
            {
                continue;
            }

            if (reward.enabled)
            {
                enabledCount++;
            }

            if (reward.items)
            {
                itemCount = itemCount + reward.items.Count();
            }
        }

        message = "Parsed reward map entries=" + input.rewards.Count().ToString() + ", enabled=" + enabledCount.ToString() + ", nested items=" + itemCount.ToString() + ".";
        return true;
    }
}
*/
