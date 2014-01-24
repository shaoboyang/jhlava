/* $Id: lsload.c 397 2007-11-26 19:04:00Z mblack $
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "../lsf.h"
#include "../lib/lib.table.h"
#include "../lib/lproto.h"
#include "../intlib/intlibout.h"

#define MAXLISTSIZE 256

extern int makeFields(struct hostLoad *, char *loadval[], char **);
extern int makewideFields(struct hostLoad *, char *loadval[], char **);
extern char *formatHeader(char **, char);
extern char *wideformatHeader(char **, char);
extern char **filterToNames(char *);
extern int num_loadindex;


#define NL_SETN  27


void
usage(char *cmd)
{
    fprintf(stderr,I18N_Usage );
    fprintf(stderr,
       ":\n%s [-h] [-V] [-N|-E] [-l | -w] [-R res_req] [-I index_list] [-n num_hosts] [host_name ...]\n", cmd);
    fprintf(stderr, "%s\n%s [-h] [-V] -s [ shared_resource_name ... ]\n", I18N_or, cmd);
    exit(-1);
}

int
main(int argc, char **argv)
{
    static char fname[] = "lsload:main";
    int i,j, num, numneeded;
    char *resreq = NULL;
    struct hostLoad *hosts;
    char *hostnames[MAXLISTSIZE];
    char statusbuf[20];
    int options = 0;
    static char **loadval;
    char *indexfilter = NULL;
    char **nlp;
    static char *defaultindex[] = {"r15s", "r1m", "r15m", "ut", "pg", "ls",
                                   "it", "tmp", "swp", "mem", NULL};
    int	achar;
    char longFormat = FALSE;
    char wideFormat = FALSE;
    char badHost = FALSE, sOption = FALSE, otherOption = FALSE;
    int extView = FALSE;
    char **shareNames, **shareValues, **formats;
    int isClus, retVal = 0;
    int rc;

    num = 0;
    numneeded = 0;
    opterr = 0;

    rc = _i18n_init ( I18N_CAT_MIN );


    if (ls_initdebug(argv[0]) < 0) {
        ls_perror("ls_initdebug");
        exit(-1);
    }
    if (logclass & (LC_TRACE))
        ls_syslog(LOG_DEBUG, "%s: Entering this routine...", fname);


    for (i = 1; i < argc; i++) {
       if (strcmp(argv[i], "-h") == 0) {
           usage(argv[0]);
           exit (0);
       } else if (strcmp(argv[i], "-V") == 0) {
           fputs(_LS_VERSION_, stderr);
           exit(0);
       } else if (strcmp(argv[i], "-s") == 0) {
           if (otherOption == TRUE) {
               usage(argv[0]);
               exit(-1);
           }
           sOption = TRUE;
           optind = i + 1;
       } else if (strcmp(argv[i], "-e") == 0) {
           if (otherOption == TRUE || sOption == FALSE) {
               usage(argv[0]);
               exit(-1);
           }
           extView = TRUE;
           optind = i + 1;
       } else if (strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "-l") == 0
                  || strcmp(argv[i], "-I") == 0 || strcmp(argv[i], "-N") == 0
                  || strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "-n") == 0
		  || strcmp(argv[i], "-w") == 0 ) {
            otherOption = TRUE;
            if (sOption == TRUE) {
                usage(argv[0]);
                exit(-1);
            }
        }
    }

    if (sOption == TRUE) {
        displayShareResource(argc, argv, optind, FALSE, extView);
        return(0);
    }

    while ((achar = getopt(argc, argv, "R:I:NEln:w")) != EOF)
    {
	switch (achar)
	{
	case 'R':
            resreq = optarg;
            break;
	case 'I':
            indexfilter = optarg;
            break;

	case 'N':
	    if (options & EFFECTIVE)
		usage(argv[0]);
            options = NORMALIZE;
	    break;

        case 'E':
	    if (options & NORMALIZE)
		usage(argv[0]);
             options = EFFECTIVE;
	     break;
	case 'n':
            numneeded = atoi(optarg);
            if (numneeded <= 0)
                usage(argv[0]);
            break;

        case 'l':
	    longFormat = TRUE;
	    if (wideFormat == TRUE)
		usage(argv[0]);
	    break;

	case 'w':
	    wideFormat = TRUE;
	    if (longFormat == TRUE)
		usage(argv[0]);
	    break;

	case 'V':
	    fputs(_LS_VERSION_, stderr);
	    exit(0);

	case 'h':
	default:
            usage(argv[0]);

        }
    }
    for ( ; optind < argc ; optind++)
    {
        if (num>=MAXLISTSIZE) {
            fprintf(stderr, _i18n_msg_get(ls_catd,NL_SETN,1951, "too many hosts specified (maximum %d)\n"), /* catgets  1951  */
			MAXLISTSIZE);
            exit(-1);
        }
        if ( (isClus = ls_isclustername(argv[optind])) < 0 ) {
	    fprintf(stderr, "lsload: %s\n", ls_sysmsg());
            badHost = TRUE;
	    continue;
	} else if ( (isClus == 0) &&
		    (!Gethostbyname_(argv[optind])) ) {
	    fprintf(stderr, "\
%s: invalid hostname %s\n", __func__, argv[optind]);
            badHost = TRUE;
            continue;
	}
        hostnames[num] = argv[optind];
        num++;
    }

    if (num == 0 && badHost)
        exit(-1);

    if (!longFormat) {
        if (indexfilter)
            nlp = filterToNames(indexfilter);
        else
            nlp = defaultindex;
    } else {
        nlp = NULL;
    }

    TIMEIT(0, (hosts = ls_loadinfo(resreq, &numneeded, options, 0, hostnames, num, &nlp)), "ls_loadinfo");

    if (!hosts) {
	ls_perror("lsload");
        exit(-10);
    }

    if (longFormat)
        printf("%s", formatHeader(nlp, longFormat));
    else
	if (wideFormat)
	    printf("%s\n", wideformatHeader(nlp, longFormat));
        else
            printf("%s\n", formatHeader(nlp, longFormat));

    if (!(loadval=(char **)malloc(num_loadindex*sizeof(char *)))) {
        lserrno=LSE_MALLOC;
        ls_perror("lsload");
        exit(-1);
    }
    for (i=0;i < numneeded; i++) {
	if (LS_ISUNAVAIL(hosts[i].status))
	    strcpy(statusbuf, I18N_unavail);
	else
	{
	    statusbuf[0] = '\0';
	    if (LS_ISRESDOWN(hosts[i].status))
		strcat(statusbuf, "-");
	    if (LS_ISOKNRES(hosts[i].status)) {
		strcat(statusbuf, I18N_ok);
	    } else if (LS_ISBUSY(hosts[i].status)
			&& !LS_ISLOCKED(hosts[i].status)) {
		    strcat(statusbuf, I18N(1958, "busy")); /* catgets 1958 */
 	    } else {
	        strcat(statusbuf, I18N(1955, "lock")); /* catgets 1955*/
	        if (LS_ISLOCKEDU(hosts[i].status)) {
	            strcat(statusbuf, I18N(1956, "U")); /* catgets 1956*/
	        }
	        if (LS_ISLOCKEDW(hosts[i].status)) {
		    strcat(statusbuf, I18N(1957, "W")); /* catgets 1957*/
	        }
	        if (LS_ISLOCKEDM(hosts[i].status)) {
		    strcat(statusbuf, I18N(1959, "M")); /* catgets 1959*/
	        }
	    }
	}

        if (longFormat) {
            retVal = makeShareField(hosts[i].hostName, FALSE, &shareNames,
                                    &shareValues, &formats);
	    if (i == 0) {

		if (retVal > 0) {

		    for (j = 0; j < retVal; j++) {
			printf(formats[j], shareNames[j]);
		    }
		}
		putchar('\n');
	    }
	    printf("%-23s %6s", hosts[i].hostName, statusbuf);
        } else
	    printf("%-15.15s %6s", hosts[i].hostName, statusbuf);

	if (!LS_ISUNAVAIL(hosts[i].status)) {
            int nf;
	    if (wideFormat){
		nf = makewideFields(&hosts[i], loadval, nlp);
	    }
	    else
	    nf = makeFields(&hosts[i], loadval, nlp);
            for(j = 0; j < nf; j++)
                printf("%s",loadval[j]);
            if (retVal > 0) {
                for (j = 0; j < retVal; j++) {
                    printf(formats[j], shareValues[j]);
                }
            }
	}
	putchar('\n');
    }

    _i18n_end ( ls_catd );

    exit (0);
}
