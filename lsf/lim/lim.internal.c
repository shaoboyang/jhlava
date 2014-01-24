/*
 * Copyright (C) 2011 David Bigagli
 *
 * $Id: lim.h 397 2007-11-26 19:04:00Z mblack $
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

#include <math.h>
#include "lim.h"

extern short  hostInactivityLimit;
static void sndConfInfo(struct sockaddr_in *);

void
masterRegister(XDR *xdrs,
               struct sockaddr_in *from,
               struct LSFHeader *reqHdr)
{
    static int checkSumMismatch;
    struct hostNode *hPtr;
    struct masterReg masterReg;

    if (!limPortOk(from))
        return;

    if (!xdr_masterReg(xdrs, &masterReg, reqHdr)) {
        ls_syslog(LOG_ERR, "%s: Failed in xdr_masterReg", __func__);
        return;
    }

    if (strcmp(myClusterPtr->clName, masterReg.clName) != 0) {
        ls_syslog(LOG_WARNING, "\
%s: Discard announce from a different cluster %s than mine %s (?)",
                  __func__, masterReg.clName, myClusterPtr->clName);
        return;
    }

    if (masterReg.checkSum != myClusterPtr->checkSum
        && checkSumMismatch < 2
        && (limParams[LSF_LIM_IGNORE_CHECKSUM].paramValue == NULL)) {

        ls_syslog(LOG_WARNING, "\
%s: Sender %s may have different config.",
                  __func__, masterReg.hostName);
        checkSumMismatch++;
    }

    if (equalHost_(myHostPtr->hostName, masterReg.hostName))
        return;

    hPtr = findHostbyList(myClusterPtr->hostList, masterReg.hostName);
    if (hPtr == NULL) {
        ls_syslog(LOG_ERR, "\
%s: Got master announcement from unused host %s; \
Run lim -C on this host to find more information",
                  __func__, masterReg.hostName);
        return;
    }
    /* Regular announce from the master.
     */
    if (myClusterPtr->masterKnown
        && hPtr == myClusterPtr->masterPtr) {

        myClusterPtr->masterInactivityCount = 0;

        if (masterReg.flags & SEND_ELIM_REQ)
            myHostPtr->callElim = TRUE ;
        else
            myHostPtr->callElim = FALSE ;

        if ((masterReg.seqNo - hPtr->lastSeqNo > 2)
            && (masterReg.seqNo > hPtr->lastSeqNo)
            && (hPtr->lastSeqNo != 0))

            ls_syslog(LOG_WARNING, "\
%s: master %s lastSeqNo=%d seqNo=%d. Packets dropped?",
                      __func__, hPtr->hostName,
                      hPtr->lastSeqNo, masterReg.seqNo);

        hPtr->lastSeqNo = masterReg.seqNo;
        hPtr->statInfo.portno = masterReg.portno;

        if (masterReg.flags & SEND_CONF_INFO)
            sndConfInfo(from);

        if (masterReg.flags & SEND_LOAD_INFO) {
            mustSendLoad = TRUE;
            ls_syslog(LOG_DEBUG, "\
%s: Master lim is probing me. Send my load in next interval", __func__);
        }

        return;

    }

    /* Someone out there is trying to be a master,
     * get lost buddy...
     */
    if (myClusterPtr->masterKnown
        && hPtr->hostNo < myHostPtr->hostNo
        && myClusterPtr->masterPtr->hostNo < hPtr->hostNo
        && myClusterPtr->masterInactivityCount <= hostInactivityLimit) {
        ls_syslog(LOG_INFO, "\
%s: Host %s is trying to take over from %s, not accepted",
                  __func__, masterReg.hostName,
                  myClusterPtr->masterPtr->hostName);
        announceMasterToHost(hPtr, SEND_NO_INFO);
        return;
    }

#ifndef LINUX
    /* if not linux system, accept everyone announce. */
    if(1){
#else
    /* A host with hostnumber higher than me has the
     * right to become a new master. Welcome.
     */
    if (hPtr->hostNo < myHostPtr->hostNo) {
#endif
        /* This is the regular master registration.
         */
        hPtr->protoVersion = reqHdr->version;
        myClusterPtr->prevMasterPtr = myClusterPtr->masterPtr;
        myClusterPtr->masterPtr   = hPtr;

        myClusterPtr->masterPtr->statInfo.portno = masterReg.portno;
        if (masterMe) {
            ls_syslog(LOG_INFO, "\
%s: Give in master to %s", __func__, masterReg.hostName);
        }
        masterMe                  = 0;
        myClusterPtr->masterKnown = 1;
        myClusterPtr->masterInactivityCount = 0;
        mustSendLoad = 1;

        if (masterReg.flags | SEND_CONF_INFO)
            sndConfInfo(from);

        if (masterReg.flags & SEND_LOAD_INFO) {
            mustSendLoad = 1;
            ls_syslog(LOG_DEBUG, "\
%s: Master lim is probing me. Send my load in next interval", __func__);
        }

        return;
    }

    if (myClusterPtr->masterKnown
        && myClusterPtr->masterInactivityCount < hostInactivityLimit) {

        announceMasterToHost(hPtr, SEND_NO_INFO);
        ls_syslog(LOG_INFO, "\
%s: Host %s is trying to take over master LIM from %s, not accepted",
                  __func__, masterReg.hostName,
                  myClusterPtr->masterPtr->hostName);
        return;

    }

    ls_syslog(LOG_INFO, "\
%s: Host %s is trying to take over master LIM, not accepted", __func__,
              masterReg.hostName);
}

static void
announceElimInstance(struct clusterNode *clPtr)
{
    struct hostNode *hostPtr;
    struct sharedResourceInstance *tmp;
    int i;

    for (tmp = sharedResourceHead; tmp ; tmp = tmp->nextPtr) {

        for (i = 0; i <tmp->nHosts; i++){
            hostPtr = tmp->hosts[i] ;
            if (hostPtr){
                if (hostPtr->infoValid){
                    hostPtr->callElim = TRUE ;
                    break ;
                }
            }
        }
    }

}

void
announceMaster(struct clusterNode *clPtr, char broadcast, char all)
{
    struct hostNode *hPtr;
    struct sockaddr_in toAddr;
    struct masterReg tmasterReg ;
    XDR    xdrs1;
    char   buf1[MSGSIZE/4];
    XDR    xdrs2;
    char   buf2[MSGSIZE/4];
    XDR    xdrs4;
    char   buf4[MSGSIZE/4];
    enum limReqCode limReqCode;
    struct masterReg masterReg;
    static int cnt = 0;
    struct LSFHeader reqHdr;
    int announceInIntvl;
    int numAnnounce;
    int i;
    int periods;

    announceElimInstance(clPtr);

    /* hostInactivityLimit = 5
     * exchIntvl = 15
     * sampleIntvl = 5
     * periods = (5 - 1) * 15/5 = 60/5 = 12
     */
    periods = (hostInactivityLimit - 1) * exchIntvl/sampleIntvl;
    if (!all && (++cnt > (periods - 1))) {
        cnt = 0;
        masterAnnSeqNo++;
    }

    limReqCode = LIM_MASTER_ANN;
    strcpy(masterReg.clName, myClusterPtr->clName);
    strcpy(masterReg.hostName, myClusterPtr->masterPtr->hostName);
    masterReg.seqNo    = masterAnnSeqNo;
    masterReg.checkSum = myClusterPtr->checkSum;
    masterReg.portno   = myClusterPtr->masterPtr->statInfo.portno;

    toAddr.sin_family = AF_INET;
    toAddr.sin_port = lim_port;

    initLSFHeader_(&reqHdr);
    reqHdr.opCode  = (short) limReqCode;
    reqHdr.refCode = 0;

    xdrmem_create(&xdrs1, buf1, MSGSIZE/4, XDR_ENCODE);
    masterReg.flags = SEND_NO_INFO ;

    if (! (xdr_LSFHeader(&xdrs1, &reqHdr)
           && xdr_masterReg(&xdrs1, &masterReg, &reqHdr))) {
        ls_syslog(LOG_ERR, "\
%s: Error in xdr_LSFHeader/xdr_masterReg", __func__);
        xdr_destroy(&xdrs1);
        return;
    }

    xdrmem_create(&xdrs2, buf2, MSGSIZE/4, XDR_ENCODE);
    masterReg.flags = SEND_CONF_INFO;
    if (! (xdr_LSFHeader(&xdrs2, &reqHdr)
           && xdr_masterReg(&xdrs2, &masterReg, &reqHdr))) {
        ls_syslog(LOG_ERR, "\
%s: Error in xdr_enum/xdr_masterReg", __func__);
        xdr_destroy(&xdrs1);
        xdr_destroy(&xdrs2);
        return;
    }

    memcpy(&tmasterReg, &masterReg, sizeof(struct masterReg));
    tmasterReg.flags = SEND_NO_INFO | SEND_ELIM_REQ;

    xdrmem_create(&xdrs4, buf4, MSGSIZE/4, XDR_ENCODE);
    if (! xdr_LSFHeader(&xdrs4, &reqHdr)) {
        ls_syslog(LOG_ERR, "\
%s: failed in xdr_LSFHeader", __func__);
        xdr_destroy(&xdrs1);
        xdr_destroy(&xdrs2);
        xdr_destroy(&xdrs4);
        return;
    }

    if (! xdr_masterReg(&xdrs4, &tmasterReg, &reqHdr)) {
        ls_syslog(LOG_ERR,"\
%s: failed in xdr_masterRegister", __func__);
        xdr_destroy(&xdrs1);
        xdr_destroy(&xdrs2);
        xdr_destroy(&xdrs4);
        return;
    }

    if (clPtr->masterKnown && ! broadcast) {

        memcpy(&toAddr.sin_addr, &clPtr->masterPtr->addr[0], sizeof(u_int));
        if (logclass & LC_COMM)
            ls_syslog(LOG_DEBUG, "\
%s: Sending request to LIM on %s: %m", __func__, sockAdd2Str_(&toAddr));

        if (chanSendDgram_(limSock,
                           buf1,
                           XDR_GETPOS(&xdrs1),
                           (struct sockaddr_in *)&toAddr) < 0)
            ls_syslog(LOG_ERR, "\
%s: Failed to send request to LIM on %s: %m", __func__,
                      sockAdd2Str_(&toAddr));
        xdr_destroy(&xdrs1);
        return;
    }

    if (all) {
        hPtr = clPtr->hostList;
        announceInIntvl = clPtr->numHosts;
    } else {
        announceInIntvl = clPtr->numHosts/periods;
        if (announceInIntvl == 0)
            announceInIntvl = 1;

        hPtr = clPtr->hostList;
        for (i = 0; i < cnt * announceInIntvl; i++) {
            if (!hPtr)
                break;
            hPtr = hPtr->nextPtr;
        }

        /* Let's announce the rest of the hosts,
         * this takes care of the reminder
         * numHosts/periods.
         */
        if (cnt == (periods - 1))
            announceInIntvl = clPtr->numHosts;
    }

    ls_syslog(LOG_DEBUG, "\
%s: all %d cnt %d announceInIntvl %d",
              __func__, all, cnt, announceInIntvl);

    for (numAnnounce = 0;
         hPtr && (numAnnounce < announceInIntvl);
         hPtr = hPtr->nextPtr, numAnnounce++) {

        if (hPtr == myHostPtr)
            continue;

        memcpy(&toAddr.sin_addr, &hPtr->addr[0], sizeof(u_int));

        if (hPtr->infoValid == TRUE) {

            if (logclass & LC_COMM)
                ls_syslog(LOG_DEBUG, "\
%s: send announce (normal) to %s %s, inactivityCount=%d",
                          __func__,
                          hPtr->hostName, sockAdd2Str_(&toAddr),
                          hPtr->hostInactivityCount);

            if (hPtr->callElim){

                if (logclass & LC_COMM)
                    ls_syslog(LOG_DEBUG,"\
%s: announcing SEND_ELIM_REQ to host %s %s",
                              __func__, hPtr->hostName,
                              sockAdd2Str_(&toAddr));

                if (chanSendDgram_(limSock,
                                   buf4,
                                   XDR_GETPOS(&xdrs4),
                                   (struct sockaddr_in *)&toAddr) < 0) {
                    ls_syslog(LOG_ERR,"\
%s: Failed to send request 1 to LIM on %s: %m", __func__,
                              hPtr->hostName);
                }

                hPtr->callElim = FALSE;

            } else {
                if (chanSendDgram_(limSock,
                                   buf1,
                                   XDR_GETPOS(&xdrs1),
                                   (struct sockaddr_in *)&toAddr) < 0)
                    ls_syslog(LOG_ERR,"\
announceMaster: Failed to send request 1 to LIM on %s: %m", hPtr->hostName);
            }

        } else {

            if (logclass & LC_COMM)
                ls_syslog(LOG_DEBUG,"\
%s: send announce (SEND_CONF) to %s %s %x, inactivityCount=%d",
                          __func__,
                          hPtr->hostName, sockAdd2Str_(&toAddr),
                          hPtr->addr[0],
                          hPtr->hostInactivityCount);

            if (chanSendDgram_(limSock,
                               buf2,
                               XDR_GETPOS(&xdrs2),
                               (struct sockaddr_in *)&toAddr) < 0)
                ls_syslog(LOG_ERR, "\
%s: Failed to send request 2 to LIM on %s: %m",
                          __func__, hPtr->hostName);
        }

    }

    xdr_destroy(&xdrs1);
    xdr_destroy(&xdrs2);
    xdr_destroy(&xdrs4);
    return;
}

void
jobxferReq(XDR *xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
    struct hostNode *hPtr;
    struct jobXfer jobXfer;
    int i;

    if (!limPortOk(from))
        return;

    if (myClusterPtr->masterKnown && myClusterPtr->masterPtr &&
        equivHostAddr(myClusterPtr->masterPtr, *(u_int *)&from->sin_addr))
        myClusterPtr->masterInactivityCount = 0;

    if (!xdr_jobXfer(xdrs, &jobXfer, reqHdr)) {
        ls_syslog(LOG_ERR, "%s: Error on xdr_jobXfer", __func__);
        return;
    }

    for (i = 0; i < jobXfer.numHosts; i++) {
        if ((hPtr = findHost(jobXfer.placeInfo[i].hostName)) != NULL) {
            hPtr->use = jobXfer.placeInfo[i].numtask;
            updExtraLoad(&hPtr, jobXfer.resReq, 1);
        } else {
            ls_syslog(LOG_ERR, "\
%s: %s not found in jobxferReq", __func__,
                      jobXfer.placeInfo[i].hostName);
        }
    }

    return;

}

void
wrongMaster(struct sockaddr_in *from,
            char *buf,
            struct LSFHeader *reqHdr,
            int s)
{
    enum limReplyCode limReplyCode;
    XDR xdrs;
    struct LSFHeader replyHdr;
    struct masterInfo masterInfo;
    int cc;
    char *replyStruct;

    if (myClusterPtr->masterKnown) {
        limReplyCode = LIME_WRONG_MASTER;
        strcpy(masterInfo.hostName, myClusterPtr->masterPtr->hostName);
        masterInfo.addr   = myClusterPtr->masterPtr->addr[0];
        masterInfo.portno = myClusterPtr->masterPtr->statInfo.portno;
        replyStruct = (char *)&masterInfo;
    } else  {
        limReplyCode = LIME_MASTER_UNKNW;
        replyStruct = (char *)NULL;
    }

    xdrmem_create(&xdrs, buf, MSGSIZE, XDR_ENCODE);
    initLSFHeader_(&replyHdr);
    replyHdr.opCode  = (short) limReplyCode;
    replyHdr.refCode = reqHdr->refCode;

    if (! xdr_encodeMsg(&xdrs,
                        replyStruct,
                        &replyHdr,
                        xdr_masterInfo,
                        0,
                        NULL)) {
        ls_syslog(LOG_ERR, "\
%s: error on xdr_LSFHeader/xdr_masterInfo", __func__);
        xdr_destroy(&xdrs);
        return;
    }

    if (logclass & LC_COMM)
        ls_syslog(LOG_DEBUG, "\
%s: Sending s%d to %s", __func__, s, sockAdd2Str_(from));

    if (s < 0) {
        cc = chanSendDgram_(limSock,
                            buf,
                            XDR_GETPOS(&xdrs),
                            (struct sockaddr_in *)from);
    } else {
        cc = chanWrite_(s, buf, XDR_GETPOS(&xdrs));
    }

    if (cc < 0) {
        ls_syslog(LOG_ERR, "\
%s: Send to %s len %d failed: %m", __func__,
                  sockAdd2Str_(from), XDR_GETPOS(&xdrs));
        xdr_destroy(&xdrs);
        return;
    }

    xdr_destroy(&xdrs);
    return;

}

void
initNewMaster(void)
{
    struct hostNode *hPtr;
    int j;

    for (hPtr = myClusterPtr->hostList; hPtr; hPtr = hPtr->nextPtr) {

        if (hPtr != myHostPtr)  {
            hPtr->status[0] |= LIM_UNAVAIL;
            for (j = 0; j < GET_INTNUM(allInfo.numIndx); j++)
                hPtr->status[j + 1] = 0;
            hPtr->hostInactivityCount = 0;
            hPtr->infoValid = FALSE;
            hPtr->lastSeqNo = 0;
        }
    }

    masterAnnSeqNo = 0;

    mustSendLoad = TRUE;
    myClusterPtr->masterKnown  = TRUE;
    myClusterPtr->prevMasterPtr = myClusterPtr->masterPtr;
    myClusterPtr->masterPtr = myHostPtr;

    announceMaster(myClusterPtr, 1, TRUE);
    myClusterPtr->masterInactivityCount = 0;
    masterMe = 1;
    ls_syslog(LOG_WARNING, "%s: I am the master now", __func__);
}

void
rcvConfInfo(XDR *xdrs,
            struct sockaddr_in *from,
            struct LSFHeader *hdr)
{
    struct statInfo sinfo;
    struct hostNode *hPtr;

    if (!limPortOk(from))
        return;

    sinfo.maxCpus = 0;
    sinfo.tp.topology = NULL;

    if (!masterMe) {
        ls_syslog(LOG_DEBUG, "rcvConfInfo: I am not the master!");
        return;
    }

    if (!xdr_statInfo(xdrs, &sinfo, hdr)) {
        ls_syslog(LOG_ERR, "rcvConfInfo: xdr_statInfo failed");
        FREEUP(sinfo.tp.topology);
        return;
    }

    hPtr = findHostbyAddr(from, "rcvConfInfo");
    if (!hPtr){
        FREEUP(sinfo.tp.topology);
	return;
        }
    if (findHostbyList(myClusterPtr->hostList, hPtr->hostName) == NULL) {
        ls_syslog(LOG_ERR, "\
%s: Got info from client-only host %s %s", __func__,
             sockAdd2Str_(from), hPtr->hostName);
        FREEUP(sinfo.tp.topology);
        return;
    }

    if (hPtr->infoValid == TRUE){
        FREEUP(sinfo.tp.topology);
        return;
    }
    if (sinfo.maxCpus <= 0 || sinfo.maxMem < 0) {
        ls_syslog(LOG_ERR, "\
%s: Invalid info received: maxCpus %d, maxMem %d", __func__,
                  sinfo.maxCpus, sinfo.maxMem);
        FREEUP(sinfo.tp.topology);
        return;
    }

    hPtr->statInfo.maxCpus = sinfo.maxCpus;
    hPtr->statInfo.maxMem  = sinfo.maxMem;
    hPtr->statInfo.maxSwap = sinfo.maxSwap;
    hPtr->statInfo.maxTmp  = sinfo.maxTmp;
    hPtr->statInfo.nDisks = sinfo.nDisks;
    hPtr->statInfo.portno = sinfo.portno;
    hPtr->hTypeNo = typeNameToNo(sinfo.hostType);
    hPtr->hModelNo = archNameToNo(sinfo.hostArch);
    hPtr->statInfo.maxPhyCpus = sinfo.maxPhyCpus;
    hPtr->infoValid      = TRUE;
    hPtr->infoMask       = 0;
    hPtr->protoVersion = hdr->version;
    hPtr->statInfo.tp.socketnum = sinfo.tp.socketnum;
    hPtr->statInfo.tp.corenum   = sinfo.tp.corenum;
    hPtr->statInfo.tp.threadnum = sinfo.tp.threadnum;
    hPtr->statInfo.tp.topology  = sinfo.tp.topology;
    hPtr->statInfo.tp.topologyflag = sinfo.tp.topologyflag;

    ls_syslog(LOG_DEBUG, "\
%s: Host %s: maxCpus=%d maxMem=%d ndisks=%d",
              __func__, hPtr->hostName,
              hPtr->statInfo.maxCpus, hPtr->statInfo.maxMem,
              hPtr->statInfo.nDisks);
}

void
sndConfInfo(struct sockaddr_in *to)
{
    char   buf[MSGSIZE/4];
    XDR    xdrs;
    enum limReqCode limReqCode;
    struct LSFHeader reqHdr;

    memset((char*)&buf, 0, sizeof(buf));
    initLSFHeader_(&reqHdr);
    ls_syslog(LOG_DEBUG, "sndConfInfo: Sending info");

    limReqCode = LIM_CONF_INFO;

    xdrmem_create(&xdrs, buf, MSGSIZE/4, XDR_ENCODE);
    reqHdr.opCode  = (short) limReqCode;
    reqHdr.refCode =  0;

    if (! xdr_LSFHeader(&xdrs, &reqHdr)
        || !xdr_statInfo(&xdrs, &myHostPtr->statInfo, &reqHdr)) {
        ls_syslog(LOG_ERR, "\
%s: Error in  xdr_LSFHeader/xdr_statInfo", __func__);
        return;
    }

    if (logclass & LC_COMM)
        ls_syslog(LOG_DEBUG, "\
%s: chanSendDgram_ info to %s", __func__, sockAdd2Str_(to));

    if (chanSendDgram_(limSock,
                       buf,
                       XDR_GETPOS(&xdrs),
                       (struct sockaddr_in *)to) < 0) {
        ls_syslog(LOG_ERR, "\
%s: chanSendDgram_ info to %s failed: %m", __func__, sockAdd2Str_(to));
        return;
    }

    xdr_destroy(&xdrs);

}

void
checkHostWd (void)
{
    struct dayhour dayhour;
    windows_t *wp;
    char   active;
    time_t now = time(0);

    if (myHostPtr->wind_edge > now || myHostPtr->wind_edge == 0)
        return;

    getDayHour(&dayhour, now);

    if (myHostPtr->week[dayhour.day] == NULL) {
        myHostPtr->status[0] |= LIM_LOCKEDW;
        myHostPtr->wind_edge = now + (24.0 - dayhour.hour) * 3600.0;
        return;
    }

    active = FALSE;
    myHostPtr->wind_edge = now + (24.0 - dayhour.hour) * 3600.0;
    for (wp = myHostPtr->week[dayhour.day]; wp; wp=wp->nextwind)
        checkWindow(&dayhour, &active, &myHostPtr->wind_edge, wp, now);
    if (!active)
        myHostPtr->status[0] |= LIM_LOCKEDW;
    else
        myHostPtr->status[0] &= ~LIM_LOCKEDW;

}

void
announceMasterToHost(struct hostNode *hPtr, int infoType )
{
    struct sockaddr_in toAddr;
    XDR    xdrs;
    char   buf[MSGSIZE/4];
    enum limReqCode limReqCode;
    struct masterReg masterReg;
    struct LSFHeader reqHdr;

    limReqCode = LIM_MASTER_ANN;
    strcpy(masterReg.clName, myClusterPtr->clName);
    strcpy(masterReg.hostName, myClusterPtr->masterPtr->hostName);
    masterReg.flags = infoType;
    masterReg.seqNo    = masterAnnSeqNo;
    masterReg.checkSum = myClusterPtr->checkSum;
    masterReg.portno   = myClusterPtr->masterPtr->statInfo.portno;

    toAddr.sin_family = AF_INET;
    toAddr.sin_port = lim_port;

    xdrmem_create(&xdrs, buf, MSGSIZE/4, XDR_ENCODE);
    initLSFHeader_(&reqHdr);
    reqHdr.opCode  = (short) limReqCode;
    reqHdr.refCode =  0;

    if (! xdr_LSFHeader(&xdrs,  &reqHdr)
        || ! xdr_masterReg(&xdrs, &masterReg, &reqHdr)) {
        ls_syslog(LOG_ERR, "\
%s: Error xdr_LSFHeader/xdr_masterReg to %s",
                  __func__, sockAdd2Str_(&toAddr));
        xdr_destroy(&xdrs);
        return;
    }

    memcpy(&toAddr.sin_addr, &hPtr->addr[0], sizeof(in_addr_t));

    ls_syslog(LOG_DEBUG, "\
%s: Sending request %d to LIM on %s",
              __func__, infoType, sockAdd2Str_(&toAddr));

    if (chanSendDgram_(limSock,
                       buf,
                       XDR_GETPOS(&xdrs),
                       (struct sockaddr_in *)&toAddr) < 0)
        ls_syslog(LOG_ERR, "\
%s: Failed to send request %d to LIM on %s: %m", __func__,
                  infoType, sockAdd2Str_(&toAddr));

    xdr_destroy(&xdrs);
}

int
probeMasterTcp(struct clusterNode *clPtr)
{
    struct hostNode *hPtr;
    struct sockaddr_in mlim_addr;
    int ch;
    int rc;
    struct LSFHeader reqHdr;

    ls_syslog (LOG_DEBUG, "%s: enter.... ", __func__);

    hPtr = clPtr->masterPtr;
    if (!hPtr)
        hPtr = clPtr->prevMasterPtr;

    ls_syslog (LOG_ERR, "%s: Last master is  UNKNOWN", __func__);

    if (!hPtr)
        return -1;

    if (hPtr == myHostPtr)
        return -1;

    ls_syslog(LOG_ERR, "\
%s: Attempting to probe last known master %s port %d timeout is %d",
              __func__, hPtr->hostName,
              ntohs(hPtr->statInfo.portno),
              probeTimeout);

    memset(&mlim_addr, 0, sizeof(mlim_addr));
    mlim_addr.sin_family      = AF_INET;
    memcpy(&mlim_addr.sin_addr, &hPtr->addr[0], sizeof(u_int));
    mlim_addr.sin_port        = hPtr->statInfo.portno;

    ch = chanClientSocket_(AF_INET, SOCK_STREAM, 0);
    if (ch < 0 ) {
        ls_syslog(LOG_ERR, "\
%s: chanClientSocket_ failed: %M", __func__);
        return -2;
    }

    rc = chanConnect_(ch, &mlim_addr, probeTimeout * 1000, 0);
    if (rc < 0) {
        ls_syslog(LOG_ERR, "\
%s: chanConnect_ %s failed: %M", __func__, sockAdd2Str_(&mlim_addr));
        return -1;
    }

    ls_syslog(LOG_ERR, "\
%s: chanConnect_ %s ok ", __func__, sockAdd2Str_(&mlim_addr));

    initLSFHeader_(&reqHdr);
    reqHdr.opCode = LIM_PING;
    writeEncodeHdr_(ch, &reqHdr, chanWrite_ );
    chanClose_(ch);

    return rc;
}

