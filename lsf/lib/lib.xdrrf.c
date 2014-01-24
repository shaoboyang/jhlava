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

#include <limits.h>
#include "lib.h"
#include "lib.xdr.h"
#include "lproto.h"

extern int currentSN;
    
int
lsRecvMsg_(int sock, char *buf, int bufLen, struct LSFHeader *hdr,
	   char *data, bool_t (*xdrFunc)(), int (*readFunc)())
{
    XDR xdrs;
    int cc;

    xdrmem_create(&xdrs, buf, bufLen, XDR_DECODE);
    
    if ((cc = readDecodeHdr_(sock, buf, readFunc, &xdrs, hdr)) < 0) {
	xdr_destroy(&xdrs);
	return (cc);
    }

    if (hdr->length == 0 || data == NULL) {
	xdr_destroy(&xdrs);
	return (0);
    }

    XDR_SETPOS(&xdrs, 0);

    if ((cc = readDecodeMsg_(sock, buf, hdr, readFunc, &xdrs, data,
			      xdrFunc, NULL))	< 0) {
	xdr_destroy(&xdrs);
	return (cc);
    }

    return (0);
} 
	
int lsSendMsg_ (int s, int opCode, int hdrLength, char *data, char *reqBuf,
		int reqLen, bool_t (*xdrFunc)(), int (*writeFunc)(),
		struct lsfAuth *auth)
{
    struct LSFHeader hdr;
    XDR xdrs;

    initLSFHeader_(&hdr);
    hdr.opCode = opCode;
    hdr.refCode = currentSN;

    if (!data) 
	hdr.length = hdrLength;

    xdrmem_create(&xdrs, reqBuf, reqLen, XDR_ENCODE);

    if (!xdr_encodeMsg(&xdrs,
                       data,
                       &hdr,
                       xdrFunc,
		       (data == NULL) ? ENMSG_USE_LENGTH : 0,
                       auth)) {
	xdr_destroy(&xdrs);
	lserrno = LSE_BAD_XDR;
	return(-1);
    }

    if ((*writeFunc)(s, (char *)reqBuf, XDR_GETPOS(&xdrs)) !=
	XDR_GETPOS(&xdrs)) {
        xdr_destroy(&xdrs);
	lserrno = LSE_MSG_SYS;
        return (-2);
    }
    
    xdr_destroy(&xdrs);    

    return (0);
} 
