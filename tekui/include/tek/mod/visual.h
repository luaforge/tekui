#ifndef _TEK_MOD_VISUAL_H
#define _TEK_MOD_VISUAL_H

/*
**	teklib/tek/mod/visual.h - Visual module definitions
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/exec.h>
#include <tek/mod/time.h>

/*****************************************************************************/
/*
**	Types
*/

typedef TTAG TVPEN;
#define TVPEN_UNDEFINED				((TVPEN)(-1))

/*****************************************************************************/
/*
**	Tags
*/

#define TVISTAGS_					(TTAG_USER + 0x600)

#define TVisual_Display				(TVISTAGS_ + 0x001)
#define TVisual_DisplayName			(TVISTAGS_ + 0x002)
#define TVisual_NewInstance			(TVISTAGS_ + 0x003)
#define TVisual_Width				(TVISTAGS_ + 0x004)
#define TVisual_Height				(TVISTAGS_ + 0x005)
#define TVisual_Title				(TVISTAGS_ + 0x006)
#define TVisual_WinLeft				(TVISTAGS_ + 0x007)
#define TVisual_WinTop				(TVISTAGS_ + 0x008)
#define TVisual_MinWidth			(TVISTAGS_ + 0x009)
#define TVisual_MinHeight			(TVISTAGS_ + 0x00a)
#define TVisual_MaxWidth			(TVISTAGS_ + 0x00b)
#define TVisual_MaxHeight			(TVISTAGS_ + 0x00c)
#define TVisual_Attach				(TVISTAGS_ + 0x00d)
#define TVisual_Center				(TVISTAGS_ + 0x00e)
#define TVisual_Borderless			(TVISTAGS_ + 0x00f)
#define TVisual_Fullscreen			(TVISTAGS_ + 0x010)
#define TVisual_EventMask			(TVISTAGS_ + 0x011)

#define	TVisual_FontName			(TVISTAGS_ + 0x100)
#define	TVisual_FontPxSize			(TVISTAGS_ + 0x101)
#define	TVisual_FontItalic			(TVISTAGS_ + 0x102)
#define	TVisual_FontBold			(TVISTAGS_ + 0x103)
#define	TVisual_FontScaleable		(TVISTAGS_ + 0x104)
#define TVisual_FontAscent			(TVISTAGS_ + 0x105)
#define TVisual_FontDescent			(TVISTAGS_ + 0x106)
#define	TVisual_FontUlPosition		(TVISTAGS_ + 0x107)
#define	TVisual_FontUlThickness		(TVISTAGS_ + 0x108)
#define TVisual_FontNumResults		(TVISTAGS_ + 0x109)
#define TVisual_FontHeight			(TVISTAGS_ + 0x10a)
#define TVisual_FontWidth			(TVISTAGS_ + 0x10b)

#define TVisual_Pen					(TVISTAGS_ + 0x10c)
#define TVisual_PenArray			(TVISTAGS_ + 0x10d)

#define TVisual_ExposeHook			(TVISTAGS_ + 0x10e)

/* Tagged rendering: */

#define TVisualDraw_Command			(TVISTAGS_ + 0x300)
#define TVisualDraw_X0				(TVISTAGS_ + 0x301)
#define TVisualDraw_Y0				(TVISTAGS_ + 0x302)
#define TVisualDraw_X1				(TVISTAGS_ + 0x303)
#define TVisualDraw_Y1				(TVISTAGS_ + 0x304)
#define TVisualDraw_NewX			(TVISTAGS_ + 0x305)
#define TVisualDraw_NewY			(TVISTAGS_ + 0x306)
#define TVisualDraw_FgPen			(TVISTAGS_ + 0x307)
#define TVisualDraw_BgPen			(TVISTAGS_ + 0x308)

#if 0
/* Platform-specific / experimental tags: */
#define TVisualHost_GrabPointer		(TVISTAGS_ + 0x201)
#define TVisualHost_GrabButton		(TVISTAGS_ + 0x202)
#endif

/*****************************************************************************/
/*
**	Visual request
*/

struct TVRequest
{
	struct TIORequest tvr_Req;
	union
	{
		struct { TAPTR Instance; TTAGITEM *Tags; TAPTR IMsgPort; } OpenVisual;
		struct { TAPTR Instance; } CloseVisual;
		struct { TAPTR Font; TTAGITEM *Tags; } OpenFont;
		struct { TAPTR Font; } CloseFont;
		struct { TAPTR Font; TTAGITEM *Tags; TUINT Num; } GetFontAttrs;
		struct { TAPTR Font; TSTRPTR Text; TINT Width; } TextSize;
		struct { TAPTR Handle; TTAGITEM *Tags; } QueryFonts;
		struct { TAPTR Handle; TTAGITEM *Attrs; } GetNextFont;
		struct { TAPTR Instance; TUINT Mask; TUINT OldMask; } SetInput;
		struct { TAPTR Instance; TTAGITEM *Tags; TUINT Num; } GetAttrs;
		struct { TAPTR Instance; TTAGITEM *Tags; TUINT Num; } SetAttrs;
		struct { TAPTR Instance; TUINT RGB; TVPEN Pen; } AllocPen;
		struct { TAPTR Instance; TVPEN Pen; } FreePen;
		struct { TAPTR Instance; TSTRPTR Font; } SetFont;
		struct { TAPTR Instance; TVPEN Pen; } Clear;
		struct { TAPTR Instance; TINT Rect[4]; TVPEN Pen; } Rect;
		struct { TAPTR Instance; TINT Rect[4]; TVPEN Pen; } FRect;
		struct { TAPTR Instance; TINT Rect[4]; TVPEN Pen; } Line;
		struct { TAPTR Instance; TINT Rect[2]; TVPEN Pen; } Plot;
		struct { TAPTR Instance; TINT X, Y; TSTRPTR Text; TINT Length;
			TVPEN FgPen, BgPen; } Text;
		struct { TAPTR Instance; TINT *Array; TINT Num; TTAGITEM *Tags; } Strip;
		struct { TAPTR Instance; TTAGITEM *Tags; } DrawTags;
		struct { TAPTR Instance; TINT *Array; TINT Num; TTAGITEM *Tags; } Fan;
		struct { TAPTR Instance; TINT Rect[4]; TINT Angle1; TINT Angle2;
			TVPEN Pen;} Arc;
		struct { TAPTR Instance; TINT Rect[4]; TINT DestX; TINT DestY;
			TTAGITEM *Tags; } CopyArea;
		struct { TAPTR Instance; TINT Rect[4]; TTAGITEM *Tags; } ClipRect;
		struct { TAPTR Instance; TINT RRect[4]; TAPTR Buf; TINT TotWidth;
			TTAGITEM *Tags; } DrawBuffer;
	} tvr_Op;
};

/* Command codes: */
#define TVCMD_OPENVISUAL	0x1001
#define TVCMD_CLOSEVISUAL	0x1002
#define TVCMD_OPENFONT		0x1003
#define TVCMD_CLOSEFONT		0x1004
#define TVCMD_GETFONTATTRS	0x1005
#define TVCMD_TEXTSIZE		0x1006
#define TVCMD_QUERYFONTS	0x1007
#define TVCMD_GETNEXTFONT	0x1008
#define TVCMD_SETINPUT		0x1009
#define TVCMD_GETATTRS		0x100a
#define TVCMD_SETATTRS		0x100b
#define TVCMD_ALLOCPEN		0x100c
#define TVCMD_FREEPEN		0x100d
#define TVCMD_SETFONT		0x100e
#define TVCMD_CLEAR			0x100f
#define TVCMD_RECT			0x1010
#define TVCMD_FRECT			0x1011
#define TVCMD_LINE			0x1012
#define TVCMD_PLOT			0x1013
#define TVCMD_TEXT			0x1014
#define TVCMD_DRAWSTRIP		0x1015
#define TVCMD_DRAWTAGS		0x1016
#define TVCMD_DRAWFAN		0x1017
#define TVCMD_DRAWARC		0x1018
#define TVCMD_COPYAREA		0x1019
#define TVCMD_SETCLIPRECT	0x101a
#define TVCMD_UNSETCLIPRECT	0x101b
#define TVCMD_DRAWFARC		0x101c
#define TVCMD_DRAWBUFFER	0x101d

/*****************************************************************************/
/*
**	Input message
*/

typedef struct TInputMessage
{
	/* Node header: */
	struct TNode timsg_Node;
	/* Timestamp: */
	TTIME timsg_TimeStamp;
	/* Message type: */
	TUINT timsg_Type;
	/* Message code: */
	TUINT timsg_Code;
	/* Keyboard qualifiers: */
	TUINT timsg_Qualifier;
	/* Mouse position: */
	TINT timsg_MouseX, timsg_MouseY;
	/* Damage rect: */
	TINT timsg_X, timsg_Y, timsg_Width, timsg_Height;
	/* UTF-8 representation of keycode: */
	unsigned char timsg_KeyCode[8];

	/* more fields may follow in the future. Do not use
	** sizeof(TIMSG); if necessary use TGetSize(imsg) */

} TIMSG;

/*****************************************************************************/
/*
**	Input types
*/

#define TITYPE_NONE				0x00000000
#define TITYPE_ALL				0xffffffff

/* Closed: */
#define TITYPE_CLOSE			0x00000001
/* Focused/unfocused: */
#define TITYPE_FOCUS			0x00000002
/* Resized: */
#define TITYPE_NEWSIZE			0x00000004
/* Needs refreshing: */
#define TITYPE_REFRESH			0x00000008
/* Mouse moved in/out: */
#define TITYPE_MOUSEOVER		0x00000010

/* key down: */
#define	TITYPE_KEYDOWN			0x00000100
/* Mouse movement: */
#define TITYPE_MOUSEMOVE		0x00000200
/* Mouse button: */
#define TITYPE_MOUSEBUTTON		0x00000400

/* 1/50s interval: */
#define TITYPE_INTERVAL			0x00000800

/* key up: */
#define TITYPE_KEYUP			0x00001000

/* "Cooked" keystroke, an alias to keydown: */
#define	TITYPE_COOKEDKEY		TITYPE_KEYDOWN

/*****************************************************************************/
/*
**	Mouse button codes
**	Styleguide notes: Some mice have only one or two buttons.
**	usage of the middle and right buttons without alternatives
**	is disencouraged.
*/

#define TMBCODE_LEFTDOWN		0x00000001
#define TMBCODE_LEFTUP			0x00000002
#define TMBCODE_RIGHTDOWN		0x00000004
#define TMBCODE_RIGHTUP			0x00000008
#define TMBCODE_MIDDLEDOWN		0x00000010
#define TMBCODE_MIDDLEUP		0x00000020
#define TMBCODE_WHEELUP			0x00000040
#define TMBCODE_WHEELDOWN		0x00000080

/*****************************************************************************/
/*
**	Keycodes (cooked)
*/

/* may still have a qualifier, though: */
#define TKEYC_NONE				0x00000000

/*
**	Named special key codes
*/

/* backspace key: */
#define TKEYC_BCKSPC			0x00000008
/* tab key: */
#define TKEYC_TAB				0x00000009
/* return: */
#define TKEYC_RETURN			0x0000000d
/* escape: */
#define TKEYC_ESC				0x0000001b
/* del key: */
#define TKEYC_DEL				0x0000007f

/*
**	Function key codes
**	Styleguide notes: Some keyboards have only 10 F-Keys.
**	Hardcoded key bindings should never rely on F11 or F12
**	without offering some kind of alternative.
*/

#define TKEYC_F1				0x0000f001
#define TKEYC_F2				0x0000f002
#define TKEYC_F3				0x0000f003
#define TKEYC_F4				0x0000f004
#define TKEYC_F5				0x0000f005
#define TKEYC_F6				0x0000f006
#define TKEYC_F7				0x0000f007
#define TKEYC_F8				0x0000f008
#define TKEYC_F9				0x0000f009
#define TKEYC_F10				0x0000f00a
#define TKEYC_F11				0x0000f00b
#define TKEYC_F12				0x0000f00c

/*
**	Cursor key codes
*/

#define	TKEYC_CRSRLEFT			0x0000f010
#define	TKEYC_CRSRRIGHT			0x0000f011
#define	TKEYC_CRSRUP			0x0000f012
#define	TKEYC_CRSRDOWN			0x0000f013

/*
**	Proprietary key codes
**	Styleguide notes: Whenever your application binds actions
**	to a key from this section, you should offer at least one
**	non-proprietary alternative
*/

/* help key: */
#define TKEYC_HELP				0x0000f020
/* insert key: */
#define TKEYC_INSERT			0x0000f021
/* overwrite key: */
#define TKEYC_OVERWRITE			0x0000f022
/* page up: */
#define	TKEYC_PAGEUP			0x0000f023
/* page down: */
#define	TKEYC_PAGEDOWN			0x0000f024
/* position one key: */
#define TKEYC_POSONE			0x0000f025
/* position end key: */
#define TKEYC_POSEND			0x0000f026
/* print key: */
#define TKEYC_PRINT				0x0000f027
/* scroll down: */
#define TKEYC_SCROLL			0x0000f028
/* pause key: */
#define TKEYC_PAUSE				0x0000f029

/*
**	Keyboard qualifiers
**	Styleguide notes: Your application's default or hardcoded key
**	bindings should never rely on TKEYQ_PROP alone. Some keyboards
**	do not have a numblock, so usage of the TKEYQ_NUMBLOCK qualifier
**	without an alternative is disencouraged, too. Also note that some
**	keyboards don't have a right control key.
*/

/* no qualifier: */
#define	TKEYQ_NONE				0x00000000
#define TKEYQ_LSHIFT			0x00000001
#define TKEYQ_RSHIFT			0x00000002
#define TKEYQ_LCTRL				0x00000004
#define TKEYQ_RCTRL				0x00000008
#define TKEYQ_LALT				0x00000010
#define TKEYQ_RALT				0x00000020
/* proprietary qualifier: */
#define TKEYQ_LPROP				0x00000040
#define TKEYQ_RPROP				0x00000080
/* is a qualifier here: */
#define TKEYQ_NUMBLOCK			0x00000100

/*	Combinations: Use e.g. msg->timsg_Qualifier & TKEYQ_SHIFT
**	to find out whether any SHIFT key was held */

#define TKEYQ_SHIFT				(TKEYQ_LSHIFT|TKEYQ_RSHIFT)
#define TKEYQ_CTRL				(TKEYQ_LCTRL|TKEYQ_RCTRL)
#define TKEYQ_ALT				(TKEYQ_LALT|TKEYQ_RALT)
#define TKEYQ_PROP				(TKEYQ_LPROP|TKEYQ_RPROP)

#endif
