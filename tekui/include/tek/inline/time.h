#ifndef _TEK_INLINE_TIME_H
#define _TEK_INLINE_TIME_H

/*
**	$Id: time.h,v 1.1 2008-06-30 12:34:58 tmueller Exp $
**	teklib/tek/inline/time.h - time inline calls
**
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/proto/time.h>

#define TSubTime(a,b) \
	(*(((TMODCALL void(**)(TAPTR,TTIME *,TTIME *))(TTimeBase))[-9]))(TTimeBase,a,b)

#define TAddTime(a,b) \
	(*(((TMODCALL void(**)(TAPTR,TTIME *,TTIME *))(TTimeBase))[-10]))(TTimeBase,a,b)

#define TCmpTime(a,b) \
	(*(((TMODCALL TINT(**)(TAPTR,TTIME *,TTIME *))(TTimeBase))[-11]))(TTimeBase,a,b)

#define TAllocTimeRequest(tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(TTimeBase))[-12]))(TTimeBase,tags)

#define TFreeTimeRequest(req) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(TTimeBase))[-13]))(TTimeBase,req)

#define TQueryTime(req,t) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TTIME *))(TTimeBase))[-14]))(TTimeBase,req,t)

#define TGetDate(req,dt,tz) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TDATE *,TINT *))(TTimeBase))[-15]))(TTimeBase,req,dt,tz)

#define TWaitTime(req,t,sig) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR,TTIME *,TUINT))(TTimeBase))[-16]))(TTimeBase,req,t,sig)

#define TWaitDate(req,dt,sig) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR,TDATE *,TUINT))(TTimeBase))[-17]))(TTimeBase,req,dt,sig)

#define TMakeDate(dt,d,m,y,t) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TDATE *,TINT,TINT,TINT,TTIME *))(TTimeBase))[-18]))(TTimeBase,dt,d,m,y,t)

#define TAddDate(dt,nd,t) \
	(*(((TMODCALL void(**)(TAPTR,TDATE *,TINT,TTIME *))(TTimeBase))[-19]))(TTimeBase,dt,nd,t)

#define TSubDate(dt,nd,t) \
	(*(((TMODCALL void(**)(TAPTR,TDATE *,TINT,TTIME *))(TTimeBase))[-20]))(TTimeBase,dt,nd,t)

#define TDiffDate(dt1,dt2,t) \
	(*(((TMODCALL TINT(**)(TAPTR,TDATE *,TDATE *,TTIME *))(TTimeBase))[-21]))(TTimeBase,dt1,dt2,t)

#define TIsLeapYear(year) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TINT))(TTimeBase))[-22]))(TTimeBase,year)

#define TIsValidDate(d,m,y) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TINT,TINT,TINT))(TTimeBase))[-23]))(TTimeBase,d,m,y)

#define TPackDate(db,dt) \
	(*(((TMODCALL TBOOL(**)(TAPTR,struct TDateBox *,TDATE *))(TTimeBase))[-24]))(TTimeBase,db,dt)

#define TUnpackDate(dt,db,flg) \
	(*(((TMODCALL void(**)(TAPTR,TDATE *,struct TDateBox *,TUINT))(TTimeBase))[-25]))(TTimeBase,dt,db,flg)

#define TDateToJulian(dt) \
	(*(((TMODCALL TDOUBLE(**)(TAPTR,TDATE *))(TTimeBase))[-26]))(TTimeBase,dt)

#define TJulianToDate(jd,dt) \
	(*(((TMODCALL void(**)(TAPTR,TDOUBLE,TDATE *))(TTimeBase))[-27]))(TTimeBase,jd,dt)

#define TMYToJulian(m,y) \
	(*(((TMODCALL TDOUBLE(**)(TAPTR,TINT,TINT))(TTimeBase))[-28]))(TTimeBase,m,y)

#define TJulianToDMY(jd,pd,pm,py) \
	(*(((TMODCALL TDOUBLE(**)(TAPTR,TDOUBLE,TINT *,TINT *,TINT *))(TTimeBase))[-29]))(TTimeBase,jd,pd,pm,py)

#define TYDayToDM(yd,y,pd,pm) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT *,TINT *))(TTimeBase))[-30]))(TTimeBase,yd,y,pd,pm)

#define TDMYToYDay(d,m,y) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT,TINT,TINT))(TTimeBase))[-31]))(TTimeBase,d,m,y)

#define TGetWeekDay(d,m,y) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT,TINT,TINT))(TTimeBase))[-32]))(TTimeBase,d,m,y)

#define TGetWeekNumber(d,m,y) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT,TINT,TINT))(TTimeBase))[-33]))(TTimeBase,d,m,y)

#define TDelay(req,t) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TTIME *))(TTimeBase))[-34]))(TTimeBase,req,t)

#endif /* _TEK_INLINE_TIME_H */
