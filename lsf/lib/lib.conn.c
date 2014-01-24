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
#include <stdlib.h>
#include <unistd.h>
#include "lib.h"
#include "lib.table.h"

hTab   conn_table;

typedef struct _hostSock {
    int   socket;
    char *hostname;
    struct _hostSock *next;
} HostSock;
static HostSock *hostSock;

static struct connectEnt connlist[MAXCONNECT];
static char   *connnamelist[MAXCONNECT+1];

int cli_nios_fd[2] = {-1, -1};

void hostIndex_(char *hostName, int sock);
extern int chanSock_(int chfd);
int delhostbysock_(int sock);

void
inithostsock_(void)
{
    hostSock = NULL;
}

void
initconntbl_(void)
{
   h_initTab_(&conn_table, 3);

}

int
connected_(char *hostName, int sock1, int sock2, int seqno)
{
    int    new;
    hEnt   *hEntPtr;
    int    *sp;

    hEntPtr = h_addEnt_(&conn_table, hostName, &new);
    if (!new) {
        sp = hEntPtr->hData;
    } else {
        sp = calloc(3, sizeof(int));
        sp[0] = -1;
        sp[1] = -1;
	sp[2] = -1;
    }

    if (sock1 >= 0) {
        sp[0] = sock1;
	hostIndex_(hEntPtr->keyname, sock1);
    }

    if (sock2 >= 0)
        sp[1] = sock2;

    if (seqno >= 0)
        sp[2] = seqno;

    hEntPtr->hData = sp;

    return (0);

}

void
hostIndex_(char *hostName, int sock)
{
    HostSock   *newSock;

    newSock = malloc(sizeof(HostSock));
    if (newSock == NULL) {
        ls_syslog(LOG_ERR, "hostIndex_ : malloc HostSock failed");
        exit(-1);
    }
    newSock->socket = sock;
    newSock->hostname = hostName;
    newSock->next = hostSock;
    hostSock = newSock;

}

int
delhostbysock_(int sock)
{
    HostSock *tmpSock;

    tmpSock = hostSock;

    if (tmpSock->socket == sock) {
      hostSock = hostSock->next;
      free(tmpSock);
      return 0;
    }

    while (tmpSock->next != NULL) {
      if (tmpSock->next->socket == sock) {
       HostSock *rmSock = tmpSock->next;
       tmpSock->next = rmSock->next;
       free(rmSock);
       return 0;
      }
      tmpSock = tmpSock->next;
    }

    return -1;
}

int
gethostbysock_(int sock, char *hostName)
{
    HostSock *tmpSock;

    if (hostName == NULL) {
        return -1;
    }

    tmpSock = hostSock;

    while (tmpSock != NULL) {
        if (tmpSock->socket == sock) {
            if (tmpSock->hostname != NULL) {
                strcpy(hostName, tmpSock->hostname);
                return 0;
			}
		}
		tmpSock = tmpSock->next;
    }

    strcpy(hostName, "LSF_HOST_NULL");
    return -1;

}

int *
_gethostdata_(char *hostName)
{
    hEnt *ent;
    int  *sp;
    struct hostent *hp;

    hp = Gethostbyname_(hostName);
    if (hp == NULL)
        return NULL;

    ent = h_getEnt_(&conn_table, hp->h_name);
    if (ent == NULL)
        return NULL;

    if (ent->hData == NULL)
        return NULL;

    sp = ent->hData;

    return sp;
}

int
_isconnected_(char *hostName, int *sock)
{
    int   *sp;

    sp = _gethostdata_(hostName);
    if (sp == NULL)
        return (FALSE);

    sock[0] = sp[0];
    sock[1] = sp[1];

    return (TRUE);
}

int
_getcurseqno_(char *hostName)
{
    int *sp;

    sp = _gethostdata_(hostName);
    if (sp == NULL)
	return(-1);

    return(sp[2]);
}

void
_setcurseqno_(char *hostName, int seqno)
{
    int *sp;

    sp = _gethostdata_(hostName);
    if (sp == NULL)
        return;

    sp[2] = seqno;
}

int
ls_isconnected(char *hostName)
{
    hEnt *hEntPtr;
    struct hostent *hp;

    hp = Gethostbyname_(hostName);
    if (hp == NULL)
        return FALSE;

    hEntPtr = h_getEnt_(&conn_table, hp->h_name);
    if (hEntPtr == NULL)
        return FALSE;

    return TRUE;
}

int
getConnectionNum_(char *hostName)
{
    hEnt *hEntPtr;
    int *connNum;
    struct hostent *hp;

    hp = Gethostbyname_(hostName);
    if (hp == NULL)
        return -1;

    if ((hEntPtr = h_getEnt_(&conn_table, hp->h_name)) == NULL)
	return -1;

    connNum = hEntPtr->hData;
    delhostbysock_(connNum[0]);
    h_rmEnt_(&conn_table, hEntPtr);

    return connNum[0];
}

int
_findmyconnections_(struct connectEnt **connPtr)
{
    int    n;
    sTab   sTab;
    hEnt   *ent;

    ent = h_firstEnt_(&conn_table, &sTab);
    if (ent == NULL) {
        return (0);
    }

    n = 0;
    while (ent) {
        int   *pfd;

        pfd = ent->hData;
        connlist[n].hostname = ent->keyname;
        connlist[n].csock[0] = pfd[0];
        connlist[n].csock[1] = pfd[1];
        ent = h_nextEnt_(&sTab);
        n++;
    }

    *connPtr = connlist;

    return (n);
}

char **
ls_findmyconnections(void)
{
    int n = 0;
    sTab hashSearchPtr;
    hEnt *hEntPtr;

    hEntPtr = h_firstEnt_(&conn_table, &hashSearchPtr);

    while (hEntPtr) {
	connnamelist[n] = hEntPtr->keyname;
	hEntPtr = h_nextEnt_(&hashSearchPtr);
        n++;
    }
    connnamelist[n] = NULL;

    return (connnamelist);

}

