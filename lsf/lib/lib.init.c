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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <pwd.h>

#include "lib.h"
#include "lproto.h"
#include "lib.queue.h"
#include "mls.h"
#define NL_SETN 23

int mlsSbdMode = FALSE;

char rootuid_ = FALSE;

int
ls_initrex(int num, int options)
{
    struct servent *sv;

    if (geteuid() == 0)
       rootuid_ = TRUE;

    if (initenv_(NULL, NULL)<0) {
        if (rootuid_ && !(options & KEEPUID))
            lsfSetUid(getuid());
        return(-1);
    }

    inithostsock_();
    lsQueueInit_(&requestQ, lsReqCmp_, NULL);
    if (requestQ == NULL) {
        lserrno = LSE_MALLOC;
        return(-1);
    }

    res_addr_.sin_family = AF_INET;

    if (genParams_[LSF_RES_PORT].paramValue) {
        if ((res_addr_.sin_port = atoi(genParams_[LSF_RES_PORT].paramValue))
            != 0)
            res_addr_.sin_port = htons(res_addr_.sin_port);
        else
            goto res_init_fail;
    } else if (genParams_[LSF_RES_DEBUG].paramValue) {
        res_addr_.sin_port = htons(RES_PORT);
    } else {
#  if defined(_COMPANY_X_)
        if ((res_addr_.sin_port =
                 get_port_number(RES_SERVICE,(char *)NULL)) == -1) {
#  else
        if ((sv = getservbyname("res", "tcp")) != NULL)
            res_addr_.sin_port = sv->s_port;
        else {
#  endif
res_init_fail:
            lserrno = LSE_RES_NREG;
            if (rootuid_ && !(options & KEEPUID))
                lsfSetUid(getuid());
            return (-1);
        }
    }

    initconntbl_();
    FD_ZERO(&connection_ok_);

    if ((rootuid_) && (genParams_[LSF_AUTH].paramValue == NULL)) {
        int i;
        i = opensocks_(num);
        if (!(options & KEEPUID))
            lsfSetUid(getuid());
        return (i);
    } else {
        return (num);
    }
}

int
opensocks_(int num)
{
    static char fname[] = "opensocks_";
    int s;
    int nextdescr;
    int i;

    totsockets_ = (num <= 0 || num > MAXCONNECT) ? LSF_DEFAULT_SOCKS : num;

    if (logclass & LC_COMM)
       ls_syslog(LOG_DEBUG,"%s: try to allocate num <%d> of socks",fname,num);

    nextdescr = FIRST_RES_SOCK;
    for (i = 0; i < totsockets_; i++) {
        if ((s = CreateSock_(SOCK_STREAM)) < 0) {
            if (logclass & LC_COMM)
                ls_syslog(LOG_DEBUG,
                   "%s: CreateSock_ failed, iter:<%d> %s",
                    fname,i,strerror(errno));
            totsockets_ = i;
            if (i > 0) {
                break;
            } else {
               return(-1);
            }
        }

        if (s != nextdescr) {
            if (dup2(s,nextdescr) < 0) {
                if (logclass & LC_COMM)
                    ls_syslog(LOG_DEBUG,
                    "%s: dup2() failed, old:<%d>, new<%d>, iter:<%d>  %s",
                               fname,s,nextdescr,i,strerror(errno));
                close(s);
                lserrno = LSE_SOCK_SYS;
                totsockets_ = i;
                if (i > 0)
                   break;
                else
                   return (-1);
            }

#if defined(FD_CLOEXEC)
            fcntl(nextdescr, F_SETFD, (fcntl(nextdescr, F_GETFD)
                       | FD_CLOEXEC)) ;
#else
#if defined(FIOCLEX)
            (void)ioctl(nextdescr, FIOCLEX, (char *)NULL);
#endif
#endif

            close(s);
        }
        nextdescr++;
    }

    currentsocket_ = FIRST_RES_SOCK;

    if (logclass & LC_COMM)
       ls_syslog(LOG_DEBUG,"%s: returning num=<%d>",fname,totsockets_);

    return (totsockets_);

}

/* ls_fdbusy()
 */
int
ls_fdbusy(int fd)
{
    sTab   hashSearchPtr;
    hEnt   *hEntPtr;

    if (fd == chanSock_(limchans_[PRIMARY])
        || fd == chanSock_(limchans_[MASTER])
        || fd == chanSock_(limchans_[UNBOUND]))
        return TRUE;

    if (fd == cli_nios_fd[0])
        return TRUE;

    hEntPtr = h_firstEnt_(&conn_table, &hashSearchPtr);
    while (hEntPtr) {
        int   *pfd;

        pfd = hEntPtr->hData;
        if (fd == pfd[0]
            || fd == pfd[1])
            return (TRUE);

        hEntPtr = h_nextEnt_(&hashSearchPtr);
    }

    if (rootuid_
        && fd >= currentsocket_
        && fd < FIRST_RES_SOCK + totsockets_)
        return TRUE;

    return FALSE;
}

int
lsfSetXUid(int flag, int ruid, int euid, int suid, int(*func)())
{
    int rtrn = -1;

    if ( func == setuid ) {
        rtrn = setuid(ruid);
    } else if (func == seteuid) {
    	rtrn = seteuid(euid);
    } else if ( func == setreuid ) {
        rtrn = setreuid(ruid, euid);
    }

    return(rtrn);
}

void
lsfExecLog(const char *cmd)
{
    static char fname[] = "lsfExecLog";
    char lsfUserName[MAXLSFNAMELEN];

    if (genParams_[LSF_MLS_LOG].paramValue &&
                ((genParams_[LSF_MLS_LOG].paramValue[0] == 'y') ||
                (genParams_[LSF_MLS_LOG].paramValue[0] == 'Y'))) {

        getLSFUser_(lsfUserName, sizeof(lsfUserName));
        syslog(LOG_INFO,
            I18N(6259,"%s: user - %s cmd - '%s'"), /* catgets 6259 */
            fname, lsfUserName, cmd);

    }
}
int
lsfExecX(char *path, char **argv, int(*func)())
{
    return(func(path, argv));
}
