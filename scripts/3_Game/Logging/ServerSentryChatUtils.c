/**
	\file ServerSentryChatUtils.c
	\brief Provides logging support for Chat utils.
*/

/**
	\brief Provides ServerSentry behavior for Chat utils.
*/
class ServerSentryChatUtils
{
    /**
    	\brief Normalizes message.
    	\param message \p receives a human-readable result or error message.
    	\return \p The resulting string value.
    */
    static string NormalizeMessage(string message)
    {
        if (message == "")
        {
            return "";
        }

        if (message.Length() > 0 && message[0] == "+")
        {
            return message.Substring(1, message.Length() - 1);
        }

        return message;
    }

    /**
    	\brief Returns whether command.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsCommand(string message)
    {
        if (message == "")
        {
            return false;
        }

        return message[0] == "!" || message[0] == "/" || message[0] == "#";
    }
}
