#ifndef _TEK_STDCALL_VISUAL_H
#define _TEK_STDCALL_VISUAL_H

/*
**	$Id: visual.h,v 1.1 2008-06-30 12:34:56 tmueller Exp $
**	teklib/tek/stdcall/visual.h - visual module interface
**
**	See copyright notice in teklib/COPYRIGHT
*/

#define TVisualOpen(visual,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(visual))[-9]))(visual,tags)

#define TVisualClose(visual,inst) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(visual))[-10]))(visual,inst)

#define TVisualAttach(visual,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(visual))[-11]))(visual,tags)

#define TVisualOpenFont(visual,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(visual))[-12]))(visual,tags)

#define TVisualCloseFont(visual,font) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(visual))[-13]))(visual,font)

#define TVisualGetFontAttrs(visual,font,tags) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TTAGITEM *))(visual))[-14]))(visual,font,tags)

#define TVisualTextSize(visual,font,text) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TSTRPTR))(visual))[-15]))(visual,font,text)

#define TVisualQueryFonts(visual,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(visual))[-16]))(visual,tags)

#define TVisualGetNextFont(visual,fqhandle) \
	(*(((TMODCALL TTAGITEM *(**)(TAPTR,TAPTR))(visual))[-17]))(visual,fqhandle)

#define TVisualGetPort(visual) \
	(*(((TMODCALL TAPTR(**)(TAPTR))(visual))[-18]))(visual)

#define TVisualSetInput(visual,clearflags,setflags) \
	(*(((TMODCALL TUINT(**)(TAPTR,TUINT,TUINT))(visual))[-19]))(visual,clearflags,setflags)

#define TVisualGetAttrs(visual,tags) \
	(*(((TMODCALL TUINT(**)(TAPTR,TTAGITEM *))(visual))[-20]))(visual,tags)

#define TVisualSetAttrs(visual,tags) \
	(*(((TMODCALL TUINT(**)(TAPTR,TTAGITEM *))(visual))[-21]))(visual,tags)

#define TVisualAllocPen(visual,rgb) \
	(*(((TMODCALL TVPEN(**)(TAPTR,TUINT))(visual))[-22]))(visual,rgb)

#define TVisualFreePen(visual,pen) \
	(*(((TMODCALL void(**)(TAPTR,TVPEN))(visual))[-23]))(visual,pen)

#define TVisualSetFont(visual,font) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(visual))[-24]))(visual,font)

#define TVisualClear(visual,pen) \
	(*(((TMODCALL void(**)(TAPTR,TVPEN))(visual))[-25]))(visual,pen)

#define TVisualRect(visual,x,y,w,h,pen) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT,TINT,TVPEN))(visual))[-26]))(visual,x,y,w,h,pen)

#define TVisualFRect(visual,x,y,w,h,pen) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT,TINT,TVPEN))(visual))[-27]))(visual,x,y,w,h,pen)

#define TVisualLine(visual,x1,y1,x2,y2,pen) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT,TINT,TVPEN))(visual))[-28]))(visual,x1,y1,x2,y2,pen)

#define TVisualPlot(visual,x,y,pen) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TVPEN))(visual))[-29]))(visual,x,y,pen)

#define TVisualText(visual,x,y,text,len,fg,bg) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TSTRPTR,TINT,TVPEN,TVPEN))(visual))[-30]))(visual,x,y,text,len,fg,bg)

#define TVisualDrawStrip(visual,array,num,tags) \
	(*(((TMODCALL void(**)(TAPTR,TINT *,TINT,TTAGITEM *))(visual))[-31]))(visual,array,num,tags)

#define TVisualDrawTags(visual,tags) \
	(*(((TMODCALL void(**)(TAPTR,TTAGITEM *))(visual))[-32]))(visual,tags)

#define TVisualDrawFan(visual,array,num,tags) \
	(*(((TMODCALL void(**)(TAPTR,TINT *,TINT,TTAGITEM *))(visual))[-33]))(visual,array,num,tags)

#define TVisualDrawArc(visual,x,y,w,h,angle1,angle2,pen) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT,TINT,TINT,TINT,TVPEN))(visual))[-34]))(visual,x,y,w,h,angle1,angle2,pen)

#define TVisualCopyArea(visual,x,y,w,h,dx,dy,tags) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT,TINT,TINT,TINT,TTAGITEM *))(visual))[-35]))(visual,x,y,w,h,dx,dy,tags)

#define TVisualSetClipRect(visual,x,y,w,h,tags) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT,TINT,TTAGITEM *))(visual))[-36]))(visual,x,y,w,h,tags)

#define TVisualOpenDisplay(visual,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(visual))[-37]))(visual,tags)

#define TVisualCloseDisplay(visual,display) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(visual))[-38]))(visual,display)

#define TVisualQueryDisplays(visual,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(visual))[-39]))(visual,tags)

#define TVisualGetNextDisplay(visual,handle) \
	(*(((TMODCALL TTAGITEM *(**)(TAPTR,TAPTR))(visual))[-40]))(visual,handle)

#define TVisualUnsetClipRect(visual) \
	(*(((TMODCALL void(**)(TAPTR))(visual))[-41]))(visual)

#define TVisualDrawFArc(visual,x,y,w,h,angle1,angle2,pen) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT,TINT,TINT,TINT,TVPEN))(visual))[-42]))(visual,x,y,w,h,angle1,angle2,pen)

#define TVisualDrawBuffer(visual,x,y,buf,w,h,totw,tags) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TAPTR,TINT,TINT,TINT,TTAGITEM *))(visual))[-43]))(visual,x,y,buf,w,h,totw,tags)

#endif /* _TEK_STDCALL_VISUAL_H */
