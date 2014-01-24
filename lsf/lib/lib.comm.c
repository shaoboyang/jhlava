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

#ifdef APPROVED

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "lib.h"

static char amSlave_ = FALSE;
static int msock_ = -1;
static int myrpid_ = -1;
void unsetenv (char *);

int
ls_minit() 
{
    char *c;

    if ( (c = getenv("LSF_RPID")) == NULL) {
        fprintf(stderr,"ls_minit: Internal error- don't know my rpid\n");
	exit(-1);
    } else {
	myrpid_ = atoi(c);
	unsetenv("LSF_RPID");
    }

    if ( (c = getenv("LSF_CALLBACK_SOCK")) == NULL) {
        fprintf(stderr,"ls_minit: Internal error-no connection to master\n");
	exit(-1);
    } else {
	msock_ = atoi(c);
	amSlave_ = TRUE;
	unsetenv("LSF_CALLBACK_SOCK");
    }
}

int 
ls_getrpid(void)
{
    if (amSlave_ == TRUE)
        return(myrpid_);
    else
	return(0);
}

int 
ls_sndmsg(int tid, char *buf, int count, task_sock)
{
    int cc, sock;
    struct tid *tid;

    if (amSlave_ == TRUE)
	sock = msock_;
    else {
        if ((tid = tid_find(tid, task_sock)) == NULL)
	    return (-1);
	sock = tid->sock;
    }

    return(b_write_fix( sock, buf, count) );
}


int 
ls_rcvmsg(int tid, char *buf, int count)
{
    int cc;
    int sock;

    if (amSlave_ == TRUE)
	sock = msock_;
    else {
	if ( tid < 0) {

	} else {
            if ( (sock = tid_find(tid, task_sock)) < 0) {
		lserrno = LSE_RES_INVCHILD;
		return(-1);
	    }
        }
    }

    return(b_read_fix( sock, buf, count) );
}
#endif

