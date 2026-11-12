/** Shared native DayZ controls; no Dabs or COT runtime classes are used. */
class ServerSentrySupportMenuBase extends UIScriptedMenu
{
    protected ref ServerSentryMessageComposer m_Composer;
    protected bool m_OwnsMenuInputExclude;
    protected ref map<Widget, ref ServerSentryRoundedSurface> m_Surfaces = new map<Widget, ref ServerSentryRoundedSurface>;

    override void OnShow()
    {
        super.OnShow();
        PPEffects.SetBlurMenu(0.5);
        GetGame().SetKeyboardHandle(this);
        SetActiveWindow(layoutRoot, true);
        Mission mission = GetGame().GetMission();
        m_OwnsMenuInputExclude = mission && !mission.IsInputExcludeActive("menu");
        if (m_OwnsMenuInputExclude) mission.AddActiveInputExcludes({"menu"});
    }

    override void OnHide()
    {
        GetGame().SetKeyboardHandle(null);
        if (m_OwnsMenuInputExclude && GetGame().GetMission()) GetGame().GetMission().RemoveActiveInputExcludes({"menu"}, true);
        m_OwnsMenuInputExclude = false;
        super.OnHide();
        PPEffects.SetBlurMenu(0);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RestoreVisibleMenuFocus, 1, false);
    }

    /** Closing a child must return input and the cursor to the still-open menu. */
    static void RestoreVisibleMenuFocus()
    {
        UIScriptedMenu menu = GetGame().GetUIManager().GetMenu();
        if (!menu || menu.IsClosing() || !menu.IsVisible()) return;
        SetActiveWindow(menu.GetLayoutRoot(), true);
        SetFocus(menu.GetLayoutRoot());
        if (menu.UseMouse()) GetGame().GetUIManager().ShowUICursor(true);
        if (ServerSentrySupportMenuBase.Cast(menu))
        {
            PPEffects.SetBlurMenu(0.5);
            GetGame().SetKeyboardHandle(menu);
        }
    }

    protected void InitSupportControls(Widget parent = null)
    {
        if (!parent)
        {
            parent = layoutRoot;
            Widget sidebar = layoutRoot.FindAnyWidget("Sidebar");
            if (sidebar)
            {
                Widget oldSection = sidebar.FindAnyWidget("ActiveSection");
                if (oldSection) oldSection.Show(false);
                GetGame().GetWorkspace().CreateWidgets("ServerSentryDayZMod/gui/components/SupportNavigation.layout", sidebar);
            }
        }
        if (parent.GetName() == "ActiveSection_Fill") ColorSurface(parent, ARGB(255, 19, 25, 40));
        PaintSupportButton(parent, false);
        Widget child = parent.GetChildren();
        while (child)
        {
            InitSupportControls(child);
            child = child.GetSibling();
        }
    }

    protected void ColorSurface(Widget fill, int color)
    {
        ServerSentryRoundedSurface surface;
        if (!m_Surfaces.Find(fill, surface))
        {
            surface = new ServerSentryRoundedSurface(fill);
            m_Surfaces.Set(fill, surface);
        }
        surface.SetColor(color);
    }

    protected void SetSupportButtonEnabled(ButtonWidget button, bool enabled)
    {
        if (!button) return;
        bool wasEnabled = (button.GetFlags() & WidgetFlags.DISABLED) == 0;
        if (wasEnabled == enabled) return;
        button.Enable(enabled);
        PaintSupportButton(button, false);
    }

    protected bool PaintSupportButton(Widget widget, bool hovered)
    {
        ButtonWidget button = ButtonWidget.Cast(widget);
        if (!button) return false;
        Widget fill = button.FindAnyWidget(button.GetName() + "_Fill");
        TextWidget label = TextWidget.Cast(button.FindAnyWidget(button.GetName() + "_Label"));
        ImageWidget icon = ImageWidget.Cast(button.FindAnyWidget(button.GetName() + "_Icon"));
        if (!fill || (!label && !icon)) return false;

        bool primary = button.GetName() == "BtnSpawn" || button.GetName() == "BtnSubmitTicket" || button.GetName() == "BtnLinkAccount" || button.GetName() == "BtnNewTicket" || button.GetName() == "BtnSendReply";
        int foreground = ARGB(255, 217, 230, 242);
        int background = ARGB(255, 12, 17, 29);
        bool disabled = (button.GetFlags() & WidgetFlags.DISABLED) != 0;
        if (disabled)
        {
            background = ARGB(255, 9, 12, 21);
            foreground = ARGB(255, 71, 92, 115);
        }
        else if (primary)
        {
            background = ARGB(255, 65, 104, 240);
            foreground = ARGB(255, 248, 251, 255);
            if (hovered) background = ARGB(255, 84, 133, 255);
        }
        else if (hovered)
        {
            background = ARGB(255, 23, 32, 54);
            foreground = ARGB(255, 131, 221, 255);
        }
        bool reimbursements = ServerSentrySupportMenu.Cast(this) != null;
        if ((button.GetName() == "NavSupport" && !reimbursements) || (button.GetName() == "NavReimbursements" && reimbursements))
            background = ARGB(255, 23, 32, 54);
        if (!label && (disabled || !hovered)) background = ARGB(0, 0, 0, 0);
        ColorSurface(fill, background);
        if (label) label.SetColor(foreground);
        if (icon) icon.SetColor(foreground);
        return true;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        ButtonWidget reimbursements = ButtonWidget.Cast(layoutRoot.FindAnyWidget("NavReimbursements"));
        Widget tickets = layoutRoot.FindAnyWidget("NavSupport");
        if (tickets) tickets.Show(manager && manager.IsTicketsEnabled());
        if (reimbursements) reimbursements.Show(manager && manager.IsSupportCasesEnabled());
        if (manager && reimbursements) SetSupportButtonEnabled(reimbursements, !manager.IsCheckingReimbursement());
        if (!m_Composer) return;
        m_Composer.Update();
        SetSupportButtonEnabled(m_Composer.GetEmojiButton(), (m_Composer.GetInput().GetFlags() & WidgetFlags.DISABLED) == 0);
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (m_Composer && m_Composer.OnChange(w)) return true;
        return super.OnChange(w, x, y, finished);
    }

    override bool OnKeyDown(Widget w, int x, int y, int key)
    {
        if (m_Composer && m_Composer.OnKeyDown(key)) return true;
        return super.OnKeyDown(w, x, y, key);
    }

    override bool OnKeyUp(Widget w, int x, int y, int key)
    {
        if (m_Composer) m_Composer.OnKeyUp(key);
        return super.OnKeyUp(w, x, y, key);
    }

    override bool OnMouseEnter(Widget w, int x, int y)
    {
        if (m_Composer && m_Composer.OnMouseEnter(w)) return true;
        if (PaintSupportButton(w, true)) return true;
        return super.OnMouseEnter(w, x, y);
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (m_Composer && m_Composer.OnMouseLeave(w)) return true;
        if (PaintSupportButton(w, false)) return true;
        return super.OnMouseLeave(w, enterW, x, y);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w && w.GetName() == "NavSupport")
        {
            ServerSentryClientManager manager = ServerSentryClientManager.Get();
            if (manager) manager.CancelReimbursementRequest();
            ShowSupportSection(ServerSentryTicketsMenu.MENU_ID);
            return true;
        }
        if (w && w.GetName() == "NavReimbursements")
        {
            ShowSupportSection(4771341);
            return true;
        }
        if (m_Composer && m_Composer.OnClick(w))
        {
            OnChange(m_Composer.GetInput(), x, y, true);
            return true;
        }
        if (w && w.GetName() == "SupportClose")
        {
            CloseSupport();
            return true;
        }
        return super.OnClick(w, x, y, button);
    }

    protected void ShowSupportSection(int menuId)
    {
        ServerSentryClientManager access = ServerSentryClientManager.Get();
        if (!access || (menuId == 4771341 && !access.IsSupportCasesEnabled()) || (menuId != 4771341 && !access.IsTicketsEnabled())) return;
        UIMenuPanel section = this;
        UIMenuPanel child;
        while (section && ServerSentrySupportMenuBase.Cast(section))
        {
            if (section.GetID() == menuId)
            {
                if (child) child.Close();
                return;
            }
            child = section;
            section = section.GetParentMenu();
        }
        ServerSentryClientManager manager = ServerSentryClientManager.Get();
        if (manager && menuId == 4771341) manager.RequestReimbursement();
    }

    protected void CloseSupport()
    {
        UIMenuPanel root = this;
        while (ServerSentrySupportMenuBase.Cast(root.GetParentMenu())) root = root.GetParentMenu();
        root.Close();
    }
}
