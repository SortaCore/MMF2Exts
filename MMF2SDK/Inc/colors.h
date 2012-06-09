
// Color definitions
// -----------------

#ifndef	_H2INC

	// RGB values
	#define	BLACK		RGB(0x00,0x00,0x00)
	#define	DARK_RED	RGB(0x80,0x00,0x00)
	#define	DARK_GREEN	RGB(0x00,0x80,0x00)
	#define	DARK_BLUE	RGB(0x00,0x00,0x80)
	#define	DARK_GRAY	RGB(0x80,0x80,0x80)
	#define	LIGHT_GRAY	RGB(0xC0,0xC0,0xC0)
	#define	RED			RGB(0xFF,0x00,0x00)
	#define	GREEN		RGB(0x00,0xFF,0x00)
	#define	BLUE		RGB(0x00,0x00,0xFF)
	#define	YELLOW		RGB(0xFF,0xFF,0x00)
	#define	WHITE		RGB(0xFF,0xFF,0xFF)
	#define	MAGENTA		RGB(0xFF,0x00,0xFF)
	#define	CYAN		RGB(0x00,0xFF,0xFF)

	#define	OPAQUE_BLACK	RGB(8,0,0)

	// System colors
	#define	SYSCOLOR_LIGHTGRAY	GetSysColor(COLOR_BTNFACE)
	#define	SYSCOLOR_DARKGRAY	GetSysColor(COLOR_BTNSHADOW)
	#define	SYSCOLOR_WHITE		GetSysColor(COLOR_BTNHIGHLIGHT)
	#define	SYSCOLOR_BLACK		GetSysColor(COLOR_BTNTEXT)
	#define	SYSCOLOR_WINDOW		GetSysColor(COLOR_WINDOW)
	#define	SYSCOLOR_WINDOWTEXT	GetSysColor(COLOR_WINDOWTEXT)
	#define	SYSCOLOR_BKSELECT	GetSysColor(COLOR_HIGHLIGHT)
	#define	SYSCOLOR_TXTSELECT	GetSysColor(COLOR_HIGHLIGHTTEXT)

#else
	#define	BLACK			0x000000
	#define	DARK_RED		0x000080
	#define	DARK_GREEN		0x008000
	#define	DARK_BLUE		0x800000
	#define	DARK_GRAY		0x808080
	#define	LIGHT_GRAY		0xC0C0C0
	#define	LIGHT_RED		0x0000FF
	#define	LIGHT_GREEN		0x00FF00
	#define	LIGHT_BLUE		0xFF0000
	#define	YELLOW			0x00FFFF
	#define	WHITE			0xFFFFFF

	#define	OPAQUE_BLACK	0x000001
#endif

