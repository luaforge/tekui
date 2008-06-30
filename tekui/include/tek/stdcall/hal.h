#ifndef _TEK_STDCALL_HAL_H
#define _TEK_STDCALL_HAL_H

/*
**	$Id: hal.h,v 1.1 2008-06-30 12:34:56 tmueller Exp $
**	teklib/tek/stdcall/hal.h - hal module interface
**
**	See copyright notice in teklib/COPYRIGHT
*/

#define THALGetAttr(hal,tag,defval) \
	(*(((TMODCALL TTAG(**)(TAPTR,TUINT,TTAG))(hal))[-9]))(hal,tag,defval)

#define THALGetSysTime(hal,time) \
	(*(((TMODCALL void(**)(TAPTR,TTIME *))(hal))[-10]))(hal,time)

#define THALAlloc(hal,size) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TUINT))(hal))[-11]))(hal,size)

#define THALFree(hal,mem,size) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TUINT))(hal))[-12]))(hal,mem,size)

#define THALRealloc(hal,mem,oldsize,newsize) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TUINT,TUINT))(hal))[-13]))(hal,mem,oldsize,newsize)

#define THALCopyMem(hal,src,dst,size) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TAPTR,TUINT))(hal))[-14]))(hal,src,dst,size)

#define THALFillMem(hal,dst,len,val) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TUINT,TUINT8))(hal))[-15]))(hal,dst,len,val)

#define THALInitLock(hal,lock) \
	(*(((TMODCALL TBOOL(**)(TAPTR,THALO *))(hal))[-16]))(hal,lock)

#define THALDestroyLock(hal,lock) \
	(*(((TMODCALL void(**)(TAPTR,THALO *))(hal))[-17]))(hal,lock)

#define THALLock(hal,lock) \
	(*(((TMODCALL void(**)(TAPTR,THALO *))(hal))[-18]))(hal,lock)

#define THALUnlock(hal,lock) \
	(*(((TMODCALL void(**)(TAPTR,THALO *))(hal))[-19]))(hal,lock)

#define THALInitThread(hal,thread,func,data) \
	(*(((TMODCALL TBOOL(**)(TAPTR,THALO *,TTASKFUNC,TAPTR))(hal))[-20]))(hal,thread,func,data)

#define THALDestroyThread(hal,thread) \
	(*(((TMODCALL void(**)(TAPTR,THALO *))(hal))[-21]))(hal,thread)

#define THALFindSelf(hal) \
	(*(((TMODCALL TAPTR(**)(TAPTR))(hal))[-22]))(hal)

#define THALWait(hal,signals) \
	(*(((TMODCALL TUINT(**)(TAPTR,TUINT))(hal))[-23]))(hal,signals)

#define THALSignal(hal,thread,signals) \
	(*(((TMODCALL void(**)(TAPTR,THALO *,TUINT))(hal))[-24]))(hal,thread,signals)

#define THALSetSignal(hal,newsigs,sigs) \
	(*(((TMODCALL TUINT(**)(TAPTR,TUINT,TUINT))(hal))[-25]))(hal,newsigs,sigs)

#define THALLoadModule(hal,name,version,possize,newgsize) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR,TUINT16,TUINT *,TUINT *))(hal))[-26]))(hal,name,version,possize,newgsize)

#define THALCallModule(hal,mod,task,data) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TAPTR,TAPTR,TAPTR))(hal))[-27]))(hal,mod,task,data)

#define THALUnloadModule(hal,mod) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(hal))[-28]))(hal,mod)

#define THALScanModules(hal,prefix,hook) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR,struct THook *))(hal))[-29]))(hal,prefix,hook)

#define THALDateToJulian(hal,date) \
	(*(((TMODCALL TDOUBLE(**)(TAPTR,TDATE *))(hal))[-30]))(hal,date)

#define THALJulianToDate(hal,jd,date) \
	(*(((TMODCALL void(**)(TAPTR,TDOUBLE,TDATE *))(hal))[-31]))(hal,jd,date)

#endif /* _TEK_STDCALL_HAL_H */
