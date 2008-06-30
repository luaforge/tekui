
/*
**	$Id: exec_mod.c,v 1.1 2008-06-30 12:34:09 tmueller Exp $
**	teklib/src/exec/exec_mod.c - TEKlib Exec module
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include "exec_mod.h"
#include <tek/debug.h>
#include <tek/teklib.h>

static TBOOL exec_init(TEXECBASE *exec, TTAGITEM *tags);
static THOOKENTRY TTAG exec_dispatch(struct THook *hook, TAPTR obj, TTAG msg);
static const TMFPTR exec_vectors[EXEC_NUMVECTORS];

/*****************************************************************************/
/*
**	Module Init
**	Note: For initialization, a pointer to the HAL module
**	must be submitted via the TExecBase_HAL tag.
*/

TMODENTRY TUINT
tek_init_exec(TAPTR selftask, struct TModule *mod, TUINT16 version,
	TTAGITEM *tags)
{
	TEXECBASE *exec = (TEXECBASE *) mod;
	if (exec == TNULL)
	{
		if (version == 0xffff)
			return sizeof(TAPTR) * EXEC_NUMVECTORS; /* negative size */

		if (version <= EXEC_VERSION)
			return sizeof(TEXECBASE); /* positive size */

		return 0;
	}

	if (exec_init(exec, tags))
	{
		exec->texb_Module.tmd_Handle.thn_Hook.thk_Entry = exec_dispatch;
		exec->texb_Module.tmd_Version = EXEC_VERSION;
		exec->texb_Module.tmd_Revision = EXEC_REVISION;
 		exec->texb_Module.tmd_Flags = TMODF_VECTORTABLE;

		TInitVectors(exec, exec_vectors, EXEC_NUMVECTORS);

		/* overwrite TExecCopyMem vector with THALCopyMem vector,
		for getting rid of one callframe */
		((TMFPTR *) exec)[-16] = ((TMFPTR *) exec->texb_HALBase)[-14];

		return TTRUE;
	}

	return 0;
}

/*****************************************************************************/
/*
**	Function vector table
*/

static const TMFPTR
exec_vectors[EXEC_NUMVECTORS] =
{
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,

 	(TMFPTR) exec_DoExec,
	(TMFPTR) exec_CreateSysTask,
	(TMFPTR) exec_GetHALBase,

	(TMFPTR) TNULL,
	(TMFPTR) exec_OpenModule,
 	(TMFPTR) exec_CloseModule,
	(TMFPTR) TNULL,

	(TMFPTR) exec_CopyMem,
	(TMFPTR) exec_FillMem,
	(TMFPTR) exec_FillMem32,

	(TMFPTR) exec_CreateMMU,
	(TMFPTR) exec_AllocMMU,
	(TMFPTR) exec_AllocMMU0,
	(TMFPTR) exec_QueryInterface,
	(TMFPTR) exec_DropInterface,
	(TMFPTR) exec_Free,
	(TMFPTR) exec_Realloc,
	(TMFPTR) exec_GetMMU,
	(TMFPTR) exec_GetSize,

	(TMFPTR) exec_CreateLock,
	(TMFPTR) exec_Lock,
	(TMFPTR) exec_Unlock,

	(TMFPTR) exec_AllocSignal,
	(TMFPTR) exec_FreeSignal,
	(TMFPTR) exec_Signal,
	(TMFPTR) exec_SetSignal,
	(TMFPTR) exec_Wait,
	(TMFPTR) exec_StrEqual,

	(TMFPTR) exec_CreatePort,
	(TMFPTR) exec_PutMsg,
	(TMFPTR) exec_GetMsg,
	(TMFPTR) exec_AckMsg,
	(TMFPTR) exec_ReplyMsg,
	(TMFPTR) exec_DropMsg,
	(TMFPTR) exec_SendMsg,
	(TMFPTR) exec_WaitPort,
	(TMFPTR) exec_GetPortSignal,
	(TMFPTR) exec_GetUserPort,
	(TMFPTR) exec_GetSyncPort,

	(TMFPTR) exec_CreateTask,
	(TMFPTR) exec_FindTask,
	(TMFPTR) exec_GetTaskData,
	(TMFPTR) exec_SetTaskData,
	(TMFPTR) exec_GetTaskMMU,
	(TMFPTR) exec_AllocMsg,
	(TMFPTR) exec_AllocMsg0,

	(TMFPTR) exec_LockAtom,
	(TMFPTR) exec_UnlockAtom,
	(TMFPTR) exec_GetAtomData,
	(TMFPTR) exec_SetAtomData,

	(TMFPTR) exec_CreatePool,
	(TMFPTR) exec_AllocPool,
	(TMFPTR) exec_FreePool,
	(TMFPTR) exec_ReallocPool,

	(TMFPTR) exec_PutIO,
	(TMFPTR) exec_WaitIO,
	(TMFPTR) exec_DoIO,
	(TMFPTR) exec_CheckIO,
	(TMFPTR) exec_AbortIO,

	(TMFPTR) exec_InsertMsg,
	(TMFPTR) exec_RemoveMsg,
	(TMFPTR) exec_GetMsgStatus,
	(TMFPTR) exec_SetMsgReplyPort,
	(TMFPTR) exec_SetPortHook,

	(TMFPTR) exec_AddModules,
	(TMFPTR) exec_RemModules,
};

/*****************************************************************************/
/*
**	ExecBase closedown
*/

static THOOKENTRY TTAG
exec_dispatch(struct THook *hook, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		TEXECBASE *exec = obj;
		THALDestroyLock(exec->texb_HALBase, &exec->texb_Lock);
		TDESTROY(&exec->texb_BaseMMU);
		TDESTROY(&exec->texb_MsgMMU);
	}
	return 0;
}

/*****************************************************************************/
/*
**	ExecBase init
*/

static TBOOL
exec_init(TEXECBASE *exec, TTAGITEM *tags)
{
	TAPTR *halp, hal;

	halp = (TAPTR *) TGetTag(tags, TExecBase_HAL, TNULL);
	if (!halp)
		return TFALSE;
	hal = *halp;

	THALFillMem(hal, exec, sizeof(TEXECBASE), 0);
	exec->texb_HALBase = hal;

	if (THALInitLock(hal, &exec->texb_Lock))
	{
		if (exec_initmmu(exec, &exec->texb_MsgMMU, TNULL, TMMUT_Message,
			TNULL))
		{
			if (exec_initmmu(exec, &exec->texb_BaseMMU, TNULL, TMMUT_TaskSafe,
				TNULL))
			{
				exec->texb_Module.tmd_Handle.thn_Name = TMODNAME_EXEC;
				exec->texb_Module.tmd_Handle.thn_Owner =
					(struct TModule *) exec;
				exec->texb_Module.tmd_ModSuper = (struct TModule *) exec;
				exec->texb_Module.tmd_InitTask = TNULL; /* inserted later */
				exec->texb_Module.tmd_HALMod = TNULL; /* inserted later */
				exec->texb_Module.tmd_NegSize =
					EXEC_NUMVECTORS * sizeof(TAPTR);
				exec->texb_Module.tmd_PosSize = sizeof(TEXECBASE);
				exec->texb_Module.tmd_RefCount = 1;
				exec->texb_Module.tmd_Flags =
					TMODF_INITIALIZED | TMODF_VECTORTABLE;

				TInitList(&exec->texb_IntModList);
				exec->texb_InitModNode.tmin_Modules = (struct TInitModule *)
					TGetTag(tags, TExecBase_ModInit, TNULL);
				if (exec->texb_InitModNode.tmin_Modules)
				{
					TAddTail(&exec->texb_IntModList,
						&exec->texb_InitModNode.tmin_Node);
				}

				TInitList(&exec->texb_AtomList);
				TInitList(&exec->texb_TaskList);
				TInitList(&exec->texb_TaskInitList);
				TInitList(&exec->texb_TaskExitList);
				TInitList(&exec->texb_ModList);
				TAddHead(&exec->texb_ModList, (struct TNode *) exec);
				TAddHead(&exec->texb_ModList, (struct TNode *) hal);

				return TTRUE;
			}
			TDESTROY(&exec->texb_MsgMMU);
		}
		THALDestroyLock(hal, &exec->texb_Lock);
	}
	return TFALSE;
}

/*****************************************************************************/
/*
**	sigs = exec_allocsignal(execbase, task, signals)
**	Alloc signal(s) from a task
*/

LOCAL TUINT
exec_allocsignal(TEXECBASE *exec, struct TTask *task, TUINT signals)
{
	TUINT newsignal = 0;
	TAPTR hal = exec->texb_HALBase;

	THALLock(hal, &task->tsk_TaskLock);

	if (signals)
	{
		if ((signals & task->tsk_SigFree) == signals)
			newsignal = signals;
	}
	else
	{
		TINT x;
		TUINT trysignal = 0x00000001;

		for (x = 0; x < 32; ++x)
		{
			if (!(trysignal & TTASK_SIG_RESERVED))
			{
				if (trysignal & task->tsk_SigFree)
				{
					newsignal = trysignal;
					break;
				}
			}
			trysignal <<= 1;
		}
	}

	task->tsk_SigFree &= ~newsignal;
	task->tsk_SigUsed |= newsignal;

	THALUnlock(hal, &task->tsk_TaskLock);

	return newsignal;
}

/*****************************************************************************/
/*
**	exec_freesignal(execbase, task, signals)
**	Return signal(s) to a task
*/

LOCAL void
exec_freesignal(TEXECBASE *exec, struct TTask *task, TUINT signals)
{
	TAPTR hal = exec->texb_HALBase;

	THALLock(hal, &task->tsk_TaskLock);

	if ((task->tsk_SigUsed & signals) != signals)
		TDBPRINTF(TDB_ERROR,
			("signals freed did not match signals allocated\n"));

	task->tsk_SigFree |= signals;
	task->tsk_SigUsed &= ~signals;

	THALUnlock(hal, &task->tsk_TaskLock);
}

/*****************************************************************************/
/*
**	success = exec_initport(exec, task, prefsignal)
**	Init a message port for the given task, with a preferred signal,
**	or allocate a new signal if prefsignal is 0
*/

static THOOKENTRY TTAG
exec_destroyport_internal(struct THook *h, TAPTR obj, TTAG msg)
{
	if (msg == 0)
	{
		struct TMsgPort *port = obj;
		TEXECBASE *exec = (TEXECBASE *) TGetExecBase(port);

		if (!TISLISTEMPTY(&port->tmp_MsgList))
			TDBPRINTF(TDB_FAIL,("Message queue was not empty\n"));

		exec_freesignal(exec, port->tmp_SigTask, port->tmp_Signal);
		THALDestroyLock(exec->texb_HALBase, &port->tmp_Lock);
	}

	return 0;
}

LOCAL TBOOL
exec_initport(TEXECBASE *exec, struct TMsgPort *port, struct TTask *task,
	TUINT signal)
{
	if (!signal)
	{
		signal = exec_allocsignal(exec, task, 0);
		if (!signal)
			return TFALSE;
	}

	if (THALInitLock(exec->texb_HALBase, &port->tmp_Lock))
	{
		port->tmp_Handle.thn_Hook.thk_Entry = exec_destroyport_internal;
		port->tmp_Handle.thn_Owner = (struct TModule *) exec;
		port->tmp_Handle.thn_Name = TNULL;
		TInitList(&port->tmp_MsgList);
		port->tmp_Hook = TNULL;
		port->tmp_Signal = signal;
		port->tmp_SigTask = task;

		return TTRUE;
	}

	return TFALSE;
}

/*****************************************************************************/
/*
**	Initialize a locking object
*/

static THOOKENTRY TTAG
destroylock(struct THook *h, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		struct TLock *lock = obj;
		TEXECBASE *exec = (TEXECBASE *) TGetExecBase(lock);
		THALDestroyLock(exec->texb_HALBase, &lock->tlk_HLock);
	}
	return 0;
}

LOCAL TBOOL
exec_initlock(TEXECBASE *exec, struct TLock *lock)
{
	if (THALInitLock(exec->texb_HALBase, &lock->tlk_HLock))
	{
		lock->tlk_Handle.thn_Name = TNULL;
		lock->tlk_Handle.thn_Hook.thk_Entry = destroylock;
		lock->tlk_Handle.thn_Owner = (struct TModule *) exec;
		TInitList(&lock->tlk_Waiters);
		lock->tlk_Owner = TNULL;
		lock->tlk_NestCount = 0;
		lock->tlk_WaitCount = 0;
		return TTRUE;
	}
	return TFALSE;
}

/*****************************************************************************/
/*
**	msg = exec_getmsg(exec, port)
**	Get next pending message from messageport
*/

LOCAL TAPTR
exec_getmsg(TEXECBASE *exec, struct TMsgPort *port)
{
	struct TMessage *msg;
	TAPTR hal = exec->texb_HALBase;

	THALLock(hal, &port->tmp_Lock);
	msg = (struct TMessage *) TRemHead(&port->tmp_MsgList);
	THALUnlock(hal, &port->tmp_Lock);

	if (msg)
	{
		if (!(msg->tmsg_Flags & TMSGF_QUEUED))
			TDBPRINTF(TDB_ERROR,("got msg with TMSGF_QUEUED not set\n"));
		msg->tmsg_Flags &= ~TMSGF_QUEUED;
		return (TAPTR)((TINT8 *) (msg + 1) + sizeof(union TMMUInfo));
	}
	return TNULL;
}

/*****************************************************************************/
/*
**	success = exec_sendmsg(exec, task, port, msg)
**	Send message, two-way, blocking
*/

LOCAL TUINT
exec_sendmsg(TEXECBASE *exec, struct TTask *task, struct TMsgPort *port,
	TAPTR mem)
{
	TAPTR hal = exec->texb_HALBase;
	struct TMessage *msg = TGETMSGPTR(mem);
	TAPTR reply;

	/* replyport is task's syncport */
	msg->tmsg_RPort = &task->tsk_SyncPort;

	/* sender is local address space */
	msg->tmsg_Sender = TNULL;

	THALLock(hal, &port->tmp_Lock);
	TAddTail(&port->tmp_MsgList, (struct TNode *) msg);
	msg->tmsg_Flags = TMSG_STATUS_SENT | TMSGF_QUEUED;
	if (port->tmp_Hook)
		TCALLHOOKPKT(port->tmp_Hook, port, (TTAG) msg);
	THALUnlock(hal, &port->tmp_Lock);

	THALSignal(hal, &port->tmp_SigTask->tsk_Thread, port->tmp_Signal);

	for (;;)
	{
		THALWait(exec->texb_HALBase, task->tsk_SyncPort.tmp_Signal);
		reply = exec_getmsg(exec, msg->tmsg_RPort);
		if (reply)
		{
			TUINT status = msg->tmsg_Flags;
			TDBASSERT(99, reply == mem);
			msg->tmsg_Flags = 0;
			return status;
		}
		TDBPRINTF(TDB_FAIL,("signal on syncport, no message!\n"));
	}
}

/*****************************************************************************/
/*
**	exec_returnmsg(exec, mem, status)
**	Return a message to its sender, or free it, transparently
*/

LOCAL void
exec_returnmsg(TEXECBASE *exec, TAPTR mem, TUINT status)
{
	struct TMessage *msg = TGETMSGPTR(mem);
	struct TMsgPort *replyport = msg->tmsg_RPort;
	if (replyport)
	{
		TAPTR hal = exec->texb_HALBase;

		THALLock(hal, &replyport->tmp_Lock);
		TAddTail(&replyport->tmp_MsgList, (struct TNode *) msg);
		msg->tmsg_Flags = status;
		if (replyport->tmp_Hook)
			TCALLHOOKPKT(replyport->tmp_Hook, replyport, (TTAG) msg);
		THALUnlock(hal, &replyport->tmp_Lock);

		THALSignal(hal, &replyport->tmp_SigTask->tsk_Thread,
			replyport->tmp_Signal);
	}
	else
	{
		exec_Free(exec, mem);	/* free one-way msg transparently */
		TDBPRINTF(TDB_TRACE,("message returned to memory manager\n"));
	}
}

