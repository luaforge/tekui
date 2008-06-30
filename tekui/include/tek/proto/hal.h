
#ifndef _TEK_PROTO_HAL_H
#define _TEK_PROTO_HAL_H

/*
**	$Id: hal.h,v 1.1 2008-06-30 12:34:54 tmueller Exp $
**	teklib/tek/proto/hal.h - HAL module prototypes
**
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/exec.h>
#include <tek/stdcall/hal.h>

extern TMODENTRY TUINT
tek_init_hal(TAPTR, struct TModule *, TUINT16, TTAGITEM *);

#endif /* _TEK_PROTO_HAL_H */
