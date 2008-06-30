
/*
**	$Id: exec_memory.c,v 1.1 2008-06-30 12:34:22 tmueller Exp $
**	teklib/src/exec/exec_memory.c - Exec memory management
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/debug.h>
#include <tek/teklib.h>
#include "exec_mod.h"

static const union TMemMsg msg_destroy = { TMMSG_DESTROY };

/*****************************************************************************/
/*
**	mmu = exec_CreateMMU(exec, allocator, mmutype, tags)
**	Create a memory manager
*/

static THOOKENTRY TTAG
exec_destroymmu(struct THook *hook, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		struct TMemManager *mmu = obj;
		TCALLHOOKPKT(&mmu->tmm_Hook, mmu, (TTAG) &msg_destroy);
		exec_Free((TEXECBASE *) mmu->tmm_Handle.thn_Owner, mmu);
	}
	return 0;
}

static THOOKENTRY TTAG
exec_destroymmu_and_allocator(struct THook *hook, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		struct TMemManager *mmu = obj;
		TCALLHOOKPKT(&mmu->tmm_Hook, mmu, (TTAG) &msg_destroy);
		TDESTROY(mmu->tmm_Allocator);
		exec_Free((TEXECBASE *) mmu->tmm_Handle.thn_Owner, mmu);
	}
	return 0;
}

static THOOKENTRY TTAG
exec_destroymmu_and_free(struct THook *hook, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		struct TMemManager *mmu = obj;
		TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
		TCALLHOOKPKT(&mmu->tmm_Hook, mmu, (TTAG) &msg_destroy);
		exec_Free(exec, mmu->tmm_Allocator);
		exec_Free(exec, mmu);
	}
	return 0;
}

EXPORT struct TMemManager *
exec_CreateMMU(TEXECBASE *exec, TAPTR allocator, TUINT mmutype,
	struct TTagItem *tags)
{
	struct TMemManager *mmu = exec_AllocMMU(exec, TNULL,
		sizeof(struct TMemManager));
	if (mmu)
	{
		THOOKENTRY THOOKFUNC destructor = TNULL;
		TBOOL success = TTRUE;
		TAPTR staticmem = TNULL;

		if ((mmutype & TMMUT_Pooled) && allocator == TNULL)
		{
			/* Create a MMU based on an internal pooled allocator */
			allocator = exec_CreatePool(exec, tags);
			destructor = exec_destroymmu_and_allocator;
		}
		else if (mmutype & TMMUT_Static)
		{
			/* Create a MMU based on a static memory block */
			TUINT blocksize = (TUINT) TGetTag(tags, TMem_StaticSize, 0);
			success = TFALSE;
			if (blocksize > sizeof(union TMemHead))
			{
				if (allocator == TNULL)
				{
					allocator = staticmem = exec_AllocMMU(exec, TNULL,
						blocksize);
					if (allocator)
					{
						destructor = exec_destroymmu_and_free;
						success = TTRUE;
					}
				}
				else
				{
					destructor = exec_destroymmu;
					success = TTRUE;
				}

				if (success)
				{
					TUINT flags = TGetTag(tags, TMem_LowFrag, TFALSE) ?
						TMEMHF_LOWFRAG : TMEMHF_NONE;
					TAPTR block = (TAPTR)
						(((union TMemHead *) allocator) + 1);
					blocksize -= sizeof(union TMemHead);
					success = exec_initmemhead(allocator, block, blocksize,
						flags, 0);
					if (!success)
						TDBPRINTF(TDB_FAIL,("static allocator failed\n"));
				}
			}
		}
		else
			destructor = exec_destroymmu;

		if (success)
		{
			if (exec_initmmu(exec, mmu, allocator, mmutype, tags))
			{
				/* Overwrite destructor. The one provided by exec_initmmu()
				doesn't know how to free the memory manager. */
				mmu->tmm_Handle.thn_Hook.thk_Entry = destructor;
				return mmu;
			}
		}

		exec_Free(exec, staticmem);
		exec_Free(exec, mmu);
	}
	return TNULL;
}

/*****************************************************************************/
/*
**	exec_CopyMem(exec, from, to, size)
**	Copy memory
*/

EXPORT void
exec_CopyMem(TEXECBASE *exec, TAPTR from, TAPTR to, TUINT numbytes)
{
	THALCopyMem(exec->texb_HALBase, from, to, numbytes);
}

/*****************************************************************************/
/*
**	exec_FillMem(exec, dest, numbytes, fillval)
**	Fill memory
*/

EXPORT void
exec_FillMem(TEXECBASE *exec, TAPTR dest, TUINT len, TUINT8 fillval)
{
	if (((TUINTPTR) dest | len) & 3)
		THALFillMem(exec->texb_HALBase, dest, len, fillval);
	else
	{
		TUINT f32;
		f32 = fillval;
		f32 = (f32 << 8) | f32;
		f32 = (f32 << 16) | f32;
		exec_FillMem32(exec, dest, len, f32);
	}
}

/*****************************************************************************/
/*
**	exec_FillMem32(exec, dest, numbytes, fillval)
**	Fill memory, 32bit aligned
*/

EXPORT void
exec_FillMem32(TEXECBASE *exec, TUINT *dest, TUINT len, TUINT fill)
{
	TUINT len8;
	len >>= 2;
	len8 = (len >> 3) + 1;

	switch (len & 7)
	{
		do
		{
					*dest++ = fill;
			case 7:	*dest++ = fill;
			case 6:	*dest++ = fill;
			case 5:	*dest++ = fill;
			case 4:	*dest++ = fill;
			case 3:	*dest++ = fill;
			case 2:	*dest++ = fill;
			case 1:	*dest++ = fill;
			case 0:	len8--;

		} while (len8);
	}
}

/*****************************************************************************/
/*
**	mem = exec_AllocMMU(exec, mmu, size)
**	Allocate memory via memory manager
*/

EXPORT TAPTR
exec_AllocMMU(TEXECBASE *exec, struct TMemManager *mmu, TUINT size)
{
	union TMMUInfo *mem = TNULL;
	if (size)
	{
		union TMemMsg msg;
		msg.tmmsg_Type = TMMSG_ALLOC;
		msg.tmmsg_Alloc.tmmsg_Size = size + sizeof(union TMMUInfo);

		if (mmu == TNULL)
			mmu = &exec->texb_BaseMMU;

		mem = (union TMMUInfo *) TCALLHOOKPKT(&mmu->tmm_Hook, mmu,
			(TTAG) &msg);
		if (mem)
		{
			mem->tmu_Node.tmu_UserSize = size;
			mem->tmu_Node.tmu_MMU = mmu;
			mem++;
		}
		else
			TDBPRINTF(TDB_WARN,("alloc failed size %d\n", size));
	}
	else
		TDBPRINTF(TDB_WARN,("called with size=0\n"));

	return (TAPTR) mem;
}

/*****************************************************************************/
/*
**	mem = exec_AllocMMU0(exec, mmu, size)
**	Allocate memory via memory manager, zero'ed out
*/

EXPORT TAPTR
exec_AllocMMU0(TEXECBASE *exec, struct TMemManager *mmu, TUINT size)
{
	TAPTR mem = exec_AllocMMU(exec, mmu, size);
	if (mem)
		exec_FillMem(exec, mem, size, 0);
	return mem;
}

/*****************************************************************************/
/*
**	mem = exec_Realloc(exec, mem, newsize)
**	Reallocate an existing memory manager allocation. Note that it is not
**	possible to allocate a fresh block of memory with this function.
*/

EXPORT TAPTR
exec_Realloc(TEXECBASE *exec, TAPTR mem, TUINT newsize)
{
	union TMMUInfo *newmem = TNULL;
	if (mem)
	{
		union TMMUInfo *mmuinfo = (union TMMUInfo *) mem - 1;
		struct TMemManager *mmu = mmuinfo->tmu_Node.tmu_MMU;
		union TMemMsg msg;

		if (mmu == TNULL)
			mmu = &exec->texb_BaseMMU;

		if (newsize)
		{
			TUINT oldsize = mmuinfo->tmu_Node.tmu_UserSize;
			if (oldsize == newsize)
				return mem;

			msg.tmmsg_Type = TMMSG_REALLOC;
			msg.tmmsg_Realloc.tmmsg_Ptr = mmuinfo;
			msg.tmmsg_Realloc.tmmsg_OSize = oldsize + sizeof(union TMMUInfo);
			msg.tmmsg_Realloc.tmmsg_NSize = newsize + sizeof(union TMMUInfo);

			newmem = (union TMMUInfo *)
				TCALLHOOKPKT(&mmu->tmm_Hook, mmu, (TTAG) &msg);

			if (newmem)
			{
				newmem->tmu_Node.tmu_UserSize = newsize;
				newmem++;
			}
		}
		else
		{
			msg.tmmsg_Type = TMMSG_FREE;
			msg.tmmsg_Free.tmmsg_Ptr = mmuinfo;
			msg.tmmsg_Free.tmmsg_Size =
				mmuinfo->tmu_Node.tmu_UserSize + sizeof(union TMMUInfo);
			TCALLHOOKPKT(&mmu->tmm_Hook, mmu, (TTAG) &msg);
		}
	}
	return newmem;
}

/*****************************************************************************/
/*
**	exec_Free(exec, mem)
**	Return memory allocated from a memory manager
*/

EXPORT void
exec_Free(TEXECBASE *exec, TAPTR mem)
{
	if (mem)
	{
		union TMMUInfo *mmuinfo = (union TMMUInfo *) mem - 1;
		struct TMemManager *mmu = mmuinfo->tmu_Node.tmu_MMU;
		union TMemMsg msg;

		if (mmu == TNULL)
			mmu = &exec->texb_BaseMMU;
		msg.tmmsg_Type = TMMSG_FREE;
		msg.tmmsg_Free.tmmsg_Ptr = mmuinfo;
		msg.tmmsg_Free.tmmsg_Size =
			mmuinfo->tmu_Node.tmu_UserSize + sizeof(union TMMUInfo);
		TCALLHOOKPKT(&mmu->tmm_Hook, mmu, (TTAG) &msg);
	}
}

/*****************************************************************************/
/*
**	success = exec_initmemhead(mh, mem, size, flags, bytealign)
**	Init memheader
*/

LOCAL TBOOL
exec_initmemhead(union TMemHead *mh, TAPTR mem, TUINT size, TUINT flags,
	TUINT bytealign)
{
	TUINT align = sizeof(TAPTR);

	bytealign = TMAX(bytealign, sizeof(union TMemNode));
	while (align < bytealign) align <<= 1;
	if (size >= align)
	{
		--align;
		size &= ~align;

		mh->tmh_Node.tmh_Mem = mem;
		mh->tmh_Node.tmh_MemEnd = ((TINT8 *) mem) + size;
		mh->tmh_Node.tmh_Free = size;
		mh->tmh_Node.tmh_Align = align;
		mh->tmh_Node.tmh_Flags = flags;

		mh->tmh_Node.tmh_FreeList = (union TMemNode *) mem;
		((union TMemNode *) mem)->tmn_Node.tmn_Next = TNULL;
		((union TMemNode *) mem)->tmn_Node.tmn_Size = size;

		return TTRUE;
	}

	return TFALSE;
}

/*****************************************************************************/
/*
**	mem = exec_staticalloc(memhead, size)
**	Allocate from a static memheader
*/

LOCAL TAPTR
exec_staticalloc(union TMemHead *mh, TUINT size)
{
	size = (size + mh->tmh_Node.tmh_Align) & ~mh->tmh_Node.tmh_Align;

	if (mh->tmh_Node.tmh_Free >= size)
	{
		union TMemNode **mnp, *mn, **x;

		mnp = &mh->tmh_Node.tmh_FreeList;
		x = TNULL;

		if (mh->tmh_Node.tmh_Flags & TMEMHF_LOWFRAG)
		{
			/* bestfit strategy */

			TUINT bestsize = 0xffffffff;
			while ((mn = *mnp))
			{
				if (mn->tmn_Node.tmn_Size == size)
				{
exactfit:			*mnp = mn->tmn_Node.tmn_Next;
					mh->tmh_Node.tmh_Free -= size;
					return (TAPTR) mn;
				}
				else if (mn->tmn_Node.tmn_Size > size)
				{
					if (mn->tmn_Node.tmn_Size < bestsize)
					{
						bestsize = mn->tmn_Node.tmn_Size;
						x = mnp;
					}
				}
				mnp = &mn->tmn_Node.tmn_Next;
			}
		}
		else
		{
			/* firstfit strategy */

			while ((mn = *mnp))
			{
				if (mn->tmn_Node.tmn_Size == size)
				{
					goto exactfit;
				}
				else if (mn->tmn_Node.tmn_Size > size)
				{
					x = mnp;
					break;
				}
				mnp = &mn->tmn_Node.tmn_Next;
			}
		}

		if (x)
		{
			mn = *x;
			*x = (union TMemNode *) ((TINT8 *) mn + size);
			(*x)->tmn_Node.tmn_Next = mn->tmn_Node.tmn_Next;
			(*x)->tmn_Node.tmn_Size = mn->tmn_Node.tmn_Size - size;
			mh->tmh_Node.tmh_Free -= size;
			return (TAPTR) mn;
		}
	}

	return TNULL;
}

/*****************************************************************************/
/*
**	exec_staticfree(mh, mem, size)
**	Return memory to a static allocator
*/

LOCAL void
exec_staticfree(union TMemHead *mh, TAPTR mem, TUINT size)
{
	union TMemNode **mnp, *mn, *pmn;

	size = (size + mh->tmh_Node.tmh_Align) & ~mh->tmh_Node.tmh_Align;
	mh->tmh_Node.tmh_Free += size;
	mnp = &mh->tmh_Node.tmh_FreeList;
	pmn = TNULL;

	while ((mn = *mnp))
	{
		if ((TINT8 *) mem < (TINT8 *) mn) break;
		pmn = mn;
		mnp = &mn->tmn_Node.tmn_Next;
	}

	if (mn && ((TINT8 *) mem + size == (TINT8 *) mn))
	{
		size += mn->tmn_Node.tmn_Size;
		/* concatenate with following free node */
		mn = mn->tmn_Node.tmn_Next;
	}

	if (pmn && ((TINT8 *) pmn + pmn->tmn_Node.tmn_Size == (TINT8 *) mem))
	{
		/* concatenate with previous free node */
		size += pmn->tmn_Node.tmn_Size;
		mem = pmn;
	}

	*mnp = (union TMemNode *) mem;
	((union TMemNode *) mem)->tmn_Node.tmn_Next = mn;
	((union TMemNode *) mem)->tmn_Node.tmn_Size = size;
}

/*****************************************************************************/
/*
**	newmem = exec_staticrealloc(hal, mh, oldmem, oldsize, newsize)
**	Realloc static memheader allocation
*/

LOCAL TAPTR
exec_staticrealloc(TAPTR hal, union TMemHead *mh, TAPTR oldmem,
	TUINT oldsize, TUINT newsize)
{
	TAPTR newmem;
	union TMemNode **mnp, *mn, *mend;
	TUINT diffsize;

	oldsize = (oldsize + mh->tmh_Node.tmh_Align) & ~mh->tmh_Node.tmh_Align;
	newsize = (newsize + mh->tmh_Node.tmh_Align) & ~mh->tmh_Node.tmh_Align;

	if (newsize == oldsize)
		return oldmem;

	/* end of old allocation */
	mend = (union TMemNode *) (((TINT8 *) oldmem) + oldsize);
	mnp = &mh->tmh_Node.tmh_FreeList;

scan:

	mn = *mnp;
	if (mn == TNULL)
		goto notfound;
	if (mn < mend)
	{
		mnp = &mn->tmn_Node.tmn_Next;
		goto scan;
	}

	if (newsize > oldsize)
	{
		/* grow allocation */
		if (mn == mend)
		{
			/* there is a free node at end */
			diffsize = newsize - oldsize;
			if (mn->tmn_Node.tmn_Size == diffsize)
			{
				/* exact match: swallow free node */
				*mnp = mn->tmn_Node.tmn_Next;
				mh->tmh_Node.tmh_Free -= diffsize;
				return oldmem;
			}
			else if (mn->tmn_Node.tmn_Size > diffsize)
			{
				/* free node is larger: move free node */
				mend = (union TMemNode *) (((TINT8 *) mend) + diffsize);
				*mnp = mend;
				mend->tmn_Node.tmn_Next = mn->tmn_Node.tmn_Next;
				mend->tmn_Node.tmn_Size = mn->tmn_Node.tmn_Size - diffsize;
				mh->tmh_Node.tmh_Free -= diffsize;
				return oldmem;
			}
			/* else not enough space */
		}
		/* else no free node at end */
	}
	else
	{
		/* shrink allocation */
		diffsize = oldsize - newsize;
		if (mn == mend)
		{
			/* merge with following free node */
			mend = (union TMemNode *) (((TINT8 *) mend) - diffsize);
			*mnp = mend;
			mend->tmn_Node.tmn_Next = mn->tmn_Node.tmn_Next;
			mend->tmn_Node.tmn_Size = mn->tmn_Node.tmn_Size + diffsize;
		}
		else
		{
			/* add new free node */
			mend = (union TMemNode *) (((TINT8 *) mend) - diffsize);
			*mnp = mend;
			mend->tmn_Node.tmn_Next = mn;
			mend->tmn_Node.tmn_Size = diffsize;
		}
		mh->tmh_Node.tmh_Free += diffsize;
		return oldmem;
	}

notfound:

	newmem = exec_staticalloc(mh, newsize);
	if (newmem)
	{
		THALCopyMem(hal, oldmem, newmem, TMIN(oldsize, newsize));
		exec_staticfree(mh, oldmem, oldsize);
	}

	return newmem;
}

/*****************************************************************************/
/*
**	size = exec_GetSize(exec, allocation)
**	Get size of an allocation made from a memory manager
*/

EXPORT TUINT
exec_GetSize(TEXECBASE *exec, TAPTR mem)
{
	return mem ? ((union TMMUInfo *) mem - 1)->tmu_Node.tmu_UserSize : 0;
}

/*****************************************************************************/
/*
**	mmu = exec_GetMMU(exec, allocation)
**	Get the memory manager an allocation was made from
*/

EXPORT TAPTR
exec_GetMMU(TEXECBASE *exec, TAPTR mem)
{
	return mem ? ((union TMMUInfo *) mem - 1)->tmu_Node.tmu_MMU : TNULL;
}

/*****************************************************************************/
/*
**	pool = exec_CreatePool(exec, tags)
**	Create memory pool
*/

static THOOKENTRY TTAG
exec_destroypool(struct THook *hook, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		struct TMemPool *pool = obj;
		TEXECBASE *exec = (TEXECBASE *) TGetExecBase(pool);
		union TMemHead *node = (union TMemHead *) pool->tpl_List.tlh_Head;
		struct TNode *nnode;
		if (pool->tpl_Flags & TMEMHF_FREE)
		{
			while ((nnode = ((struct TNode *) node)->tln_Succ))
			{
				exec_Free(exec, node);
				node = (union TMemHead *) nnode;
			}
		}
		exec_Free(exec, pool);
	}
	return 0;
}

EXPORT TAPTR
exec_CreatePool(TEXECBASE *exec, struct TTagItem *tags)
{
	TUINT fixedsize = TGetTag(tags, TPool_StaticSize, 0);
	if (fixedsize)
	{
		struct TMemPool *pool = exec_AllocMMU(exec, TNULL,
			sizeof(struct TMemPool));
		if (pool)
		{
			union TMemHead *fixed = (TAPTR) TGetTag(tags, TPool_Static, TNULL);
			pool->tpl_Flags = TMEMHF_FIXED;
			if (fixed == TNULL)
			{
				TAPTR mmu = (TAPTR) TGetTag(tags, TPool_MMU, TNULL);
				fixed = exec_AllocMMU(exec, mmu, fixedsize);
				pool->tpl_Flags |= TMEMHF_FREE;
			}

			if (fixed)
			{
				pool->tpl_Align = sizeof(union TMemNode) - 1;
				pool->tpl_Flags |= ((TBOOL) TGetTag(tags, TMem_LowFrag,
					(TTAG) TFALSE)) ? TMEMHF_LOWFRAG : TMEMHF_NONE;

				pool->tpl_Handle.thn_Owner = (struct TModule *) exec;
				pool->tpl_Handle.thn_Hook.thk_Entry = exec_destroypool;

				TINITLIST(&pool->tpl_List);
				exec_initmemhead(fixed, fixed + 1,
					fixedsize - sizeof(union TMemHead),
					pool->tpl_Flags, pool->tpl_Align + 1);
				TAddHead(&pool->tpl_List, (struct TNode *) fixed);
			}
			else
			{
				exec_Free(exec, pool);
				pool = TNULL;
			}
		}

		return pool;
	}
	else
	{
		TUINT pudsize = (TUINT) TGetTag(tags, TPool_PudSize, (TTAG) 1024);
		TUINT thressize = (TUINT) TGetTag(tags, TPool_ThresSize, (TTAG) 256);
		if (pudsize >= thressize)
		{
			struct TMemPool *pool = exec_AllocMMU(exec, TNULL,
				sizeof(struct TMemPool));
			if (pool)
			{
				pool->tpl_Align = sizeof(union TMemNode) - 1;
				pool->tpl_PudSize = pudsize;
				pool->tpl_ThresSize = thressize;

				pool->tpl_Flags = TMEMHF_FREE;
				pool->tpl_Flags |= ((TBOOL) TGetTag(tags,
					TPool_AutoAdapt, (TTAG) TTRUE)) ?
						TMEMHF_AUTO : TMEMHF_NONE;
				pool->tpl_Flags |= ((TBOOL) TGetTag(tags, TMem_LowFrag,
					(TTAG) TFALSE)) ? TMEMHF_LOWFRAG : TMEMHF_NONE;

				pool->tpl_Handle.thn_Owner = (struct TModule *) exec;
				pool->tpl_Handle.thn_Hook.thk_Entry = exec_destroypool;

				pool->tpl_MMU = (TAPTR) TGetTag(tags, TPool_MMU, TNULL);

				TINITLIST(&pool->tpl_List);
			}

			return pool;
		}
	}

	return TNULL;
}

/*****************************************************************************/
/*
**	mem = exec_AllocPool(exec, pool, size)
**	Alloc memory from a pool
*/

EXPORT TAPTR
exec_AllocPool(TEXECBASE *exec, struct TMemPool *pool, TUINT size)
{
	if (size && pool)
	{
		union TMemHead *node;

		if (pool->tpl_Flags & TMEMHF_FIXED)
		{
			node = (union TMemHead *) pool->tpl_List.tlh_Head;
			return exec_staticalloc(node, size);
		}

		if (pool->tpl_Flags & TMEMHF_AUTO)
		{
			/* auto-adapt pool parameters */

			TINT p, t;

			t = pool->tpl_ThresSize;
			/* tend to 4x typical size */
			t += (((TINT) size) - (t >> 2)) >> 2;
			p = pool->tpl_PudSize;
			/* tend to 8x thressize */
			p += (t - (p >> 3)) >> 3;
			if (t > p) p = t;

			pool->tpl_ThresSize = t;
			pool->tpl_PudSize = p;
		}

		if (size <= ((pool->tpl_ThresSize + pool->tpl_Align) &
			~pool->tpl_Align))
		{
			/* regular pool allocation */

			TAPTR mem;
			struct TNode *tempnode;
			TUINT pudsize;

			node = (union TMemHead *) pool->tpl_List.tlh_Head;
			while ((tempnode = ((struct TNode *) node)->tln_Succ))
			{
				if (node->tmh_Node.tmh_Flags & TMEMHF_LARGE)
					break;
				mem = exec_staticalloc(node, size);
				if (mem)
					return mem;
				node = (union TMemHead *) tempnode;
			}

			pudsize = (pool->tpl_PudSize + pool->tpl_Align) & ~pool->tpl_Align;
			node = exec_AllocMMU(exec, pool->tpl_MMU,
				sizeof(union TMemHead) + pudsize);
			if (node)
			{
				exec_initmemhead(node, node + 1, pudsize, pool->tpl_Flags,
					pool->tpl_Align + 1);
				TAddHead(&pool->tpl_List, (struct TNode *) node);
				return exec_staticalloc(node, size);
			}
		}
		else
		{
			/* large allocation */

			size = (size + pool->tpl_Align) & ~pool->tpl_Align;
			node = exec_AllocMMU(exec, pool->tpl_MMU,
				sizeof(union TMemHead) + size);
			if (node)
			{
				exec_initmemhead(node, node + 1, size,
					pool->tpl_Flags | TMEMHF_LARGE, pool->tpl_Align + 1);
				TAddTail(&pool->tpl_List, (struct TNode *) node);
				return exec_staticalloc(node, size);
			}
		}
	}
	return TNULL;
}

/*****************************************************************************/
/*
**	exec_FreePool(exec, pool, mem, size)
**	Return memory to a pool
*/

EXPORT void
exec_FreePool(TEXECBASE *exec, struct TMemPool *pool, TINT8 *mem, TUINT size)
{
	TDBASSERT(99, (mem == TNULL) == (size == 0));
	if (mem)
	{
		union TMemHead *node = (union TMemHead *) pool->tpl_List.tlh_Head;
		if (pool->tpl_Flags & TMEMHF_FIXED)
			exec_staticfree(node, mem, size);
		else
		{
			struct TNode *temp;
			TINT8 *memend = mem + size;
			while ((temp = ((struct TNode *) node)->tln_Succ))
			{
				if (mem >= node->tmh_Node.tmh_Mem && memend <=
					node->tmh_Node.tmh_MemEnd)
				{
					exec_staticfree(node, mem, size);
					if (node->tmh_Node.tmh_Free ==
						(TUINTPTR) node->tmh_Node.tmh_MemEnd -
						(TUINTPTR) node->tmh_Node.tmh_Mem)
					{
						/* flush puddle */
						TREMOVE((struct TNode *) node);
						exec_Free(exec, node);
					}
					else
					{
						/* recently used node moves up */
						TNodeUp((struct TNode *) node);
					}

					return;
				}
				node = (union TMemHead *) temp;
			}
			TDBASSERT(99, TFALSE);
		}
	}
}

/*****************************************************************************/
/*
**	newmem = exec_ReallocPool(exec, pool, oldmem, oldsize, newsize)
**	Realloc pool allocation
*/

EXPORT TAPTR
exec_ReallocPool(TEXECBASE *exec, struct TMemPool *pool, TINT8 *oldmem,
	TUINT oldsize, TUINT newsize)
{
	if (oldmem && oldsize)
	{
		struct TNode *tempnode;
		union TMemHead *node;
		TINT8 *memend;

		if (newsize == 0)
		{
			exec_FreePool(exec, pool, oldmem, oldsize);
			return TNULL;
		}

		node = (union TMemHead *) pool->tpl_List.tlh_Head;

		if (pool->tpl_Flags & TMEMHF_FIXED)
		{
			return exec_staticrealloc(exec->texb_HALBase, node, oldmem,
				oldsize, newsize);
		}

		memend = oldmem + oldsize;

		while ((tempnode = ((struct TNode *) node)->tln_Succ))
		{
			if (oldmem >= node->tmh_Node.tmh_Mem && memend <=
				node->tmh_Node.tmh_MemEnd)
			{
				TAPTR newmem;

				newmem = exec_staticrealloc(exec->texb_HALBase, node, oldmem,
					oldsize, newsize);
				if (newmem)
				{
					if (node->tmh_Node.tmh_Flags & TMEMHF_LARGE)
					{
						/* this is no longer a large node */
						TREMOVE((struct TNode *) node);
						TAddHead(&pool->tpl_List, (struct TNode *) node);
						node->tmh_Node.tmh_Flags &= ~TMEMHF_LARGE;
					}
					return newmem;
				}

				newmem = exec_AllocPool(exec, pool, newsize);
				if (newmem)
				{
					THALCopyMem(exec->texb_HALBase, oldmem, newmem,
						TMIN(oldsize, newsize));
					exec_staticfree(node, oldmem, oldsize);
					if (node->tmh_Node.tmh_Free ==
						(TUINTPTR) node->tmh_Node.tmh_MemEnd -
						(TUINTPTR) node->tmh_Node.tmh_Mem)
					{
						/* flush puddle */
						TREMOVE((struct TNode *) node);
						exec_Free(exec, node);
					}
					else
					{
						/* recently used node moves up */
						TNodeUp((struct TNode *) node);
					}
				}
				return newmem;
			}
			node = (union TMemHead *) tempnode;
		}
	}
	else if ((oldmem == TNULL) && (oldsize == 0))
		return exec_AllocPool(exec, pool, newsize);

	TDBASSERT(99, TFALSE);
	return TNULL;
}

/*****************************************************************************/
/*
**	TNULL message allocator -
**	Note that this kind of allocator is using execbase->texb_Lock, not
**	the lock supplied with the MMU. There may be no 'self'-context available
**	when a message MMU is used to allocate the initial task structures
**	during the Exec setup.
*/

static TAPTR
exec_mmu_msgalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TAPTR hal = exec->texb_HALBase;
	TINT8 *mem;
	THALLock(hal, &exec->texb_Lock);
	mem = THALAlloc(hal, size +
		sizeof(struct TNode) + sizeof(struct TMessage));
	if (mem)
	{
		TAddTail(&mmu->tmm_TrackList, (struct TNode *) mem);
		THALUnlock(hal, &exec->texb_Lock);
		((struct TMessage *) (mem + sizeof(struct TNode)))->tmsg_Flags =
			TMSG_STATUS_FAILED;
		return (TAPTR) (mem + sizeof(struct TNode) + sizeof(struct TMessage));
	}
	THALUnlock(hal, &exec->texb_Lock);
	return TNULL;
}

static void
exec_mmu_msgfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TAPTR hal = exec->texb_HALBase;
	THALLock(hal, &exec->texb_Lock);
	TREMOVE((struct TNode *) (mem - sizeof(struct TNode) -
		sizeof(struct TMessage)));
	THALFree(hal, mem - sizeof(struct TNode) - sizeof(struct TMessage),
		size + sizeof(struct TNode) + sizeof(struct TMessage));
	THALUnlock(hal, &exec->texb_Lock);
}

static void
exec_mmu_msgdestroy(struct TMemManager *mmu)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TAPTR hal = exec->texb_HALBase;
	struct TNode *nextnode, *node = mmu->tmm_TrackList.tlh_Head;
	TINT numfreed = 0;
	while ((nextnode = node->tln_Succ))
	{
		THALFree(hal, node, ((union TMMUInfo *) ((TINT8 *) (node + 1) +
			sizeof(struct TMessage)))->tmu_Node.tmu_UserSize +
				sizeof(struct TNode) +
			sizeof(struct TMessage) + sizeof(union TMMUInfo));
		numfreed++;
		node = nextnode;
	}
	if (numfreed)
		TDBPRINTF(TDB_ERROR,("freed %d pending allocations\n", numfreed));
}

static THOOKENTRY TTAG
exec_mmu_msg(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			exec_mmu_msgdestroy(mmu);
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_msgalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_msgfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			TDBPRINTF(TDB_ERROR,("messages cannot be reallocated\n"));
			break;
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	static memheader allocator
*/

static TAPTR
exec_mmu_staticalloc(struct TMemManager *mmu, TUINT size)
{
	return exec_staticalloc(mmu->tmm_Allocator, size);
}

static void
exec_mmu_staticfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	exec_staticfree(mmu->tmm_Allocator, mem, size);
}

static TAPTR
exec_mmu_staticrealloc(struct TMemManager *mmu, TINT8 *oldmem, TUINT oldsize,
	TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	return exec_staticrealloc(exec->texb_HALBase, mmu->tmm_Allocator, oldmem,
		oldsize, newsize);
}

static THOOKENTRY TTAG
exec_mmu_static(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_staticalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_staticfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_staticrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	pooled allocator
*/

static TAPTR
exec_mmu_poolalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	return exec_AllocPool(exec, mmu->tmm_Allocator, size);
}

static void
exec_mmu_poolfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	exec_FreePool(exec, mmu->tmm_Allocator, mem, size);
}

static TAPTR
exec_mmu_poolrealloc(struct TMemManager *mmu, TINT8 *oldmem, TUINT oldsize,
	TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	return exec_ReallocPool(exec, mmu->tmm_Allocator, oldmem,
		oldsize, newsize);
}

static THOOKENTRY TTAG
exec_mmu_pool(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_poolalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_poolfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_poolrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	pooled+tasksafe allocator
*/

static TAPTR
exec_mmu_pooltaskalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *mem;
	exec_Lock(exec, &mmu->tmm_Lock);
	mem = exec_AllocPool(exec, mmu->tmm_Allocator, size);
	exec_Unlock(exec, &mmu->tmm_Lock);
	return mem;
}

static void
exec_mmu_pooltaskfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	exec_Lock(exec, &mmu->tmm_Lock);
	exec_FreePool(exec, mmu->tmm_Allocator, mem, size);
	exec_Unlock(exec, &mmu->tmm_Lock);
}

static TAPTR
exec_mmu_pooltaskrealloc(struct TMemManager *mmu, TINT8 *oldmem, TUINT oldsize,
	TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *newmem;
	exec_Lock(exec, &mmu->tmm_Lock);
	newmem = exec_ReallocPool(exec, mmu->tmm_Allocator, oldmem, oldsize,
		newsize);
	exec_Unlock(exec, &mmu->tmm_Lock);
	return newmem;
}

static void
exec_mmu_pooltaskdestroy(struct TMemManager *mmu)
{
	TDESTROY(&mmu->tmm_Lock);
}

static THOOKENTRY TTAG
exec_mmu_pooltask(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			exec_mmu_pooltaskdestroy(mmu);
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_pooltaskalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_pooltaskfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_pooltaskrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	static memheader allocator, task-safe
*/

static TAPTR
exec_mmu_statictaskalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *mem;
	exec_Lock(exec, &mmu->tmm_Lock);
	mem = exec_staticalloc(mmu->tmm_Allocator, size);
	exec_Unlock(exec, &mmu->tmm_Lock);
	return mem;
}

static void
exec_mmu_statictaskfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	exec_Lock(exec, &mmu->tmm_Lock);
	exec_staticfree(mmu->tmm_Allocator, mem, size);
	exec_Unlock(exec, &mmu->tmm_Lock);
}

static TAPTR
exec_mmu_statictaskrealloc(struct TMemManager *mmu, TINT8 *oldmem,
	TUINT oldsize, TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *newmem;
	exec_Lock(exec, &mmu->tmm_Lock);
	newmem = exec_staticrealloc(exec->texb_HALBase, mmu->tmm_Allocator,
		oldmem, oldsize, newsize);
	exec_Unlock(exec, &mmu->tmm_Lock);
	return newmem;
}

static void
exec_mmu_statictaskdestroy(struct TMemManager *mmu)
{
	TDESTROY(&mmu->tmm_Lock);
}

static THOOKENTRY TTAG
exec_mmu_statictask(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			exec_mmu_statictaskdestroy(mmu);
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_statictaskalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_statictaskfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_statictaskrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	tasksafe MMU allocator
*/

static TAPTR
exec_mmu_taskalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *mem;
	exec_Lock(exec, &mmu->tmm_Lock);
	mem = exec_AllocMMU(exec, mmu->tmm_Allocator, size);
	exec_Unlock(exec, &mmu->tmm_Lock);
	return mem;
}

static TAPTR
exec_mmu_taskrealloc(struct TMemManager *mmu, TINT8 *oldmem, TUINT oldsize,
	TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *newmem;
	exec_Lock(exec, &mmu->tmm_Lock);
	newmem = exec_Realloc(exec, oldmem, newsize);
	exec_Unlock(exec, &mmu->tmm_Lock);
	return newmem;
}

static void
exec_mmu_taskfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	exec_Lock(exec, &mmu->tmm_Lock);
	exec_Free(exec, mem);
	exec_Unlock(exec, &mmu->tmm_Lock);
}

static void
exec_mmu_taskdestroy(struct TMemManager *mmu)
{
	TDESTROY(&mmu->tmm_Lock);
}

static THOOKENTRY TTAG
exec_mmu_task(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			exec_mmu_taskdestroy(mmu);
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_taskalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_taskfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_taskrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	tracking MMU allocator
*/

static TAPTR
exec_mmu_trackalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *mem = exec_AllocMMU(exec, mmu->tmm_Allocator,
		size + sizeof(struct TNode));
	if (mem)
	{
		TAddHead(&mmu->tmm_TrackList, (struct TNode *) mem);
		return (TAPTR) (mem + sizeof(struct TNode));
	}
	return TNULL;
}

static TAPTR
exec_mmu_trackrealloc(struct TMemManager *mmu, TINT8 *oldmem, TUINT oldsize,
	TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *newmem;
	TREMOVE((struct TNode *) (oldmem - sizeof(struct TNode)));
	newmem = exec_Realloc(exec, oldmem - sizeof(struct TNode),
		newsize + sizeof(struct TNode));
	if (newmem)
	{
		TAddHead(&mmu->tmm_TrackList, (struct TNode *) newmem);
		newmem += sizeof(struct TNode);
	}
	return (TAPTR) newmem;
}

static void
exec_mmu_trackfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TREMOVE((struct TNode *) (mem - sizeof(struct TNode)));
	exec_Free(exec, mem - sizeof(struct TNode));
}

static void
exec_mmu_trackdestroy(struct TMemManager *mmu)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	struct TNode *nextnode, *node = mmu->tmm_TrackList.tlh_Head;
	TINT numfreed = 0;
	while ((nextnode = node->tln_Succ))
	{
		exec_Free(exec, node);
		numfreed++;
		node = nextnode;
	}
	if (numfreed)
		TDBPRINTF(TDB_ERROR,("freed %d pending allocations\n", numfreed));
}

static THOOKENTRY TTAG
exec_mmu_track(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			exec_mmu_trackdestroy(mmu);
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_trackalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_trackfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_trackrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	tasksafe+tracking MMU allocator
*/

static TAPTR
exec_mmu_tasktrackalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *mem;
	exec_Lock(exec, &mmu->tmm_Lock);
	mem = exec_AllocMMU(exec, mmu->tmm_Allocator, size + sizeof(struct TNode));
	if (mem)
	{
		TAddHead(&mmu->tmm_TrackList, (struct TNode *) mem);
		mem += sizeof(struct TNode);
	}
	exec_Unlock(exec, &mmu->tmm_Lock);
	return (TAPTR) mem;
}

static TAPTR
exec_mmu_tasktrackrealloc(struct TMemManager *mmu, TINT8 *oldmem,
	TUINT oldsize, TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	TINT8 *newmem;
	exec_Lock(exec, &mmu->tmm_Lock);
	TREMOVE((struct TNode *) (oldmem - sizeof(struct TNode)));
	newmem = exec_Realloc(exec, oldmem - sizeof(struct TNode),
		newsize + sizeof(struct TNode));
	if (newmem)
	{
		TAddHead(&mmu->tmm_TrackList, (struct TNode *) newmem);
		newmem += sizeof(struct TNode);
	}
	exec_Unlock(exec, &mmu->tmm_Lock);
	return (TAPTR) newmem;
}

static void
exec_mmu_tasktrackfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	exec_Lock(exec, &mmu->tmm_Lock);
	TREMOVE((struct TNode *) (mem - sizeof(struct TNode)));
	exec_Free(exec, mem - sizeof(struct TNode));
	exec_Unlock(exec, &mmu->tmm_Lock);
}

static void
exec_mmu_tasktrackdestroy(struct TMemManager *mmu)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	struct TNode *nextnode, *node = mmu->tmm_TrackList.tlh_Head;
	TINT numfreed = 0;
	while ((nextnode = node->tln_Succ))
	{
		exec_Free(exec, node);
		numfreed++;
		node = nextnode;
	}
	TDESTROY(&mmu->tmm_Lock);
	if (numfreed)
		TDBPRINTF(TDB_ERROR,("freed %d pending allocations\n", numfreed));
}

static THOOKENTRY TTAG
exec_mmu_tasktrack(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			exec_mmu_tasktrackdestroy(mmu);
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_tasktrackalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_tasktrackfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_tasktrackrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	TNULL tasksafe+tracking allocator
*/

static TAPTR
exec_mmu_kntasktrackalloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	struct TNode *mem;
	exec_Lock(exec, &mmu->tmm_Lock);
	mem = exec_AllocMMU(exec, &exec->texb_BaseMMU,
		size + sizeof(struct TNode));
	if (mem)
	{
		TAddHead(&mmu->tmm_TrackList, mem);
		mem++;
	}
	exec_Unlock(exec, &mmu->tmm_Lock);
	return (TAPTR) mem;
}

static TAPTR
exec_mmu_kntasktrackrealloc(struct TMemManager *mmu, TINT8 *oldmem,
	TUINT oldsize, TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	struct TNode *newmem;
	oldmem -= sizeof(struct TNode);
	exec_Lock(exec, &mmu->tmm_Lock);
	TREMOVE((struct TNode *) oldmem);
	newmem = exec_Realloc(exec, oldmem, newsize + sizeof(struct TNode));
	if (newmem)
	{
		TAddHead(&mmu->tmm_TrackList, newmem);
		newmem++;
	}
	exec_Unlock(exec, &mmu->tmm_Lock);
	return (TAPTR) newmem;
}

static void
exec_mmu_kntasktrackfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	mem -= sizeof(struct TNode);
	exec_Lock(exec, &mmu->tmm_Lock);
	TREMOVE((struct TNode *) mem);
	exec_Free(exec, mem);
	exec_Unlock(exec, &mmu->tmm_Lock);
}

static void
exec_mmu_kntasktrackdestroy(struct TMemManager *mmu)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	struct TNode *nextnode, *node = mmu->tmm_TrackList.tlh_Head;
	TINT numfreed = 0;

	while ((nextnode = node->tln_Succ))
	{
		exec_Free(exec, node);
		numfreed++;
		node = nextnode;
	}

	TDESTROY(&mmu->tmm_Lock);

	#ifdef TDEBUG
	if (numfreed > 0)
		TDBPRINTF(TDB_ERROR,("freed %d pending allocations\n",numfreed));
	#endif
}

static THOOKENTRY TTAG
exec_mmu_kntasktrack(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			exec_mmu_kntasktrackdestroy(mmu);
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_kntasktrackalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_kntasktrackfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_kntasktrackrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	MMU-on-MMU allocator
*/

static TAPTR
exec_mmu_mmualloc(struct TMemManager *mmu, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	return exec_AllocMMU(exec, mmu->tmm_Allocator, size);
}

static TAPTR
exec_mmu_mmurealloc(struct TMemManager *mmu, TINT8 *oldmem, TUINT oldsize,
	TUINT newsize)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	return exec_Realloc(exec, oldmem, newsize);
}

static void
exec_mmu_mmufree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(mmu);
	exec_Free(exec, mem);
}

static THOOKENTRY TTAG
exec_mmu_mmu(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_mmualloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_mmufree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_mmurealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	TNULL allocator
*/

static TAPTR
exec_mmu_kernelalloc(struct TMemManager *mmu, TUINT size)
{
	TAPTR hal = ((TEXECBASE *) TGetExecBase(mmu))->texb_HALBase;
	TAPTR mem = THALAlloc(hal, size);
	return mem;
}

static void
exec_mmu_kernelfree(struct TMemManager *mmu, TINT8 *mem, TUINT size)
{
	TAPTR hal = ((TEXECBASE *) TGetExecBase(mmu))->texb_HALBase;
	THALFree(hal, mem, size);
}

static TAPTR
exec_mmu_kernelrealloc(struct TMemManager *mmu, TINT8 *oldmem, TUINT oldsize,
	TUINT newsize)
{
	TAPTR hal = ((TEXECBASE *) TGetExecBase(mmu))->texb_HALBase;
	return THALRealloc(hal, oldmem, oldsize, newsize);
}

static THOOKENTRY TTAG
exec_mmu_kernel(struct THook *hook, TAPTR obj, TTAG m)
{
	struct TMemManager *mmu = obj;
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
			break;
		case TMMSG_ALLOC:
			return (TTAG) exec_mmu_kernelalloc(mmu,
				msg->tmmsg_Alloc.tmmsg_Size);
		case TMMSG_FREE:
			exec_mmu_kernelfree(mmu,
				msg->tmmsg_Free.tmmsg_Ptr,
				msg->tmmsg_Free.tmmsg_Size);
			break;
		case TMMSG_REALLOC:
			return (TTAG) exec_mmu_kernelrealloc(mmu,
				msg->tmmsg_Realloc.tmmsg_Ptr,
				msg->tmmsg_Realloc.tmmsg_OSize,
				msg->tmmsg_Realloc.tmmsg_NSize);
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	void allocator
*/

static THOOKENTRY TTAG
exec_mmu_void(struct THook *hook, TAPTR obj, TTAG m)
{
	union TMemMsg *msg = (union TMemMsg *) m;
	switch (msg->tmmsg_Type)
	{
		case TMMSG_DESTROY:
		case TMMSG_ALLOC:
		case TMMSG_FREE:
		case TMMSG_REALLOC:
			break;
		default:
			TDBPRINTF(TDB_FAIL,("unknown hookmsg\n"));
	}
	return 0;
}

/*****************************************************************************/
/*
**	success = exec_initmmu(exec, mmu, allocator, mmutype, tags)
**	Initialize Memory Manager
*/

static THOOKENTRY TTAG
exec_mmudestroyfunc(struct THook *hook, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		struct TMemManager *mmu = obj;
		TCALLHOOKPKT(&mmu->tmm_Hook, mmu, (TTAG) &msg_destroy);
	}
	return 0;
}

LOCAL TBOOL
exec_initmmu(TEXECBASE *exec, struct TMemManager *mmu, TAPTR allocator,
	TUINT mmutype, TTAGITEM *tags)
{
	exec_FillMem(exec, mmu, sizeof(struct TMemManager), 0);
	mmu->tmm_Handle.thn_Hook.thk_Entry = exec_mmudestroyfunc;
	mmu->tmm_Type = mmutype;
	mmu->tmm_Handle.thn_Owner = (struct TModule *) exec;
	mmu->tmm_Allocator = allocator;

	switch (mmutype)
	{
		#if 0
		case TMMUT_Debug:
			if (exec_initlock(exec, &mmu->tmm_Lock))
			{
				TINITLIST(&mmu->tmm_TrackList);
				mmu->tmm_Hook.thk_Entry = mmu_db;
				return TTRUE;
			}
			break;
		#endif

		case TMMUT_MMU:
			/* MMU on top of another MMU - no additional functionality */
			if (allocator)
				mmu->tmm_Hook.thk_Entry = exec_mmu_mmu;
			else
				mmu->tmm_Hook.thk_Entry = exec_mmu_kernel;
			return TTRUE;

		case TMMUT_Tracking:
			TINITLIST(&mmu->tmm_TrackList);
			if (allocator)
			{
				/* implement memory-tracking on top of another MMU */
				mmu->tmm_Hook.thk_Entry = exec_mmu_track;
				return TTRUE;
			}
			else
			{
				/* If tracking is requested for a TNULL allocator, we must
				additionally provide locking for the tracklist, because
				kernel allocators are tasksafe by definition. */
				if (exec_initlock(exec, &mmu->tmm_Lock))
				{
					mmu->tmm_Hook.thk_Entry = exec_mmu_kntasktrack;
					return TTRUE;
				}
			}
			break;

		case TMMUT_TaskSafe:
			if (allocator)
			{
				/* implement task-safety on top of another MMU */
				if (exec_initlock(exec, &mmu->tmm_Lock))
				{
					mmu->tmm_Hook.thk_Entry = exec_mmu_task;
					return TTRUE;
				}
			}
			else
			{
				/* a TNULL MMU is task-safe by definition */
				mmu->tmm_Hook.thk_Entry = exec_mmu_kernel;
				return TTRUE;
			}
			break;

		case TMMUT_TaskSafe | TMMUT_Tracking:
			/* implement task-safety and tracking on top of another MMU */
			if (exec_initlock(exec, &mmu->tmm_Lock))
			{
				TINITLIST(&mmu->tmm_TrackList);
				if (allocator)
					mmu->tmm_Hook.thk_Entry = exec_mmu_tasktrack;
				else
					mmu->tmm_Hook.thk_Entry = exec_mmu_kntasktrack;
				return TTRUE;
			}
			break;

		case TMMUT_Message:
			if (allocator == TNULL) /* must be TNULL for now */
			{
				/* note that we use the execbase lock */
				TINITLIST(&mmu->tmm_TrackList);
				mmu->tmm_Hook.thk_Entry = exec_mmu_msg;
				return TTRUE;
			}
			break;

		case TMMUT_Static:
			/*	MMU on top of memheader */
			if (allocator)
			{
				mmu->tmm_Hook.thk_Entry = exec_mmu_static;
				return TTRUE;
			}
			break;

		case TMMUT_Static | TMMUT_TaskSafe:
			/*	MMU on top of memheader, task-safe */
			if (allocator)
			{
				if (exec_initlock(exec, &mmu->tmm_Lock))
				{
					mmu->tmm_Hook.thk_Entry = exec_mmu_statictask;
					return TTRUE;
				}
			}
			break;

		case TMMUT_Pooled:
			/*	MMU on top of a pool */
			if (allocator)
			{
				mmu->tmm_Hook.thk_Entry = exec_mmu_pool;
				return TTRUE;
			}
			break;

		case TMMUT_Pooled | TMMUT_TaskSafe:
			/*	MMU on top of a pool, task-safe */
			if (allocator)
			{
				if (exec_initlock(exec, &mmu->tmm_Lock))
				{
					mmu->tmm_Hook.thk_Entry = exec_mmu_pooltask;
					return TTRUE;
				}
			}
			break;

		case TMMUT_Void:
			mmu->tmm_Hook.thk_Entry = exec_mmu_void;
			mmu->tmm_Allocator = TNULL;
			mmu->tmm_Type = TMMUT_Void;
			return TTRUE;
	}

	/* As a fallback, initialize a void MMU that is incapable of allocating.
	** This allows safe usage of an MMU without checking the return value of
	** TInitMMU() */

	mmu->tmm_Hook.thk_Entry = exec_mmu_void;
	mmu->tmm_Allocator = TNULL;
 	mmu->tmm_Type = TMMUT_Void;
	return TFALSE;
}
