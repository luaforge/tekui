
#ifndef _TEK_MOD_HAL_H
#define _TEK_MOD_HAL_H

/*
**	$Id: hal.h,v 1.1 2008-06-30 12:34:54 tmueller Exp $
**	teklib/tek/mod/hal.h - HAL module private
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/proto/hal.h>

/*****************************************************************************/
/* 
**	Access macros for the generic HAL object type.
**	Some platform-specific structures may fit into sizeof(THALO),
**	in which case an allocation can be avoided. The compiler should
**	optimize away the non-applicable path.
*/

#define THALNewObject(hal,obj,type) \
	((type *)((sizeof(THALO)<sizeof(type))?THALAlloc(hal,sizeof(type)):(obj)))

#define THALDestroyObject(hal,obj,type) \
	((void)(sizeof(THALO)<sizeof(type)?THALFree(hal,obj,sizeof(type)),(0):(0)))

#define THALSetObject(obj,type,obj2) \
	((void)(sizeof(THALO)<sizeof(type)?*((type **)obj)=obj2,(0):(0)))
	
#define THALGetObject(obj,type) \
	((sizeof(THALO)<sizeof(type))?*((type **)(obj)):(type *)(obj))

/*****************************************************************************/

struct THALScanModMsg
{
	TSTRPTR tsmm_Name;
	TUINT tsmm_Length;
	TUINT tsmm_Flags;
};

#endif
