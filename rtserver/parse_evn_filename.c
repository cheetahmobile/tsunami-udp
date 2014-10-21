/*========================================================================
 * parse_evn_filename.c  --  EVN filename parser routines
 *
 * Accepts filenames not entirely EVN filename format compliant.
 *
 * Accepted formats:
 *   [experimentName]_[stationCode]_[scanName]_[startTimeUTCday].vsi
 *   [experimentName]_[stationCode]_[scanName]_[startTimeUTCday]_[auxinfo1..N=val1..N].vsi
 *
 * Auxinfo:
 *   These auxiliary infos are parsed outside of parse_evn_filename.c,
 *   right now only in protocol.c
 *
 *   Currently used auxinfos are, for example:
 *      sr[n]     - samplerate
 *      sl[n]     - total slots nr in time multiplexing (==how many servers)
 *      sn[n]     - slot nr in time multiplexing
 *      flen[n]   - length of data to send
 *      dl[n]     - -"- but more EVN Filenaming Convention (2-char IDs)
 *
 * Example filenames:
 *   gre53_ef_scan035_154d12h43m10s.vsi
 *   gre53_ef_scan035_154d12h43m10s_flen=14400000.vsi
 *   gre53_ef_scan035_2006-11-21T08:45:00_flen=14400000.vsi
 *   gre53_ef_scan035_2006065084500_fl=14400000.vsi
 *
 * Unit test:
 *   gcc parse_evn_filename.c -DUNIT_TEST -o parsetest -lm
 *
 *========================================================================
 */
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "parse_evn_filename.h"

char* substrdup(char* str, size_t maxlength) {
   // a replacement for strndup() missing on BSD, Mac OS X / Darwin, some UNIX
   char* s = malloc(sizeof(char)*(maxlength+1));
   if (!s) return NULL;
   strncpy(s, str, maxlength);
   s[maxlength] = '\0';
   return s;
}

void add_aux_entry(struct evn_filename *ef, char *auxentry) {
	char **p;
	ef->nr_auxinfo++;
	p = realloc(ef->auxinfo, sizeof(char*) * ef->nr_auxinfo);
	assert(p);
	ef->auxinfo = p;
	ef->auxinfo[ef->nr_auxinfo - 1] = auxentry;
}

	
time_t year_to_utc(int year) {
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));
	tm.tm_year = year - 1900;
    tm.tm_mday = 1;
	return (double)mktime(&tm);
}

int get_current_year() {
	time_t t;
	struct tm *tm;
	time(&t);
	tm = gmtime(&t);
	return tm->tm_year + 1900;
}

double day_to_utc(int day) { return (day-1)*24*60*60; }
double hour_to_utc(int hour) { return hour*60*60; }
double minute_to_utc(int minute) { return minute*60; }


/*
 *  interpret_as_utc() - Convert specified UTC date into seconds 
 *
 *  Input:   interprets time as UTC
 *  Returns: corresponding number of seconds as in time_t specifications.
 *
 *  If daycount=0, uses normal month & day style of date.
 *  If daycount!=0 this is the number of days since the beginning of
 *  the specified year. If year=0 then the current year is assumed.
 *
 */

double interpret_as_utc(int year, int daycount, int month, int day, int hour, int min, int sec) {
    time_t ret;
    struct tm tt;
    struct tm* p_tt2;
    time_t timet;

	memset(&tt, 0, sizeof(tt));

    if (daycount==0 && (day!=0 && month!=0)) {
       tt.tm_mday = day;        // 1..31
       tt.tm_mon  = month - 1;  // 0..11
    } else if(daycount!=0 && (day==0 && month==0)) {
       tt.tm_mday = 1;
       tt.tm_mon  = 0;          // see kludge 1, unfortunately no use of yday
    } else if(daycount==0 && day==0 && month==0) {
       time(&timet);
       p_tt2 = gmtime(&timet);  // default to current day and month
       tt.tm_mon  = p_tt2->tm_mon;
       tt.tm_mday = p_tt2->tm_mday;
       year = 0;
    }

    if (year==0) {              // default to current year
       year = get_current_year();
    }
    tt.tm_year = year - 1900;

    tt.tm_sec = sec;
    tt.tm_min = min;
    tt.tm_hour = hour;

    tt.tm_isdst = 0;

    ret = mktime(&tt);

    // kludge 1: stupid mktime() can't handle yday or wday
    if (daycount != 0) { 
       ret += (daycount-1) * 24L*60L*60L;
    }

    // kludge 2: mktime() assumes given time is in local timezone, can't specify own (!?)
	ret -= timezone; // 'timezone' is in seconds, a glibc static global, yuck

    printf("DEBUG: interpret_as_utc(%d, %d days, %d, %d, %dh, %dm, %ds) : fixed_mktime()=%f\n", 
           year, daycount, month, day, hour, min, sec, (double)ret);

    return (double)ret;
}



/*
 * parse_time() - Parse the specified filename string into a UTC seconds count
 *
 */

int parse_time(const char *str, double *retval) {
   int yyyy = 0, mm = 0, dd = 0, hh = 0, min = 0, yday = 0, sec = 0;
   double dsec = 0.0;
   float  fsec = 0.0;
   int consumed = 0;
   struct timeval tvnow;

   *retval = 0;
    
   /* ISO basic extended */
   if (sscanf(str, "%4d-%2d-%2dT%2d:%2d:%lg%n",
             &yyyy, &mm, &dd,
             &hh, &min, &dsec, 
             &consumed) == 6 && consumed == strlen(str)) {

        *retval  = interpret_as_utc(yyyy, 0, mm, dd, hh, min, floor(dsec));
        *retval += (dsec - floor(dsec));
        fprintf(stderr, "Detected time format: ISO basic extended\n");

   /* yyyydddhhmmss - 13 digit*/
   } else if (strlen(str) == 13 && sscanf(str, "%04d%03d%02d%02d%02d%n",
             &yyyy, &yday,
             &hh, &mm, &sec,
             &consumed) == 5 && consumed == strlen(str)) {

       *retval = interpret_as_utc(yyyy, yday, 0, 0, hh, mm, sec);
       fprintf(stderr, "Detected time format: yyyydddhhmmss\n");

   /* dddhhmmss - 9 digit */
   } else if (strlen(str) == 9 && sscanf(str, "%03d%02d%02d%02d%n",
             &yday,
             &hh, &mm, &sec,
             &consumed) == 4 && consumed == strlen(str)) {

       *retval = interpret_as_utc(0, yday, 0, 0, hh, mm, sec);
       fprintf(stderr, "Detected time format: dddhhmmss\n");

    /* [yyyy]y[d..]d */
    } else if (sscanf(str, "%4dy%dd%n",
		     &yyyy, &yday,
		     &consumed) == 2 && consumed == strlen(str)) {
        *retval = interpret_as_utc(yyyy, yday, 0, 0, 0, 0, 0);
        fprintf(stderr, "Detected time format: [yyyy]y[dd]d\n");

    /* [yyyy]y[d..]d[h..]h[m..]m[s..]s :   some .snp omit zero valued fields e.g. min or sec can be missing...! TODO */
    } else if (sscanf(str, "%dy%dd%dh%dm%ds%n",
             &yyyy, &yday,
             &hh, &mm, &sec, &consumed) == 5 && consumed == strlen(str)) {

        *retval = interpret_as_utc(yyyy, yday, 0, 0, hh, mm, sec);
        fprintf(stderr, "Detected time format: [yyyy]y[dd]d[hh]h[mm]m[ss]s\n");

    /* yyyyddd - 7 digit */
	} else if (strlen(str) == 7 && sscanf(str, "%04d%03d%n",
              &yyyy, &yday,
              &consumed) == 2 && consumed == strlen(str)) {

        *retval = interpret_as_utc(yyyy, yday, 0, 0, 0, 0, 0);
        fprintf(stderr, "Detected time format: yyyyddd\n");

    /*  [d]d[h]h[m]m[s] */
	} else if (sscanf(str, "%dd%dh%dm%ds%n",
              &yday, 
              &hh, &mm, &sec,
              &consumed) == 4 && consumed == strlen(str)) {

        *retval  = interpret_as_utc(0, yday, 0, 0, hh, mm, sec);
        fprintf(stderr, "Detected time format: [d]d[h]h[m]m[s]s\n");

    /* hhmmss.cc - 6 digit plus decimals */
    } else if (strlen(str)>=6 && sscanf(str, "%02d%02d%02f%n",  // %02f parse does not quite detect decimal dot '.' yet
              &hh, &mm, &fsec,
              &consumed) == 3 && consumed == strlen(str)) {

        *retval = interpret_as_utc(0, 0, 0, 0, hh, mm, floor(fsec));
        *retval += (fsec - floor(fsec));
        fprintf(stderr, "Detected time format: hhmmss.cc\n");
   
	} else {
        // parse failed
        fprintf(stderr, "Warning: string with unknown time format passed to parse_time(), assuming it is aux entries.\n");
        return 1;
    }
    
    if (gettimeofday(&tvnow, NULL)==0 && difftime(*retval, tvnow.tv_sec)<=0) {
        fprintf(stderr, "Start time in the past\n");
        *retval = 0.0;
    }    
	return 0;
}


char *get_aux_entry(char *key, char **auxinfo, int nr_auxinfo) {
	int i;
	for (i=0; i<nr_auxinfo; i++) {
		
		if(strlen(auxinfo[i]) > strlen(key)
		   &&!strncmp(auxinfo[i], key, strlen(key))
		   && auxinfo[i][strlen(key)] == '=') {
			return strdup(auxinfo[i]+strlen(key)+1);
		}
	}
	return NULL;
}
			
/*
 * Return one token from underscore delimited string.
 * @param pointer to pointer to part of the string that is currently
 * parsed. Function updates the pointer so that it points to the next
 * element.
 * @return element allocated with malloc(), caller must free() it.
 */
char *get_token(char **str) {
	char *p, *retval;
	if (!*str || !*str[0])
		return NULL;
	p = strchr(*str, (int)'_');
	if (!p) {
		p = strchr(*str, (int)'\0');
		if(!p) return NULL; // assert(p);
		retval = strdup(*str);
		*str = p;
	} else {
		retval = (char*)substrdup(*str, p - *str);
		*str = p + 1;
	}
	return retval;
}
/* Parse EVN filename
 * @param pointer to filename
 * @return malloc()'d struct that contains malloc()'d filename elements.
 */
struct evn_filename *parse_evn_filename(char *filename) {
	struct evn_filename *ef;
	char *parsebuf, *parseptr;
	
	ef = calloc(sizeof(struct evn_filename), 1);
	assert(ef);

	parseptr = parsebuf = strdup(filename);
	assert(parsebuf);

    ef->data_start_time_ascii = NULL;
    ef->valid = 1;
    
	/* Extract filetype from parsebuf. Overwrite dot with zero so
	   that filetype does not complicate parsing of other parts.*/
	{
		char *dot, *filetype;
		dot = strrchr(parseptr, (int)'.');
		if (!dot) { fprintf(stderr, "parse_evn_filename: assert(dot)\n"); ef->valid = 0; return ef; }
		filetype = dot + 1;
		ef->file_type = get_token(&filetype);
		if(!ef->file_type) { fprintf(stderr, "parse_evn_filename: assert(ef->file_type)\n");  ef->valid = 0; return ef; } 
		if(strlen(ef->file_type) < 2) { fprintf(stderr, "parse_evn_filename: assert(strlen(ef->file_type)>=2)\n");  ef->valid = 0; return ef; } 
		*dot = 0;
	}

	ef->exp_name = get_token(&parseptr);
    if(!ef->exp_name) { fprintf(stderr, "parse_evn_filename: assert(ef->exp_name)\n");  ef->valid = 0; return ef; }
	if(strlen(ef->exp_name) > 6) { fprintf(stderr, "parse_evn_filename: assert(strlen(ef->exp_name) <= 6)\n");  ef->valid = 0; return ef; }

	ef->station_code = get_token(&parseptr);
    if(!ef->station_code) { fprintf(stderr, "parse_evn_filename: assert(ef->station_code)\n");  ef->valid = 0; return ef; }
    if(strlen(ef->station_code) < 2) { fprintf(stderr, "parse_evn_filename: assert(strlen(ef->station_code) >= 2)\n");  ef->valid = 0; return ef; }

	ef->scan_name = get_token(&parseptr);
    if(!ef->scan_name) { fprintf(stderr, "parse_evn_filename: assert(ef->scan_name)\n");  ef->valid = 0; return ef; }
    if(strlen(ef->scan_name) > 16) { fprintf(stderr, "parse_evn_filename: assert(strlen(ef->scan_name) <= 16)\n"); ef->valid = 0; return ef; }

	/* All mandatory elements read. */

	ef->data_start_time_ascii = get_token(&parseptr);
	if (ef->data_start_time_ascii) {
        if(strlen(ef->data_start_time_ascii) < 2) { 
            ef->data_start_time_ascii=NULL;  ef->valid = 0;
            fprintf(stderr, "parse_evn_filename: assert(strlen(ef->data_start_time_ascii) >= 2)\n");            
            return ef; 
        } 
        if (parse_time(ef->data_start_time_ascii, &ef->data_start_time)) {
            /* Does not look like date, must be auxentry instead. */
            add_aux_entry(ef, ef->data_start_time_ascii);
            ef->data_start_time_ascii = NULL;
		}
	}

	{
		char *auxentry;
		while ((auxentry = get_token(&parseptr)) != NULL)
			add_aux_entry(ef, auxentry);
	}

	free(parsebuf);
	return ef;
}

#ifdef UNIT_TEST
int main(int argc, char *argv[]) {
	struct evn_filename *ef;
	int i, cnt;
    u_int64_t li;
    char **filenames;
    int    filenamecount;

    if (argc < 2) {
      // filenames = precoded;
      // filenamecount = PRECODELEN - 1;

      printf("Cut&paste to try:                                      Expected:\n");
      printf("-----------------------------------------------------------------\n");
      printf("gre53_Ef_scan035_154d12h43m10s.vsi                     1180874590 (in 2007)\n");
      printf("gre53_Ef_scan035_154124310.vsi                         1180874590 (in 2007)\n");
      printf("  date -u --date \"01/01/2007 12:43:10 + 153 days\" +%%s\n");
      printf("R1262_On_037-1240b_2007037124050_flen=5408000000.evn   1170765650\n");
      printf("  date -u --date \"01/01/2007 12:40:50 + 36 days\" +%%s\n");
      printf("R1262_On_037-1240b_2007037_dl=5408000000.vsi           1170720000\n");
      printf("R1262_On_037-1240b_2007y037d_dl=5408000000.vsi         1170720000\n");
      printf("  date -u --date \"01/01/2007 + 36 days 0:0:0\" +%%s\n");
      printf("R1262_On_037-1240b_2007y037d12h6m1s_dl=5408000000.vsi  1170763561\n");
      printf("  date -u --date \"01/01/2007 + 36 days 12:06:01\" +%%s\n");
      printf("gre53_Ef_scan035_2006-11-21T08:45:00_dl=14400000.vsi   1164098700\n");
      printf("  date -u --date \"11/21/2006 08:45:00\" +%%s\n");
      printf("dummy_Mh_scan01_124500.00_dl=1500.vsi\n");
      printf("\n");

      return 0;
    } else {
      filenames = argv;
      filenamecount = argc - 1;
    }
    cnt = 1;
    while (cnt<=filenamecount) {
       printf("----- Parsing name: %s -----\n\n", filenames[cnt]);
       ef = parse_evn_filename(filenames[cnt]);
       if (ef->valid) { printf("Name seems to be valid\n"); } 
       else { printf("Name is invalid\n"); }
	   printf("ef->exp_name = %s\t", ef->exp_name);
	   printf("ef->station_code = %s\t", ef->station_code);
	   printf("ef->scan_name = %s\t", ef->scan_name);
	   printf("ef->file_type = %s\n", ef->file_type);       
	   printf("ef->data_start_time_ascii = %s\t", ef->data_start_time_ascii);
	   printf("ef->data_start_time = %f\n", ef->data_start_time);
	   for (i=0; i<ef->nr_auxinfo; i++)
		  printf("ef->auxinfo[%d] = %s\n", i, ef->auxinfo[i]);
       if (get_aux_entry("flen", ef->auxinfo, ef->nr_auxinfo) != 0) {
            sscanf(get_aux_entry("flen", ef->auxinfo, ef->nr_auxinfo), "%lld", &li);
            printf("  parsed flen param: %lld\n", li);
       } 
       if (get_aux_entry("dl", ef->auxinfo, ef->nr_auxinfo) != 0) {
            sscanf(get_aux_entry("dl", ef->auxinfo, ef->nr_auxinfo), "%lld", &li);
            printf("  parsed dl param: %lld\n", li);
       } 
       printf("\n");
       cnt++;
    }
	return 0;
}
#endif

/*
 * $Log: parse_evn_filename.c,v $
 * Revision 1.11  2008/05/22 17:47:18  jwagnerhki
 * strndup() replacement for Mac OS, BSD
 *
 * Revision 1.10  2007/02/26 12:39:39  jwagnerhki
 * flen and dl printout fix, long long int
 *
 * Revision 1.9  2007/02/13 13:47:31  jwagnerhki
 * UTC parse fixes and extensions, dl in addition to flen for realtime bytelength
 *
 * Revision 1.8  2007/02/12 13:41:21  jwagnerhki
 * mktime() post-fix now parsing really to utc, added 'dl' same as 'flen', added some EVN formats, added parse validity flag
 *
 * Revision 1.7  2006/11/21 09:22:33  jwagnerhki
 * past time catching
 *
 * Revision 1.6  2006/11/21 08:08:06  jwagnerhki
 * added ISO basic ext time format parse
 *
 * Revision 1.5  2006/11/21 07:24:30  jwagnerhki
 * auxinfo values set with equal sign
 *
 * Revision 1.4  2006/11/20 14:32:02  jwagnerhki
 * documented the format slightly better, unittest accepts several cmdline filenames
 *
 * Revision 1.3  2006/10/25 15:25:17  jwagnerhki
 * removed heaps of asserts from copied code
 *
 * Revision 1.2  2006/10/25 12:14:06  jwagnerhki
 * added cvs log line
 *
 */


