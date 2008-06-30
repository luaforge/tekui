
#ifndef _TEKLIB_H
#define _TEKLIB_H

/*
**	$Id: teklib.h,v 1.1 2008-06-30 12:34:46 tmueller Exp $
**	teklib/tek/teklib.h - Link library functions for bootstrapping
**	and for operating on elementary, public data structures
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

/*****************************************************************************/

#include <tek/exec.h>
#include <tek/mod/time.h>

#ifdef __cplusplus
extern "C" {
#endif

TLIBAPI TAPTR TEKCreate(TTAGITEM *tags);
TLIBAPI void TDestroy(TAPTR handle);
TLIBAPI void TDestroyList(struct TList *list);
TLIBAPI TAPTR TNewInstance(TAPTR mod, TUINT possize, TUINT negsize);
TLIBAPI void TFreeInstance(TAPTR mod);
TLIBAPI void TInitVectors(TAPTR mod, const TMFPTR *vectors, TUINT numv);
TLIBAPI TTAG TGetTag(TTAGITEM *taglist, TUINT tag, TTAG defvalue);
TLIBAPI void TInitList(struct TList *list);
TLIBAPI void TAddHead(struct TList *list, struct TNode *node);
TLIBAPI void TAddTail(struct TList *list, struct TNode *node);
TLIBAPI struct TNode *TRemHead(struct TList *list);
TLIBAPI struct TNode *TRemTail(struct TList *list);
TLIBAPI void TRemove(struct TNode *node);
TLIBAPI void TNodeUp(struct TNode *node);
TLIBAPI void TInsert(struct TList *list, struct TNode *node,
	struct TNode *prednode);
TLIBAPI TBOOL TForEachTag(struct TTagItem *taglist, struct THook *hook);
TLIBAPI struct THandle *TFindHandle(struct TList *list, TSTRPTR s2);
TLIBAPI void TInitHook(struct THook *hook, THOOKFUNC func, TAPTR data);
TLIBAPI TTAG TCallHookPkt(struct THook *hook, TAPTR obj, TTAG msg);

#ifdef __cplusplus
}
#endif

#endif
