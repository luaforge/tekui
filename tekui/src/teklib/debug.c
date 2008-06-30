
/*
**	$Id: debug.c,v 1.1 2008-06-30 12:34:40 tmueller Exp $
**	teklib/src/teklib/debug.c - Debug library
**
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <stdarg.h>
#include <stdio.h>
#include <tek/teklib.h>

TLIBAPI TINT
TDebugPutS(TSTRPTR s)
{
	TDEBUG_PLATFORM_PUTS(s);
	return 0;
}

TLIBAPI TINT
TDebugPrintF(TSTRPTR fmt, ...)
{
	static char buf[2048];
	TINT numc;

	va_list args;

	va_start(args, fmt);

	numc = vsprintf(buf, fmt, args);

	va_end(args);

	buf[sizeof(buf) - 1] = 0;

	TDEBUG_PLATFORM_PUTS(buf);

	return numc;
}

TLIBAPI TINT
TDebugFatal(void)
{
	TDEBUG_PLATFORM_FATAL();
	return 0;
}
