ServerSentry's bundled Unicode emoji images use Twemoji v17.0.3.
Source: https://github.com/jdecked/twemoji/tree/v17.0.3
Copyright Twitter, Inc. and other contributors.
Graphics license: CC BY 4.0, https://creativecommons.org/licenses/by/4.0/
The images were resized and packed into DayZ PAA atlases. See LICENSE-TWEMOJI.txt.
Each 32-pixel sprite includes a transparent 2-pixel inset to protect its contour when scaled.

These assets add no runtime mod dependency and need no HTTP downloads in game.
Custom Discord server emoji are shown by name when no local image is available.

To regenerate: download the v17.0.3 source archive into .workbench/emoji-source,
then run workbench/Build-EmojiAssets.ps1 with DayZ Tools installed.
Commit the generated PAA and imageset files together with the matching Agent catalog.

Picker names and categories: Unicode 17.0 emoji-test.txt
https://www.unicode.org/Public/17.0.0/emoji/emoji-test.txt
Copyright Unicode, Inc. See LICENSE-UNICODE.txt.
EmojiPicker.tsv contains the 3,953 standard keyboard entries; legacy non-RGI images
remain available for displaying existing messages. All 270 flags have their own tab.

The catalog is UTF-8 TSV without a header. Its six columns are shortcode, name,
category, atlas number, sprite name and editable glyph. ServerSentryEmojiCatalog
loads it once with OpenFile/FGets, splits each line on tabs and retains the lookup
maps in memory. Keep the .tsv file in the PBO alongside the imagesets and textures.

After regenerating atlases, download emoji-test.txt to
.workbench/emoji-source/emoji-test-17.0.txt and run workbench/Build-EmojiPicker.ps1
with -MarkdigAssembly pointing to the Agent's installed Markdig.dll.
Copy .workbench/emoji-source/EmojiCatalog.tsv to the Agent's DayZ/Formatting directory.
The third column is the picker shortcode. Deploy the updated Agent with the mod.
