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
#include <sys/types.h>
#include "lib.h"
#include "lproto.h"

bool_t 
xdr_resConnect (XDR *xdrs, struct resConnect *connectPtr,
		struct LSFHeader *hdr)
{

    if (!xdr_lenData(xdrs, &connectPtr->eexec)) {
	    return (FALSE);
    }

    return (TRUE);
} 

bool_t 
xdr_niosConnect (XDR *xdrs, struct niosConnect *conn, struct LSFHeader *hdr)
{
    

    if (!(xdr_int(xdrs, &conn->rpid)))
	return (FALSE);

    
    return (xdr_int(xdrs, &conn->exitStatus) &&
            xdr_int(xdrs, &conn->terWhiPendStatus));

    return (TRUE);
}

bool_t 
xdr_niosStatus (XDR *xdrs, struct niosStatus *st, struct LSFHeader *hdr)
{
    struct lsfRusage lsfRu;
   
    memset((char*)&lsfRu, 0, sizeof(lsfRu)); 
    if (!xdr_int(xdrs, (int *) &st->ack))
	return (FALSE);

    if (st->ack != RESE_SIGCHLD)
	return (TRUE);

    if (!xdr_int(xdrs, &st->s.ss))
	return (FALSE);

    

    if (xdrs->x_op == XDR_ENCODE) {
	ls_ruunix2lsf(&(st->s.ru), &lsfRu);
    };
	
    if (!xdr_arrayElement(xdrs, (char *) &lsfRu, hdr, xdr_lsfRusage))
    	return (FALSE);    

    if (xdrs->x_op == XDR_ENCODE)
	return (TRUE);

    

    ls_rulsf2unix(&lsfRu, &(st->s.ru));
    
    return (TRUE);
} 


bool_t 
xdr_resSignal (XDR *xdrs, struct resSignal *sig, struct LSFHeader *hdr)
{
    return (xdr_int(xdrs, &sig->pid) &&
	    xdr_int(xdrs, &sig->sigval));
}

