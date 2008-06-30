#ifndef _TEK_STDCALL_TIME_H
#define _TEK_STDCALL_TIME_H

/*
**	$Id: time.h,v 1.1 2008-06-30 12:34:58 tmueller Exp $
**	teklib/tek/stdcall/time.h - time module interface
**
**	See copyright notice in teklib/COPYRIGHT
*/

#define TTimeSubTime(time,a,b) \
	(*(((TMODCALL void(**)(TAPTR,TTIME *,TTIME *))(time))[-9]))(time,a,b)

#define TTimeAddTime(time,a,b) \
	(*(((TMODCALL void(**)(TAPTR,TTIME *,TTIME *))(time))[-10]))(time,a,b)

#define TTimeCmpTime(time,a,b) \
	(*(((TMODCALL TINT(**)(TAPTR,TTIME *,TTIME *))(time))[-11]))(time,a,b)

#define TTimeAllocTimeRequest(time,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(time))[-12]))(time,tags)

#define TTimeFreeTimeRequest(time,req) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(time))[-13]))(time,req)

#define TTimeQueryTime(time,req,t) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TTIME *))(time))[-14]))(time,req,t)

#define TTimeGetDate(time,req,dt,tz) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TDATE *,TINT *))(time))[-15]))(time,req,dt,tz)

#define TTimeWaitTime(time,req,t,sig) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR,TTIME *,TUINT))(time))[-16]))(time,req,t,sig)

#define TTimeWaitDate(time,req,dt,sig) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR,TDATE *,TUINT))(time))[-17]))(time,req,dt,sig)

#define TTimeMakeDate(time,dt,d,m,y,t) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TDATE *,TINT,TINT,TINT,TTIME *))(time))[-18]))(time,dt,d,m,y,t)

#define TTimeAddDate(time,dt,nd,t) \
	(*(((TMODCALL void(**)(TAPTR,TDATE *,TINT,TTIME *))(time))[-19]))(time,dt,nd,t)

#define TTimeSubDate(time,dt,nd,t) \
	(*(((TMODCALL void(**)(TAPTR,TDATE *,TINT,TTIME *))(time))[-20]))(time,dt,nd,t)

#define TTimeDiffDate(time,dt1,dt2,t) \
	(*(((TMODCALL TINT(**)(TAPTR,TDATE *,TDATE *,TTIME *))(time))[-21]))(time,dt1,dt2,t)

#define TTimeIsLeapYear(time,year) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TINT))(time))[-22]))(time,year)

#define TTimeIsValidDate(time,d,m,y) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TINT,TINT,TINT))(time))[-23]))(time,d,m,y)

#define TTimePackDate(time,db,dt) \
	(*(((TMODCALL TBOOL(**)(TAPTR,struct TDateBox *,TDATE *))(time))[-24]))(time,db,dt)

#define TTimeUnpackDate(time,dt,db,flg) \
	(*(((TMODCALL void(**)(TAPTR,TDATE *,struct TDateBox *,TUINT))(time))[-25]))(time,dt,db,flg)

#define TTimeDateToJulian(time,dt) \
	(*(((TMODCALL TDOUBLE(**)(TAPTR,TDATE *))(time))[-26]))(time,dt)

#define TTimeJulianToDate(time,jd,dt) \
	(*(((TMODCALL void(**)(TAPTR,TDOUBLE,TDATE *))(time))[-27]))(time,jd,dt)

#define TTimeMYToJulian(time,m,y) \
	(*(((TMODCALL TDOUBLE(**)(TAPTR,TINT,TINT))(time))[-28]))(time,m,y)

#define TTimeJulianToDMY(time,jd,pd,pm,py) \
	(*(((TMODCALL TDOUBLE(**)(TAPTR,TDOUBLE,TINT *,TINT *,TINT *))(time))[-29]))(time,jd,pd,pm,py)

#define TTimeYDayToDM(time,yd,y,pd,pm) \
	(*(((TMODCALL void(**)(TAPTR,TINT,TINT,TINT *,TINT *))(time))[-30]))(time,yd,y,pd,pm)

#define TTimeDMYToYDay(time,d,m,y) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT,TINT,TINT))(time))[-31]))(time,d,m,y)

#define TTimeGetWeekDay(time,d,m,y) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT,TINT,TINT))(time))[-32]))(time,d,m,y)

#define TTimeGetWeekNumber(time,d,m,y) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT,TINT,TINT))(time))[-33]))(time,d,m,y)

#define TTimeDelay(time,req,t) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TTIME *))(time))[-34]))(time,req,t)

#endif /* _TEK_STDCALL_TIME_H */
