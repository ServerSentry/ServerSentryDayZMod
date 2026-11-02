/**
	\file missionbase.c
	\brief Defines ServerSentry support for Missionbase.
*/

/**
	\brief Extends the DayZ Mission base class with ServerSentry integration.
*/
modded class MissionBase 
{
    /**
    	\brief Creates scripted menu.
    	\param id \p identifier used by the operation.
    	\return \p The resulting UI scripted menu instance, or null when unavailable.
    */
    override UIScriptedMenu CreateScriptedMenu (int id) 
    {
        UIScriptedMenu menu = NULL;
        menu = super.CreateScriptedMenu(id);
        if (!menu) 
        {
            ServerSentryClientManager manager = ServerSentryClientManager.Get();
            switch (id) 
            {
                case ServerSentryTicketConversationMenu.MENU_ID:
                    if (!manager || !manager.IsTicketsEnabled()) break;
                    menu = new ServerSentryTicketConversationMenu;
                break;
                case ServerSentryTicketsMenu.MENU_ID:
                    if (!manager || !manager.IsTicketsEnabled()) break;
                    menu = new ServerSentryTicketsMenu;
                break;
                case ServerSentrySupportDeskMenu.MENU_ID:
                    if (!manager || !manager.IsTicketsEnabled()) break;
                    menu = new ServerSentrySupportDeskMenu;
                break;
                case 4771341:
                    if (!manager || !manager.IsSupportCasesEnabled()) break;
                    menu = new ServerSentrySupportMenu;
                break;
            }

            if (menu)
            {
                menu.SetID(id);
            }
        }
        return menu;
    }
};
