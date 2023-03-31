//
//  Menus.h
//  MacRuntime
//

#import <Foundation/Foundation.h>
@class CArrayList;
@class CRunApp;
@class CFile;

#define MF_INSERT          0x0000
#define MF_CHANGE          0x0080
#define MF_APPEND          0x0100
#define MF_DELETE          0x0200
#define MF_REMOVE          0x1000
#define MF_BYCOMMAND       0x0000
#define MF_BYPOSITION      0x0400
#define MF_SEPARATOR       0x0800
#define MF_ENABLED         0x0000
#define MF_GRAYED          0x0001
#define MF_DISABLED        0x0002
#define MF_UNCHECKED       0x0000
#define MF_CHECKED         0x0008
#define MF_USECHECKBITMAPS 0x0200
#define MF_STRING          0x0000
#define MF_BITMAP          0x0004
#define MF_OWNERDRAW       0x0100
#define MF_POPUP           0x0010
#define MF_MENUBARBREAK    0x0020
#define MF_MENUBREAK       0x0040
#define MF_UNHILITE        0x0000
#define MF_HILITE          0x0080
#define MF_SYSMENU         0x2000
#define MF_HELP            0x4000
#define MF_MOUSESELECT     ((short)0x8000)
#define MF_END             0x0080

#define MENU_QUIT			1009
#define MENU_NEW			1010
#define MENU_PASSWORD		1011
#define MENU_PAUSE			1012
#define MENU_PLAYERS		1013
#define MENU_PLAYSAMPLES	1020
#define MENU_PLAYMUSICS		1021
#define MENU_HIDEMENU		1022
#define MENU_HELP			1023
#define MENU_ABOUT			1024
#define MENU_FULLSCREEN		1025


@interface Menus : NSObject
{
@public
	CArrayList* menuItems;
	CRunApp* runApp;
	
	short* accelsKey;
	short* accelsID;
	unsigned char* accelsShift;
}
-(id)initWithRunApp:(CRunApp*)app;
-(void)dealloc;
-(void)load:(CFile*)file;
-(void)loadItems:(CFile*)file;
-(void)deleteItem:(int)itemID andSeparator:(bool)deleteSeparator;
-(void)createMenu;
-(int)createItems:(int)count menuBar:(NSMenu*)menuBar andParent:(NSMenu*)parent;
-(NSMenuItem*)getMenuItemFromID:(int)menuID;
@end



@interface MenuDefItem : NSObject
{
@public
	CRunApp* runApp;
	NSString* name;
	short flags;
	short itemID;
	unichar mnemonic;
	NSMenuItem* menuItem;
}
-(id)initWithRunApp:(CRunApp*)app;
-(void)load:(CFile*)file;
@end




