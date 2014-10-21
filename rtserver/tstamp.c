/* tstamp.c  --  UTC timestamp <--> ASCII conversions. */
/* $Id: tstamp.c,v 1.1.1.1 2006/07/20 09:21:20 jwagnerhki Exp $ */

/*
 * $Log: tstamp.c,v $
 * Revision 1.1.1.1  2006/07/20 09:21:20  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:37:21  jwagnerhki
 * added to trunk
 *
 * Revision 1.6  2002/12/18 07:31:51  amn
 * Upped revision.
 *
 * Revision 1.2  2002/12/18 07:31:49  amn
 * intermediate
 *
 * Revision 1.1  2002/12/18 07:23:46  amn
 * Recovered from /home disk crash by re-creating 'common' module in CVS.
 *
 * Revision 1.5  2002/11/03 10:43:50  amn
 * Changed C++ 'struct timeval& atv's into normal C pointers.
 *
 * Revision 1.4  2002/11/03 09:46:44  amn
 * Auto£og entry was still slightly wrong, corrected to ' *'.
 *
 * Revision 1.3  2002/11/03 09:45:17  amn
 * Changed tstamp.cc to be only C, so that it can become tstamp.c.
 *
 * Revision 1.2  2002/11/03 09:23:20  amn
 * Moved UTC time ASCII --> double conversions to tstamp.cc.
 *
 * Revision 1.1  2000/07/28 09:28:20  amn
 * Changed to using VC 'open'(1) and added logging 'log=' to antcli.
 */

#include "registration.h"
RCSID(tstamp_cc, "$Id: tstamp.c,v 1.1.1.1 2006/07/20 09:21:20 jwagnerhki Exp $");

int
getDuration(
  const char *word,
  double *result
) {
  double temp;

  if (sscanf(word, "%lg", &temp) == 1) {
    /* Conversion successful. */
    /* xxx: For now, we'll just allow everything, taken to be seconds. */
    *result = temp;
    return 0;
  } else {
    /* Conversion failed, leave the result alone/unchanged. */
    return 1;
  }
}  /* getDuration */

int
getDateTime(
  const char *word,
  double *result
) {
  int yyyy,mm,dd,hh,min;
  double sec;
  struct tm tt;
  time_t temp;
  int success = 0;

  /* Convert UTC timestamp in 'yyyymmddhhmmss.sss' format
   * to Unix fractional seconds.  Interpret some of the complete
   * ISO 8601 basic and extended formats. */
  if (sscanf(word, "%4d-%2d-%2d %2d:%2d:%lg",  /* like ISO basic extended, */
             &yyyy,  /* but 'T' replaced with a blank */
             &mm,
             &dd,
             &hh,
             &min,
             &sec) == 6) {
    success = 1;
  } else if (sscanf(word, "%4d-%2d-%2dT%2d:%2d:%lg",  /* ISO basic extended */
             &yyyy,
             &mm,
             &dd,
             &hh,
             &min,
             &sec) == 6) {
    success = 1;
  } else if (sscanf(word, "%4d%2d%2dT%2d%2d%lg",  /* ISO basic complete */
             &yyyy,
             &mm,
             &dd,
             &hh,
             &min,
             &sec) == 6) {
    success = 1;
#if 0
  int centisec;
  /* This would be Metsahovi standard log file time stamp format,
   * difficult to distinguish between '0123' and '01.345'... */
  } else if (sscanf(word, "%4d%2d%2d%2d%2d%4d",  /* local "one string" /w csec */
             &yyyy,
             &mm,
             &dd,
             &hh,
             &min,
             &centisec) == 6) {
    sec = (double)centisec / 100.0;
    success = 1;
#endif
  } else if (sscanf(word, "%4d%2d%2d%2d%2d%lg",  /* local "one string" /w dec-s */
             &yyyy,
             &mm,
             &dd,
             &hh,
             &min,
             &sec) == 6) {
    success = 1;
  }
  if (success) {
    /* Conversion successful, convert to fractional Unix seconds. */
    tt.tm_sec = (int)floor(sec);
    tt.tm_min = min;
    tt.tm_hour = hh;
    tt.tm_mday = dd;
    tt.tm_mon = mm - 1;
    tt.tm_year = yyyy - 1900;
    tt.tm_isdst = 0;
    temp = mktime(&tt);
    *result = (double)temp + (sec - floor(sec));
    return 0;
  } else {
    /* Conversion failed, leave the result alone/unchanged. */
    return 1;
  }
}  /* getDateTime */

/* Timestamp for log files. */
char *formatTimestamp(struct timeval *atv, char *p)
{
  time_t temptime;
  struct tm *ti;
  static char bu[17];
  int centisec;

  if (p == NULL) {
    p = bu;
  }
  
  temptime = (time_t)atv->tv_sec;
  ti = gmtime(&(temptime));
  centisec = atv->tv_usec / 10000;
  if (centisec > 99) {
    centisec = 99;
  }
  assert( sprintf(p, "%04d%02d%02d%02d%02d%02d%02d",
          ti->tm_year + 1900,
          ti->tm_mon + 1,
          ti->tm_mday,
          ti->tm_hour,
          ti->tm_min,
          ti->tm_sec,
          centisec
                  /* int     tm_wday;        * day of the week */
                  /* int     tm_yday;        * day in the year */
                  /* int     tm_isdst;       * daylight saving time */
          ) <= ((int)sizeof(bu)-1) );
  return p;
}  /* formatTimestamp */

/* Current computer clock UTC timestamp for log files. */
char *getUTC(char *p)
{
  struct timeval temptv;
  
  assert( gettimeofday(&temptv, NULL) == 0 );
  return formatTimestamp(&temptv, p);
}  /* getUTC */

/* Any previous computer clock UTC Unix fractional seconds time to
 * timestamp for log files. */
char *toTimestamp(double t, char *p)
{
  struct timeval temptv;
  
  temptv.tv_sec = (long)floor(t);
  temptv.tv_usec = (long)(1000000.0 * (t - floor(t)));
  return formatTimestamp(&temptv, p);
}  /* toTimestamp */
