/* $Id: lshosts.c 397 2007-11-26 19:04:00Z mblack $
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <netdb.h>
#include "../lsf.h"

#include "../lib/lproto.h"
#include "../intlib/intlibout.h"

#include <math.h>

#define NL_SETN 27


static void usage(char *);
static void print_long(struct hostInfo *hostInfo);
static char *stripSpaces(char *);


struct indexFmt {
    char *name;
    char *hdr;
    char *busy;
    char *ok;
    float scale;
};


struct indexFmt fmt1[] = {
{ "r15s", "%6s",  "*%4.1f",   "%5.1f",      1.0 },
{ "r1m",  "%6s",  "*%4.1f",   "%5.1f",      1.0 },
{ "r15m", "%6s",  "*%4.1f",   "%5.1f",      1.0 },
{ "ut",   "%5s", "*%3.0f%%", "%3.0f%%",  100.0},
{ "pg",   "%6s", "*%4.1f",   "%4.1f",     1.0},
{ "io",   "%6s", "*%4.0f",   "%4.0f",     1.0},
{ "ls",   "%5s", "*%2.0f",   "%2.0f",     1.0},
{ "it",   "%5s", "*%3.0f",   "%4.0f" ,     1.0},
{ "tmp",  "%6s", "*%4.0fM",  "%4.0fM",     1.0},
{ "swp",  "%6s", "*%3.0fM",  "%4.0fM",     1.0},
{ "mem",  "%6s", "*%3.0fM",  "%4.0fM",     1.0},
{ "dflt", "%7s", "*%6.1f"  , "%6.1f",      1.0 },
{  NULL,  "%7s", "*%6.1f"  , " %6.1f",      1.0 }
}, *fmt;

static char *
stripSpaces(char *field)
{
    char *cp;
    int len, i;

    cp = field;
    while (*cp == ' ')
        cp++;

    len = strlen(field);
    i = len - 1;
    while((i > 0) && (field[i] == ' '))
        i--;
    if (i < len-1)
        field[i] = '\0';
    return(cp);
}

static void
usage(char *cmd)
{
    fprintf(stderr, "%s: %s [-h] [-V] [-w | -l] [-R res_req] [host_name ...]\n", I18N_Usage, cmd);
    fprintf(stderr, "%s\n %s [-h] [-V] -s [static_resouce_name ...]\n", I18N_or, cmd);
}

static void
print_long(struct hostInfo *hostInfo)
{
    int i;
    float *li;
    char  *sp;
    static char first = TRUE;
    static char line[132];
    static char newFmt[10];
    int newIndexLen, retVal;
    char **indxnames;
    char **shareNames, **shareValues, **formats;
    char strbuf1[30],strbuf2[30],strbuf3[30];


    if (first) {
        char tmpbuf[MAXLSFNAMELEN];
        int  fmtid;


        if(!(fmt=(struct indexFmt *)
            malloc((hostInfo->numIndx+2)*sizeof (struct indexFmt)))) {
            lserrno=LSE_MALLOC;
            ls_perror("print_long");
            exit(-1);
        }
        for (i=0; i<NBUILTINDEX+2; i++)
            fmt[i]=fmt1[i];

        TIMEIT(0, (indxnames = ls_indexnames(NULL)), "ls_indexnames");
        if (indxnames == NULL) {
            ls_perror("ls_indexnames");
            exit(-1);
        }
        for(i=0; indxnames[i]; i++) {
            if (i > MEM)
                fmtid = MEM + 1;
            else
                fmtid = i;

            if ((fmtid == MEM +1) && (newIndexLen = strlen(indxnames[i])) >= 7) {
	        sprintf(newFmt, "%s%d%s", "%", newIndexLen+1, "s");
		sprintf(tmpbuf, newFmt, indxnames[i]);
	    }
            else
                sprintf(tmpbuf, fmt[fmtid].hdr, indxnames[i]);
            strcat(line, tmpbuf);
        }
        first = FALSE;
    }

    printf("\n%s:  %s\n",
	_i18n_msg_get(ls_catd,NL_SETN, 1601, "HOST_NAME"), /* catgets 1601 */
	hostInfo->hostName);
    {
        char *buf1, *buf2, *buf3, *buf4, *buf5, *buf6, *buf7, *buf8, *buf9, *buf10, *buf11, *buf12, *buf13;

	buf1 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1602, "type")); /* catgets 1602 */
	buf2 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1603, "model")); /* catgets 1603 */
	buf3 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1604, "cpuf")); /* catgets 1604 */
	buf4 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1605, "ncpus")); /* catgets 1605 */
	buf5 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1606, "ndisks")); /* catgets 1606 */
	buf6 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1607, "maxmem")); /* catgets 1607 */
	buf7 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1608, "maxswp")); /* catgets 1608 */
	buf8 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1609, "maxtmp")); /* catgets 1609 */
	buf9 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1610, "rexpri")); /* catgets 1610 */
	buf10= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "server")); /* catgets 1611 */
	buf11= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "nsocket"));
	buf12= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "ncore"));
	buf13= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "nthread"));

    	printf("%-10.10s %11.11s %5.5s %5.5s %6.6s %6.6s %6.6s %6.6s %6.6s %6.6s %7.7s %5.5s %7.7s\n",
               buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9, buf10, buf11, buf12, buf13);

	FREEUP(buf1);
	FREEUP(buf2);
	FREEUP(buf3);
	FREEUP(buf4);
	FREEUP(buf5);
	FREEUP(buf6);
	FREEUP(buf7);
	FREEUP(buf8);
	FREEUP(buf9);
	FREEUP(buf10);
	FREEUP(buf11);
	FREEUP(buf12);
	FREEUP(buf13);

    }
    if (hostInfo->isServer) {
        sprintf(strbuf1,"%-10s",hostInfo->hostType);strbuf1[10]='\0';
        sprintf(strbuf2,"%11s",hostInfo->hostModel);strbuf2[11]='\0';
        sprintf(strbuf3,"%5.1f",hostInfo->cpuFactor);strbuf3[5]='\0';
        printf("%-10s %11s %5s ",strbuf1,strbuf2,strbuf3);
        if (hostInfo->maxCpus > 0)
            if (hostInfo->tp.threadnum > 0)
                printf("%5d %6d %5dM %5dM %5dM %6d %6s %7d %5d %7d\n",
                       hostInfo->maxCpus, hostInfo->nDisks, hostInfo->maxMem,
                       hostInfo->maxSwap, hostInfo->maxTmp,
                       hostInfo->rexPriority, I18N_Yes,
                       hostInfo->tp.socketnum,
                       hostInfo->tp.corenum,
                       hostInfo->tp.threadnum);
            else
                printf("%5d %6d %5dM %5dM %5dM %6d %6s %7s %5s %7s\n",
                       hostInfo->maxCpus, hostInfo->nDisks, hostInfo->maxMem,
                       hostInfo->maxSwap, hostInfo->maxTmp,
                       hostInfo->rexPriority, I18N_Yes,
                       "-", "-", "-");

        else
            printf("%5s %6s %6s %6s %6s %6d %6s %7s %5s %7s\n",
                   "-", "-", "-", "-", "-", hostInfo->rexPriority,
                   I18N_Yes, "-", "-", "-"); /* catgets 1612  */
    } else {
        sprintf(strbuf1,"%-10s",hostInfo->hostType);strbuf1[10]='\0';
        sprintf(strbuf2,"%11s",hostInfo->hostModel);strbuf2[11]='\0';
        sprintf(strbuf3,"%5.1f",hostInfo->cpuFactor);strbuf3[5]='\0';
        printf("%-10s %11s %5s ",strbuf1,strbuf2,strbuf3);
        printf("%5s %6s %6s %6s %6s %6s %6s %7s %5s %7s\n",
               "-", "-", "-", "-", "-", "-",
               I18N_No, "-", "-", "-"); /* catgets 1613 */
    }


    if (sharedResConfigured_ == TRUE) {
        if ((retVal = makeShareField(hostInfo->hostName, TRUE, &shareNames,
            &shareValues, &formats)) > 0) {


            for (i = 0; i < retVal; i++) {
                printf(formats[i], shareNames[i]);
            }
            printf("\n");
            for (i = 0; i < retVal; i++) {
                printf(formats[i], shareValues[i]);
            }

            printf("\n");
        }
    }

    printf("\n");
    printf("%s: ",
	_i18n_msg_get(ls_catd,NL_SETN,1614, "RESOURCES")); /* catgets 1614 */
    if (hostInfo->nRes) {
        int first = TRUE;
	for (i=0; i < hostInfo->nRes; i++) {
            if (! first)
               printf(" ");
            else
               printf("(");
	    printf("%s", hostInfo->resources[i]);
            first = FALSE;
        }
        printf(")\n");
    } else {
        printf("%s\n",
	    _i18n_msg_get(ls_catd,NL_SETN,1615, "Not defined")); /* catgets 1615 */
    }

    printf("%s: ",
	_i18n_msg_get(ls_catd,NL_SETN,1616, "RUN_WINDOWS")); /* catgets 1616  */
    if (hostInfo->isServer) {
	if (strcmp(hostInfo->windows, "-") == 0)
	    fputs(
		_i18n_msg_get(ls_catd,NL_SETN,1617, " (always open)\n"), /* catgets 1617 */
		stdout);
	else
	    printf("%s\n", hostInfo->windows);
    } else {
	printf(_i18n_msg_get(ls_catd,NL_SETN,1618, "Not applicable for client-only host\n")); /* catgets 1618 */
    }

    if (! hostInfo->isServer) {
        printf("\n");
	return;
    }


    printf("\n");
    printf(_i18n_msg_get(ls_catd,NL_SETN,1626, "LOAD_THRESHOLDS:")); /* catgets 1626 */
    printf("\n%s\n",line);
    li = hostInfo->busyThreshold;
    for(i=0; indxnames[i]; i++) {
        char tmpfield[MAXLSFNAMELEN];
        int id;

        if (i > MEM)
            id = MEM + 1;
        else
            id = i;
        if (fabs(li[i]) >= (double) INFINIT_LOAD)
            sp = "-";
        else {
            sprintf(tmpfield, fmt[id].ok,  li[i] * fmt[id].scale);
            sp = stripSpaces(tmpfield);
        }
	if ((id == MEM + 1) && (newIndexLen = strlen (indxnames[i])) >= 7 ){
	    sprintf(newFmt, "%s%d%s", "%", newIndexLen+1, "s");
            printf(newFmt, sp);
        }
	else
            printf(fmt[id].hdr, sp);
    }

    printf("\n");
}

int
main(int argc, char **argv)
{
    static char fname[] = "lshosts/main";
    char   *namebufs[256];
    struct hostInfo *hostinfo;
    int    numhosts = 0;
    int    i, j;
    char   *resReq = NULL;
    char   longformat = FALSE;
    char   longname = FALSE;
    char   staticResource = FALSE, otherOption = FALSE;
    int extView = FALSE;
    int achar;
    int     unknown;
    int     options=0;
    int isClus;
    int rc;


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
            staticResource = TRUE;
            optind = i + 1;
        } else if (strcmp(argv[i], "-e") == 0) {
            if (otherOption == TRUE || staticResource == FALSE) {
                usage(argv[0]);
                exit(-1);
            }
            optind = i + 1;
            extView = TRUE;
        } else if (strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "-l") == 0
                  || strcmp(argv[i], "-w") == 0) {
            otherOption = TRUE;
            if (staticResource == TRUE) {
                usage(argv[0]);
                exit(-1);
            }
        }
    }

    if (staticResource == TRUE) {
        displayShareResource(argc, argv, optind, TRUE, extView );
    } else {
        while ((achar = getopt(argc, argv, "R:lw")) != EOF) {
 	    switch (achar) {
            case 'R':
	        if (strlen(optarg) > MAXLINELEN) {
                        printf(" %s", I18N(1645, "The resource requirement string exceeds the maximum length of 512 characters. Specify a shorter resource requirement.\n")); /* catgets  1645  */
		        exit (-1);
	        }
	        resReq = optarg;
	        break;
            case 'l':
	        longformat = TRUE;
	        break;
            case 'w':
	        longname = TRUE;
	        break;
            default:
	        usage(argv[0]);
	        exit(-1);
	    }
        }

        i=0;
        unknown = 0;
        for ( ; optind < argc ; optind++) {
    	    if (strcmp(argv[optind],"allclusters") == 0) {
	        options = ALL_CLUSTERS;
	        i = 0;
                break;
            }
            if ( (isClus = ls_isclustername(argv[optind])) < 0 ) {
	        fprintf(stderr, "lshosts: %s\n", ls_sysmsg());
                unknown = 1;
                continue;
            } else if ((isClus == 0) &&
                       (!(Gethostbyname_(argv[optind])))){
                fprintf(stderr, "\
%s: gethostbyname() failed for host %s.\n", __func__, argv[optind]);
                unknown = 1;
                continue;
            }
            namebufs[i] = argv[optind];
            if (namebufs[i] == NULL) {
                perror("strdup()");
                exit(-1);
            }
            i++;
        }

        if (i == 0 && unknown == 1)
            exit(-1);

        if (i == 0) {
            TIMEIT(0, (hostinfo = ls_gethostinfo(resReq, &numhosts, NULL, 0,
                                                 options)), "ls_gethostinfo");
            if (hostinfo == NULL) {
                ls_perror("ls_gethostinfo()");
                exit(-1);
            }
        } else {
    	    TIMEIT(0, (hostinfo = ls_gethostinfo(resReq, &numhosts, namebufs,
                                                 i, 0)), "ls_gethostinfo");
	    if (hostinfo == NULL) {
	        ls_perror("ls_gethostinfo");
	        exit(-1);
	    }
        }

        if (!longformat && !longname) {
	    char *buf1, *buf2, *buf3, *buf4, *buf5;
	    char *buf6, *buf7, *buf8, *buf9, *buf10, *buf11, *buf12;

	    buf1 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1628, "HOST_NAME")); /* catgets 1628 */
	    buf2 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1602, "type")); /* catgets  1602  */
	    buf3 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1603, "model")); /* catgets  1603  */
	    buf4 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1604, "cpuf")); /* catgets  1604 */
	    buf5 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1605, "ncpus")); /* catgets  1605  */
	    buf6 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1607, "maxmem")); /* catgets  1607  */
	    buf7 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1608, "maxswp")); /* catgets  1608  */
	    buf8 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "server")); /* catgets  1611  */
	    buf9 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1614, "RESOURCES")); /* catgets  1614  */
        buf10= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "nsocket")); /* catgets 1611 */
        buf11= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "ncore")); /* catgets 1611 */
        buf12= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "nthread")); /* catgets 1611 */
            printf("%-11.11s %7.7s %8.8s %5.5s %5.5s %6.6s %6.6s %6.6s %7.7s %5.5s %7.7s %9.9s\n",
                   buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf10, buf11, buf12, buf9);

	    FREEUP(buf1);
	    FREEUP(buf2);
	    FREEUP(buf3);
	    FREEUP(buf4);
	    FREEUP(buf5);
	    FREEUP(buf6);
	    FREEUP(buf7);
	    FREEUP(buf8);
	    FREEUP(buf9);
	    FREEUP(buf10);
	    FREEUP(buf11);
	    FREEUP(buf12);
        } else if (longname) {
	    char *buf1, *buf2, *buf3, *buf4, *buf5;
	    char *buf6, *buf7, *buf8, *buf9, *buf10, *buf11, *buf12;

	    buf1 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1628, "HOST_NAME")); /* catgets  1628 */
	    buf2 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1602, "type")); /* catgets  1602  */
	    buf3 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1603, "model")); /* catgets  1603  */
	    buf4 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1604, "cpuf")); /* catgets  1604  */
	    buf5 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1605, "ncpus")); /* catgets  1605  */
	    buf6 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1607, "maxmem")); /* catgets  1607  */
	    buf7 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1608, "maxswp")); /* catgets  1608  */
	    buf8 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "server")); /* catgets  1611  */
	    buf9 = putstr_(_i18n_msg_get(ls_catd,NL_SETN,1614, "RESOURCES")); /* catgets  1614  */
        buf10= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "nsocket")); /* catgets 1611 */
        buf11= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "ncore")); /* catgets 1611 */
        buf12= putstr_(_i18n_msg_get(ls_catd,NL_SETN,1611, "nthread")); /* catgets 1611 */
            printf("%-25.25s %10.10s %11.11s %5.5s %5.5s %6.6s %6.6s %6.6s %7.7s %5.5s %7.7s %9.9s\n",
                   buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf10, buf11, buf12, buf9);

	    FREEUP(buf1);
	    FREEUP(buf2);
	    FREEUP(buf3);
	    FREEUP(buf4);
	    FREEUP(buf5);
	    FREEUP(buf6);
	    FREEUP(buf7);
	    FREEUP(buf8);
	    FREEUP(buf9);
	    FREEUP(buf10);
	    FREEUP(buf11);
	    FREEUP(buf12);
        }

        for (i=0;i<numhosts;i++) {
            char *server;
            int first;

            if (longformat) {
                print_long(&hostinfo[i]);
                continue;
            }

            if (hostinfo[i].isServer)
                server = I18N_Yes;
            else
                server = I18N_No;


    	    if(longname)
	        printf("%-25s %10s %11s %5.1f ", hostinfo[i].hostName,
	               hostinfo[i].hostType, hostinfo[i].hostModel,
                       hostinfo[i].cpuFactor);
            else
	        printf("%-11.11s %7.7s %8.8s %5.1f ", hostinfo[i].hostName,
	               hostinfo[i].hostType, hostinfo[i].hostModel,
                       hostinfo[i].cpuFactor);

	    if (hostinfo[i].maxCpus > 0)
	        printf("%5d",hostinfo[i].maxCpus);
	    else
                printf("%5.5s", "-");

	    if (hostinfo[i].maxMem > 0)
	        printf(" %5dM",hostinfo[i].maxMem);
	    else
	        printf(" %6.6s", "-");

        if (hostinfo[i].maxSwap > 0)
	        printf(" %5dM",hostinfo[i].maxSwap);
        else
	        printf(" %6.6s", "-");

        printf(" %6.6s", server);

        if(hostinfo[i].tp.socketnum > 0)
            printf(" %7d", hostinfo[i].tp.socketnum);
        else
            printf(" %7.7s", "-");
        if(hostinfo[i].tp.corenum > 0)
            printf(" %5d", hostinfo[i].tp.corenum);
        else
            printf(" %5.5s", "-");
        if(hostinfo[i].tp.threadnum > 0)
            printf(" %7d", hostinfo[i].tp.threadnum);
        else
            printf(" %7.7s", "-");        
        
        printf(" (");

        first = TRUE;
	    for (j=0; j<hostinfo[i].nRes; j++) {
                if (! first)
                   printf(" ");
	        printf("%s", hostinfo[i].resources[j]);
                first = FALSE;
            }

            fputs(")\n", stdout);
        }


        _i18n_end ( ls_catd );
        exit(0);
    }

    _i18n_end ( ls_catd );
    return(0);
}

