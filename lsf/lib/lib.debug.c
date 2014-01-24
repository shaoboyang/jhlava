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

#include <unistd.h>
#include <fcntl.h>

#include "lib.h"
#include "lproto.h"

int
ls_initdebug (char *appName)
{
    char *logMask;
    struct config_param *pPtr;
    struct config_param debParams[] = {
        {"LSF_DEBUG_CMD", NULL},
        {"LSF_TIME_CMD", NULL},
        {"LSF_CMD_LOGDIR", NULL},
        {"LSF_CMD_LOG_MASK", NULL},
        {"LSF_LOG_MASK", NULL},
        {NULL, NULL}
    };

#define LSF_DEBUG_CMD    0
#define LSF_TIME_CMD     1
#define LSF_CMD_LOGDIR   2
#define LSF_CMD_LOG_MASK 3

#ifdef LSF_LOG_MASK
#undef LSF_LOG_MASK
#endif
#define LSF_LOG_MASK     4


    if (initenv_(debParams, NULL) < 0)
        return -1;                         

    if (debParams[LSF_CMD_LOG_MASK].paramValue != NULL)
        logMask = debParams[LSF_CMD_LOG_MASK].paramValue;
    else
        logMask = debParams[LSF_LOG_MASK].paramValue;

    if (appName == NULL)
        ls_openlog("lscmd", debParams[LSF_CMD_LOGDIR].paramValue,
              (debParams[LSF_CMD_LOGDIR].paramValue == NULL), logMask);
    else {
		if (strrchr(appName, '/') != 0)
			appName = strrchr(appName, '/')+1;
        ls_openlog(appName, debParams[LSF_CMD_LOGDIR].paramValue,
              (debParams[LSF_CMD_LOGDIR].paramValue == NULL), logMask);
	}

    getLogClass_(debParams[LSF_DEBUG_CMD].paramValue,
                 debParams[LSF_TIME_CMD].paramValue);

    for (pPtr = debParams; pPtr->paramName != NULL; pPtr++)
        FREEUP (pPtr->paramValue);

    return 0;

} 

