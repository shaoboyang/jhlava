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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "lib.h"
#include "lib.xdrres.h"
#include "lproto.h"
#include "lib.rf.h"

bool_t 
xdr_resCmdBill (XDR *xdrs, struct resCmdBill *cmd, struct LSFHeader *hdr)
{
    char *sp;
    int i, argc, nlimits;

    if (!(xdr_portno(xdrs, &cmd->retport) &&
	  xdr_int(xdrs, &cmd->rpid) &&
	  xdr_int(xdrs, &cmd->filemask) &&
	  xdr_int(xdrs, &cmd->priority) &&
	  xdr_int(xdrs, &cmd->options)))
	return (FALSE);

    sp = cmd->cwd;
    if (xdrs->x_op == XDR_DECODE)
	sp[0] = '\0';

    if (!xdr_string(xdrs, &sp, MAXPATHLEN))
	return (FALSE);

    if (xdrs->x_op == XDR_ENCODE)
	for (argc=0; cmd->argv[argc]; argc++);
    
    if (!xdr_int(xdrs, &argc))
	return (FALSE);
    
    if (xdrs->x_op == XDR_DECODE) {
	if ((cmd->argv = (char **) calloc(argc + 1, sizeof(char *))) == NULL)
	    return (FALSE);
    }
	
    for (i=0; i<argc; i++) {
	if (!xdr_var_string(xdrs, &(cmd->argv[i]))) {
            if (xdrs->x_op == XDR_DECODE) {
		while (i--)
		    free(cmd->argv[i]);
		free(cmd->argv);
	    }
	    return (FALSE);
	}
    }

    if (xdrs->x_op == XDR_DECODE)
	cmd->argv[argc] = NULL;
    else
	nlimits = LSF_RLIM_NLIMITS;

    if (!xdr_int(xdrs, &nlimits)) {
	if (xdrs->x_op == XDR_DECODE)
	    goto DecodeQuit;
	else
	    return (FALSE);
    }

    
    
    if (xdrs->x_op == XDR_DECODE)
	nlimits = (nlimits < LSF_RLIM_NLIMITS) ? nlimits : LSF_RLIM_NLIMITS;

    for (i=0; i<nlimits; i++) {
	if (!xdr_arrayElement(xdrs, (char *) &(cmd->lsfLimits[i]), hdr,
			      xdr_lsfLimit)) {
	    if (xdrs->x_op == XDR_DECODE)
		goto DecodeQuit;
	    else
		return (FALSE);
	}
    }

    return (TRUE);
    
  DecodeQuit:
    for (i=0; i<argc; i++)
	free(cmd->argv[i]);
    free(cmd->argv);
    return (FALSE);
}


bool_t 
xdr_resSetenv (XDR *xdrs, struct resSetenv *envp, struct LSFHeader *hdr)
{
    char *sp;
    int i, nenv;

    if (xdrs->x_op == XDR_ENCODE)
	for (nenv=0; envp->env[nenv]; nenv++);

    if (!xdr_int(xdrs, &nenv))
	return(FALSE);

    if (xdrs->x_op == XDR_DECODE) {
	if ((envp->env = (char **) calloc(nenv + 1, sizeof(char *))) == NULL)
	    return (FALSE);
    }
    
    for (i=0; i<nenv; i++) {
	if (xdrs->x_op == XDR_DECODE) {
	    if (!xdr_var_string(xdrs, &envp->env[i])) {
		while (i--)
		    free(envp->env[i]);
		free(envp->env);
		return (FALSE);
	    }
	} else { 
	    sp = envp->env[i]; 

            
            if (strncmp(sp, "DISPLAY=", 8) == 0) {
                sp = chDisplay_(sp);
            }

	    if (!xdr_var_string(xdrs, &sp))
		return (FALSE);
	}
    }
    
    if (xdrs->x_op == XDR_DECODE)
	envp->env[nenv] = NULL;

    return (TRUE);
} 

bool_t 
xdr_resRKill (XDR *xdrs, struct resRKill *rkill, struct LSFHeader *hdr)
{
    if (! (xdr_int(xdrs, &rkill->rid) &&
	   xdr_int(xdrs, &rkill->whatid) &&
	   xdr_int(xdrs, &rkill->signal)))
	return (FALSE);

    return (TRUE);
}

bool_t 
xdr_resGetpid (XDR *xdrs, struct resPid *pidreq, struct LSFHeader *hdr)
{
    if (! (xdr_int(xdrs, &pidreq->rpid) &&
	   xdr_int(xdrs, &pidreq->pid)))
	return (FALSE);

    return (TRUE);
} 

bool_t 
xdr_resGetRusage (XDR *xdrs, struct resRusage *rusageReq,struct LSFHeader *hdr)
{
    if (! (xdr_int(xdrs, &rusageReq->rid) 
	   && xdr_int(xdrs, &rusageReq->whatid)
	   && xdr_int(xdrs, &rusageReq->options))) {
	return (FALSE);
    }

    return (TRUE);
} 


bool_t 
xdr_resChdir (XDR *xdrs, struct resChdir *ch, struct LSFHeader *hdr)
{
    char *sp = ch->dir;
    
    if (xdrs->x_op == XDR_DECODE)
	sp[0] = '\0';

    if (!xdr_string(xdrs, &sp, MAXFILENAMELEN))
	return (FALSE);

    
    return (TRUE);
}


bool_t 
xdr_resControl (XDR *xdrs, struct resControl *ctrl, struct LSFHeader *hdr)
{
    if (! (xdr_int(xdrs, &ctrl->opCode) 
           && xdr_int(xdrs, &ctrl->data))) 
	return (FALSE);
    return (TRUE);
}


bool_t 
xdr_resStty (XDR *xdrs, struct resStty *tty, struct LSFHeader *hdr)
{
    if (xdrs->x_op == XDR_ENCODE) {
	if (!encodeTermios_(xdrs, &tty->termattr))
	    return (FALSE);
    } else {
	if (!decodeTermios_(xdrs, &tty->termattr))
	    return (FALSE);
    }

    
    if (! (xdr_u_short(xdrs, &tty->ws.ws_row) &&
	   xdr_u_short(xdrs, &tty->ws.ws_col) &&
	   xdr_u_short(xdrs, &tty->ws.ws_xpixel) &&
	   xdr_u_short(xdrs, &tty->ws.ws_ypixel)))
	return (FALSE);
    
    return (TRUE);
}

bool_t 
xdr_ropenReq (XDR *xdrs, struct ropenReq *req, struct LSFHeader *hdr)
{
    char *sp = req->fn;
    int len, flags;

    if (xdrs->x_op == XDR_DECODE) {
	len = MAXFILENAMELEN;
	sp[0] = '\0';
    } else {
	len = strlen(req->fn) + 1;
    }

    if (!xdr_string(xdrs, &sp, len)) {
	return (FALSE);
    }

    if (xdrs->x_op == XDR_ENCODE) {
        flags = 0;
	if (req->flags & O_WRONLY)
	    flags |= LSF_O_WRONLY;
	if (req->flags & O_RDWR)
	    flags |= LSF_O_RDWR;
#ifdef O_NDELAY	
	if (req->flags & O_NDELAY)
	    flags |= LSF_O_NDELAY;
#endif 	
#ifdef O_NONBLOCK
	if (req->flags & O_NONBLOCK)
	    flags |= LSF_O_NONBLOCK;
#endif 
	if (req->flags & O_APPEND)
	    flags |= LSF_O_APPEND;
	if (req->flags & O_CREAT)
	    flags |= LSF_O_CREAT;
	if (req->flags & O_TRUNC)
	    flags |= LSF_O_TRUNC;
	if (req->flags & O_EXCL)
	    flags |= LSF_O_EXCL;
#ifdef O_NOCTTY
	if (req->flags & O_NOCTTY)
	    flags |= LSF_O_NOCTTY;
#endif 
	if (req->flags & LSF_O_CREAT_DIR) {
	    flags |= LSF_O_CREAT_DIR;
        }
	if (!xdr_int(xdrs, &flags))
	    return (FALSE);
    } else {
	if (!xdr_int(xdrs, &flags))
	    return (FALSE);
	req->flags = 0;
	if (flags & LSF_O_WRONLY)
	    req->flags |= O_WRONLY;
	if (flags & LSF_O_RDWR)
	    req->flags |= O_RDWR;
#ifdef O_NDELAY	
	if (flags & LSF_O_NDELAY)
	    req->flags |= O_NDELAY;
#endif 	
#ifdef O_NONBLOCK
	if (flags & LSF_O_NONBLOCK)
	    req->flags |= O_NONBLOCK;
#endif 
	if (flags & LSF_O_APPEND)
	    req->flags |= O_APPEND;
	if (flags & LSF_O_CREAT)
	    req->flags |= O_CREAT;
	if (flags & LSF_O_TRUNC)
	    req->flags |= O_TRUNC;
	if (flags & LSF_O_EXCL)
	    req->flags |= O_EXCL;
#ifdef O_NOCTTY
	if (flags & LSF_O_NOCTTY)
	    req->flags |= O_NOCTTY;
#endif 
	if (flags & LSF_O_CREAT_DIR) {
	    req->flags |= LSF_O_CREAT_DIR;
        }
    }

    if (!xdr_int(xdrs, &req->mode))
	return (FALSE);

    return (TRUE);
} 

bool_t 
xdr_rrdwrReq (XDR *xdrs, struct rrdwrReq *req, struct LSFHeader *hdr)
{
    return (xdr_int(xdrs, &req->fd) && xdr_int(xdrs, &req->len));
} 

bool_t 
xdr_rlseekReq (XDR *xdrs, struct rlseekReq *req, struct LSFHeader *hdr)
{
    int whence = 0;
#define LSF_SEEK_SET 0x1
#define LSF_SEEK_CUR 0x2
#define LSF_SEEK_END 0x4    
    
    if (xdrs->x_op == XDR_ENCODE) {
	if (req->whence & SEEK_SET)
	    whence |= LSF_SEEK_SET;
	if (req->whence & SEEK_CUR)
	    whence |= LSF_SEEK_CUR;
	if (req->whence & SEEK_END)
	    whence |= LSF_SEEK_END;
    }

    if (!(xdr_int(xdrs, &req->fd) && xdr_int(xdrs, &whence) &&
	  xdr_int(xdrs, &req->offset)))
	return (FALSE);

    if (xdrs->x_op == XDR_DECODE) {
	req->whence = 0;
	if (whence & LSF_SEEK_SET)
	    whence |= SEEK_SET;
	if (whence & LSF_SEEK_CUR)
	    req->whence |= SEEK_CUR;
	if (whence & LSF_SEEK_END)
	    req->whence |= SEEK_END;
    }

    return (TRUE);
} 

bool_t
xdr_noxdr(XDR *xdrs, int size, struct LSFHeader *header)
{
    XDR_SETPOS(xdrs, size + LSF_HEADER_LEN);
    return (TRUE);
} 
     

