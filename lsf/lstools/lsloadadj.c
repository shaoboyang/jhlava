/*
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
#include <sys/errno.h>
#include <stdlib.h>
#include <netdb.h>

#include "../lsf.h"



#include "../lib/lproto.h"
#define MAXLISTSIZE 256

static void usage(char *);
extern int errno;

#define NL_SETN 27 


int
main(int argc, char **argv)
{
    static char fname[] = "lsloadadj/main";
    char *resreq = NULL;
    struct placeInfo  placeadvice[MAXLISTSIZE];
    char *p, *hname;
    int cc = 0;
    int	achar;
    int rc;

    rc = _i18n_init ( I18N_CAT_MIN );	


    if (ls_initdebug(argv[0]) < 0) {
        ls_perror("ls_initdebug");
        exit(-1);
    }
    if (logclass & (LC_TRACE))
        ls_syslog(LOG_DEBUG, "%s: Entering this routine...", fname);

    opterr = 0;
    while ((achar = getopt(argc, argv, "VhR:")) != EOF)
    {
	switch (achar)
	{
	case 'R':
	    resreq = optarg;
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
        if (cc >= MAXLISTSIZE)
	{
	    fprintf(stderr, 
		_i18n_msg_get(ls_catd,NL_SETN,1951, "%s: too many hostnames (maximum %d)\n"), /* catgets  1951  */
		fname, MAXLISTSIZE);
	    usage(argv[0]);
	}
 
        p = strchr(argv[optind],':');
        if ( (p != NULL) && (*(p+1) != '\0') )  {
             *p++ = '\0';
             placeadvice[cc].numtask = atoi(p);
             if (errno == ERANGE) {
                 fprintf(stderr,
		     _i18n_msg_get(ls_catd,NL_SETN,1952, "%s: invalid format for number of components\n"), /* catgets 1952 */  
		     fname); 
                 usage(argv[0]);
             }
        } else {
             placeadvice[cc].numtask = 1;
        }

        if (!Gethostbyname_(argv[optind])) {
	    fprintf(stderr, "\
%s: invalid hostname %s\n", __func__, argv[optind]);
	    usage(argv[0]);
	}
        strcpy(placeadvice[cc++].hostName, argv[optind]);
    }

    if (cc == 0) {
        
	if ((hname = ls_getmyhostname()) == NULL) {
	    ls_perror("ls_getmyhostname");
	    exit(-1);
        }
        strcpy(placeadvice[0].hostName, hname);
        placeadvice[0].numtask = 1;    
        cc = 1;
    }

    if (ls_loadadj(resreq, placeadvice, cc) < 0) {
        ls_perror("lsloadadj");
        exit(-1);
    } else 


    _i18n_end ( ls_catd );			

    exit(0);
} 

static void usage(char *cmd)
{
    printf("%s: %s [-h] [-V] [-R res_req] [host_name[:num_task] host_name[:num_task] ...]\n",I18N_Usage, cmd); 
    exit(-1);
}
