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
#include <errno.h>

#include <pwd.h>
#include <syslog.h>

#include <fcntl.h>

#include <math.h>
#include <limits.h>

#include "./lsi18n.h"

#define UTMP_FILENAME "/var/adm/utmpx"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif 

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif 


int
createUtmpEntry( char* uname, pid_t job_pid, char* current_tty ) {

    int err=0;

    return( err );

} 



int
removeUtmpEntry( pid_t job_pid) {

    return(0);

} 
