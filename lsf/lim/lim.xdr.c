/*
 * Copyright (C) 2011 David Bigagli
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
#include "lim.h"
#include <math.h>

#define NL_SETN 24

#define MAX_FLOAT16 4.227858E+09

static u_short encfloat16_(float );
static float   decfloat16_(u_short);
static void freeResPairs (struct resPair *, int);
static bool_t xdr_resPair (XDR *, struct resPair *, struct LSFHeader *);


bool_t
xdr_loadvector(XDR *xdrs,
               struct loadVectorStruct *lvp,
               struct LSFHeader *hdr)
{
    int i;
    static struct resPair *resPairs;
    static int numResPairs;

    if (!(xdr_int(xdrs, &lvp->hostNo) &&
          xdr_u_int(xdrs, &lvp->seqNo) &&
          xdr_int(xdrs, &lvp->numResPairs) &&
          xdr_int(xdrs, &lvp->checkSum) &&
          xdr_int(xdrs, &lvp->flags) &&
          xdr_int(xdrs, &lvp->numIndx) &&
          xdr_int(xdrs, &lvp->numUsrIndx))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE
        && !limParams[LSF_LIM_IGNORE_CHECKSUM].paramValue) {

        if (myClusterPtr->checkSum != lvp->checkSum) {
            if (limParams[LSF_LIM_IGNORE_CHECKSUM].paramValue == NULL) {
                ls_syslog(LOG_DEBUG, "\
%s: Sender has a different configuration", __func__);
            }
        }

        if (allInfo.numIndx != lvp->numIndx
            || allInfo.numUsrIndx != lvp->numUsrIndx) {

            ls_syslog(LOG_ERR, "\
%s: Sender has a different number of load index vectors. It will be rejected from the cluster by the master host.", __func__);
            return FALSE;
        }
    }

    for (i = 0; i < 1 + GET_INTNUM(lvp->numIndx); i++) {
        if (!xdr_int(xdrs, (int *) &lvp->status[i])) {
            return FALSE;
        }
    }
    if (!xdr_lvector(xdrs, lvp->li, lvp->numIndx)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        freeResPairs (resPairs, numResPairs);
        resPairs = NULL;
        numResPairs = 0;
        if (lvp->numResPairs > 0) {
            resPairs = calloc(lvp->numResPairs,  sizeof(struct resPair));
            lvp->resPairs = resPairs;
        } else
            lvp->resPairs = NULL;
    }
    for (i = 0; i < lvp->numResPairs; i++) {
        if (!xdr_arrayElement(xdrs,
                              (char *)&lvp->resPairs[i],
                              hdr,
                              xdr_resPair)) {
            if (xdrs->x_op == XDR_DECODE) {
                freeResPairs (lvp->resPairs, i);
                resPairs = NULL;
                numResPairs = 0;
            }
            return FALSE;
        }
    }
    if (xdrs->x_op == XDR_DECODE)
        numResPairs = lvp->numResPairs;

    return TRUE;
}

static void
freeResPairs (struct resPair *resPairs, int num)
{
    int i;

    for (i = 0; i < num; i++) {
        FREEUP (resPairs[i].name);
        FREEUP (resPairs[i].value);
    }
    FREEUP (resPairs);
}


static bool_t
xdr_resPair (XDR *xdrs, struct resPair *resPair, struct LSFHeader *hdr)
{
    if (!(xdr_var_string(xdrs, &resPair->name)
          && xdr_var_string(xdrs, &resPair->value)))
        return FALSE;
    return TRUE;

}


bool_t
xdr_loadmatrix(XDR *xdrs, int len, struct loadVectorStruct *lmp,
               struct LSFHeader *hdr)
{
    return(TRUE);
}


bool_t
xdr_masterReg(XDR *xdrs, struct masterReg *masterRegPtr, struct LSFHeader *hdr)
{
    char *sp1;
    char *sp2;

    sp1 = masterRegPtr->clName;
    sp2 = masterRegPtr->hostName;

    if (xdrs->x_op == XDR_DECODE) {
        sp1[0] = '\0';
        sp2[0] = '\0';
    }

    if (! xdr_string(xdrs, &sp1, MAXLSFNAMELEN)
        || !xdr_string(xdrs, &sp2, MAXHOSTNAMELEN)
        || !xdr_int(xdrs,&masterRegPtr->flags)
        || !xdr_u_int(xdrs,&masterRegPtr->seqNo)
        || !xdr_int(xdrs,&masterRegPtr->checkSum)
        || !xdr_portno(xdrs, &masterRegPtr->portno)) {
        return FALSE;
    }

    return TRUE;

}

bool_t
xdr_statInfo(XDR *xdrs, struct statInfo *sip, struct LSFHeader *hdr)
{
    char *sp1, *sp2;
    int cpu_num = 0, i = 0;
    sp1 = sip->hostType;
    sp2 = sip->hostArch;

    if (!( xdr_int(xdrs, &(sip->maxCpus)) &&
           xdr_int(xdrs, &(sip->maxMem)) &&
           xdr_int(xdrs, &(sip->nDisks)) &&
           xdr_portno(xdrs, &(sip->portno)) &&
           xdr_short(xdrs, &(sip->hostNo)) &&
           xdr_int(xdrs, &(sip->maxSwap)) &&
           xdr_int(xdrs, &(sip->maxTmp)) ))
        return(FALSE);


    if (xdrs->x_op == XDR_DECODE) {
        sp1[0]='\0';
        sp2[0]='\0';
    }

    if (!(xdr_string(xdrs, &sp1, MAXLSFNAMELEN) &&
          xdr_string(xdrs, &sp2,  MAXLSFNAMELEN))) {
        return (FALSE);
    }
    if(!xdr_int(xdrs, &(sip->maxPhyCpus)))
        return(FALSE);

    if(!(xdr_int(xdrs, &(sip->tp.socketnum)) &&
         xdr_int(xdrs, &(sip->tp.corenum)) &&
         xdr_int(xdrs, &(sip->tp.threadnum)) &&
         xdr_int(xdrs, &(sip->tp.topologyflag)) ))
        return(FALSE);
    cpu_num = sip->tp.socketnum * sip->tp.corenum * sip->tp.threadnum;
    if(sip->tp.topologyflag == 1){
        if(xdrs->x_op == XDR_DECODE){
            if(sip->tp.topology != NULL){
                free(sip->tp.topology);
                sip->tp.topology = NULL;
            }
            sip->tp.topology = (int*)calloc(cpu_num, sizeof(int));
            if(sip->tp.topology == NULL)
                return (FALSE);
        }
        for(i=0; i<cpu_num; ++i){
            if(!xdr_int(xdrs, &(sip->tp.topology[i]))){
                return (FALSE);           
            }
        }
    }else{
        sip->tp.topology = NULL;
    }
    return(TRUE);
}

#define MIN_FLOAT16  2.328306E-10
static u_short
encfloat16_(float f)
{
    int expo, mant;
    u_short result;
    double temp,fmant;

    temp = f;
    if (temp <= MIN_FLOAT16)
        temp = MIN_FLOAT16;
    if (temp >= INFINIT_LOAD)
        return((u_short)0x7fff);

    fmant = frexp(temp, &expo);

    if (expo < 0)
        expo =  0x20 | (expo & 0x1F);
    else
        expo = expo & 0x1F;

    fmant -= 0.5;
    fmant *= 2048;
    mant   = fmant;

    result = expo << 10;
    result = result + mant;
    return (result);
}

static float
decfloat16_(u_short sf)
{
    int expo, mant;
    double fmant;
    float result;

    if (sf == 0x7fff)
        return(INFINIT_LOAD);

    expo = (sf >> 10) & 0x0000003F;
    if (expo & 0x20)
        expo =  (expo & 0x1F) - 32;
    else
        expo = expo & 0x1F;
    mant = sf & 0x000003FF;
    fmant = (double)mant / 2048;
    fmant += 0.5;
    result = (float)ldexp(fmant, expo);
    if (result <= MIN_FLOAT16)
        result=0.0;
    return(result);
}

bool_t
xdr_lvector(XDR *xdrs, float *li, int nIndices)
{
    u_short indx, temps;
    unsigned int *a;
    int i,j;

    a = malloc(allInfo.numIndx*sizeof(unsigned int));

    if (xdrs->x_op == XDR_ENCODE) {

        for (i = 0, j = 0; i < NBUILTINDEX; i=i+2) {
            indx = encfloat16_(li[i]);
            a[j] = indx << 16;
            if (i == nIndices - 1)
                indx = 0;
            else
                indx = encfloat16_(li[i+1]);
            a[j] = a[j] + indx;
            j++;
        }
    }

    for (i = 0; i < (NBUILTINDEX/2 + 1); i++) {
        if (!xdr_u_int(xdrs, &a[i])) {
            FREEUP (a);
            return (FALSE);
        }
    }

    if (xdrs->x_op == XDR_DECODE) {
        for(i = 0, j = 0; i < NBUILTINDEX; i = i + 2) {
            temps = (a[j] >> 16) & 0x0000ffff;
            li[i] = decfloat16_(temps);
            if (i == NBUILTINDEX - 1)
                break;
            a[j]  = a[j] << 16;
            temps = (a[j] >> 16) & 0x0000ffff;
            li[i+1] = decfloat16_(temps);
            j++;
        }
    }

    for (i = NBUILTINDEX; i<nIndices; i++) {
        if (! xdr_float(xdrs, &li[i])) {
            FREEUP (a);
            return (FALSE);
        }
    }

    FREEUP (a);
    return(TRUE);

}
