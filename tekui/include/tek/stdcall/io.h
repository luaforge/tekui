#ifndef _TEK_STDCALL_IO_H
#define _TEK_STDCALL_IO_H

/*
**	$Id: io.h,v 1.1 2008-06-30 12:34:54 tmueller Exp $
**	teklib/tek/stdcall/io.h - io module interface
**
**	See copyright notice in teklib/COPYRIGHT
*/

#define TIOLockFile(io,name,mode,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR,TUINT,TTAGITEM *))(io))[-9]))(io,name,mode,tags)

#define TIOUnlockFile(io,lock) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(io))[-10]))(io,lock)

#define TIOOpenFile(io,name,mode,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR,TUINT,TTAGITEM *))(io))[-11]))(io,name,mode,tags)

#define TIOCloseFile(io,fh) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TAPTR))(io))[-12]))(io,fh)

#define TIORead(io,fh,buf,len) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TAPTR,TINT))(io))[-13]))(io,fh,buf,len)

#define TIOWrite(io,fh,buf,len) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TAPTR,TINT))(io))[-14]))(io,fh,buf,len)

#define TIOFlush(io,fh) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TAPTR))(io))[-15]))(io,fh)

#define TIOSeek(io,fh,offs,offshi,mode) \
	(*(((TMODCALL TUINT(**)(TAPTR,TAPTR,TINT,TINT *,TINT))(io))[-16]))(io,fh,offs,offshi,mode)

#define TIOFPutC(io,fh,c) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TINT))(io))[-17]))(io,fh,c)

#define TIOFGetC(io,fh) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR))(io))[-18]))(io,fh)

#define TIOFEoF(io,fh) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TAPTR))(io))[-19]))(io,fh)

#define TIOFRead(io,fh,buf,len) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TAPTR,TINT))(io))[-20]))(io,fh,buf,len)

#define TIOFWrite(io,fh,buf,len) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TAPTR,TINT))(io))[-21]))(io,fh,buf,len)

#define TIOExamine(io,lock,tags) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TTAGITEM *))(io))[-22]))(io,lock,tags)

#define TIOExNext(io,lock,tags) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TTAGITEM *))(io))[-23]))(io,lock,tags)

#define TIOChangeDir(io,lock) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(io))[-24]))(io,lock)

#define TIOParentDir(io,lock) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(io))[-25]))(io,lock)

#define TIONameOf(io,lock,buf,len) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TSTRPTR,TINT))(io))[-26]))(io,lock,buf,len)

#define TIODupLock(io,lock) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(io))[-27]))(io,lock)

#define TIOOpenFromLock(io,lock) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TAPTR))(io))[-28]))(io,lock)

#define TIOAddPart(io,p1,p2,buf,len) \
	(*(((TMODCALL TINT(**)(TAPTR,TSTRPTR,TSTRPTR,TSTRPTR,TINT))(io))[-29]))(io,p1,p2,buf,len)

#define TIOAssignLate(io,name,path) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR,TSTRPTR))(io))[-30]))(io,name,path)

#define TIOAssignLock(io,name,lock) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR,TAPTR))(io))[-31]))(io,name,lock)

#define TIORename(io,name,newname) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR,TSTRPTR))(io))[-32]))(io,name,newname)

#define TIOMakeDir(io,name,tags) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR,TTAGITEM *))(io))[-33]))(io,name,tags)

#define TIODeleteFile(io,name) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR))(io))[-34]))(io,name)

#define TIOSetIOErr(io,newerr) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT))(io))[-35]))(io,newerr)

#define TIOGetIOErr(io) \
	(*(((TMODCALL TINT(**)(TAPTR))(io))[-36]))(io)

#define TIOObtainPacket(io,path,namepart) \
	(*(((TMODCALL TAPTR(**)(TAPTR,TSTRPTR,TSTRPTR *))(io))[-37]))(io,path,namepart)

#define TIOReleasePacket(io,packet) \
	(*(((TMODCALL void(**)(TAPTR,TAPTR))(io))[-38]))(io,packet)

#define TIOFault(io,err,buf,len,tags) \
	(*(((TMODCALL TINT(**)(TAPTR,TINT,TSTRPTR,TINT,TTAGITEM *))(io))[-39]))(io,err,buf,len,tags)

#define TIOWaitChar(io,fh,timeout) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TAPTR,TINT))(io))[-40]))(io,fh,timeout)

#define TIOIsInteractive(io,fh) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TAPTR))(io))[-41]))(io,fh)

#define TIOOutputFH(io) \
	(*(((TMODCALL TAPTR(**)(TAPTR))(io))[-42]))(io)

#define TIOInputFH(io) \
	(*(((TMODCALL TAPTR(**)(TAPTR))(io))[-43]))(io)

#define TIOErrorFH(io) \
	(*(((TMODCALL TAPTR(**)(TAPTR))(io))[-44]))(io)

#define TIOMakeName(io,name,dest,dlen,mode,tags) \
	(*(((TMODCALL TINT(**)(TAPTR,TSTRPTR,TSTRPTR,TINT,TINT,TTAGITEM *))(io))[-45]))(io,name,dest,dlen,mode,tags)

#define TIOMount(io,name,action,tags) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR,TINT,TTAGITEM *))(io))[-46]))(io,name,action,tags)

#define TIOFUngetC(io,fh,c) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TINT))(io))[-47]))(io,fh,c)

#define TIOFPutS(io,fh,s) \
	(*(((TMODCALL TINT(**)(TAPTR,TAPTR,TSTRPTR))(io))[-48]))(io,fh,s)

#define TIOFGetS(io,fh,buf,len) \
	(*(((TMODCALL TSTRPTR(**)(TAPTR,TAPTR,TSTRPTR,TINT))(io))[-49]))(io,fh,buf,len)

#define TIOSetFileDate(io,name,date,tags) \
	(*(((TMODCALL TBOOL(**)(TAPTR,TSTRPTR,TDATE *,TTAGITEM *))(io))[-50]))(io,name,date,tags)

#endif /* _TEK_STDCALL_IO_H */
