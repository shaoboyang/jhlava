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
#include <errno.h>
#include <unistd.h>


#define SYSV_NICE_0              20
#define MAX_PRIORITY             20
#define MIN_PRIORITY            -20

#define LSF_TO_SYSV(x)          ((x) + SYSV_NICE_0)
#define SYSV_TO_LSF(x)          ((x) - SYSV_NICE_0)

int
ls_setpriority(int newPriority)
{
    int increment;

    if ( newPriority > MAX_PRIORITY * 2) {
        newPriority = MAX_PRIORITY * 2;
    } else if ( newPriority < MIN_PRIORITY * 2) {
        newPriority = MIN_PRIORITY * 2;
    }
    increment = newPriority;

    errno = 0;


    if (-1 == nice(increment) && (0 != errno)) {
        return 0;
    }

    return 1;
}

