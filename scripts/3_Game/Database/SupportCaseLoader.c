/**
	\file SupportCaseLoader.c
	\brief Provides persistence support for Support case loader.
*/

/**
	\brief Loads and persists data for Support case.
*/
class ServerSentrySupportCaseLoader
{
    /**
    	\brief Constant value for dir.
    */
    static const string SUPPORT_CASE_DIR = "$profile:ServerSentry/SupportCases";

    /**
    	\brief Loads the fulfillment receipt stored for a key.
    	\param steamId \p Steam ID of the target player.
    	\return \p The resulting support case instance, or null when unavailable.
    */
    static ServerSentrySupportCase Load(string steamId)
    {   
        if (!FileExist(SUPPORT_CASE_DIR))
        {
            MakeDirectory(SUPPORT_CASE_DIR);
            Print("[Server Sentry] Created directory: " + SUPPORT_CASE_DIR);
            return null;
        }

        string fileName = SUPPORT_CASE_DIR + "/" + steamId + ".json";
        if (!FileExist(fileName))
        {
            return null;
        }

        ServerSentrySupportCase supportCase;
        JsonFileLoader<ServerSentrySupportCase>.JsonLoadFile(fileName, supportCase);

        if(!supportCase) 
            return null;

        return supportCase;
    }

    /**
    	\brief Deletes the requested operation.
    	\param steamId \p Steam ID of the target player.
    */
    static void Delete(string steamId)
    {
        string fileName = SUPPORT_CASE_DIR + "/" + steamId + ".json";
        DeleteFile(fileName);
    }
};
