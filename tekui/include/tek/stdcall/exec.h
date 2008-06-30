#ifndef _TEK_STDCALL_EXEC_H
#define _TEK_STDCALL_EXEC_H

/*
**	$Id: exec.h,v 1.1 2008-06-30 12:34:56 tmueller Exp $
**	teklib/tek/stdcall/exec.h - exec module interface
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

/* -- Functions for bootstrapping Exec, not needed outside init code -- */

#define TExecDoExec(exec,tags) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TTAGITEM *))(exec))[-9]))(exec,tags)

#define TExecCreateSysTask(exec,func,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTASKFUNC,TTAGITEM *))(exec))[-10]))(exec,func,tags)

/* -- Grant access to the HAL module base, needed by device drivers -- */

#define TExecGetHALBase(exec) \
	(*(((TMODCALL TAPTR(**)(TAPTR))(exec))[-11]))(exec)

/* -- General public Exec API -- */

#define TExecLoadModule(exec,name,version,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR,TUINT16,TTAGITEM *))(exec))[-12]))(exec,name,version,tags)

#define TExecOpenModule(exec,name,version,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR,TUINT16,TTAGITEM *))(exec))[-13]))(exec,name,version,tags)

#define TExecCloseModule(exec,mod) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-14]))(exec,mod)

#define TExecUnloadModule(exec,mod) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-15]))(exec,mod)

#define TExecCopyMem(exec,src,dst,len) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TAPTR,TUINT))(exec))[-16]))(exec,src,dst,len)

#define TExecFillMem(exec,dst,len,val) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TUINT,TUINT8))(exec))[-17]))(exec,dst,len,val)

#define TExecFillMem32(exec,dst,len,val) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TUINT,TUINT))(exec))[-18]))(exec,dst,len,val)

#define TExecCreateMMU(exec,object,type,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TUINT,TTAGITEM *))(exec))[-19]))(exec,object,type,tags)

#define TExecAlloc(exec,mmu,size) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TUINT))(exec))[-20]))(exec,mmu,size)

#define TExecAlloc0(exec,mmu,size) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TUINT))(exec))[-21]))(exec,mmu,size)

#define TExecQueryInterface(exec,mod,name,version,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TSTRPTR,TUINT16,TTAGITEM *))(exec))[-22]))(exec,mod,name,version,tags)

#define TExecDropInterface(exec,mod,iface) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TAPTR))(exec))[-23]))(exec,mod,iface)

#define TExecFree(exec,mem) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-24]))(exec,mem)

#define TExecRealloc(exec,mem,nsize) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TUINT))(exec))[-25]))(exec,mem,nsize)

#define TExecGetMMU(exec,mem) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(exec))[-26]))(exec,mem)

#define TExecGetSize(exec,mem) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR))(exec))[-27]))(exec,mem)

#define TExecCreateLock(exec,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(exec))[-28]))(exec,tags)

#define TExecLock(exec,lock) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-29]))(exec,lock)

#define TExecUnlock(exec,lock) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-30]))(exec,lock)

#define TExecAllocSignal(exec,sig) \
	(*(((TMODCALL TUINT(**)(TAPTR,TUINT))(exec))[-31]))(exec,sig)

#define TExecFreeSignal(exec,sig) \
	(*(((TMODCALL void(**)(TAPTR,TUINT))(exec))[-32]))(exec,sig)

#define TExecSignal(exec,task,sig) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TUINT))(exec))[-33]))(exec,task,sig)

#define TExecSetSignal(exec,newsig,sigmask) \
	(*(((TMODCALL TUINT(**)(TAPTR,TUINT,TUINT))(exec))[-34]))(exec,newsig,sigmask)

#define TExecWait(exec,sig) \
	(*(((TMODCALL TUINT(**)(TAPTR,TUINT))(exec))[-35]))(exec,sig)

#define TExecStrEqual(exec,s1,s2) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR,TSTRPTR))(exec))[-36]))(exec,s1,s2)

#define TExecCreatePort(exec,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(exec))[-37]))(exec,tags)

#define TExecPutMsg(exec,port,a,msg) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TAPTR,TAPTR))(exec))[-38]))(exec,port,a,msg)

#define TExecGetMsg(exec,port) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(exec))[-39]))(exec,port)

#define TExecAckMsg(exec,msg) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-40]))(exec,msg)

#define TExecReplyMsg(exec,msg) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-41]))(exec,msg)

#define TExecDropMsg(exec,msg) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(exec))[-42]))(exec,msg)

#define TExecSendMsg(exec,port,msg) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR,TAPTR))(exec))[-43]))(exec,port,msg)

#define TExecWaitPort(exec,port) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(exec))[-44]))(exec,port)

#define TExecGetPortSignal(exec,port) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR))(exec))[-45]))(exec,port)

#define TExecGetUserPort(exec,task) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(exec))[-46]))(exec,task)

#define TExecGetSyncPort(exec,task) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(exec))[-47]))(exec,task)

#define TExecCreateTask(exec,func,ifunc,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTASKFUNC,TINITFUNC,TTAGITEM *))(exec))[-48]))(exec,func,ifunc,tags)

#define TExecFindTask(exec,name) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR))(exec))[-49]))(exec,name)

#define TExecGetTaskData(exec,task) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(exec))[-50]))(exec,task)

#define TExecSetTaskData(exec,task,data) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TAPTR))(exec))[-51]))(exec,task,data)

#define TExecGetTaskMMU(exec,task) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(exec))[-52]))(exec,task)

#define TExecAllocMsg(exec,size) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TUINT))(exec))[-53]))(exec,size)

#define TExecAllocMsg0(exec,size) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TUINT))(exec))[-54]))(exec,size)

#define TExecLockAtom(exec,atom,mode) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TUINT))(exec))[-55]))(exec,atom,mode)

#define TExecUnlockAtom(exec,atom,mode) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TUINT))(exec))[-56]))(exec,atom,mode)

#define TExecGetAtomData(exec,atom) \
	(*(((TMODCALL TTAG(**)(TAPTR,TAPTR))(exec))[-57]))(exec,atom)

#define TExecSetAtomData(exec,atom,data) \
	(*(((TMODCALL TTAG(**)(TAPTR,TAPTR,TTAG))(exec))[-58]))(exec,atom,data)

#define TExecCreatePool(exec,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TTAGITEM *))(exec))[-59]))(exec,tags)

#define TExecAllocPool(exec,pool,size) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TUINT))(exec))[-60]))(exec,pool,size)

#define TExecFreePool(exec,pool,mem,size) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TAPTR,TUINT))(exec))[-61]))(exec,pool,mem,size)

#define TExecReallocPool(exec,pool,mem,oldsize,newsize) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR,TAPTR,TUINT,TUINT))(exec))[-62]))(exec,pool,mem,oldsize,newsize)

#define TExecPutIO(exec,ioreq) \
	(*(((TMODCALL void(**)(TAPTR,struct TIORequest *))(exec))[-63]))(exec,ioreq)

#define TExecWaitIO(exec,ioreq) \
	(*(((TMODCALL TINT(**)(TAPTR,struct TIORequest *))(exec))[-64]))(exec,ioreq)

#define TExecDoIO(exec,ioreq) \
	(*(((TMODCALL TINT(**)(TAPTR,struct TIORequest *))(exec))[-65]))(exec,ioreq)

#define TExecCheckIO(exec,ioreq) \
	(*(((TMODCALL TINT(**)(TAPTR,struct TIORequest *))(exec))[-66]))(exec,ioreq)

#define TExecAbortIO(exec,ioreq) \
	(*(((TMODCALL TINT(**)(TAPTR,struct TIORequest *))(exec))[-67]))(exec,ioreq)

/* -- Semi-private; to manipulate msgs in ports, not normally needed -- */

#define TExecInsertMsg(exec,port,msg,prevmsg,status) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TAPTR,TAPTR,TUINT))(exec))[-68]))(exec,port,msg,prevmsg,status)

#define TExecRemoveMsg(exec,port,msg) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR,TAPTR))(exec))[-69]))(exec,port,msg)

#define TExecGetMsgStatus(exec,msg) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR))(exec))[-70]))(exec,msg)

#define TExecSetMsgReplyPort(exec,msg,rport) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR,TAPTR))(exec))[-71]))(exec,msg,rport)

#define TExecSetPortHook(exec,port,hook) \
	(*(((TMODCALL struct THook *(**)(TAPTR,TAPTR,struct THook *))(exec))[-72]))(exec,port,hook)

/* -- on with public functions -- */

#define TExecAddModules(exec,im,flags) \
	(*(((TMODCALL TBOOL(**)(TAPTR,struct TModInitNode *,TUINT))(exec))[-73]))(exec,im,flags)

#define TExecRemModules(exec,im,flags) \
	(*(((TMODCALL TBOOL(**)(TAPTR,struct TModInitNode *,TUINT))(exec))[-74]))(exec,im,flags)

#endif /* _TEK_STDCALL_EXEC_H */
