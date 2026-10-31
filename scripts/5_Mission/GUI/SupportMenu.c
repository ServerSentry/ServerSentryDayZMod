/**
	\file SupportMenu.c
	\brief Implements the Support menu user interface.
*/

/**
	\brief Provides ServerSentry behavior for Support menu.
*/
class ServerSentrySupportMenu extends ServerSentrySupportMenuBase
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
    protected TextWidget            m_CaseIdLbl;
    protected TextWidget            m_ItemCountLbl;
    protected MultilineTextWidget   m_ClaimStatus;
    protected WrapSpacerWidget     m_ItemsList;
    protected bool                 m_CanClaim;
    protected bool                 m_IsClaiming;

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
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/SupportMenu.layout");
        if (!layoutRoot)
            return null;
        m_SpawnItemsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BtnSpawn"));
        m_CloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseBtn"));
        m_WelcomeLbl = TextWidget.Cast(layoutRoot.FindAnyWidget("PlayerName"));

        m_NoteBox = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("NoteValue"));
        m_CaseIdLbl = TextWidget.Cast(layoutRoot.FindAnyWidget("CaseId"));
        m_ItemCountLbl = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemCount"));
        m_ClaimStatus = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("ClaimStatus"));
        m_ItemsList = WrapSpacerWidget.Cast(layoutRoot.FindAnyWidget("CaseItemsList"));
        SetSupportButtonEnabled(m_SpawnItemsBtn, false);

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
        m_CanClaim = false;
        m_IsClaiming = false;
        ClearItemRows();

        int itemCount = 0;
        if (m_CaseData && m_CaseData.Items)
        {
            foreach (ServerSentrySupportCaseItem item: m_CaseData.Items)
            {
                if (!item || item.ClassName == "")
                    continue;
                itemCount++;
                AddItemRow(item, false);

                if (item.Attachments && m_ItemsList)
                {
                    foreach (ServerSentrySupportCaseItem attachment: item.Attachments)
                    {
                        if (attachment && attachment.ClassName != "")
                            AddItemRow(attachment, true);
                    }
                }
            }
        }

        m_CanClaim = itemCount > 0;
        SetSupportButtonEnabled(m_SpawnItemsBtn, m_CanClaim);
        if (m_ItemsList)
            m_ItemsList.Update();
        if (m_ItemCountLbl)
            m_ItemCountLbl.SetText(itemCount.ToString() + " ITEMS");
        if (m_CaseIdLbl)
        {
            string caseLabel = "SUPPORT CASE";
            if (m_CaseData && m_CaseData.SupportCaseId != "")
                caseLabel = m_CaseData.SupportCaseId;
            m_CaseIdLbl.SetText(caseLabel);
        }
        if (m_NoteBox)
        {
            string reason = "No additional details were provided.";
            if (m_CaseData && m_CaseData.Reason != "")
                reason = m_CaseData.Reason;
            m_NoteBox.SetText(reason);
        }
        if (m_ClaimStatus)
        {
            if (m_CanClaim)
                m_ClaimStatus.SetText("Items are delivered to your inventory, or placed at your feet if it is full.");
            else
                m_ClaimStatus.SetText("There are no items available to claim in this case.");
        }
        DayZPlayer localPlayer = DayZPlayer.Cast(GetGame().GetPlayer());
        if (m_WelcomeLbl && localPlayer && localPlayer.GetIdentity())
            m_WelcomeLbl.SetText(localPlayer.GetIdentity().GetName());
    }

    protected void ClearItemRows()
    {
        if (!m_ItemsList) return;
        Widget row = m_ItemsList.GetChildren();
        while (row)
        {
            Widget next = row.GetSibling();
            row.Unlink();
            row = next;
        }
    }

    protected void AddItemRow(ServerSentrySupportCaseItem item, bool attachment)
    {
        if (!m_ItemsList) return;
        Widget row = GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/SupportItemRow.layout", m_ItemsList);
        if (!row) return;
        TextWidget name = TextWidget.Cast(row.FindAnyWidget("ItemName"));
        TextWidget meta = TextWidget.Cast(row.FindAnyWidget("ItemMeta"));
        TextWidget tag = TextWidget.Cast(row.FindAnyWidget("ItemTag"));
        if (name) name.SetText(GetItemLabel(item));
        if (attachment)
        {
            if (meta) meta.SetText("Included attachment");
            if (tag) tag.SetText("INCLUDED");
        }
    }

    /** Uses the server-supplied name, falling back to the item class. */
    protected string GetItemLabel(ServerSentrySupportCaseItem item)
    {
        if (item.DisplayName != "")
            return item.DisplayName;
        return item.ClassName;
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
        if(m_CloseBtn && w == m_CloseBtn)
        {
            GetGame().GetUIManager().Back();
            return true;
        }
        else if(m_SpawnItemsBtn && w == m_SpawnItemsBtn)
        {
            if (!m_CanClaim || m_IsClaiming)
                return true;
            m_IsClaiming = true;
            SetSupportButtonEnabled(m_SpawnItemsBtn, false);
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
		SetFocus( layoutRoot );
    }

    /**
    	\brief Handles the DayZ Hide callback.
    */
    override void OnHide()
	{
		super.OnHide();
		PPEffects.SetBlurMenu(0);
	}
};
