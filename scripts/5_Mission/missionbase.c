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
            switch (id) 
            {
                case 4771341:
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
