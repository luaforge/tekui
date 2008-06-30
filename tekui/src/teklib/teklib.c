
/*
**	$Id: teklib.c,v 1.1 2008-06-30 12:34:40 tmueller Exp $
**	teklib/src/teklib/teklib.c - 'tek' link library implementation
**
**	The library functions operate on public exec structures, do not
**	depend on private data or functions, and constitute the most
**	conservative and immutable part of TEKlib.
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/teklib.h>
#include <tek/proto/exec.h>

/*****************************************************************************/
/*
**	TInitList(list)
**	Prepare list header
*/

TLIBAPI void
TInitList(struct TList *list)
{
	list->tlh_TailPred = (struct TNode *) list;
	list->tlh_Tail = TNULL;
	list->tlh_Head = (struct TNode *) &list->tlh_Tail;
}

/*****************************************************************************/
/*
**	TAddHead(list, node)
**	Add a node at the head of a list
*/

TLIBAPI void
TAddHead(struct TList *list, struct TNode *node)
{
	struct TNode *temp = list->tlh_Head;
	list->tlh_Head = node;
	node->tln_Succ = temp;
	node->tln_Pred = (struct TNode *) &list->tlh_Head;
	temp->tln_Pred = node;
}

/*****************************************************************************/
/*
**	TAddTail(list, node)
**	Add a node at the tail of a list
*/

TLIBAPI void
TAddTail(struct TList *list, struct TNode *node)
{
	struct TNode *temp = list->tlh_TailPred;
	list->tlh_TailPred = node;
	node->tln_Succ = (struct TNode *) &list->tlh_Tail;
	node->tln_Pred = temp;
	temp->tln_Succ = node;
}

/*****************************************************************************/
/*
**	node = TRemHead(list)
**	Unlink and return a list's first node
*/

TLIBAPI struct TNode *
TRemHead(struct TList *list)
{
	struct TNode *temp = list->tlh_Head;
	if (temp->tln_Succ)
	{
		list->tlh_Head = temp->tln_Succ;
		temp->tln_Succ->tln_Pred = (struct TNode *) &list->tlh_Head;
		return temp;
	}
	return TNULL;
}

/*****************************************************************************/
/*
**	node = TRemTail(list)
**	Unlink and return a list's last node
*/

TLIBAPI struct TNode *
TRemTail(struct TList *list)
{
	struct TNode *temp = list->tlh_TailPred;
	if (temp->tln_Pred)
	{
		list->tlh_TailPred = temp->tln_Pred;
		temp->tln_Pred->tln_Succ = (struct TNode *) &list->tlh_Tail;
		return temp;
	}
	return TNULL;
}

/*****************************************************************************/
/*
**	TRemove(node)
**	Unlink node from a list
*/

TLIBAPI void
TRemove(struct TNode *node)
{
	struct TNode *temp = node->tln_Succ;
	node->tln_Pred->tln_Succ = temp;
	temp->tln_Pred = node->tln_Pred;
}

/*****************************************************************************/
/*
**	TNodeUp(node)
**	Move a node one position up in a list
*/

TLIBAPI void
TNodeUp(struct TNode *node)
{
	struct TNode *temp = node->tln_Pred;
	if (temp->tln_Pred)
	{
		temp->tln_Pred->tln_Succ = node;
		node->tln_Pred = temp->tln_Pred;
		temp->tln_Succ = node->tln_Succ;
		temp->tln_Pred = node;
		node->tln_Succ->tln_Pred = temp;
		node->tln_Succ = temp;
	}
}

/*****************************************************************************/
/*
**	TInsert(list, node, prednode)
**	Insert node after prednode
*/

TLIBAPI void
TInsert(struct TList *list, struct TNode *node, struct TNode *prednode)
{
	if (list)
	{
		if (prednode)
		{
			struct TNode *temp = prednode->tln_Succ;
			if (temp)
			{
				node->tln_Succ = temp;
				node->tln_Pred = prednode;
				temp->tln_Pred = node;
				prednode->tln_Succ = node;
			}
			else
			{
				node->tln_Succ = prednode;
				temp = prednode->tln_Pred;
				node->tln_Pred = temp;
				prednode->tln_Pred = node;
				temp->tln_Succ = node;
			}
		}
		else
			TAddHead(list, node);
	}
}

/*****************************************************************************/
/*
**	TDestroy(handle)
**	Invoke destructor on a handle
*/

TLIBAPI void
TDestroy(TAPTR handle)
{
	if (handle)
		TCALLHOOKPKT(&((struct THandle *) handle)->thn_Hook, handle,
			TMSG_DESTROY);
}

/*****************************************************************************/
/*
**	TDestroyList(list)
**	Unlink and invoke destructor on handles in a list
*/

TLIBAPI void
TDestroyList(struct TList *list)
{
	struct TNode *nextnode, *node = list->tlh_Head;
	while ((nextnode = node->tln_Succ))
	{
		TREMOVE(node);
		TCALLHOOKPKT(&((struct THandle *) node)->thn_Hook, node, TMSG_DESTROY);
		node = nextnode;
	}
}

/*****************************************************************************/
/*
**	modinst = TNewInstance(mod, possize, negsize)
**	Get module instance copy
*/

TLIBAPI TAPTR
TNewInstance(TAPTR mod, TUINT possize, TUINT negsize)
{
	TAPTR exec = TGetExecBase(mod);
	TAPTR inst = TExecAlloc(exec, TNULL, possize + negsize);
	if (inst)
	{
		TUINT size = TMIN(((struct TModule *) mod)->tmd_NegSize, negsize);
		inst = (TINT8 *) inst + negsize;
		if (size > 0)
			TExecCopyMem(exec, (TINT8 *) mod - size, (TINT8 *) inst - size,
				size);
		size = TMIN(((struct TModule *) mod)->tmd_PosSize, possize);
		TExecCopyMem(exec, mod, inst, size);
		((struct TModule *) inst)->tmd_PosSize = possize;
		((struct TModule *) inst)->tmd_NegSize = negsize;
		((struct TModule *) inst)->tmd_InitTask = TExecFindTask(exec, TNULL);
	}
	return inst;
}

/*****************************************************************************/
/*
**	TFreeInstance(mod)
**	Free module instance
*/

TLIBAPI void
TFreeInstance(TAPTR mod)
{
	if (mod)
	{
		TAPTR exec = TGetExecBase(mod);
		TExecFree(exec, (TINT8 *) mod - ((struct TModule *) mod)->tmd_NegSize);
	}
}

/*****************************************************************************/
/*
**	TInitVectors(mod, vectors, num)
**	Init module vectors
*/

TLIBAPI void
TInitVectors(TAPTR mod, const TMFPTR *vectors, TUINT numv)
{
	TMFPTR *vecp = mod;
	while (numv--)
		*(--vecp) = *vectors++;
}

/*****************************************************************************/
/*
**	complete = TForEachTag(taglist, hook)
*/

TLIBAPI TBOOL
TForEachTag(struct TTagItem *taglist, struct THook *hook)
{
	TBOOL complete = TFALSE;
	if (hook)
	{
		complete = TTRUE;
		while (taglist && complete)
		{
			switch ((TUINT) taglist->tti_Tag)
			{
				case TTAG_DONE:
					return complete;

				case TTAG_MORE:
					taglist = (struct TTagItem *) taglist->tti_Value;
					break;

				case TTAG_SKIP:
					taglist += 1 + (TINT) taglist->tti_Value;
					break;

				case TTAG_GOSUB:
					complete =
						TForEachTag((struct TTagItem *) taglist->tti_Value,
							hook);
					taglist++;
					break;

				default:
					complete = (TBOOL)
						TCALLHOOKPKT(hook, taglist, TMSG_FOREACHTAG);

				case TTAG_IGNORE:
					taglist++;
					break;
			}
		}
	}
	return TFALSE;
}

/*****************************************************************************/
/*
**	tag = TGetTag(taglist, tag, defvalue)
**	Get tag value
*/

TLIBAPI TTAG
TGetTag(struct TTagItem *taglist, TUINT tag, TTAG defvalue)
{
	TUINT listtag;
	while (taglist)
	{
		listtag = taglist->tti_Tag;
		switch (listtag)
		{
			case TTAG_DONE:
				return defvalue;

			case TTAG_MORE:
				taglist = (struct TTagItem *) taglist->tti_Value;
				break;

			case TTAG_SKIP:
				taglist += 1 + (TINT) taglist->tti_Value;
				break;

			case TTAG_GOSUB:
			{
				TTAG res = TGetTag((struct TTagItem *) taglist->tti_Value,
					tag,defvalue);
				if (res != defvalue)
					return res;
				taglist++;
				break;
			}

			default:
				if (tag == listtag)
					return taglist->tti_Value;

			case TTAG_IGNORE:
				taglist++;
				break;
		}
	}
	return defvalue;
}

/*****************************************************************************/
/*
**	handle = TFindHandle(list, name)
**	Find named handle
*/

TLIBAPI struct THandle *
TFindHandle(struct TList *list, TSTRPTR name)
{
	struct TNode *nnode, *node;
	for (node = list->tlh_Head; (nnode = node->tln_Succ); node = nnode)
	{
		TSTRPTR s1 = ((struct THandle *) node)->thn_Name;
		if (s1 && name)
		{
			TSTRPTR s2 = name;
			TINT a;
			while ((a = *s1++) == *s2++)
			{
				if (a == 0)
					return (struct THandle *) node;
			}
		}
		else if (s1 == TNULL && name == TNULL)
			return (struct THandle *) node;
	}
	return TNULL;
}

/*****************************************************************************/
/*
**	TEKlib hooks allow transitions from e.g. register- to stack-based
**	calling conventions. hook->thk_Entry is invoked by TCallHookPkt()
**	and follows TEKlib's per-platform calling conventions (as declared
**	with THOOKENTRY). It however may point to a stub function that calls
**	the actual user function in hook->thk_SubEntry, which in turn may
**	be entirely language/compiler specific.
*/

static THOOKENTRY TTAG
_THookEntry(struct THook *hook, TAPTR obj, TTAG msg)
{
	TTAG (*func)(struct THook *, TAPTR, TTAG) =
		(TTAG (*)(struct THook *, TAPTR, TTAG)) hook->thk_SubEntry;
	return (*func)(hook, obj, msg);
}

TLIBAPI void
TInitHook(struct THook *hook, THOOKFUNC func, TAPTR data)
{
	hook->thk_Entry = _THookEntry;
	hook->thk_SubEntry = func;
	hook->thk_Data = data;
}

TLIBAPI TTAG
TCallHookPkt(struct THook *hook, TAPTR obj, TTAG msg)
{
	return (*hook->thk_Entry)(hook, obj, msg);
}

