/**
	\file SupportMenu.c
	\brief Implements the Support menu user interface.
*/

/**
	\brief Provides ServerSentry behavior for Support menu.
*/
class ServerSentrySupportMenu extends UIScriptedMenu
{
    /**
    	\brief Instance state for spawn items btn.
    */
    protected ButtonWidget          m_SpawnItemsBtn;
    /**
    	\brief Instance state for close btn.
    */
    protected ButtonWidget          m_CloseBtn;
    /**
    	\brief Instance state for note box.
    */
    protected MultilineTextWidget   m_NoteBox;
    /**
    	\brief Instance state for welcome lbl.
    */
    protected TextWidget            m_WelcomeLbl;

    /**
    	\brief Instance state for case data.
    */
    protected ref ServerSentrySupportCase m_CaseData;

    /**
    	\brief Processes the init operation.
    	\return \p The resulting widget instance, or null when unavailable.
    */
    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("ServerSentryMod/gui/SupportMenu.layout");
        m_SpawnItemsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnSpawn"));
        m_CloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseBtn"));
        m_WelcomeLbl = TextWidget.Cast(layoutRoot.FindAnyWidget("PlayerName"));

        m_NoteBox = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("NoteValue"));

        return layoutRoot;
    }

    /**
    	\brief Updates the requested operation.
    	\param timeslice \p elapsed frame time supplied by DayZ.
    */
    override void Update(float timeslice)
	{
        super.Update(timeslice);
		if (GetUApi().GetInputByID(UAUIBack).LocalPress())
		{
			GetGame().GetUIManager().Back();
		}
    }

    /**
    	\brief Sets case data.
    	\param data \p data processed by the operation.
    */
    void SetCaseData(ServerSentrySupportCase data)
    {
        m_CaseData = data;
        m_NoteBox.SetText(m_CaseData.Reason);
        DayZPlayer localPlayer = DayZPlayer.Cast(GetGame().GetPlayer());
        m_WelcomeLbl.SetText("Welcome: " + localPlayer.GetIdentity().GetName());
    }


    /**
    	\brief Handles the DayZ Click callback.
    	\param w \p w used by the operation.
    	\param x \p x used by the operation.
    	\param y \p y used by the operation.
    	\param button \p button used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool OnClick(Widget w, int x, int y, int button)
	{
        if(w == m_CloseBtn)
        {
            GetGame().GetUIManager().Back();
            return true;
        }
        else if(w == m_SpawnItemsBtn)
        {
            GetRPCManager().SendRPC("ServerSentryBot", "SpawnSupportCase", null, true);
            GetGame().GetUIManager().Back();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    /**
    	\brief Handles the DayZ Show callback.
    */
    override void OnShow() 
    {
        super.OnShow();
		PPEffects.SetBlurMenu(0.5);
		GetGame().GetInput().ChangeGameFocus(1);
		SetFocus( layoutRoot );
		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
        GetGame().GetUIManager().ShowUICursor( true );
    }

    /**
    	\brief Handles the DayZ Hide callback.
    */
    override void OnHide()
	{
		super.OnHide();
		PPEffects.SetBlurMenu(0);
		GetGame().GetInput().ResetGameFocus();
		GetGame().GetMission().PlayerControlEnable(false);
        GetGame().GetUIManager().ShowUICursor( false );
	}
};
