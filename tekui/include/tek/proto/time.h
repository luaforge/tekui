
#ifndef _TEK_PROTO_TIME_H
#define _TEK_PROTO_TIME_H

/*
**	$Id: time.h,v 1.1 2008-06-30 12:34:54 tmueller Exp $
**	teklib/tek/proto/time.h - Time module prototypes
**
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/mod/time.h>
#include <tek/stdcall/time.h>

extern TMODENTRY TUINT
tek_init_time(TAPTR, struct TModule *, TUINT16, TTAGITEM *);

#if !defined(TTIME_NO_API_COMPAT)
/*
**	old API functions (for compatibility)
*/
#define TTimeSub(mod,a,b)					TTimeSubTime(mod,a,b)
#define TTimeAdd(mod,a,b)					TTimeAddTime(mod,a,b)
#define TTimeCmp(mod,a,b)					TTimeCmpTime(mod,a,b)
#define TTimeAllocRequest(mod,tags)			TTimeAllocTimeRequest(mod,tags)
#define TTimeFreeRequest(mod,req)			TTimeFreeTimeRequest(mod,req)
#define TTimeQuery(mod,req,time)			TTimeQueryTime(mod,req,time)
#define TTimeWait(mod,req,time,sig)			TTimeWaitTime(mod,req,time,sig)

#endif

#endif /* _TEK_PROTO_TIME_H */
