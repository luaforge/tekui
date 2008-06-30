
/*
**	$Id: time_mod.c,v 1.1 2008-06-30 12:34:23 tmueller Exp $
**	teklib/src/time/time_mod.c - Time and date functions
**
**	Written by Frank Pagels <copper at coplabs.org>
**	and Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <math.h>
#include <tek/debug.h>
#include <tek/teklib.h>
#include <tek/proto/exec.h>
#include <tek/proto/hal.h>
#include <tek/mod/time.h>

/*****************************************************************************/

#define TIME_VERSION	3
#define TIME_REVISION	0
#define TIME_NUMVECTORS	34

typedef struct
{
	struct TModule mod;
	TAPTR hal;

} TMOD_TIME;

#ifndef LOCAL
#define LOCAL	static
#endif

#ifndef EXPORT
#define EXPORT	static TMODAPI
#endif

/*****************************************************************************/
/*
**	Prototypes
*/

EXPORT void time_sub(TMOD_TIME *tmod, TTIME *a, TTIME *b);
EXPORT void time_add(TMOD_TIME *tmod, TTIME *a, TTIME *b);
EXPORT TINT time_cmp(TMOD_TIME *tmod, TTIME *a, TTIME *b);

EXPORT struct TTimeRequest *time_allocreq(TMOD_TIME *tmod, TTAGITEM *tags);
EXPORT void time_freereq(TMOD_TIME *tmod, struct TTimeRequest *req);
EXPORT void time_query(TMOD_TIME *tmod, struct TTimeRequest *req,
	TTIME *timep);
EXPORT TINT time_getdate(TMOD_TIME *tmod, struct TTimeRequest *tr, TDATE *dtp,
	TINT *tzp);
EXPORT TUINT time_wait(TMOD_TIME *tmod, struct TTimeRequest *tr, TTIME *timep,
	TUINT sig);
EXPORT TUINT time_waitdate(TMOD_TIME *tmod, struct TTimeRequest *tr,
	TDATE *date, TUINT sig);

EXPORT TBOOL time_makedate(TMOD_TIME *tmod, TDATE *td, TINT d, TINT m, TINT y,
	TTIME *tm);
EXPORT void time_adddate(TMOD_TIME *tmod, TDATE *d, TINT ndays, TTIME *tm);
EXPORT void time_subdate(TMOD_TIME *tmod, TDATE *d, TINT ndays, TTIME *tm);
EXPORT TINT time_diffdate(TMOD_TIME *tmod, TDATE *d1, TDATE *d2, TTIME *tm);
EXPORT TBOOL time_isleapyear(TMOD_TIME *tmod, TINT y);
EXPORT TBOOL time_isvalid(TMOD_TIME *tmod, TINT d, TINT m, TINT y);
EXPORT TBOOL time_pack(TMOD_TIME *tmod, struct TDateBox *db, TDATE *td);
EXPORT void time_unpack(TMOD_TIME *tmod, TDATE *td, struct TDateBox *db,
	TUINT16 f);

EXPORT TDOUBLE time_datetojulian(TMOD_TIME *tmod, TDATE *td);
EXPORT void time_juliantodate(TMOD_TIME *tmod, TDOUBLE jd, TDATE *td);
EXPORT TDOUBLE time_mytojulian(TMOD_TIME *tmod, TINT m, TINT y);
EXPORT TDOUBLE time_juliantodmy(TMOD_TIME *tmod, TDOUBLE jd, TINT *pd, TINT *pm,
	TINT *py);
EXPORT void time_ydaytodm(TMOD_TIME *tmod, TINT yday, TINT year, TINT *pd,
	TINT *pm);
EXPORT TINT time_dmytoyday(TMOD_TIME *tmod, TINT d, TINT m, TINT y);
EXPORT TINT time_getweekday(TMOD_TIME *tmod, TINT d, TINT m, TINT y);
EXPORT TINT time_getweeknumber(TMOD_TIME *tmod, TINT d, TINT m, TINT y);
EXPORT void time_delay(TMOD_TIME *tmod, struct TTimeRequest *tr,
	TTIME *timep);

static const TMFPTR time_vectors[TIME_NUMVECTORS];

/*****************************************************************************/
/*
**	Module init
*/

TMODENTRY TUINT
tek_init_time(TAPTR task, struct TModule *mod, TUINT16 version, TTAGITEM *tags)
{
	TMOD_TIME *tmod = (TMOD_TIME *) mod;

	if (tmod == TNULL)
	{
		if (version == 0xffff)
			return sizeof(TMFPTR) * TIME_NUMVECTORS; /* negative size */

		if (version <= TIME_VERSION)
			return sizeof(TMOD_TIME); /* positive size */

		return 0;
	}

	tmod->hal = TExecGetHALBase(TGetExecBase(tmod));

	tmod->mod.tmd_Version = TIME_VERSION;
	tmod->mod.tmd_Revision = TIME_REVISION;
	tmod->mod.tmd_Flags = TMODF_VECTORTABLE;

	TInitVectors(tmod, time_vectors, TIME_NUMVECTORS);

	return TTRUE;
}

/*****************************************************************************/

static const TMFPTR
time_vectors[TIME_NUMVECTORS] =
{
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,
	(TMFPTR) TNULL,

	(TMFPTR) time_sub,
	(TMFPTR) time_add,
	(TMFPTR) time_cmp,
	(TMFPTR) time_allocreq,
	(TMFPTR) time_freereq,
	(TMFPTR) time_query,
	(TMFPTR) time_getdate,
	(TMFPTR) time_wait,
	(TMFPTR) time_waitdate,
	(TMFPTR) time_makedate,
	(TMFPTR) time_adddate,
	(TMFPTR) time_subdate,
	(TMFPTR) time_diffdate,
	(TMFPTR) time_isleapyear,
	(TMFPTR) time_isvalid,
	(TMFPTR) time_pack,
	(TMFPTR) time_unpack,
	(TMFPTR) time_datetojulian,
	(TMFPTR) time_juliantodate,
	(TMFPTR) time_mytojulian,
	(TMFPTR) time_juliantodmy,
	(TMFPTR) time_ydaytodm,
	(TMFPTR) time_dmytoyday,
	(TMFPTR) time_getweekday,
	(TMFPTR) time_getweeknumber,
	(TMFPTR) time_delay,
};

/*****************************************************************************/

EXPORT void
time_sub(TMOD_TIME *tmod, TTIME *a, TTIME *b)
{
	if (a->ttm_USec < b->ttm_USec)
	{
		a->ttm_Sec = a->ttm_Sec - b->ttm_Sec - 1;
		a->ttm_USec = 1000000 - (b->ttm_USec - a->ttm_USec);
	}
	else
	{
		a->ttm_Sec = a->ttm_Sec - b->ttm_Sec;
		a->ttm_USec = a->ttm_USec - b->ttm_USec;
	}
}

/*****************************************************************************/

EXPORT void
time_add(TMOD_TIME *tmod, TTIME *a, TTIME *b)
{
	a->ttm_Sec += b->ttm_Sec;
	a->ttm_USec += b->ttm_USec;
	if (a->ttm_USec >= 1000000)
	{
		a->ttm_USec -= 1000000;
		a->ttm_Sec++;
	}
}

/*****************************************************************************/

EXPORT TINT
time_cmp(TMOD_TIME *tmod, TTIME *a, TTIME *b)
{
	if (a->ttm_Sec < b->ttm_Sec) return -1;
	if (a->ttm_Sec > b->ttm_Sec) return 1;
	if (a->ttm_USec == b->ttm_USec) return 0;
	if (a->ttm_USec > b->ttm_USec) return 1;
	return -1;
}

/*****************************************************************************/

EXPORT struct TTimeRequest *
time_allocreq(TMOD_TIME *tmod, TTAGITEM *tags)
{
	TAPTR exec = TGetExecBase(tmod);
	return TExecOpenModule(exec, TMODNAME_TIMER, 0, TNULL);
}

/*****************************************************************************/

EXPORT void
time_freereq(TMOD_TIME *tmod, struct TTimeRequest *req)
{
	if (req)
	{
		TAPTR exec = TGetExecBase(tmod);
		TExecCloseModule(exec, req->ttr_Req.io_Device);
		TExecFree(exec, req);
	}
}

/*****************************************************************************/
/*
**	time_query(time, treq, time)
**	Insert system time into *time
*/

EXPORT void
time_query(TMOD_TIME *tmod, struct TTimeRequest *tr, TTIME *timep)
{
#if 1
	THALGetSysTime(tmod->hal, timep);
#else
	if (tr)
	{
		TAPTR saverp = tr->ttr_Req.io_ReplyPort;
		tr->ttr_Req.io_ReplyPort = TNULL;
		tr->ttr_Req.io_Command = TTREQ_GETTIME;

		if (TExecDoIO(TExecBase, (struct TIORequest *) tr) == 0)
		{
			if (timep)
				*timep = tr->ttr_Data.ttr_Time;
		}
		tr->ttr_Req.io_ReplyPort = saverp;
	}
#endif
}

/*****************************************************************************/
/*
**	err = time_getdate(time, treq, date, tzsec)
**
**	Insert datetime into *date. If tzsec is NULL, *date will
**	be set to local time. Otherwise, *date will be set to UT,
**	and *tzsec will be set to seconds west of UT.
**
**	err = -1 - illegal arguments
**	err = 0 - ok
**	err = 1 - no date resource available
**	err = 2 - no timezone info available
*/

EXPORT TINT
time_getdate(TMOD_TIME *tmod, struct TTimeRequest *tr, TDATE *dtp, TINT *tzp)
{
	TAPTR exec = TGetExecBase(tmod);
	TINT err = -1;
	if (tr)
	{
		TAPTR saverp = tr->ttr_Req.io_ReplyPort;

		tr->ttr_Req.io_ReplyPort = TNULL;
		tr->ttr_Data.ttr_Date.ttr_TimeZone = 1000000;	/* absurd */

		if (tzp)
			tr->ttr_Req.io_Command = TTREQ_GETUNIDATE;
		else
			tr->ttr_Req.io_Command = TTREQ_GETLOCALDATE;

		if (TExecDoIO(exec, (struct TIORequest *) tr) == 0)
		{
			err = 0;

			if (dtp)
				*dtp = tr->ttr_Data.ttr_Date.ttr_Date;

			if (tzp)
			{
				if (tr->ttr_Data.ttr_Date.ttr_TimeZone == 1000000)
					err = 2;
				else
					*tzp = tr->ttr_Data.ttr_Date.ttr_TimeZone;
			}
		}
		else
			err = 1;

		tr->ttr_Req.io_ReplyPort = saverp;
	}

	return err;
}

/*****************************************************************************/
/*
**	sig = time_wait(time, treq, time, sigmask)
**	wait an amount of time, or for a set of signals
*/

EXPORT TUINT
time_wait(TMOD_TIME *tmod, struct TTimeRequest *tr, TTIME *timeout,
	TUINT sigmask)
{
	TAPTR exec = TGetExecBase(tmod);
	TUINT sig = 0;

	if (timeout && (timeout->ttm_Sec || timeout->ttm_USec))
	{
		TAPTR saverp = tr->ttr_Req.io_ReplyPort;

		tr->ttr_Req.io_ReplyPort = TExecGetSyncPort(exec, TNULL);
		tr->ttr_Req.io_Command = TTREQ_ADDTIME;
		tr->ttr_Data.ttr_Time = *timeout;

		TExecPutIO(exec, (struct TIORequest *) tr);
		sig = TExecWait(exec, TTASK_SIG_SINGLE | sigmask);
		TExecAbortIO(exec, (struct TIORequest *) tr);
		TExecWaitIO(exec, (struct TIORequest *) tr);
		TExecSetSignal(exec, 0, TTASK_SIG_SINGLE);

		tr->ttr_Req.io_ReplyPort = saverp;

		sig &= sigmask;
	}
	else
	{
		if (sigmask)
			sig = TExecWait(exec, sigmask);
	}

	return sig;
}

/*****************************************************************************/
/*
**	sig = time_waitdate(time, treq, date, sigmask)
**	wait for an universal date, or a set of signals
*/

EXPORT TUINT
time_waitdate(TMOD_TIME *tmod, struct TTimeRequest *tr, TDATE *date,
	TUINT sigmask)
{
	TAPTR exec = TGetExecBase(tmod);
	TUINT sig = 0;

	if (date)
	{
		TAPTR saverp = tr->ttr_Req.io_ReplyPort;

		tr->ttr_Req.io_ReplyPort = TExecGetSyncPort(exec, TNULL);
		tr->ttr_Req.io_Command = TTREQ_ADDLOCALDATE;
		tr->ttr_Data.ttr_Date.ttr_Date = *date;

		TExecPutIO(exec, (struct TIORequest *) tr);
		sig = TExecWait(exec, TTASK_SIG_SINGLE | sigmask);
		TExecAbortIO(exec, (struct TIORequest *) tr);
		TExecWaitIO(exec, (struct TIORequest *) tr);
		TExecSetSignal(exec, 0, TTASK_SIG_SINGLE);

		tr->ttr_Req.io_ReplyPort = saverp;

		sig &= sigmask;
	}
	else
	{
		if (sigmask)
			sig = TExecWait(exec, sigmask);
	}

	return sig;
}

/*****************************************************************************/
/*
**	jd = time_datetojulian(date, td)
**	Get a julian day from a date
*/

EXPORT TDOUBLE
time_datetojulian(TMOD_TIME *tmod, TDATE *td)
{
	return THALDateToJulian(tmod->hal, td);
}

/*****************************************************************************/
/*
**	time_juliantodate(date, jd, td)
**	Set a date from a julian day
*/

EXPORT void
time_juliantodate(TMOD_TIME *tmod, TDOUBLE jd, TDATE *td)
{
	THALJulianToDate(tmod->hal, jd, td);
}

/*****************************************************************************/
/*
**	isleap = time_isleapyear(date, y)
**	Check if year is a leap year
*/

EXPORT TBOOL
time_isleapyear(TMOD_TIME *tmod, TINT y)
{
	if (y & 3) return TFALSE;
	if (y < 1582) return TTRUE;
	if (y % 100) return TTRUE;
	if (y % 400) return TFALSE;
	return TTRUE;
}

/*****************************************************************************/
/*
**	valid = time_isvaliddate(date, d, m, y)
**	Check if a date is valid
*/

EXPORT TBOOL
time_isvalid(TMOD_TIME *tmod, TINT d, TINT m, TINT y)
{
	static const TINT8 MDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if (m < 1 || m > 12) return TFALSE;
	if (m == 2 && d == 29) return time_isleapyear(tmod, y);
	if (y == 1582 && m == 10 && (d > 4 && d < 15)) return TFALSE;
	if (d < 1 || d > MDays[m - 1]) return TFALSE;
	return TTRUE;
}

/*****************************************************************************/
/*
**	jd = time_mytojulian(date, m, y)
**	Convert month, year to a Julian date
*/

EXPORT TDOUBLE
time_mytojulian(TMOD_TIME *tmod, TINT m, TINT y)
{
	TDOUBLE jd = -1524.5;
	TINT a;

	if (m < 3)
	{
		y--;
		m += 12;
	}

	a = 30.6001 * (m + 1);
	jd += a;

	if (y > 1582)
	{
		a = y / 100;
		jd += 2 - a + a / 4;
	}

	a = 365.25 * (y + 4716);
	jd += a;

	return jd;
}

/*****************************************************************************/
/*
**	dayfract = time_juliantodmy(date, jd, pd, pm, py)
**	convert Julian date to Day, Month, Year.
**	Returns the fraction of the day
*/

EXPORT TDOUBLE
time_juliantodmy(TMOD_TIME *tmod, TDOUBLE jd, TINT *pd, TINT *pm, TINT *py)
{
	TINT z, y, m, a;

	jd += 0.5;
	z = jd;

	if (z >= 2299161)
	{
		a = (z - 1867216.25) / 36524.25;
		z = z + 1 + a - a / 4;
	}

	z += 1524;
	y = (z - 122.1) / 365.25;
	a = 365.25 * y;
	z -= a;
	m = z / 30.6001;

	if (pd)
	{
		a = 30.6001 * m;
		*pd = z - a;
	}

	m -= m < 14 ? 1 : 13;

	if (pm)
		*pm = m;

	if (py)
	{
		if (m > 2)
			y--;
		*py = y - 4715;
	}

	return jd - (TINT) jd;
}

/*****************************************************************************/
/*
**	time_ydaytodm(date, yday, year, pd, pm)
**	Calculate a day and month from a yearday and year
*/

EXPORT
void time_ydaytodm(TMOD_TIME *tmod, TINT n, TINT y, TINT *pd, TINT *pm)
{
	TINT k, m;

	k = 2 - time_isleapyear(tmod, y);

	if (n < 32)
		m = 1;
	else
		m = 9 * (TFLOAT) (k + n) / 275.0 + 0.98;

	if (pm)
		*pm = m;

	if (pd)
		*pd = n - (TINT) ((TFLOAT) m * 275 / 9) +
			k * (TINT) (((TFLOAT) m + 9) / 12) + 30;
}

/*****************************************************************************/
/*
**	yday = time_dmytoyday(date, d, m, y)
**	Calculate the yearday from a date
*/

EXPORT TINT
time_dmytoyday(TMOD_TIME *tmod, TINT d, TINT m, TINT y)
{
	TINT k, n;

	k = 2 - time_isleapyear(tmod, y);
	k *= (m + 9) / 12;
	n = 275 * m / 9;
	n += d - 30 - k;

	return n;
}

/*****************************************************************************/
/*
**	valid = time_pack(date, datebox, td)
**	pack a datebox structure to a TDATE
*/

EXPORT TBOOL
time_pack(TMOD_TIME *tmod, struct TDateBox *db, TDATE *td)
{
	TBOOL success = TFALSE;

	if (db && td)
	{
		TUINT16 f = db->tdb_Fields;
		TINT y = db->tdb_Year;
		TINT m = 0, d = 0;	/* this makes compilers happy */

		if ((f & (TDB_YEAR|TDB_MONTH|TDB_DAY)) == (TDB_YEAR|TDB_MONTH|TDB_DAY))
		{
			d = db->tdb_Day;
			m = db->tdb_Month;
			success = time_isvalid(tmod, d, m, y);
		}
		else if ((f & (TDB_YEAR|TDB_YDAY)) == (TDB_YEAR|TDB_YDAY))
		{
			d = db->tdb_YDay;
			time_ydaytodm(tmod, d, y, TNULL, &m);
			success = TTRUE;
		}

		if (success)
		{
			TDOUBLE t, jd = time_mytojulian(tmod, m, y) + d;

			if (f & TDB_HOUR)
			{
				t = db->tdb_Hour;
				jd += t / 24;
			}

			if (f & TDB_MINUTE)
			{
				t = db->tdb_Minute;
				jd += t / 1440;
			}

			if (f & (TDB_SEC|TDB_USEC))
			{
				t = 0;

				if (f & TDB_USEC)
				{
					t = db->tdb_USec;
					t /= 1000000;
				}

				if (f & TDB_SEC)
					t += db->tdb_Sec;

				t /= 86400;
				jd += t;
			}

			THALJulianToDate(tmod->hal, jd, td);
		}
	}

	return success;
}

/*****************************************************************************/
/*
**	time_unpack(date, datebox, tdate, fields)
**	unpack a TDATE to a datebox structure
*/

EXPORT void
time_unpack(TMOD_TIME *tmod, TDATE *td, struct TDateBox *db, TUINT16 rf)
{
	if (td && db)
	{
		TUINT16 wf;
		TINT d, m, t;
		TDOUBLE jd, df;

		jd = THALDateToJulian(tmod->hal, td);
		df = time_juliantodmy(tmod, jd, &d, &m, &db->tdb_Year);
		db->tdb_Month = m;
		db->tdb_Day = d;

		wf = TDB_YEAR|TDB_MONTH|TDB_DAY;

		if (rf & (TDB_HOUR|TDB_MINUTE|TDB_SEC|TDB_USEC))
		{
			t = df *= 24;
			df -= db->tdb_Hour = t;
			wf |= TDB_HOUR;
			if (rf & (TDB_MINUTE|TDB_SEC|TDB_USEC))
			{
				t = df *= 60;
				df -= db->tdb_Minute = t;
				wf |= TDB_MINUTE;
				if (rf & (TDB_SEC|TDB_USEC))
				{
					t = df *= 60;
					df -= db->tdb_Sec = t;
					wf |= TDB_SEC;
					if (rf & TDB_USEC)
					{
						t = df *= 1000000;
						db->tdb_USec = t;
					}
				}
			}
		}

		if (rf & TDB_WEEK)
			db->tdb_Week = time_getweeknumber(tmod, d, m, db->tdb_Year);

		if (rf & TDB_YDAY)
			db->tdb_YDay = time_dmytoyday(tmod, d, m, db->tdb_Year);

		if (rf & TDB_WDAY)
			db->tdb_WDay = time_getweekday(tmod, d, m, db->tdb_Year);

		db->tdb_Fields = wf | rf;
	}
}

/*****************************************************************************/
/*
**	time_getweekday(date, d,m,y)
**	Get weekday number from date
*/

EXPORT TINT
time_getweekday(TMOD_TIME *tmod, TINT d, TINT m, TINT y)
{
	TDOUBLE jd = time_mytojulian(tmod, m, y) + d + 1.5;
	return (TINT) jd % 7;
}

/*****************************************************************************/
/*
**	week = time_getweeknumber(date, d, m, y)
**	Calculate a date's week number
*/

EXPORT TINT
time_getweeknumber(TMOD_TIME *tmod, TINT d, TINT m, TINT y)
{
	TINT J, Jb, dd, w;

	J = time_mytojulian(tmod, m, y) + d + 0.5;
	Jb = time_mytojulian(tmod, 1, y) + 1.5;
	dd = (Jb + 3) % 7;
	w = (J + dd - Jb + 4) / 7;

	if ((w >= 1 && w <= 52) || (w == 53 && d == 6))
		return w;

	if (w == 53 && d == 6 && time_isleapyear(tmod, y))
		return w;

	if (w == 53)
		return 1;	/* first week of next year */

	if (w == 0)
	{
		Jb = time_mytojulian(tmod, 1, y - 1) + 1.5;
		dd = (Jb + 3) % 7;
		w = (J + dd - Jb + 4)/7;
	}

	return w;
}

/*****************************************************************************/
/*
**	valid = time_makedate(date, td, d, m, y, time)
**	Make a date from day, month, year, optionally time
*/

EXPORT TBOOL
time_makedate(TMOD_TIME *tmod, TDATE *td, TINT d, TINT m, TINT y, TTIME *tm)
{
	TBOOL success = TFALSE;
	if (td)
	{
		if (time_isvalid(tmod, d, m, y))
		{
			TDOUBLE jd = time_mytojulian(tmod, m, y) + d;
			if (tm)
			{
				TDOUBLE s = tm->ttm_USec;
				s /= 1000000;
				s += tm->ttm_Sec;
				s /= 86400;
				jd += s;
			}
			THALJulianToDate(tmod->hal, jd, td);
			success = TTRUE;
		}
	}
	return success;
}

/*****************************************************************************/
/*
**	time_adddate(date, d, ndays, time)
**	Add a number of days, and optionally a time, to date d1.
*/

EXPORT void
time_adddate(TMOD_TIME *tmod, TDATE *d, TINT ndays, TTIME *tm)
{
	TDOUBLE jd = THALDateToJulian(tmod->hal, d);
	jd += ndays;
	if (tm)
	{
		TDOUBLE s = tm->ttm_USec;
		s /= 1000000;
		s += tm->ttm_Sec;
		s /= 86400;
		jd += s;
	}
	THALJulianToDate(tmod->hal, jd, d);
}

/*****************************************************************************/
/*
**	time_subdate(date, d, ndays, time)
**	Subtract a number of days, and optionally a time, from a date.
*/

EXPORT void
time_subdate(TMOD_TIME *tmod, TDATE *d, TINT ndays, TTIME *tm)
{
	TDOUBLE jd = THALDateToJulian(tmod->hal, d);
	jd -= ndays;
	if (tm)
	{
		TDOUBLE s = tm->ttm_USec;
		s /= 1000000;
		s += tm->ttm_Sec;
		s /= 86400;
		jd -= s;
	}
	THALJulianToDate(tmod->hal, jd, d);
}

/*****************************************************************************/
/*
**	ndays = time_diffdate(date, d1, d2, tm)
**	Get the number of days difference between two dates,
**	and optionally the number of seconds/microseconds
*/

EXPORT TINT
time_diffdate(TMOD_TIME *tmod, TDATE *d1, TDATE *d2, TTIME *tm)
{
	TDOUBLE jd;
	TINT nd;

	jd = THALDateToJulian(tmod->hal, d1);
	jd -= THALDateToJulian(tmod->hal, d2);
	nd = jd;

	if (tm)
	{
		TINT t;

		if (nd < 0)
			jd = -jd - nd;
		else
			jd -= nd;

		t = jd *= 86400;
		jd -= tm->ttm_Sec = t;
		t = jd *= 1000000;
		tm->ttm_USec = t;
	}

	return nd;
}

/*****************************************************************************/
/*
**	time_delay(time, treq, time)
**	wait an amount of time
*/

EXPORT void
time_delay(TMOD_TIME *tmod, struct TTimeRequest *tr, TTIME *timeout)
{
	if (timeout && (timeout->ttm_Sec || timeout->ttm_USec))
	{
		tr->ttr_Req.io_ReplyPort = TNULL;
			/*TExecGetSyncPort(TExecBase, TNULL);*/
		tr->ttr_Req.io_Command = TTREQ_ADDTIME;
		tr->ttr_Data.ttr_Time = *timeout;
		TExecDoIO(TGetExecBase(tmod), (struct TIORequest *) tr);
	}
}

