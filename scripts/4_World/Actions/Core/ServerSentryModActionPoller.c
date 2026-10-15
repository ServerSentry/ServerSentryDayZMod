/**
	\file ServerSentryModActionPoller.c
	\brief Implements live-action support for Mod action poller.
*/

/**
	\brief Polls and executes requests for Mod action.
*/
class ServerSentryModActionPoller
{
    /**
    	\brief Shared started state used by all instances.
    */
    protected static bool s_Started;
    /**
    	\brief Shared processing state used by all instances.
    */
    protected static ref map<string, bool> s_Processing;

    /**
    	\brief Processes the start operation.
    */
    static void Start()
    {
        if (s_Started)
        {
            return;
        }

        if (!GetGame() || !GetGame().IsServer())
        {
            return;
        }

        EnsureState();
        EnsureDirectories();
        s_Started = true;
        Print("[ServerSentry][LiveAction] Poller started. Inbox=" + InboxDirectory() + " Outbox=" + OutboxDirectory());
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Poll, 1000, true);
    }

    /**
    	\brief Polls the requested operation.
    */
    static void Poll()
    {
        if (!GetGame() || !GetGame().IsServer())
        {
            return;
        }

        EnsureState();
        if (!EnsureDirectories())
        {
            return;
        }

        string fileName;
        FileAttr fileAttr;
        string pattern = InboxDirectory() + "/*.json";
        // CF.FindFileEx applies the DayZ 1.30 filesystem workaround (DZEXP-134).
        FindFileHandle handle = CF.FindFileEx(pattern, fileName, fileAttr, FindFileFlags.ALL);
        if (!handle)
        {
            return;
        }

        // Finish enumerating before executing actions that delete inbox files.
        array<string> fileNames = new array<string>();
        while (true)
        {
            if (fileName != "")
            {
                fileNames.Insert(fileName);
            }

            if (!FindNextFile(handle, fileName, fileAttr))
            {
                break;
            }
        }

        CloseFindFile(handle);

        foreach (string pendingFileName : fileNames)
        {
            ProcessFile(InboxDirectory() + "/" + pendingFileName);
        }
    }

    /**
    	\brief Processes file.
    	\param filePath \p path of the file to read or write.
    */
    protected static void ProcessFile(string filePath)
    {
        if (filePath == "")
        {
            return;
        }

        if (s_Processing.Contains(filePath))
        {
            return;
        }

        s_Processing.Set(filePath, true);

        ServerSentryLiveActionRequestDto request = new ServerSentryLiveActionRequestDto();
        bool loaded = false;
        string message = "";

        if (!FileExist(filePath))
        {
            s_Processing.Remove(filePath);
            return;
        }

        JsonFileLoader<ServerSentryLiveActionRequestDto>.JsonLoadFile(filePath, request);
        loaded = request && request.actionId != "";
        if (!loaded || !request || request.actionId == "")
        {
            message = "Invalid live action JSON file.";
            Print("[ServerSentry][LiveAction] " + message + " File=" + filePath);
            WriteResult(request, false, message);
            DeleteFile(filePath);
            s_Processing.Remove(filePath);
            return;
        }

        Print("[ServerSentry][LiveAction] Picked up action file. File=" + filePath + " requestId=" + request.requestId + " actionId=" + request.actionId);

        ServerSentryModActionContext ctx = ToContext(request);
        bool success = ServerSentryModActionRegistry.Execute(ctx, message);
        WriteResult(request, success, message);
        DeleteFile(filePath);
        s_Processing.Remove(filePath);
    }

    /**
    	\brief Converts to context.
    	\param request \p request data to process.
    	\return \p The resulting mod action context instance, or null when unavailable.
    */
    protected static ServerSentryModActionContext ToContext(ServerSentryLiveActionRequestDto request)
    {
        ServerSentryModActionContext ctx = new ServerSentryModActionContext();
        ctx.requestId = request.requestId;
        ctx.fulfillmentKey = request.fulfillmentKey;
        ctx.actionId = request.actionId;
        ctx.inputJson = request.inputJson;
        ctx.requestedBy = request.requestedBy;

        if (request.target)
        {
            ctx.targetType = request.target.type;
            ctx.objectId = request.target.objectId;
            ctx.steamId = request.target.steamId;
            ctx.bohemiaId = request.target.bohemiaId;
            ctx.className = request.target.className;
            if (request.target.position)
            {
                ctx.position = request.target.position;
            }
        }

        return ctx;
    }

    /**
    	\brief Writes result.
    	\param request \p request data to process.
    	\param success \p true to enable success.
    	\param message \p receives a human-readable result or error message.
    */
    protected static void WriteResult(ServerSentryLiveActionRequestDto request, bool success, string message)
    {
        ServerSentryLiveActionResultDto result = new ServerSentryLiveActionResultDto();
        if (request)
        {
            result.requestId = request.requestId;
            result.actionId = request.actionId;
            if (request.target)
            {
                result.targetType = request.target.type;
                result.targetObjectId = request.target.objectId;
            }
        }

        result.success = success;
        result.message = message;
        result.executedAtUtc = ServerSentryTime.NowUtc();

        string requestId = result.requestId;
        if (requestId == "")
        {
            requestId = "unknown-" + GetGame().GetTime().ToString();
        }

        string resultPath = OutboxDirectory() + "/" + requestId + ".result.json";
        JsonFileLoader<ServerSentryLiveActionResultDto>.JsonSaveFile(resultPath, result);
        string successText = "false";
        if (success)
        {
            successText = "true";
        }

        Print("[ServerSentry][LiveAction] Wrote action result. File=" + resultPath + " success=" + successText + " message=" + message);
    }

    /**
    	\brief Initializes the registry collections when they are first used.
    */
    protected static void EnsureState()
    {
        if (!s_Processing)
        {
            s_Processing = new map<string, bool>();
        }
    }

    /**
    	\brief Creates the fulfillment storage directories when necessary.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected static bool EnsureDirectories()
    {
        if (!FileExist(ServerSentryDefaults.RootDirectory()))
        {
            if (!MakeDirectory(ServerSentryDefaults.RootDirectory()))
            {
                Print("[ServerSentry][LiveAction] Failed to create root directory: " + ServerSentryDefaults.RootDirectory());
                return false;
            }
        }

        if (!FileExist(ActionsDirectory()))
        {
            if (!MakeDirectory(ActionsDirectory()))
            {
                Print("[ServerSentry][LiveAction] Failed to create actions directory: " + ActionsDirectory());
                return false;
            }
        }

        if (!FileExist(InboxDirectory()))
        {
            if (!MakeDirectory(InboxDirectory()))
            {
                Print("[ServerSentry][LiveAction] Failed to create actions inbox directory: " + InboxDirectory());
                return false;
            }
        }

        if (!FileExist(OutboxDirectory()))
        {
            if (!MakeDirectory(OutboxDirectory()))
            {
                Print("[ServerSentry][LiveAction] Failed to create actions outbox directory: " + OutboxDirectory());
                return false;
            }
        }

        return true;
    }

    /**
    	\brief Processes the actions directory operation.
    	\return \p The resulting string value.
    */
    static string ActionsDirectory()
    {
        return "$profile:ServerSentry/actions";
    }

    /**
    	\brief Processes the inbox directory operation.
    	\return \p The resulting string value.
    */
    static string InboxDirectory()
    {
        return "$profile:ServerSentry/actions/inbox";
    }

    /**
    	\brief Processes the outbox directory operation.
    	\return \p The resulting string value.
    */
    static string OutboxDirectory()
    {
        return "$profile:ServerSentry/actions/outbox";
    }
}
