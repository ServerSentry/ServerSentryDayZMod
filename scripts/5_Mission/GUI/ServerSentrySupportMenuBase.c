/** Shared native DayZ controls; no Dabs or COT runtime classes are used. */
class ServerSentrySupportMenuBase extends UIScriptedMenu
{
    protected void SetSupportButtonEnabled(ButtonWidget button, bool enabled)
    {
        if (!button) return;
        button.Enable(enabled);
        PaintSupportButton(button, false);
    }

    protected bool PaintSupportButton(Widget widget, bool hovered)
    {
        ButtonWidget button = ButtonWidget.Cast(widget);
        if (!button) return false;
        Widget fill = button.FindAnyWidget(button.GetName() + "_Fill");
        TextWidget label = TextWidget.Cast(button.FindAnyWidget(button.GetName() + "_Label"));
        if (!fill || !label) return false;

        bool primary = button.GetName() == "BtnSpawn" || button.GetName() == "BtnSubmitTicket";
        if (button.GetFlags() & WidgetFlags.DISABLED)
        {
            fill.SetColor(ARGB(255, 3, 5, 7));
            label.SetColor(ARGB(255, 71, 92, 115));
        }
        else if (primary)
        {
            if (hovered)
                fill.SetColor(ARGB(255, 23, 140, 100));
            else
                fill.SetColor(ARGB(255, 14, 107, 74));
            label.SetColor(ARGB(255, 1, 4, 3));
        }
        else
        {
            if (hovered)
                fill.SetColor(ARGB(255, 10, 16, 23));
            else
                fill.SetColor(ARGB(255, 5, 7, 10));
            label.SetColor(ARGB(255, 217, 230, 242));
        }
        return true;
    }

    override bool OnMouseEnter(Widget w, int x, int y)
    {
        if (PaintSupportButton(w, true)) return true;
        return super.OnMouseEnter(w, x, y);
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (PaintSupportButton(w, false)) return true;
        return super.OnMouseLeave(w, enterW, x, y);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w && w.GetName() == "SupportClose")
        {
            Close();
            return true;
        }
        return super.OnClick(w, x, y, button);
    }
}
