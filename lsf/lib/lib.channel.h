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
#ifndef CHANNEL_H
#define CHANNEL_H
#include <sys/types.h>
#include "lib.hdr.h"


enum chanState {CH_FREE,
                CH_DISC,
                CH_PRECONN,
                CH_CONN,
                CH_WAIT,
                CH_INACTIVE    
               };

enum chanType {CH_TYPE_UDP, CH_TYPE_TCP, CH_TYPE_LOCAL, CH_TYPE_PASSIVE,
	       CH_TYPE_NAMEDPIPE};

#define CHAN_OP_PPORT  		0x01
#define CHAN_OP_CONNECT		0x02
#define CHAN_OP_RAW		0x04
#define CHAN_OP_NONBLOCK        0x10
#define CHAN_OP_CLOEXEC         0x20
#define CHAN_OP_SOREUSE         0x40


#define CHAN_MODE_BLOCK 	0x01
#define CHAN_MODE_NONBLOCK 	0x02

#define CLOSECD(c) { chanClose_((c)); (c) = -1; }

#define CHAN_INIT_BUF(b)  memset((b), 0, sizeof(struct Buffer));

struct Buffer {
    struct Buffer  *forw;
    struct Buffer  *back;
    char  *data;
    int    pos;
    int    len;
    int stashed;
};

struct Masks {
    fd_set rmask;
    fd_set wmask;
    fd_set emask;
};


struct chanData {
    int  handle;		
    enum chanType type;
    enum chanState state;
    enum chanState prestate;   
    int chanerr; 
    struct Buffer *send;
    struct Buffer *recv;
    
};

#define  CHANE_NOERR      0
#define  CHANE_CONNECTED  1
#define  CHANE_NOTCONN    2
#define  CHANE_SYSCALL    3
#define  CHANE_INTERNAL   4
#define  CHANE_NOCHAN     5
#define  CHANE_MALLOC     6
#define  CHANE_BADHDR     7
#define  CHANE_BADCHAN    8
#define  CHANE_BADCHFD    9
#define  CHANE_NOMSG      10
#define  CHANE_CONNRESET  11

int chanInit_(void);


#define chanSend_  chanEnqueue_
#define chanRecv_  chanDequeue_

int chanOpen_(u_int, u_short, int);
int chanEnqueue_(int chfd, struct Buffer *buf);
int chanDequeue_(int chfd, struct Buffer **buf);

int chanSelect_(struct Masks *, struct Masks *, struct timeval *timeout);
int chanClose_(int chfd);
void chanCloseAll_(void);
int chanSock_(int chfd);

int chanServSocket_(int, u_short, int, int);
int chanAccept_(int, struct sockaddr_in *);

int chanClientSocket_(int, int, int);
int chanConnect_(int, struct sockaddr_in *, int , int);

int chanSendDgram_(int, char *, int , struct sockaddr_in *);
int chanRcvDgram_(int , char *, int, struct sockaddr_in *, int);
int chanRpc_(int , struct Buffer *, struct Buffer *, struct LSFHeader *, int timeout); 
int chanRead_(int, char *, int);
int chanReadNonBlock_(int, char *, int, int);
int chanWrite_(int, char *, int);

int chanAllocBuf_(struct Buffer **buf, int size);
int chanFreeBuf_(struct Buffer *buf);
int chanFreeStashedBuf_(struct Buffer *buf);
int chanOpenSock_(int , int);
int chanSetMode_(int, int);

extern int chanIndex;
extern int cherrno;

#endif

