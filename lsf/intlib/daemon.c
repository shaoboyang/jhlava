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

#include "intlibout.h"

void
daemonize_(void)
{
    int i;
    struct rlimit rlp;     
    char  errMsg[MAXLINELEN]; 

    ls_closelog();
    switch (fork()) {
        case 0:
            break;
        case -1:
            sprintf(errMsg, "\
%s: fork() failed :%s", __func__, strerror(errno));
            perror(errMsg);
            exit(-1);
        default:
            exit(0);
    }
    
    setsid();
    
#ifdef RLIMIT_NOFILE
        
    getrlimit (RLIMIT_NOFILE, &rlp);
    rlp.rlim_cur = rlp.rlim_max;
    setrlimit (RLIMIT_NOFILE, &rlp);
#endif 

#ifdef RLIMIT_CORE
    
    getrlimit (RLIMIT_CORE, &rlp);
    rlp.rlim_cur = rlp.rlim_max;
    setrlimit (RLIMIT_CORE, &rlp);
#endif 

    

    for (i = 0 ; i < 3 ; i++)
	close(i);
    
    i = open(LSDEVNULL, O_RDWR);
    if (i != 0) {
	dup2(i, 0);
	close(i);
    }
    dup2(0, 1);
    dup2(0, 2);
}  


static char daemon_dir[MAXPATHLEN];
void
saveDaemonDir_(char *argv0)
{
    int i;

    daemon_dir[0]='\0';
    if (argv0[0] != '/') {
        getcwd(daemon_dir, sizeof(daemon_dir));
        strcat(daemon_dir,"/");
    }
    strcat(daemon_dir, argv0);
    for (i = strlen(daemon_dir); i >= 0 && daemon_dir[i] != '/'; i--)
        ;

    daemon_dir[i] = '\0';
}

char *
getDaemonPath_(char *name, char *serverdir)
{
    static char daemonpath[MAXPATHLEN];

    strcpy(daemonpath, daemon_dir);
    strcat(daemonpath, name);
    if (access(daemonpath, X_OK) < 0) {
        ls_syslog(LOG_ERR, "\
%s: Can't access %s: %s. Trying LSF_SERVERDIR.",
                  __func__, daemonpath, strerror(errno));

       strcpy(daemonpath, serverdir);
       strcat(daemonpath, name);
    }

    return daemonpath;
}


