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


#ifndef ECHKPNT_LIB_H
#define ECHKPNT_LIB_H

#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include "../lsf/lsf.h"
#include "../lsf/lib/lproto.h"


 
#define  ECHKPNT_STDOUT_FILE                  "/echkpnt.out"
#define  ECHKPNT_STDERR_FILE                  "/echkpnt.err"
#define  ERESTART_STDOUT_FILE                 "/erestart.out"
#define  ERESTART_STDERR_FILE                 "/erestart.err"
#define  ECHKPNT_DEFAULT_OUTPUT_FILE          "/dev/null"


     
#define  ECHKPNT_PROGRAM                      "echkpnt"
#define  ERESTART_PROGRAM                     "erestart"


#define  ECHKPNT_METHOD                       "LSB_ECHKPNT_METHOD"
#define  ECHKPNT_METHOD_DIR                   "LSB_ECHKPNT_METHOD_DIR"
#define  ECHKPNT_KEEP_OUTPUT                  "LSB_ECHKPNT_KEEP_OUTPUT"
#define  ECHKPNT_OLD_JOBID                    "LSB_OLD_JOBID"
#define  ECHKPNT_RESTART_USRCMD               "LSB_ERESTART_USRCMD"


#define  ECHKPNT_JOBID			      "LSB_JOBID"         
#define  ECHKPNT_CHKPNT_DIR                   "LSB_CHKPNT_DIR"
#define  ECHKPNT_STDERR_FD                    "LSB_STDERR_FD"
#define  ECHKPNT_JOBFILENAME                  "LSB_CHKFILENAME"

#define  ECHKPNT_ACCT_FILE		      "LSB_ACCT_FILE" 

#define  ECHKPNT_OPEN_OUTPUT                  "Y"
#define  ECHKPNT_OPEN_OUTPUT_L                "y"
#define  ECHKPNT_DEFAULT_METHOD               "default"
#define  ECHKPNT_RESTART_CMD_MARK             "LSB_RESTART_CMD="
#define  ECHKPNT_RESTART_CMD_FILE             ".restart_cmd"
#define  ECHKPNT_LOG_FILENAME                 "chkpnt.log"
#define  PID_MSG_HEADER                       "pid="
#define  PGID_MSG_HEADER                      "pgid="
#define  ECHKPNT_NEWJOBFILE_SUFFIX            ".restart"

#define  ECHKPNT_FILE_MODE                    (S_IRUSR | S_IWUSR)

#define  MAX_ARGS			      16
#define  MAX_TIME_STRING		      50

char  *getEchkpntMethodDir(char *, const char *, const char *, const char *);
int   getChkpntDirFile(char *, const char *);
int   redirectFd(const char *, int );
int   initLog(char *);
void  logMesg(const char *);
void  closeLog();
void  setMesgHeader(char *);


#endif 
