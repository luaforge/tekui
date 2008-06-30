
#ifndef _TEK_HAL_HAL_MOD_H
#define	_TEK_HAL_HAL_MOD_H

/*
**	$Id: hal_mod.h,v 1.1 2008-06-30 12:34:04 tmueller Exp $
**	teklib/mods/hal/hal_mod.h - HAL module internal definitions
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/exec.h>
#include <tek/teklib.h>
#include <tek/mod/hal.h>
#include <tek/mod/time.h>

#ifndef LOCAL
#define LOCAL
#endif

#ifndef EXPORT
#define EXPORT TMODAPI
#endif

/*****************************************************************************/
/*
**	HAL module structure
*/

typedef struct
{
	/* Module header */
	struct TModule hmb_Module;
	/* Ptr to platform-specific data */
	TAPTR hmb_Specific;
	/* Ptr to boot handle */
	TAPTR hmb_BootHnd;
} TMOD_HAL;

/*****************************************************************************/
/*
**	internal prototypes
*/

LOCAL TBOOL hal_init(TMOD_HAL *hal, TTAGITEM *tags);
LOCAL void hal_exit(TMOD_HAL *hal);

LOCAL TAPTR hal_allocself(TAPTR boot, TUINT size);
LOCAL void hal_freeself(TAPTR boot, TAPTR mem, TUINT size);

LOCAL struct TTimeRequest *hal_open(TMOD_HAL *hal, TAPTR task, TTAGITEM *tags);
LOCAL void hal_close(TMOD_HAL *hal, TAPTR task);

LOCAL void hal_subtime(TTIME *a, TTIME *b);
LOCAL void hal_addtime(TTIME *a, TTIME *b);
LOCAL TINT hal_cmptime(TTIME *a, TTIME *b);

/*****************************************************************************/
/*
**	API prototypes
*/

EXPORT void hal_beginio(TMOD_HAL *hal, struct TTimeRequest *req);
EXPORT TINT hal_abortio(TMOD_HAL *hal, struct TTimeRequest *req);

EXPORT TAPTR hal_alloc(TMOD_HAL *hal, TUINT size);
EXPORT void hal_free(TMOD_HAL *hal, TAPTR mem, TUINT size);
EXPORT TAPTR hal_realloc(TMOD_HAL *hal, TAPTR mem, TUINT oldsize,
	TUINT newsize);
EXPORT void hal_copymem(TMOD_HAL *hal, TAPTR from, TAPTR to, TUINT numbytes);
EXPORT void hal_fillmem(TMOD_HAL *hal, TAPTR dest, TUINT numbytes,
	TUINT8 fillval);
EXPORT TBOOL hal_initlock(TMOD_HAL *hal, THALO *lock);
EXPORT void hal_destroylock(TMOD_HAL *hal, THALO *lock);
EXPORT void hal_lock(TMOD_HAL *hal, THALO *lock);
EXPORT void hal_unlock(TMOD_HAL *hal, THALO *lock);
EXPORT TBOOL hal_initthread(TMOD_HAL *hal, THALO *thread,
	TTASKENTRY void (*function)(TAPTR task), TAPTR data);
EXPORT void hal_destroythread(TMOD_HAL *hal, THALO *thread);
EXPORT TAPTR hal_findself(TMOD_HAL *hal);
EXPORT TAPTR hal_loadmodule(TMOD_HAL *hal, TSTRPTR name, TUINT16 version,
	TUINT *psize, TUINT *nsize);
EXPORT TBOOL hal_callmodule(TMOD_HAL *hal, TAPTR halmod, TAPTR task,
	TAPTR mod);
EXPORT void hal_unloadmodule(TMOD_HAL *hal, TAPTR halmod);

EXPORT TBOOL hal_scanmodules(TMOD_HAL *hal, TSTRPTR path, struct THook *hook);
EXPORT TTAG hal_getattr(TMOD_HAL *hal, TUINT tag, TTAG defval);
EXPORT TUINT hal_wait(TMOD_HAL *hal, TUINT signals);
EXPORT void hal_signal(TMOD_HAL *hal, THALO *thread, TUINT signals);
EXPORT TUINT hal_setsignal(TMOD_HAL *hal, TUINT newsig, TUINT sigmask);
EXPORT void hal_getsystime(TMOD_HAL *hal, TTIME *time);

EXPORT TDOUBLE hal_datetojulian(TMOD_HAL *hal, TDATE *date);
EXPORT void hal_juliantodate(TMOD_HAL *hal, TDOUBLE jd, TDATE *date);

#endif
