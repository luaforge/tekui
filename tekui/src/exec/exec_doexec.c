
/*
**	$Id: exec_doexec.c,v 1.1 2008-06-30 12:34:17 tmueller Exp $
**	teklib/src/exec/exec_doexec.c - Exec task contexts
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/debug.h>
#include <tek/teklib.h>
#include <tek/proto/io.h>
#include "exec_mod.h"

static void exec_requestmod(TEXECBASE *exec, struct TTask *taskmsg);
static void exec_closemod(TEXECBASE *exec, struct TTask *taskmsg);
static void exec_replymod(TEXECBASE *exec, struct TTask *taskmsg);
static void exec_main(TEXECBASE *exec, struct TTask *task,
	struct TTagItem *tags);
static void exec_ramlib(TEXECBASE *exec, struct TTask *task,
	struct TTagItem *tags);
static void exec_loadmod(TEXECBASE *exec, struct TTask *task,
	struct TTask *taskmsg);
static void exec_unloadmod(TEXECBASE *exec, struct TTask *task,
	struct TTask *taskmsg);
static void exec_newtask(TEXECBASE *exec, struct TTask *taskmsg);
static struct TTask *exec_createtask(TEXECBASE *exec, TTASKFUNC func,
	TINITFUNC initfunc, struct TTagItem *tags);
static TTASKENTRY void exec_taskentryfunc(struct TTask *task);
static void exec_childinit(TEXECBASE *exec);
static void exec_childexit(TEXECBASE *exec);
static void exec_lockatom(TEXECBASE *exec, struct TTask *msg);
static void exec_unlockatom(TEXECBASE *exec, struct TTask *msg);

/*****************************************************************************/
/*
**	success = TDoExec(exec, tags)
**	entrypoint to exec internal services and initializations
*/

EXPORT TBOOL
exec_DoExec(TEXECBASE *exec, struct TTagItem *tags)
{
	struct TTask *task = exec_FindTask(exec, TNULL);

	if (task == exec_FindTask(exec, (TSTRPTR) TTASKNAME_EXEC))
	{
		/* perform execbase context */
		exec_main(exec, task, tags);
		return TTRUE;
	}
	else if (task == exec_FindTask(exec, (TSTRPTR) TTASKNAME_RAMLIB))
	{
		/* perform ramlib context */
		exec_ramlib(exec, task, tags);
		return TTRUE;
	}
	else if (task == exec_FindTask(exec, (TSTRPTR) TTASKNAME_ENTRY))
	{
		/* place for initializations in entry context */
		return TTRUE;
	}

	return TFALSE;
}

/*****************************************************************************/
/*
**	Ramlib task - Module loading
*/

static void
exec_ramlib(TEXECBASE *exec, struct TTask *task, struct TTagItem *tags)
{
	struct TMsgPort *port = &task->tsk_UserPort;
	TUINT waitsig = TTASK_SIG_ABORT | port->tmp_Signal;

	TUINT sig;
	struct TTask *taskmsg;
	TINT n = 0;

	for (;;)
	{
		sig = THALWait(exec->texb_HALBase, waitsig);
		if (sig & TTASK_SIG_ABORT) break;

		while ((taskmsg = exec_getmsg(exec, port)))
		{
			switch (taskmsg->tsk_ReqCode)
			{
				case TTREQ_LOADMOD:
					exec_loadmod(exec, task, taskmsg);
					break;

				case TTREQ_UNLOADMOD:
					exec_unloadmod(exec, task, taskmsg);
					break;

				case TTREQ_ADDMOD:
					TAddHead(&exec->texb_IntModList, (struct TNode *)
						taskmsg->tsk_Request.trq_AddRemMod.trm_ModInitNode);
					taskmsg->tsk_Request.trq_AddRemMod.trm_Result = TTRUE;
					exec_ReplyMsg(exec, taskmsg);
					break;

				case TTREQ_REMMOD:
					/* TODO: removal should only succeed if flags == 0 or
					flags == TMODF_INITIALIZED and nestcount == 0 */
					TRemove((struct TNode *)
						taskmsg->tsk_Request.trq_AddRemMod.trm_ModInitNode);
					taskmsg->tsk_Request.trq_AddRemMod.trm_Result = TTRUE;
					exec_ReplyMsg(exec, taskmsg);
					break;
			}
		}
	}

	while ((taskmsg = exec_getmsg(exec, port)))
		n++;
	if (n > 0)
	{
		TDBPRINTF(TDB_FAIL,("Module requests pending: %d\n", n));
		TDBFATAL();
	}
}

/*****************************************************************************/

static void
exec_loadmod(TEXECBASE *exec, struct TTask *task, struct TTask *taskmsg)
{
	TAPTR halmod = TNULL;
	TAPTR hal = exec->texb_HALBase;
	union TTaskRequest *req = &taskmsg->tsk_Request;
	TSTRPTR modname = req->trq_Mod.trm_InitMod.tmd_Handle.thn_Name;
	TUINT nsize = 0, psize = 0;
	struct TInitModule *imod = TNULL;
	struct TNode *nnode, *node;

	/* try to open from list of internal modules: */

	node = exec->texb_IntModList.tlh_Head;
	for (; (nnode = node->tln_Succ); node = nnode)
	{
		struct TInitModule *im =
			((struct TModInitNode *) node)->tmin_Modules;
		TSTRPTR tempn;

		for (; (tempn = im->tinm_Name); im++)
		{
			if (exec_StrEqual(exec, tempn, modname))
			{
				psize = (*im->tinm_InitFunc)(TNULL, TNULL,
					req->trq_Mod.trm_InitMod.tmd_Version, TNULL);
				if (psize)
					nsize = (*im->tinm_InitFunc)(TNULL, TNULL, 0xffff,
						TNULL);
				imod = im;
				break;
			}
		}
	}

	if (psize == 0)
	{
		/* try to load module via HAL interface: */

		halmod = THALLoadModule(hal, modname,
			req->trq_Mod.trm_InitMod.tmd_Version, &psize, &nsize);
	}

	if (psize)
	{
		TINT nlen;
		TSTRPTR s = modname;
		TINT8 *mmem;

		while (*s++);
		nlen = s - modname;

		mmem = exec_AllocMMU(exec, TNULL, psize + nsize + nlen);
		if (mmem)
		{
			TBOOL success = TFALSE;
			struct TModule *mod = (struct TModule *) (mmem + nsize);
			TSTRPTR d = (TSTRPTR) mmem + nsize + psize;

			exec_FillMem(exec, mmem, psize + nsize, 0);

			s = modname;
			/* insert and copy name: */
			mod->tmd_Handle.thn_Name = d;
			while ((*d++ = *s++));

			/* modsuper: */
			mod->tmd_Handle.thn_Hook.thk_Data = mod;
			mod->tmd_Handle.thn_Owner = (struct TModule *) exec;
			mod->tmd_ModSuper = mod;
			mod->tmd_InitTask = task;
			mod->tmd_HALMod = halmod;
			mod->tmd_PosSize = psize;
			mod->tmd_NegSize = nsize;
			mod->tmd_RefCount = 1;

			if (halmod)
				success = THALCallModule(hal, halmod, task, mod);
			else
				success = (*imod->tinm_InitFunc)(task, mod,
					req->trq_Mod.trm_InitMod.tmd_Version, TNULL);

			if (success)
			{
				if ((mod->tmd_Flags & TMODF_VECTORTABLE) &&
					nsize < sizeof(TMFPTR) * TMODV_NUMRESERVED)
				{
					TDBPRINTF(TDB_FAIL,
						("Module %s hasn't reserved enough vectors\n",
						mod->tmd_Handle.thn_Name));
					TDBFATAL();
				}
				req->trq_Mod.trm_Module = mod;
				exec_ReplyMsg(exec, taskmsg);
				return;
			}

			exec_Free(exec, mmem);
		}

		if (halmod)
			THALUnloadModule(hal, halmod);
	}

	/* failed */
	exec_DropMsg(exec, taskmsg);
}

/*****************************************************************************/

static void
exec_unloadmod(TEXECBASE *exec, struct TTask *task, struct TTask *taskmsg)
{
	TAPTR hal = exec->texb_HALBase;
	union TTaskRequest *req = &taskmsg->tsk_Request;
	struct TModule *mod = req->trq_Mod.trm_Module;

	TDBPRINTF(TDB_TRACE,("unload mod: %s\n", mod->tmd_Handle.thn_Name));

	/* invoke module destructor */
	TDESTROY(mod);

	/* unload */
	if (mod->tmd_HALMod)
		THALUnloadModule(hal, mod->tmd_HALMod);

	/* free */
	exec_Free(exec, (TINT8 *) mod - mod->tmd_NegSize);

	/* restore original replyport */
	TGETMSGPTR(taskmsg)->tmsg_RPort = req->trq_Mod.trm_RPort;

	/* return to caller */
	exec_returnmsg(exec, taskmsg, TMSG_STATUS_ACKD | TMSGF_QUEUED);
}

/*****************************************************************************/
/*
**	Exec task
*/

static void
exec_checkmodules(TEXECBASE *exec)
{
	TINT n = 0;
	struct TNode *nnode, *node = exec->texb_ModList.tlh_Head;
	for (; (nnode = node->tln_Succ); node = nnode)
	{
		struct TModule *mod = (struct TModule *) node;
		if (mod == (TAPTR) exec || mod == exec->texb_HALBase)
			continue;
		TDBPRINTF(TDB_FAIL,
			("Module not closed: %s\n", mod->tmd_Handle.thn_Name));
		n++;
	}

	if (n > 0)
	{
		TDBPRINTF(TDB_FAIL,("Applications must close all modules"));
		TDBFATAL();
	}
}

static void
exec_main(TEXECBASE *exec, struct TTask *exectask, struct TTagItem *tags)
{
	struct TMsgPort *execport = exec->texb_ExecPort;
	struct TMsgPort *modreply = exec->texb_ModReply;
	TUINT waitsig = TTASK_SIG_ABORT | execport->tmp_Signal |
		modreply->tmp_Signal | TTASK_SIG_CHILDINIT | TTASK_SIG_CHILDEXIT;

	TUINT sig;
	struct TTask *taskmsg;

	for (;;)
	{
		sig = THALWait(exec->texb_HALBase, waitsig);
		if (sig & TTASK_SIG_ABORT)
			break;

		if (sig & modreply->tmp_Signal)
			while ((taskmsg = exec_getmsg(exec, modreply)))
				exec_replymod(exec, taskmsg);

		if (sig & execport->tmp_Signal)
		{
			while ((taskmsg = exec_getmsg(exec, execport)))
			{
				switch (taskmsg->tsk_ReqCode)
				{
					case TTREQ_OPENMOD:
						exec_requestmod(exec, taskmsg);
						break;

					case TTREQ_CLOSEMOD:
						exec_closemod(exec, taskmsg);
						break;

					case TTREQ_CREATETASK:
						exec_newtask(exec, taskmsg);
						break;

					case TTREQ_DESTROYTASK:
					{
						/* insert backptr to self */
						taskmsg->tsk_Request.trq_Task.trt_Parent = taskmsg;
						/* add request to taskexitlist */
						TAddTail(&exec->texb_TaskExitList,
							(struct TNode *) &taskmsg->tsk_Request);
						/* force list processing */
						sig |= TTASK_SIG_CHILDEXIT;
						break;
					}

					case TTREQ_LOCKATOM:
						exec_lockatom(exec, taskmsg);
						break;

					case TTREQ_UNLOCKATOM:
						exec_unlockatom(exec, taskmsg);
						break;
				}
			}
		}

		if (sig & TTASK_SIG_CHILDINIT)
			exec_childinit(exec);

		if (sig & TTASK_SIG_CHILDEXIT)
			exec_childexit(exec);
	}

	if (exec->texb_NumTasks || exec->texb_NumInitTasks)
	{
		TDBPRINTF(TDB_FAIL,("Number of tasks running: %d - initializing: %d\n",
			exec->texb_NumTasks, exec->texb_NumInitTasks));
		TDBFATAL();
	}

	exec_checkmodules(exec);
}

/*****************************************************************************/

static void
exec_requestmod(TEXECBASE *exec, struct TTask *taskmsg)
{
	struct TModule *mod;
	union TTaskRequest *req;

	req = &taskmsg->tsk_Request;
	mod = (struct TModule *) TFindHandle(&exec->texb_ModList,
		req->trq_Mod.trm_InitMod.tmd_Handle.thn_Name);

	if (mod == TNULL)
	{
		/* prepare load request */
		taskmsg->tsk_ReqCode = TTREQ_LOADMOD;

		/* backup msg original replyport */
		req->trq_Mod.trm_RPort = TGETMSGREPLYPORT(taskmsg);

		/* init list of waiters */
		TINITLIST(&req->trq_Mod.trm_Waiters);

		/* mark module as uninitialized */
		req->trq_Mod.trm_InitMod.tmd_Flags = 0;

		/* insert request as fake/uninitialized module to modlist */
		TAddTail(&exec->texb_ModList, (struct TNode *) req);

		/* forward this request to I/O task */
		exec_PutMsg(exec, &exec->texb_IOTask->tsk_UserPort,
			exec->texb_ModReply, taskmsg);

		return;
	}

	if (mod->tmd_Version < req->trq_Mod.trm_InitMod.tmd_Version)
	{
		/* mod version insufficient */
		exec_returnmsg(exec, taskmsg, TMSG_STATUS_FAILED | TMSGF_QUEUED);
		return;
	}

	if (mod->tmd_Flags & TMODF_INITIALIZED)
	{
		/* request succeeded, reply */
		mod->tmd_RefCount++;
		req->trq_Mod.trm_Module = mod;
		exec_returnmsg(exec, taskmsg, TMSG_STATUS_REPLIED | TMSGF_QUEUED);
		return;
	}

	/* this mod is an initializing request.
	add new request to list of its waiters */

	req->trq_Mod.trm_ReqTask = taskmsg;

	TAddTail(&((union TTaskRequest *) mod)->trq_Mod.trm_Waiters,
		(struct TNode *) req);
}

/*****************************************************************************/

static void
exec_replymod(TEXECBASE *exec, struct TTask *taskmsg)
{
	struct TModule *mod;
	union TTaskRequest *req;
	struct TNode *node, *tnode;

	req = &taskmsg->tsk_Request;
	node = req->trq_Mod.trm_Waiters.tlh_Head;
	mod = req->trq_Mod.trm_Module;

	/* unlink fake/initializing module request from modlist */
	TREMOVE((struct TNode *) req);

	/* restore original replyport */
	TGETMSGPTR(taskmsg)->tmsg_RPort = req->trq_Mod.trm_RPort;

	if (TGETMSGSTATUS(taskmsg) == TMSG_STATUS_FAILED)
	{
		/* fail-reply to all waiters */
		while ((tnode = node->tln_Succ))
		{
			TREMOVE(node);

			/* reply to opener */
			exec_returnmsg(exec,
				((union TTaskRequest *) node)->trq_Mod.trm_ReqTask,
				TMSG_STATUS_FAILED | TMSGF_QUEUED);

			node = tnode;
		}

		/* forward failure to opener */
		exec_returnmsg(exec, taskmsg, TMSG_STATUS_FAILED | TMSGF_QUEUED);
		return;
	}

	/* mark as ready */
	mod->tmd_Flags |= TMODF_INITIALIZED;

	/* link real module to modlist */
	TAddTail(&exec->texb_ModList, (struct TNode *) mod);

	/* send replies to all waiters */
	while ((tnode = node->tln_Succ))
	{
		TREMOVE(node);

		/* insert module */
		((union TTaskRequest *) node)->trq_Mod.trm_Module = mod;

		/* reply to opener */
		exec_returnmsg(exec,
			((union TTaskRequest *) node)->trq_Mod.trm_ReqTask,
			TMSG_STATUS_REPLIED | TMSGF_QUEUED);

		mod->tmd_RefCount++;
		node = tnode;
	}

	/* forward success to opener */
	exec_returnmsg(exec, taskmsg, TMSG_STATUS_REPLIED | TMSGF_QUEUED);
}

/*****************************************************************************/

static void
exec_closemod(TEXECBASE *exec, struct TTask *taskmsg)
{
	struct TModule *mod;
	union TTaskRequest *req;

	req = &taskmsg->tsk_Request;
	mod = req->trq_Mod.trm_Module;

	if (--mod->tmd_RefCount == 0)
	{
		/* remove from modlist */
		TREMOVE((struct TNode *) mod);

		/* prepare unload request */
		taskmsg->tsk_ReqCode = TTREQ_UNLOADMOD;

		/* backup msg original replyport */
		req->trq_Mod.trm_RPort = TGETMSGREPLYPORT(taskmsg);

		/* forward this request to I/O task */
		exec_PutMsg(exec, &exec->texb_IOTask->tsk_UserPort, TNULL, taskmsg);

		return;
	}

	/* confirm */
	exec_returnmsg(exec, taskmsg, TMSG_STATUS_ACKD | TMSGF_QUEUED);
}

/*****************************************************************************/

static void
exec_newtask(TEXECBASE *exec, struct TTask *taskmsg)
{
	union TTaskRequest *req;
	struct TTask *newtask;

	req = &taskmsg->tsk_Request;

	req->trq_Task.trt_Task = newtask =
		exec_createtask(exec, req->trq_Task.trt_Func,
		req->trq_Task.trt_InitFunc, req->trq_Task.trt_Tags);

	if (newtask)
	{
		/* insert ptr to self, i.e. the requesting parent */
		req->trq_Task.trt_Parent = taskmsg;
		/* add request (not taskmsg) to list of initializing tasks */
		TAddTail(&exec->texb_TaskInitList, (struct TNode *) req);
		exec->texb_NumInitTasks++;
	}
	else
	{
		/* failed */
		exec_returnmsg(exec, taskmsg, TMSG_STATUS_FAILED | TMSGF_QUEUED);
	}
}

/*****************************************************************************/
/*
**	create task
*/

static THOOKENTRY TTAG
exec_usertaskdestroy(struct THook *h, TAPTR obj, TTAG msg)
{
	if (msg == TMSG_DESTROY)
	{
		struct TTask *task = obj;
		TEXECBASE *exec = (TEXECBASE *) TGetExecBase(task);
		struct TTask *self = THALFindSelf(exec->texb_HALBase);
		union TTaskRequest *req = &self->tsk_Request;
		self->tsk_ReqCode = TTREQ_DESTROYTASK;
		req->trq_Task.trt_Task = task;
		exec_sendmsg(exec, self, exec->texb_ExecPort, self);
	}
	return 0;
}

static struct TTask *
exec_createtask(TEXECBASE *exec, TTASKFUNC func, TINITFUNC initfunc,
	struct TTagItem *tags)
{
	TAPTR hal = exec->texb_HALBase;
	TUINT tnlen = 0;
	struct TTask *newtask;
	TSTRPTR tname;

	tname = (TSTRPTR) TGetTag(tags, TTask_Name, TNULL);
	if (tname)
	{
		TSTRPTR t = tname;
		while (*t++);
		tnlen = t - tname;
	}

	/* note that tasks are message allocations */
	newtask = exec_AllocMMU(exec, &exec->texb_MsgMMU,
		sizeof(struct TTask) + tnlen);
	if (newtask == TNULL)
		return TNULL;

	exec_FillMem(exec, newtask, sizeof(struct TTask), 0);
	if (THALInitLock(hal, &newtask->tsk_TaskLock))
	{
		if (exec_initmmu(exec, &newtask->tsk_HeapMMU, TNULL,
			TMMUT_TaskSafe | TMMUT_Tracking, TNULL))
		{
			if (exec_initport(exec, &newtask->tsk_UserPort, newtask,
				TTASK_SIG_USER))
			{
				if (exec_initport(exec, &newtask->tsk_SyncPort, newtask,
					TTASK_SIG_SINGLE))
				{
					if (tname)
					{
						TSTRPTR t = (TSTRPTR) (newtask + 1);
						newtask->tsk_Handle.thn_Name = t;
						while ((*t++ = *tname++));
					}

					newtask->tsk_Handle.thn_Hook.thk_Entry =
						exec_usertaskdestroy;
					newtask->tsk_Handle.thn_Owner = (struct TModule *) exec;
					newtask->tsk_UserData =
						(TAPTR) TGetTag(tags, TTask_UserData, TNULL);
					newtask->tsk_SigFree = ~((TUINT) TTASK_SIG_RESERVED);
					newtask->tsk_SigUsed = TTASK_SIG_RESERVED;
					newtask->tsk_Status = TTASK_INITIALIZING;

					newtask->tsk_Request.trq_Task.trt_Func = func;
					newtask->tsk_Request.trq_Task.trt_InitFunc = initfunc;
					newtask->tsk_Request.trq_Task.trt_Tags = tags;

					if (THALInitThread(hal, &newtask->tsk_Thread,
						(TTASKENTRY void (*)(TAPTR data)) exec_taskentryfunc,
							newtask))
					{
						/* kick it to life */
						THALSignal(hal, &newtask->tsk_Thread,
							TTASK_SIG_SINGLE);
						return newtask;
					}

					TDESTROY(&newtask->tsk_SyncPort);
				}
				TDESTROY(&newtask->tsk_UserPort);
			}
			TDESTROY(&newtask->tsk_HeapMMU);
		}
		THALDestroyLock(hal, &newtask->tsk_TaskLock);
	}

	return TNULL;
}

/*****************************************************************************/
/*
**	Task entry
*/

static void
exec_closetaskfh(struct TTask *task, TAPTR fh, TUINT flag)
{
	if (task->tsk_Flags & flag)
	{
		TREMOVE((struct TNode *) fh);
		TIOCloseFile(task->tsk_IOBase, fh);
	}
}

static TTASKENTRY void
exec_taskentryfunc(struct TTask *task)
{
	TEXECBASE *exec = (TEXECBASE *) TGetExecBase(task);
	union TTaskRequest *req = &task->tsk_Request;

	TTASKFUNC func = req->trq_Task.trt_Func;
	TINITFUNC initfunc = req->trq_Task.trt_InitFunc;
	TTAGITEM *tags = req->trq_Task.trt_Tags;

	TAPTR currentdir = (TAPTR) TGetTag(tags, TTask_CurrentDir, TNULL);
	TUINT status = 0;

	if (currentdir)
	{
		task->tsk_IOBase = exec_OpenModule(exec, (TSTRPTR) "io", 0, TNULL);
		if (task->tsk_IOBase)
		{
			TAPTR newcd = TIODupLock(task->tsk_IOBase, currentdir);
			if (newcd)
			{
				/* new lock on currentdir duplicated from parent */
				TIOChangeDir(task->tsk_IOBase, newcd);
			}
			else
				goto closedown;
		}
		else
			goto closedown;
	}

	task->tsk_FHIn = (TAPTR) TGetTag(tags, TTask_InputFH, TNULL);
	task->tsk_FHOut = (TAPTR) TGetTag(tags, TTask_OutputFH, TNULL);
	task->tsk_FHErr = (TAPTR) TGetTag(tags, TTask_ErrorFH, TNULL);

	if (initfunc)
		status = (*initfunc)(task);
	else
		status = 1;

	if (status)
	{
		/* change from initializing to running state */

		task->tsk_Status = TTASK_RUNNING;
		THALSignal(exec->texb_HALBase, &exec->texb_ExecTask->tsk_Thread,
			TTASK_SIG_CHILDINIT);

		if (func)
			(*func)(task);
	}

	if (task->tsk_Flags & (TTASKF_CLOSEOUTPUT | TTASKF_CLOSEINPUT |
		TTASKF_CLOSEERROR))
	{
		if (task->tsk_IOBase == TNULL)
		{
			/* Someone passed a filehandle to this task, but we don't
			even have the I/O module open. It should be investigated if
			this can fail: */
			task->tsk_IOBase = exec_OpenModule(exec, (TSTRPTR) "io", 0, TNULL);
			if (task->tsk_IOBase == TNULL)
				TDBFATAL();
		}
		exec_closetaskfh(task, task->tsk_FHOut, TTASKF_CLOSEOUTPUT);
		exec_closetaskfh(task, task->tsk_FHIn, TTASKF_CLOSEINPUT);
		exec_closetaskfh(task, task->tsk_FHErr, TTASKF_CLOSEERROR);
	}

closedown:

	if (task->tsk_IOBase)
	{
		/* if we are responsible for a currentdir lock, close it */
		TAPTR cd = TIOChangeDir(task->tsk_IOBase, TNULL);
		if (cd)
			TIOUnlockFile(task->tsk_IOBase, cd);
		exec_CloseModule(exec, task->tsk_IOBase);
	}

	if (status)
	{
		/* succeeded */
		task->tsk_Status = TTASK_FINISHED;
		THALSignal(exec->texb_HALBase, &exec->texb_ExecTask->tsk_Thread,
				TTASK_SIG_CHILDEXIT);
	}
	else
	{
		/* system initialization failed */
		task->tsk_Status = TTASK_FAILED;
		THALSignal(exec->texb_HALBase, &exec->texb_ExecTask->tsk_Thread,
			TTASK_SIG_CHILDINIT);
	}
}

/*****************************************************************************/

static void
exec_destroytask(TEXECBASE *exec, struct TTask *task)
{
	TAPTR hal = exec->texb_HALBase;
	THALDestroyThread(hal, &task->tsk_Thread);
	TDESTROY(&task->tsk_SyncPort);
	TDESTROY(&task->tsk_UserPort);
	TDESTROY(&task->tsk_HeapMMU);
	THALDestroyLock(hal, &task->tsk_TaskLock);
	exec_Free(exec, task);
}

/*****************************************************************************/
/*
**	handle tasks that change from initializing to running state
*/

static void
exec_childinit(TEXECBASE *exec)
{
	TAPTR hal = exec->texb_HALBase;
	struct TNode *nnode, *node;

	node = exec->texb_TaskInitList.tlh_Head;
	while ((nnode = node->tln_Succ))
	{
		union TTaskRequest *req = (union TTaskRequest *) node;
		struct TTask *task = req->trq_Task.trt_Task;
		struct TTask *taskmsg = req->trq_Task.trt_Parent;

		if (task->tsk_Status == TTASK_FAILED)
		{
			/* remove request from taskinitlist */
			TREMOVE((struct TNode *) req);
			exec->texb_NumInitTasks--;

			/* destroy task corpse */
			exec_destroytask(exec, task);

			/* fail-reply taskmsg to sender */
			exec_returnmsg(exec, taskmsg, TMSG_STATUS_FAILED | TMSGF_QUEUED);
		}
		else if (task->tsk_Status != TTASK_INITIALIZING)
		{
			/* remove taskmsg from taskinitlist */
			TREMOVE((struct TNode *) req);
			exec->texb_NumInitTasks--;

			/* link task to list of running tasks */
			THALLock(hal, &exec->texb_Lock);
			/* note: using node as tempnode argument here */
			TAddTail(&exec->texb_TaskList, (struct TNode *) task);
			THALUnlock(hal, &exec->texb_Lock);
			exec->texb_NumTasks++;

			/* reply taskmsg */
			exec_returnmsg(exec, taskmsg, TMSG_STATUS_REPLIED | TMSGF_QUEUED);
		}

		node = nnode;
	}
}

/*****************************************************************************/
/*
**	handle exiting tasks
*/

static void
exec_childexit(TEXECBASE *exec)
{
	TAPTR hal = exec->texb_HALBase;
	struct TNode *nnode, *node;

	node = exec->texb_TaskExitList.tlh_Head;
	while ((nnode = node->tln_Succ))
	{
		union TTaskRequest *req = (union TTaskRequest *) node;
		struct TTask *task = req->trq_Task.trt_Task;
		struct TTask *taskmsg = req->trq_Task.trt_Parent;

		if (task->tsk_Status == TTASK_FINISHED)
		{
			/* unlink task from list of running tasks */
			THALLock(hal, &exec->texb_Lock);
			TREMOVE((struct TNode *) task);
			THALUnlock(hal, &exec->texb_Lock);

			/* destroy task */
			exec_destroytask(exec, task);

			/* unlink taskmsg from list of exiting tasks */
			TREMOVE((struct TNode *) req);
			exec->texb_NumTasks--;

			/* reply to caller */
			exec_returnmsg(exec, taskmsg, TMSG_STATUS_REPLIED | TMSGF_QUEUED);
		}

		node = nnode;
	}
}

/*****************************************************************************/
/*
**	Atoms
*/

static void
exec_replyatom(TEXECBASE *exec, struct TTask *msg, struct TAtom *atom)
{
	msg->tsk_Request.trq_Atom.tra_Atom = atom;
	exec_returnmsg(exec, msg, TMSG_STATUS_REPLIED | TMSGF_QUEUED);
}

static TAPTR
exec_lookupatom(TEXECBASE *exec, TSTRPTR name)
{
	return TFindHandle(&exec->texb_AtomList, name);
}

static struct TAtom *
exec_newatom(TEXECBASE *exec, TSTRPTR name)
{
	struct TAtom *atom;
	TSTRPTR s, d;

	s = d = name;
	while (*s++);

	atom = exec_AllocMMU0(exec, TNULL, sizeof(struct TAtom) + (s - d));
	if (atom)
	{
		atom->tatm_Handle.thn_Owner = (struct TModule *) exec;
		s = d;
		d = (TSTRPTR) (atom + 1);
		atom->tatm_Handle.thn_Name = d;
		while ((*d++ = *s++));
		TINITLIST(&atom->tatm_Waiters);
		atom->tatm_State = TATOMF_LOCKED;
		atom->tatm_Nest = 1;
		TAddHead(&exec->texb_AtomList, (struct TNode *) atom);
		TDBPRINTF(TDB_TRACE,("atom %s created - nest: 1\n", name));
	}

	return atom;
}

/*****************************************************************************/

static void
exec_lockatom(TEXECBASE *exec, struct TTask *msg)
{
	TUINT mode = msg->tsk_Request.trq_Atom.tra_Mode;
	struct TAtom *atom = msg->tsk_Request.trq_Atom.tra_Atom;
	struct TTask *task = msg->tsk_Request.trq_Atom.tra_Task;

	switch (mode & (TATOMF_CREATE|TATOMF_SHARED|TATOMF_NAME|TATOMF_TRY))
	{
		case TATOMF_CREATE | TATOMF_SHARED | TATOMF_NAME:
		case TATOMF_CREATE | TATOMF_NAME:

			atom = exec_lookupatom(exec, (TSTRPTR) atom);
			if (atom)
				goto obtain;
			goto create;

		case TATOMF_CREATE | TATOMF_SHARED | TATOMF_TRY | TATOMF_NAME:
		case TATOMF_CREATE | TATOMF_TRY | TATOMF_NAME:

			atom = exec_lookupatom(exec, (TSTRPTR) atom);
			if (atom)
			{
				atom = TNULL; /* already exists - deny */
				goto reply;
			}

		create:
			atom = exec_newatom(exec, msg->tsk_Request.trq_Atom.tra_Atom);

		reply:
			exec_replyatom(exec, msg, atom);
			return;

		case TATOMF_NAME | TATOMF_SHARED | TATOMF_TRY:
		case TATOMF_NAME | TATOMF_SHARED:
		case TATOMF_NAME | TATOMF_TRY:
		case TATOMF_NAME:

			atom = exec_lookupatom(exec, (TSTRPTR) atom);

		case TATOMF_SHARED | TATOMF_TRY:
		case TATOMF_SHARED:
		case TATOMF_TRY:
		case 0:

			if (atom)
				goto obtain;

		fail:
		default:

			atom = TNULL;
			goto reply;

		obtain:

			if (atom->tatm_State & TATOMF_LOCKED)
			{
				if (atom->tatm_State & TATOMF_SHARED)
				{
					if (mode & TATOMF_SHARED)
					{
		nest:			atom->tatm_Nest++;
						TDBPRINTF(TDB_TRACE,("nest: %d\n", atom->tatm_Nest));
						goto reply;
					}
				}
				else
					if (atom->tatm_Owner == task)
						goto nest;

				if (mode & TATOMF_TRY)
					goto fail;
			}
			else
			{
				if (atom->tatm_Nest)
					TDBPRINTF(TDB_FAIL,
						("atom->nestcount %d!\n", atom->tatm_Nest));

				atom->tatm_State = TATOMF_LOCKED;
				if (mode & TATOMF_SHARED)
				{
					atom->tatm_State |= TATOMF_SHARED;
					atom->tatm_Owner = TNULL;
				}
				else
					atom->tatm_Owner = task;
				atom->tatm_Nest = 1;
				TDBPRINTF(TDB_TRACE,
					("atom taken. nest: %d\n", atom->tatm_Nest));
				goto reply;
			}

			/* put this request into atom's list of waiters */
			msg->tsk_Request.trq_Atom.tra_Mode = mode & TATOMF_SHARED;
			TAddTail(&atom->tatm_Waiters, &msg->tsk_Request.trq_Atom.tra_Node);
			TDBPRINTF(TDB_TRACE,("must wait\n"));
	}
}

/*****************************************************************************/

static void
exec_unlockatom(TEXECBASE *exec, struct TTask *msg)
{
	TUINT mode = msg->tsk_Request.trq_Atom.tra_Mode;
	struct TAtom *atom = msg->tsk_Request.trq_Atom.tra_Atom;

	atom->tatm_Nest--;
	TDBPRINTF(TDB_TRACE,("unlock. nest: %d\n", atom->tatm_Nest));

	if (atom->tatm_Nest == 0)
	{
		union TTaskRequest *waiter;

		atom->tatm_State = 0;
		atom->tatm_Owner = TNULL;

		if (mode & TATOMF_DESTROY)
		{
			struct TNode *nextnode, *node = atom->tatm_Waiters.tlh_Head;
			while ((nextnode = node->tln_Succ))
			{
				waiter = (union TTaskRequest *) node;
				TREMOVE(node);
				exec_replyatom(exec, waiter->trq_Atom.tra_Task, TNULL);
				node = nextnode;
			}

			TDBPRINTF(TDB_TRACE,
				("atom free: %s\n", atom->tatm_Handle.thn_Name));
			TREMOVE((struct TNode *) atom);
			exec_Free(exec, atom);
		}
		else
		{
			waiter = (union TTaskRequest *) TRemHead(&atom->tatm_Waiters);
			if (waiter)
			{
				TUINT waitmode = waiter->trq_Atom.tra_Mode;

				atom->tatm_Nest++;
				atom->tatm_State = TATOMF_LOCKED;
				TDBPRINTF(TDB_TRACE,
					("restarted waiter. nest: %d\n", atom->tatm_Nest));
				exec_replyatom(exec, waiter->trq_Atom.tra_Task, atom);

				/* just restarted a shared waiter?
					then restart ALL shared waiters */

				if (waitmode & TATOMF_SHARED)
				{
					struct TNode *nextnode, *node =
						atom->tatm_Waiters.tlh_Head;
					atom->tatm_State |= TATOMF_SHARED;
					while ((nextnode = node->tln_Succ))
					{
						waiter = (union TTaskRequest *) node;
						if (waiter->trq_Atom.tra_Mode & TATOMF_SHARED)
						{
							TREMOVE(node);
							atom->tatm_Nest++;
							TDBPRINTF(TDB_TRACE,
								("restarted waiter. nest: %d\n",
								atom->tatm_Nest));
							exec_replyatom(exec, waiter->trq_Atom.tra_Task,
								atom);
						}
						node = nextnode;
					}
				}
				else
					atom->tatm_Owner = waiter->trq_Atom.tra_Task;
			}
		}
	}

	exec_returnmsg(exec, msg, TMSG_STATUS_ACKD | TMSGF_QUEUED);
}
