
/*
**	teklib/src/visual/visual_mod.c - Visual module
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include "visual_mod.h"

static TMOD_VIS *vis_modopen(TMOD_VIS *mod, TTAGITEM *tags);
static void vis_modclose(TMOD_VIS *mod);
static TBOOL vis_init(TMOD_VIS *mod);
static void vis_exit(TMOD_VIS *mod);

static const TMFPTR
vis_vectors[VISUAL_NUMVECTORS] =
{
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,

	(TMFPTR) vis_openvisual,
	(TMFPTR) vis_closevisual,
	(TMFPTR) vis_attach,
	(TMFPTR) vis_openfont,
	(TMFPTR) vis_closefont,
	(TMFPTR) vis_getfattrs,
	(TMFPTR) vis_textsize,
	(TMFPTR) vis_queryfonts,
	(TMFPTR) vis_getnextfont,

	(TMFPTR) vis_getport,
	(TMFPTR) vis_setinput,
	(TMFPTR) vis_getattrs,
	(TMFPTR) vis_setattrs,
	(TMFPTR) vis_allocpen,
	(TMFPTR) vis_freepen,
	(TMFPTR) vis_setfont,
	(TMFPTR) vis_clear,
	(TMFPTR) vis_rect,
	(TMFPTR) vis_frect,
	(TMFPTR) vis_line,
	(TMFPTR) vis_plot,
	(TMFPTR) vis_text,

	(TMFPTR) vis_drawstrip,
	(TMFPTR) vis_drawtags,
	(TMFPTR) vis_drawfan,
	(TMFPTR) vis_drawarc,
	(TMFPTR) vis_copyarea,
	(TMFPTR) vis_setcliprect,

	(TMFPTR) vis_opendisplay,
	(TMFPTR) vis_closedisplay,
	(TMFPTR) vis_querydisplays,
	(TMFPTR) vis_getnextdisplay,

	(TMFPTR) vis_unsetcliprect,
	(TMFPTR) vis_drawfarc,

	(TMFPTR) vis_drawbuffer,

};

static void
vis_destroy(TMOD_VIS *mod)
{
	TDestroy(mod->vis_Lock);
}

static THOOKENTRY TTAG
vis_dispatch(struct THook *hook, TAPTR obj, TTAG msg)
{
	TMOD_VIS *mod = (TMOD_VIS *) hook->thk_Data;
	switch (msg)
	{
		case TMSG_DESTROY:
			vis_destroy(mod);
			break;
		case TMSG_OPENMODULE:
			return (TTAG) vis_modopen(mod, obj);
		case TMSG_CLOSEMODULE:
			vis_modclose(obj);
	}
	return 0;
}

TMODENTRY TUINT
tek_init_visual(TAPTR task, struct TModule *vis, TUINT16 version,
	TTAGITEM *tags)
{
	TMOD_VIS *mod = (TMOD_VIS *) vis;
	if (mod == TNULL)
	{
		if (version == 0xffff)
			return sizeof(TAPTR) * VISUAL_NUMVECTORS;

		if (version <= VISUAL_VERSION)
			return sizeof(TMOD_VIS);

		return 0;
	}

	for (;;)
	{
		mod->vis_ExecBase = TGetExecBase(mod);
		mod->vis_Lock = TExecCreateLock(mod->vis_ExecBase, TNULL);
		if (mod->vis_Lock == TNULL) break;

		mod->vis_Module.tmd_Version = VISUAL_VERSION;
		mod->vis_Module.tmd_Revision = VISUAL_REVISION;
		mod->vis_Module.tmd_Handle.thn_Hook.thk_Entry = vis_dispatch;
		mod->vis_Module.tmd_Flags = TMODF_VECTORTABLE | TMODF_OPENCLOSE;
		TInitVectors(mod, vis_vectors, VISUAL_NUMVECTORS);
		return TTRUE;
	}

	vis_destroy(mod);
	return TFALSE;
}

/*****************************************************************************/
/*
**	Module open/close
*/

static TMOD_VIS *
vis_modopen(TMOD_VIS *mod, TTAGITEM *tags)
{
	TMOD_VIS *inst = TNULL;
	TBOOL success = TTRUE;

	TExecLock(mod->vis_ExecBase, mod->vis_Lock);

	if (mod->vis_RefCount == 0)
		success = vis_init(mod);

	if (success)
		mod->vis_RefCount++;

	TExecUnlock(mod->vis_ExecBase, mod->vis_Lock);

	if (success)
	{
		TMOD_VIS *base = (TMOD_VIS *) TGetTag(tags, TVisual_Attach, TNULL);
		if (TGetTag(tags, TVisual_NewInstance, TFALSE)) base = mod;

		if (base)
		{
			inst = TNewInstance(base, base->vis_Module.tmd_PosSize,
				base->vis_Module.tmd_NegSize);
			if (inst)
			{
				TInitList(&inst->vis_ReqPool);
				TInitList(&inst->vis_WaitList);
				inst->vis_IMsgPort =
					TExecCreatePort(inst->vis_ExecBase, TNULL);
				inst->vis_CmdRPort =
					TExecCreatePort(inst->vis_ExecBase, TNULL);
				if (inst->vis_IMsgPort == TNULL || inst->vis_CmdRPort == TNULL)
				{
					TDestroy(inst->vis_CmdRPort);
					TDestroy(inst->vis_IMsgPort);
					TFreeInstance(inst);
					inst = TNULL;
				}
			}
			if (inst == TNULL)
				vis_modclose(mod);
		}
		else
			inst = mod;
	}

	return inst;
}

static void
vis_modclose(TMOD_VIS *inst)
{
	TMOD_VIS *mod = (TMOD_VIS *) inst->vis_Module.tmd_ModSuper;
	if (inst != mod)
	{
		struct TNode *node;
		TAPTR imsg;

		while ((imsg = TExecGetMsg(inst->vis_ExecBase, inst->vis_IMsgPort)))
			TExecAckMsg(inst->vis_ExecBase, imsg);

		while ((node = TRemHead(&inst->vis_ReqPool)))
			TExecFree(inst->vis_ExecBase, node);

		while ((node = TRemHead(&inst->vis_WaitList)))
		{
			TExecWaitIO(inst->vis_ExecBase, (struct TIORequest *) node);
			TExecFree(inst->vis_ExecBase, node);
		}

		TDestroy(inst->vis_CmdRPort);
		TDestroy(inst->vis_IMsgPort);
		TFreeInstance(inst);
	}
	TExecLock(mod->vis_ExecBase, mod->vis_Lock);
	if (--mod->vis_RefCount == 0)
		vis_exit(mod);
	TExecUnlock(mod->vis_ExecBase, mod->vis_Lock);
}

/*****************************************************************************/
/*
**	Module init/exit
*/

static TBOOL
vis_init(TMOD_VIS *mod)
{
	for (;;)
	{
		mod->vis_Displays = vis_createhash(mod, TNULL);
		if (mod->vis_Displays == TNULL) break;

		return TTRUE;
	}
	vis_exit(mod);
	return TFALSE;
}

static void
vis_exit(TMOD_VIS *mod)
{
	if (mod->vis_Displays)
	{
		struct TList dlist;
		struct TNode *next, *node;

		if (mod->vis_InitRequest)
			TDisplayFreeReq(mod->vis_InitRequest->tvr_Req.io_Device,
				mod->vis_InitRequest);

		TInitList(&dlist);
		vis_hashtolist(mod, mod->vis_Displays, &dlist);
		for (node = dlist.tlh_Head; (next = node->tln_Succ); node = next)
		{
			struct TModule *dmod = (struct TModule *)
				((struct vis_HashNode *) node)->value;
			TExecCloseModule(mod->vis_ExecBase, dmod);
		}

		vis_destroyhash(mod, mod->vis_Displays);
	}
}
