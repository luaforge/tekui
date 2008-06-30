
#ifndef _TEK_MOD_EXEC_H
#define _TEK_MOD_EXEC_H

/*
**	$Id: exec.h,v 1.1 2008-06-30 12:34:54 tmueller Exp $
**	teklib/tek/mod/exec.h - Exec module private
**	See copyright notice in teklib/COPYRIGHT
**
**	Do not depend on this include file, as it may break your code's
**	binary compatibility with newer versions of the Exec module.
**	Normal applications require only tek/exec.h
*/

#include <tek/exec.h>
#include <tek/mod/time.h>

/*****************************************************************************/
/*
**	MMU allocation header
*/

union TMMUInfo
{
	struct
	{
		/* Ptr to memory manager */
		struct TMemManager *tmu_MMU;
		/* Size of allocation (excl. mmuinfo) */
		TUINT tmu_UserSize;
	} tmu_Node;
	/* Enforce per-platform alignment */
	struct TMMUInfoAlign tmu_Align;
};

/*****************************************************************************/
/*
**	Memory free node
*/

union TMemNode
{
	struct
	{
		/* Next free node, or TNULL */
		union TMemNode *tmn_Next;
		/* Size of this node, in bytes */
		TUINT tmn_Size;
	} tmn_Node;
	/* Enforce per-platform alignment */
	struct TMemNodeAlign tmn_Align;
};

/*****************************************************************************/
/*
**	Memheader and pool node
*/

union TMemHead
{
	struct
	{
		/* Node header */
		struct TNode tmh_Node;
		/* Singly linked list of free nodes */
		union TMemNode *tmh_FreeList;
		/* Memory block */
		TINT8 *tmh_Mem;
		/* End of memory block, aligned */
		TINT8 *tmh_MemEnd;
		/* Padding */
		TAPTR tmh_Pad1;
		/* Number of free bytes */
		TUINTPTR tmh_Free;
		/* Alignment in bytes - 1 */
		TUINT tmh_Align;
		/* Flags, see below */
		TUINT tmh_Flags;
		/* Padding */
		TUINT tmh_Pad2;
	} tmh_Node;
	/* enforce alignment */
	struct TMemHeadAlign tmh_Align;
};

/* First match allocation strategy */
#define TMEMHF_NONE		0
/* Best match allocation strategy */
#define TMEMHF_LOWFRAG	1
/* Used in pools: large puddle */
#define TMEMHF_LARGE	2
/* Used in pools: auto adaptive */
#define TMEMHF_AUTO		4
/* Used in pools: fixed size */
#define TMEMHF_FIXED	8
/* Used in pools: free allocations */
#define TMEMHF_FREE		16

/*****************************************************************************/
/*
**	Pooled memory header
*/

struct TMemPool
{
	/* Exec object handle */
	struct THandle tpl_Handle;
	/* List of puddles */
	struct TList tpl_List;
	/* Parent allocator */
	TAPTR tpl_MMU;
	/* Size of puddles */
	TUINT tpl_PudSize;
	/* Threshold for large allocations */
	TUINT tpl_ThresSize;
	/* Alignment in bytes - 1 */
	TUINT16 tpl_Align;
	/* Flags (passed to memheader) */
	TUINT16 tpl_Flags;
};

/*****************************************************************************/
/*
**	Locking object
*/

struct TLock
{
	/* Exec object handle */
	struct THandle tlk_Handle;
	/* List of lockwait requests */
	struct TList tlk_Waiters;
	/* HAL locking object */
	struct THALObject tlk_HLock;
	/* Current owner */
	struct TTask *tlk_Owner;
	/* Recursion counter */
	TUINT16 tlk_NestCount;
	/* Number of waiters */
	TUINT16 tlk_WaitCount;
};

/*
**	Lockwait request
*/

struct TLockWait
{
	/* Link to tlk_Waiters */
	struct TNode tlr_Node;
	/* Task waiting for the lock */
	struct TTask *tlr_Task;
};

/*****************************************************************************/
/*
**	Message port
**	access point for inter-tasks messages, coupled with a task signal
*/

struct TMsgPort
{
	/* Exec object handle */
	struct THandle tmp_Handle;
	/* List of queued messages */
	struct TList tmp_MsgList;
	/* HAL locking object */
	struct THALObject tmp_Lock;
	/* Task to be signalled on arrival */
	struct TTask *tmp_SigTask;
	/* Hook to be invoked on msg arrival */
	struct THook *tmp_Hook;
	/* Signal to appear in sigtask */
	TUINT tmp_Signal;
};

/*****************************************************************************/
/*
**	Memory manager, aka 'MMU'
*/

#define TMMSG_DESTROY		0
#define TMMSG_ALLOC			1
#define TMMSG_FREE			2
#define TMMSG_REALLOC		3

union TMemMsg
{
	TUINT tmmsg_Type;
	struct
	{
		TUINT tmmsg_Type;
		TUINT tmmsg_Size;
	} tmmsg_Alloc;
	struct
	{
		TUINT tmmsg_Type;
		TAPTR tmmsg_Ptr;
		TUINT tmmsg_Size;
	} tmmsg_Free;
	struct
	{
		TUINT tmmsg_Type;
		TAPTR tmmsg_Ptr;
		TUINT tmmsg_OSize;
		TUINT tmmsg_NSize;
	} tmmsg_Realloc;
};

struct TMemManager
{
	/* Exec object handle */
	struct THandle tmm_Handle;
	/* Callback hook */
	struct THook tmm_Hook;
	/* MMU's underlying allocator */
	TAPTR tmm_Allocator;
	/* List header for tracking managers */
	struct TList tmm_TrackList;
	/* Locking for thread-safe managers */
	struct TLock tmm_Lock;
	/* MMU type and capability flags */
	TUINT tmm_Type;
};

/*****************************************************************************/
/*
**	Task request
**	The task structure is actually a message. A command code and request
**	structure determines what kind of action is to be taken when a task
**	is being messaged to Exec.
*/

union TTaskRequest
{
	struct
	{
		/* Link to sysbase task lists */
		struct TNode trt_Node;
		/* Task to be created/closed */
		struct TTask *trt_Task;
		/* Backptr to initiating task */
		struct TTask *trt_Parent;
		/* Task function */
		TTASKFUNC trt_Func;
		/* Task init function */
		TINITFUNC trt_InitFunc;
		/* User-supplied tags for creation */
		TTAGITEM *trt_Tags;
	} trq_Task;

	struct
	{
		/* Module 'stub' during initialization */
		struct TModule trm_InitMod;
		/* Loaded/initialized module */
		struct TModule *trm_Module;
		/* List of waiters for this module */
		struct TList trm_Waiters;
		/* Backup of original replyport */
		struct TMsgPort *trm_RPort;
		/* Backup of requesting task */
		struct TTask *trm_ReqTask;
	} trq_Mod;

	struct
	{
		/* Link to atom's list of waiters */
		struct TNode tra_Node;
		/* Requesting/waiting task */
		struct TTask *tra_Task;
		/* Ptr to name or atom */
		TAPTR tra_Atom;
		/* Access mode */
		TUINT tra_Mode;
	} trq_Atom;

	struct
	{
		/* Module init node */
		struct TModInitNode *trm_ModInitNode;
		/* Flags */
		TUINT trm_Flags;
		/* Result */
		TBOOL trm_Result;
	} trq_AddRemMod;
};

/*
**	Request codes for task->tsk_ReqCode
*/

/* Get a new module: */
#define TTREQ_OPENMOD		0
/* Close a module: */
#define TTREQ_CLOSEMOD		1
/* Load a module from ramlib: */
#define TTREQ_LOADMOD		2
/* Let ramlib unload module: */
#define TTREQ_UNLOADMOD		3
/* Create a new task: */
#define TTREQ_CREATETASK	4
/* Destroy task: */
#define TTREQ_DESTROYTASK	5
/* Lock a named atom: */
#define TTREQ_LOCKATOM		6
/* Unlock a named atom: */
#define TTREQ_UNLOCKATOM	7
/* Add a module: */
#define TTREQ_ADDMOD		8
/* Remove a module: */
#define TTREQ_REMMOD		9

/*****************************************************************************/
/*
**	Task. TEKlib tasks are 'heavyweight threads', as they come with
**	predefined message ports and signals, a memory manager, and they
**	can inherit a currentdir and I/O descriptors for stdin/out/err
*/

struct TTask
{
	/* Exec object handle */
	struct THandle tsk_Handle;
	/* User/init data */
	TAPTR tsk_UserData;

	/* Task request code */
	TUINT tsk_ReqCode;
	/* Task request data */
	union TTaskRequest tsk_Request;

	/* Task's primary async user port */
	struct TMsgPort tsk_UserPort;
	/* Task's internal sync replyport */
	struct TMsgPort tsk_SyncPort;

	/* Heap memory manager */
	struct TMemManager tsk_HeapMMU;

	/* HAL locking object */
	struct THALObject tsk_TaskLock;
	/* HAL thread object */
	struct THALObject tsk_Thread;

	/* Free signals */
	TUINT tsk_SigFree;
	/* Used signals */
	TUINT tsk_SigUsed;
	/* Task status */
	TUINT tsk_Status;

	/* Lock to current directory */
	TAPTR tsk_CurrentDir;
	/* Last I/O error */
	TINT tsk_IOErr;

	/* I/O module base ptr */
	TAPTR tsk_IOBase;

	/* Input stream */
	TAPTR tsk_FHIn;
	/* Output stream */
	TAPTR tsk_FHOut;
	/* Error stream */
	TAPTR tsk_FHErr;

	/* Task flags, see below */
	TUINT tsk_Flags;
};

/*
**	Task flags
*/

/* Close input FH on task exit */
#define TTASKF_CLOSEINPUT	1
/* Close output FH on task exit */
#define TTASKF_CLOSEOUTPUT	2
/* Close error FH on task exit */
#define TTASKF_CLOSEERROR	4

/*
**	Task status
*/

/* Task is initializing */
#define TTASK_INITIALIZING	0
/* Task is running */
#define TTASK_RUNNING		1
/* Task has concluded */
#define TTASK_FINISHED		2
/* Task failed to initialize */
#define TTASK_FAILED		3

/*
**	Set of signals reserved for the system
*/

#define TTASK_SIG_RESERVED 	0x0000000f

/*****************************************************************************/
/*
**	Execbase structure
*/

typedef struct
{
	/* Module header */
	struct TModule texb_Module;
	/* HAL module base */
	struct TModule *texb_HALBase;
	/* General purpose memory manager */
	struct TMemManager texb_BaseMMU;
	/* Message memory manager */
	struct TMemManager texb_MsgMMU;
	/* Locking for Execbase structures */
	struct THALObject texb_Lock;
	/* List of modules */
	struct TList texb_ModList;
	/* List of tasks */
	struct TList texb_TaskList;
	/* List of initializing tasks */
	struct TList texb_TaskInitList;
	/* List of closing tasks */
	struct TList texb_TaskExitList;
	/* List of named atoms */
	struct TList texb_AtomList;
	/* List of internal modules */
	struct TList texb_IntModList;
	/* Node of initial modules (passed from init): */
	struct TModInitNode texb_InitModNode;
	/* Ptr to Execbase task */
	struct TTask *texb_ExecTask;
	/* Ptr to task for module loading */
	struct TTask *texb_IOTask;
	/* Ptr to &exectask->userport */
	struct TMsgPort *texb_ExecPort;
	/* Ptr to &exectask->syncport */
	struct TMsgPort *texb_ModReply;
	/* Number of user tasks running */
	TINT texb_NumTasks;
	/* Number of initializing tasks */
	TINT texb_NumInitTasks;

} TEXECBASE;

/*
**	Execbase signals
*/

/* Task change from init to running */
#define TTASK_SIG_CHILDINIT	0x80000000
/* Task exited */
#define TTASK_SIG_CHILDEXIT	0x40000000

/*****************************************************************************/
/*
**	Message
**	Note that in TEKlib the message header is encapsulated in the
**	message allocation, and hence invisible to the application.
*/

struct TMessage
{
	/* Node header */
	struct TNode tmsg_Node;
	/* Port to which msg is returned */
	struct TMsgPort *tmsg_RPort;
	/* Sender object (reserved) */
	TAPTR tmsg_Sender;
	/* Proxy object (reserved) */
	TAPTR tmsg_Proxy;
	/* Delivery status */
	TUINT tmsg_Flags;
};

/*
**	Support macros
*/

#define TGETMSGPTR(mem) \
	((struct TMessage *) ((TINT8 *) (mem) - sizeof(union TMMUInfo)) - 1)
#define TGETMSGSTATUS(mem) \
	TGETMSGPTR(mem)->tmsg_Flags
#define TGETMSGREPLYPORT(mem) \
	TGETMSGPTR(mem)->tmsg_RPort

/*****************************************************************************/
/*
**	Atoms
*/

struct TAtom
{
	/* Exec object handle */
	struct THandle tatm_Handle;
	/* List of waiting tasks */
	struct TList tatm_Waiters;
	/* User data field */
	TTAG tatm_Data;
	/* Exclusive owner task */
	TAPTR tatm_Owner;
	/* State */
	TUINT tatm_State;
	/* Nest count */
	TUINT tatm_Nest;
};

/* Internal use only: */
#define TATOMF_LOCKED	0x0020
#define TATOMF_UNLOCK	0x0040

#endif
