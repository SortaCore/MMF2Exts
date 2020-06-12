###How to compile zlib from source:###

1. Download [latest zlib](http://zlib.net/)
2. Extract all.
3. Open (extracted zlib)/win32/Makefile.msc in a text editor.
4. Prepend to `WFLAGS` line the define: `-D_USING_V110_SDK71_`. Example:
`WFLAGS  = -D_USING_V110_SDK71_ -D...`
This will make it target Windows XP compiler.
5. In same MSC file, edit the `CFLAGS` line, replace `-MD` with `-MT`.
This will make it not reliant on VS Redistributable.
6. Save and close the msc file.
7. Open VS Command Prompt for VS 2019 x86 Native, in start menu.
8. CD into the extracted zlib folder.
9. Then run:
`nmake /f win32/Makefile.msc staticlib`
10. Job done. See **zlib.lib**, **zconf.h**, and **zlib.h** in the zlib folder (where you CD'd). Don't use zdll.lib!
11. Note: to rebuild/clean, use:
`nmake /f win32/Makefile.msc clean`
