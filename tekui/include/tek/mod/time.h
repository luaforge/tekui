
#ifndef _TEK_MOD_TIME_H
#define _TEK_MOD_TIME_H

/*
**	$Id: time.h,v 1.1 2008-06-30 12:34:54 tmueller Exp $
**	teklib/tek/mod/time.h - Time/date module definitions
**
**	Written by Frank Pagels <copper at coplabs.org>
**	and Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/exec.h>
#include <tek/mod/ioext.h>

/*****************************************************************************/
/*
**	Time and Date structure
*/

struct TTime
{
	TINT ttm_Sec;			/* Seconds */
	TINT ttm_USec;			/* 1/1000000th Seconds */
};

struct TDate
{
	TDATE_T tdt_Day;		/* Private */
};

typedef struct TTime TTIME;
typedef struct TDate TDATE;

/*****************************************************************************/
/*
**	Weekday definitions
*/

#define TDT_SUNDAY		0
#define TDT_MONDAY		1
#define TDT_TUESDAY		2
#define TDT_WEDNESDAY	3
#define TDT_THURSDAY	4
#define TDT_FRIDAY		5
#define TDT_SATURDAY	6

/*****************************************************************************/
/* 
**	"Datebox" - Date/Time container
*/

struct TDateBox
{
	TUINT16 tdb_Fields;		/* 0  Fields, see below */
	TUINT16 tdb_Reserved1;	/* 2  Reserved for future use */
	TINT tdb_Year;			/* 4  Year */
	TUINT16 tdb_YDay;		/* 8  Day of year 1...366 */
	TUINT16 tdb_Month;		/* 10 Month 1...12 */
	TUINT16 tdb_Week;		/* 12 Week of year 1...53 */
	TUINT16 tdb_WDay;		/* 14 Day of week 0 (sunday) ... 6 (saturday) */
	TUINT16 tdb_Day;		/* 16 Day of month 1...31 */
	TUINT16 tdb_Hour;		/* 18 Hour of day 0...23 */
	TUINT16 tdb_Minute;		/* 20 Minute of hour 0...59 */
	TUINT16 tdb_Sec;		/* 22 Second of minute 0...59 */
	TUINT tdb_USec;			/* 24 Microsecond of second 0... 999999 */
	TUINT tdb_Reserved2;	/* 28 Reserved for future extensions */
};							/* 32 bytes */

/* 
**	Corresponding flags in datebox->tdb_Fields if value present
*/

#define TDB_YEAR		0x0001
#define TDB_YDAY		0x0002
#define TDB_MONTH		0x0004
#define TDB_WEEK		0x0008
#define TDB_WDAY		0x0010
#define TDB_DAY			0x0020
#define TDB_HOUR		0x0040
#define TDB_MINUTE		0x0080
#define TDB_SEC			0x0100
#define TDB_USEC		0x0200
#define TDB_ALL         0x03ff

/*****************************************************************************/
/* 
**	Timer I/O request
*/

struct TTimeRequest
{
	struct TIORequest ttr_Req;		/* I/O request header */
	union
	{
		TTIME ttr_Time;				/* Relative time */

		struct
		{
			TDATE ttr_Date;			/* Local or UT time */
			TINT ttr_TimeZone;		/* Seconds west of UT */

		} ttr_Date;
		
		TINT ttr_DSSec;				/* Number of seconds Daylight Saving */

	} ttr_Data;
};

/* 
**	I/O Commands
*/

#define TTREQ_GETTIME		(TIOCMD_EXTENDED + 0)	/* Get system rel. time */
#define	TTREQ_GETUNIDATE	(TIOCMD_EXTENDED + 1)	/* Get universal */
#define	TTREQ_GETLOCALDATE	(TIOCMD_EXTENDED + 2)	/* Get local */
#define	TTREQ_ADDTIME		(TIOCMD_EXTENDED + 3)	/* Wait relative time */
#define	TTREQ_ADDUNIDATE	(TIOCMD_EXTENDED + 4)	/* Wait for universal */
#define	TTREQ_ADDLOCALDATE	(TIOCMD_EXTENDED + 5)	/* Wait for local */
#define TTREQ_GETDSFROMDATE (TIOCMD_EXTENDED + 6)	/* Daylight saving sec. */

/*****************************************************************************/
/*
**	Revision History
**	$Log: time.h,v $
**	Revision 1.1  2008-06-30 12:34:54  tmueller
**	Initial revision
**
**	Revision 1.1.1.1  2006/08/20 22:15:25  tmueller
**	intermediate import
**
**	Revision 1.4  2006/08/19 11:26:16  tmueller
**	renamed ttr_Sec to ttr_DSSec
**
**	Revision 1.3  2006/06/25 22:23:21  tmueller
**	added TTREQ_GETDSFROMDATE request
**
**	Revision 1.2  2005/09/13 02:45:09  tmueller
**	updated copyright reference
**	
**	Revision 1.1.1.1  2003/12/11 07:17:50  tmueller
**	Krypton import
**	
**	Revision 1.2  2003/09/17 16:51:38  tmueller
**	(TTAG) casts removed
**	
**	Revision 1.1.1.1  2003/03/08 18:28:40  tmueller
**	Import to new chrooted pserver repository.
**	
**	Revision 1.2  2003/01/06 19:20:21  copper
**	add Schwerin in townlist
**	
**	Revision 1.1.1.1  2002/11/30 05:15:33  bifat
**	import
*/

#endif
