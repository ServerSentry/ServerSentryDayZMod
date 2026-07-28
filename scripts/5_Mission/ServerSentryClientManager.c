/**
	\file ServerSentryClientManager.c
	\brief Defines ServerSentry support for Client manager.
*/

[CF_RegisterModule(ServerSentryClientManager)]
/**
	\brief Coordinates the lifecycle of Client.
*/
class ServerSentryClientManager: CF_ModuleWorld
{
    /**
    	\brief Instance state for menu.
    */
    protected ref ServerSentrySupportMenu m_Menu;
    /**
    	\brief Instance state for is waiting for response.
    */
    protected bool m_IsWaitingForResponse;

    /**
    	\brief Handles the DayZ Init callback.
    */
    override void OnInit()
    {
        Print("On Init Client Menu.");
        super.OnInit();
        m_CF_Bindings.Bind("RequestSupportCaseFromServer", "UAToggleSupportMenu", false);

        GetRPCManager().AddRPC("ServerSentryBot", "SupportCaseResponse", this, SingleplayerExecutionType.Client);
    }

    /**
    	\brief Processes the support case response operation.
    	\param type \p type name or value type used by the operation.
    	\param ctx \p action context containing request, target, and input data.
    	\param sender \p identity of the RPC sender.
    	\param target \p target object associated with the operation.
    */
    void SupportCaseResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client)
            return;

        Print("[ServerSentry] -> recived server data!");
        m_IsWaitingForResponse = false;

        Print("[ServerSentry] -> chance waiting state!");
        Param1<ServerSentrySupportCase> data;
        if (!ctx.Read(data))
        {
            Print("[ServerSentry] -> nope ctx cant read!");
            return;
        }

        if (!data.param1)
        {
            Print("Player has not case active! skipping!");
            return;
        }

        OpenMenu();
        if(!m_Menu) return;

        m_Menu.SetCaseData(data.param1);
    }

    /**
    	\brief Opens menu.
    */
    void OpenMenu()
    {
        if(GetGame().GetUIManager().GetMenu() == NULL && m_Menu == null)
        {
            m_Menu = ServerSentrySupportMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(4771341, null));
        }
        else
        {
            if(GetGame().GetUIManager().GetMenu() == NULL && !m_Menu.IsVisible())
            {
                GetGame().GetUIManager().ShowScriptedMenu(m_Menu, NULL);
            }
        }
    }

    /**
    	\brief Requests support case from server.
    */
    void RequestSupportCaseFromServer()
    {
        Print("[ServerSentry] -> Pressed input!");
        if (m_IsWaitingForResponse) return;

        Print("[ServerSentry] -> not waiting for response!");

        m_IsWaitingForResponse = true;
        GetRPCManager().SendRPC("ServerSentryBot", "GetSupportCase", null, true);
    }
};
