
#ifndef _TEK_PROTO_EXEC_H
#define _TEK_PROTO_EXEC_H

/*
**	$Id: exec.h,v 1.1 2008-06-30 12:34:54 tmueller Exp $
**	teklib/tek/proto/exec.h - Exec module prototypes
**
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/exec.h>
#include <tek/stdcall/exec.h>

extern TMODENTRY TUINT
tek_init_exec(TAPTR, struct TModule *, TUINT16, TTAGITEM *);

#endif /* _TEK_PROTO_EXEC_H */
