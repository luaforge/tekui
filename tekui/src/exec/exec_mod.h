
#ifndef _TEK_MOD_EXEC_MOD_H
#define _TEK_MOD_EXEC_MOD_H

/*
**	$Id: exec_mod.h,v 1.1 2008-06-30 12:34:09 tmueller Exp $
**	Exec module internal definitions
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/mod/exec.h>
#include <tek/mod/ioext.h>
#include <tek/proto/hal.h>

/*****************************************************************************/

#define EXEC_VERSION	5
#define EXEC_REVISION	0
#define EXEC_NUMVECTORS	74

/*****************************************************************************/

#ifndef LOCAL
#define LOCAL	TMODINTERN
#endif

#ifndef EXPORT
#define EXPORT	TMODAPI
#endif

/*****************************************************************************/
/*
**	Reserved module calls
*/

#define _TModBeginIO(dev,msg)	\
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(dev))[TMODV_BEGINIO]))(dev,msg)
#define _TModAbortIO(dev,msg)	\
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR))(dev))[TMODV_ABORTIO]))(dev,msg)

/*****************************************************************************/
/*
**	Internal
*/

LOCAL TBOOL exec_initmmu(TEXECBASE *exec, struct TMemManager *mmu,
	TAPTR allocator, TUINT mmutype, struct TTagItem *tags);
LOCAL TBOOL exec_initmemhead(union TMemHead *mh, TAPTR mem, TUINT size,
	TUINT flags, TUINT bytealign);
LOCAL TBOOL exec_initport(TEXECBASE *exec, struct TMsgPort *port,
	struct TTask *task, TUINT prefsignal);
LOCAL TUINT exec_allocsignal(TEXECBASE *exec, struct TTask *task,
	TUINT signals);
LOCAL void exec_freesignal(TEXECBASE *exec, struct TTask *task,
	TUINT signals);
LOCAL TBOOL exec_initlock(TEXECBASE *exec, struct TLock *lock);
LOCAL TAPTR exec_getmsg(TEXECBASE *exec, struct TMsgPort *port);
LOCAL void exec_returnmsg(TEXECBASE *exec, TAPTR mem, TUINT status);
LOCAL TUINT exec_sendmsg(TEXECBASE *exec, struct TTask *task,
	struct TMsgPort *port, TAPTR mem);

/*****************************************************************************/
/*
**	External
*/

EXPORT TBOOL exec_StrEqual(TEXECBASE *exec, TSTRPTR s1, TSTRPTR s2);
EXPORT TBOOL exec_DoExec(TEXECBASE *exec, struct TTagItem *tags);
EXPORT struct TTask *exec_CreateSysTask(TEXECBASE *exec, TTASKFUNC func,
	struct TTagItem *tags);
EXPORT TUINT exec_AllocSignal(TEXECBASE *exec, TUINT signals);
EXPORT void exec_FreeSignal(TEXECBASE *exec, TUINT signal);
EXPORT struct TMsgPort *exec_CreatePort(TEXECBASE *exec,
	struct TTagItem *tags);
EXPORT void exec_CloseModule(TEXECBASE *exec, struct TModule *module);
EXPORT struct TTask *exec_CreateTask(TEXECBASE *exec, TTASKFUNC function,
	TINITFUNC initfunc, struct TTagItem *tags);
EXPORT struct TAtom *exec_LockAtom(TEXECBASE *exec, TAPTR atom, TUINT mode);
EXPORT struct TMemManager *exec_CreateMMU(TEXECBASE *exec, TAPTR allocator,
	TUINT mmutype, struct TTagItem *tags);
EXPORT TUINT exec_SendMsg(TEXECBASE *exec, struct TMsgPort *port, TAPTR msg);
EXPORT TUINT exec_SetSignal(TEXECBASE *exec, TUINT newsignals, TUINT sigmask);
EXPORT void exec_Signal(TEXECBASE *exec, struct TTask *task, TUINT signals);
EXPORT TUINT exec_Wait(TEXECBASE *exec, TUINT sigmask);
EXPORT void exec_UnlockAtom(TEXECBASE *exec, struct TAtom *atom, TUINT mode);
EXPORT void exec_Lock(TEXECBASE *exec, struct TLock *lock);
EXPORT void exec_Unlock(TEXECBASE *exec, struct TLock *lock);
EXPORT void exec_AckMsg(TEXECBASE *exec, TAPTR msg);
EXPORT void exec_DropMsg(TEXECBASE *exec, TAPTR msg);
EXPORT void exec_Free(TEXECBASE *exec, TAPTR mem);
EXPORT struct TTask *exec_FindTask(TEXECBASE *exec, TSTRPTR name);
EXPORT TAPTR exec_GetMsg(TEXECBASE *exec, struct TMsgPort *port);
EXPORT void exec_CopyMem(TEXECBASE *exec, TAPTR from, TAPTR to,
	TUINT numbytes);
EXPORT void exec_FillMem(TEXECBASE *exec, TAPTR dest, TUINT numbytes,
	TUINT8 fillval);
EXPORT TAPTR exec_AllocMMU(TEXECBASE *exec, struct TMemManager *mmu,
	TUINT size);
EXPORT TAPTR exec_AllocMMU0(TEXECBASE *exec, struct TMemManager *mmu,
	TUINT size);
EXPORT TAPTR exec_OpenModule(TEXECBASE *exec, TSTRPTR modname, TUINT16 version,
	struct TTagItem *tags);
EXPORT TAPTR exec_QueryInterface(TEXECBASE *exec, struct TModule *mod,
	TSTRPTR name, TUINT16 version, struct TTagItem *tags);
EXPORT void exec_DropInterface(TEXECBASE *exec, struct TModule *mod,
	TAPTR iface);
EXPORT void exec_PutMsg(TEXECBASE *exec, struct TMsgPort *msgport,
	struct TMsgPort *replyport, TAPTR msg);
EXPORT TAPTR exec_Realloc(TEXECBASE *exec, TAPTR mem, TUINT newsize);
EXPORT void exec_ReplyMsg(TEXECBASE *exec, TAPTR msg);
EXPORT TAPTR exec_WaitPort(TEXECBASE *exec, struct TMsgPort *port);
EXPORT TAPTR exec_GetTaskMMU(TEXECBASE *exec, struct TTask *task);
EXPORT TAPTR exec_GetUserPort(TEXECBASE *exec, struct TTask *task);
EXPORT TAPTR exec_GetSyncPort(TEXECBASE *exec, struct TTask *task);
EXPORT TAPTR exec_GetTaskData(TEXECBASE *exec, struct TTask *task);
EXPORT TAPTR exec_SetTaskData(TEXECBASE *exec, struct TTask *task, TAPTR data);
EXPORT TAPTR exec_GetHALBase(TEXECBASE *exec);
EXPORT TAPTR exec_AllocMsg(TEXECBASE *exec, TUINT size);
EXPORT TAPTR exec_AllocMsg0(TEXECBASE *exec, TUINT size);
EXPORT TAPTR exec_CreateLock(TEXECBASE *exec, struct TTagItem *tags);
EXPORT TUINT exec_GetPortSignal(TEXECBASE *exec, TAPTR port);
EXPORT TTAG exec_GetAtomData(TEXECBASE *exec, struct TAtom *atom);
EXPORT TTAG exec_SetAtomData(TEXECBASE *exec, struct TAtom *atom, TTAG data);
EXPORT TUINT exec_GetSize(TEXECBASE *exec, TAPTR mem);
EXPORT TAPTR exec_GetMMU(TEXECBASE *exec, TAPTR mem);
EXPORT TAPTR exec_CreatePool(TEXECBASE *exec, struct TTagItem *tags);
EXPORT TAPTR exec_AllocPool(TEXECBASE *exec, struct TMemPool *pool,
	TUINT size);
EXPORT void exec_FreePool(TEXECBASE *exec, struct TMemPool *pool, TINT8 *mem,
	TUINT size);
EXPORT TAPTR exec_ReallocPool(TEXECBASE *exec, struct TMemPool *pool,
	TINT8 *oldmem, TUINT oldsize, TUINT newsize);
EXPORT void exec_FillMem32(TEXECBASE *exec, TUINT *dest, TUINT len,
	TUINT fill);
EXPORT void exec_PutIO(TEXECBASE *exec, struct TIORequest *ioreq);
EXPORT TINT exec_WaitIO(TEXECBASE *exec, struct TIORequest *ioreq);
EXPORT TINT exec_DoIO(TEXECBASE *exec, struct TIORequest *ioreq);
EXPORT TBOOL exec_CheckIO(TEXECBASE *exec, struct TIORequest *ioreq);
EXPORT TINT exec_AbortIO(TEXECBASE *exec, struct TIORequest *ioreq);
EXPORT TBOOL exec_AddModules(TEXECBASE *exec, struct TModInitNode *tmin,
	TUINT flags);
EXPORT TBOOL exec_RemModules(TEXECBASE *exec, struct TModInitNode *tmin,
	TUINT flags);

/*****************************************************************************/
/*
**	External, private functions
*/

EXPORT void exec_InsertMsg(TEXECBASE *exec, struct TMsgPort *port, TAPTR msg,
	TAPTR predmsg, TUINT status);
EXPORT void exec_RemoveMsg(TEXECBASE *exec, struct TMsgPort *port, TAPTR msg);
EXPORT TUINT exec_GetMsgStatus(TEXECBASE *exec, TAPTR mem);
EXPORT struct TMsgPort *exec_SetMsgReplyPort(TEXECBASE *exec, TAPTR mem,
	struct TMsgPort *rport);
EXPORT struct THook *exec_SetPortHook(TEXECBASE *exec, struct TMsgPort *port,
	struct THook *hook);

#endif
