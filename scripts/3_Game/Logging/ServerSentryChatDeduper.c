/**
	\file ServerSentryChatDeduper.c
	\brief Provides logging support for Chat deduper.
*/

/**
	\brief Provides ServerSentry behavior for Chat deduper.
*/
class ServerSentryChatDeduper
{
    /**
    	\brief Shared recent state used by all instances.
    */
    protected static ref map<string, int> s_Recent;

    /**
    	\brief Returns whether emit.
    	\param data \p data processed by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool ShouldEmit(ServerSentryChatMessageData data)
    {
        if (!data)
        {
            return false;
        }

        if (data.message == "")
        {
            return false;
        }

        if (!s_Recent)
        {
            s_Recent = new map<string, int>();
        }

        int now = 0;
        if (GetGame())
        {
            now = GetGame().GetTime();
        }

        string key = BuildKey(data);
        int previous;
        if (s_Recent.Find(key, previous))
        {
            if (now - previous <= ServerSentryDefaults.ChatDedupWindowMs())
            {
                return false;
            }
        }

        s_Recent.Set(key, now);
        Cleanup(now);
        return true;
    }

    /**
    	\brief Builds key.
    	\param data \p data processed by the operation.
    	\return \p The resulting string value.
    */
    protected static string BuildKey(ServerSentryChatMessageData data)
    {
        string senderKey = data.senderSteamId;
        if (senderKey == "")
        {
            senderKey = data.senderBohemiaId;
        }
        if (senderKey == "")
        {
            senderKey = data.senderName;
        }

        return data.channelId.ToString() + "|" + senderKey + "|" + data.message;
    }

    /**
    	\brief Processes the cleanup operation.
    	\param now \p now used by the operation.
    */
    protected static void Cleanup(int now)
    {
        if (!s_Recent)
        {
            return;
        }

        if (s_Recent.Count() <= 128)
        {
            return;
        }

        array<string> removeKeys = new array<string>();
        foreach (string key, int seenAt: s_Recent)
        {
            if (now - seenAt > ServerSentryDefaults.ChatDedupWindowMs() * 4)
            {
                removeKeys.Insert(key);
            }
        }

        foreach (string removeKey: removeKeys)
        {
            s_Recent.Remove(removeKey);
        }
    }
}
