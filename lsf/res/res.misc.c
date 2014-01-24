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
#include <errno.h>
#include <unistd.h>
#include "res.h"
#include "../lim/limout.h"

#define NL_SETN		29

bool_t
xdr_resChildInfo(XDR  *xdrs,
                 struct resChildInfo *childInfo,
                 struct LSFHeader *hdr)
{
   if (!xdr_lsfAuth(xdrs, childInfo->lsfAuth, hdr)
       || !xdr_resConnect(xdrs, childInfo->resConnect, hdr))
      return(FALSE);

   if (!xdr_var_string(xdrs, &childInfo->pw->pw_name)
       || !xdr_var_string(xdrs, &childInfo->pw->pw_dir)
       || !xdr_var_string(xdrs, &childInfo->pw->pw_shell)
       || !xdr_int(xdrs, (int *)&childInfo->pw->pw_uid)
       || !xdr_int(xdrs, (int *)&childInfo->pw->pw_gid)) {
       return(FALSE);
   }

   if (!xdr_var_string(xdrs, &childInfo->host->h_name))
      return(FALSE);

   if (xdrs->x_op == XDR_DECODE)
      childInfo->host->h_aliases = NULL;

   if (!xdr_portno(xdrs, &childInfo->parentPort))
       return(FALSE);

   if (!xdr_int(xdrs, &childInfo->currentRESSN))
       return(FALSE);

   return(TRUE);

}
