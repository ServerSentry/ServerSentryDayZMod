Composer16 derives from the installed stable DayZ MetronBook16 bitmap font.
The original glyphs and texture remain unchanged. Additional private-use glyphs
reserve transparent 18px slots; the composer draws the matching emoji images.
These slots are UI-only: drafts, validation and sending use decoded shortcodes.

Regenerate after changing EmojiPicker.txt:
  BankRev.exe -prefix -f .workbench/font-source <DayZ>/dta/gui.pbo
  python workbench/Build-ComposerFont.py .workbench/font-source/gui/fonts

The source font and texture are Bohemia Interactive game assets, for use with DayZ.
