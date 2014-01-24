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

#ifndef _LSF_OSAL_H_
#define _LSF_OSAL_H_

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>

typedef int LS_ULONG_T;
typedef int LS_ID_T;
typedef int LS_HANDLE_T;

#if !defined(MAXLINELEN)
#define MAXLINELEN      512
#endif
#define RECORD_LEN      128   
typedef struct lsLimitT_ {
    int          notAvail;      
} LS_LIMIT_T;


typedef struct lsProcStartT_ {
    int          numHds;        
    int*         flgHds;        
    LS_HANDLE_T* handles;       
    int          priority;      
    char*        lsfName;  	
    char*        userName;      
    char*        projName;      
    char*        wrkDir;        
    char*        fromHost;      
    char        *deskTop;        
    LS_LIMIT_T*  limits;        
} LS_PROC_START_T;

#define CHLD_KEEP_STDIN_LS  (0x00000001) 
#define CHLD_KEEP_STDOUT_LS (0x00000002) 
#define CHLD_KEEP_STDERR_LS (0x00000004) 

#define CHLD_KEEP_MSK_LS  (CHLD_KEEP_STDIN_LS          \
                                        | CHLD_KEEP_STDOUT_LS         \
                                        | CHLD_KEEP_STDERR_LS)


#define PROC_OSPRIO_LS(priority)    (priority)  
#define THRD_OSPRIO_LS(priority)    (priority)  
#define PROC_LSPRIO_LS(priority)    (priority)  
#define THRD_LSPRIO_LS(priority)    (priority)  

typedef struct lsProcessT_ {
    LS_ID_T     idThrd;         
    LS_HANDLE_T hdThrd;         
    LS_ID_T     idProc;         
    LS_HANDLE_T hdProc;         
    LS_ID_T     idProcGrp;      
    LS_HANDLE_T hdProcGrp;      
    char*       userName;       
    char*       wrkDir;         
    int         misc;           
} LS_PROCESS_T;

#define MSCAVA_PROC_LS      (0x00000001) 
#define MSCEXT_PROC_LS      (0x00000002) 
#define MSCSTS_PROC_LS      (MSCAVA_PROC_LS \
			     | MSCEXT_PROC_LS) 
#define MSCJOB_PROC_LS      (0x00000004) 

#define PROC_HD_PROCI_LS(procInfo)    ((procInfo)->hdProc)
#define PROC_ID_PROCI_LS(procInfo)    ((procInfo)->idProc)
#define THRD_HD_PROCI_LS(procInfo)    ((procInfo)->hdThrd)
#define THRD_ID_PROCI_LS(procInfo)    ((procInfo)->idThrd)

#define PROC_NEW_GROUP_LS   (0x00000001) 
#define PROC_NOT_INH_HD_LS  (0x00000002) 
#define PROC_JOB_PROC_LS    (0x00000004) 
#define PROC_MAPACCT_LS     (0x00000008) 
#define PROC_INDEP_PROC_LS  (0x00000010) 
#define PROC_NEW_CONSOL_LS  (0x00000020) 
#define PROC_DEBUG_PROC_LS  (0x00000040) 
#define PROC_UNMANAGED_LS   (0x00000080) 
#define PROC_SUSP_PROC_LS   (0x00000100) 
#define PROC_DETACHED_LS    (0x00000200) 
#define PROC_INHERITPRIO_LS (0x00000400) 

#define PROC_JOB_MAN_LS    (0x00000001)  

#define CNTRL_JOB_PROC        (0x10000000)  

#define SEND_GROUP_PROC    (0x00000001)  


typedef struct lsfRusage LS_RUSAGE_T;

typedef void (*LS_THREAD_FUNC_T)(void *);

typedef struct lsThrdStartT_ {
    int         priority;       
    LS_ULONG_T  stackSize;      
} LS_THRD_START_T;

typedef struct lsThreadT_ {
    LS_HANDLE_T hdThrd;         
    LS_ID_T     idThrd;         
} LS_THREAD_T;

#define THRD_HD_THRDI_LS(thrdInfo)    ((thrdInfo)->hdThrd)
#define THRD_ID_THRDI_LS(thrdInfo)    ((thrdInfo)->idThrd)

#define JCNTL_TKNSEND_LS	(1024)
#define JCNTL_TKNACKW_LS	(1025)
#define JCNTL_TKNUSAG_LS	(1026)
#define JCNTL_TKNRDNT_LS	(1027)
#define JCNTL_TKNDENY_LS	(1028)
#define JCNTL_TKNFAIL_LS	(1029)
#define JCNTL_TKNUNKW_LS	(1030)
#define JCNTL_TKNOKEY_LS	(1031)
#define JCNTL_TKNFNSD_LS	(1032)

#define PASSWD_FILE_LS          "passwd.lsfuser"

extern int procStart_(char**, char**, int, 
					  LS_PROC_START_T*, LS_PROCESS_T*);
extern int procLsInit_(int);
extern LS_ID_T procWaitChld_(LS_ID_T, LS_WAIT_T*, LS_RUSAGE_T*, int);
extern int procSendSignal_(LS_ID_T, int, int, int);
extern int procManage_(int);
extern int procEnablePriv_(int);
extern int procGetRusage_(LS_ID_T, struct jRusage *, int);


extern int thrdStart_(LS_THREAD_FUNC_T, void*, int, LS_THRD_START_T*,
                                                        LS_THREAD_T*);
extern LS_HANDLE_T thrdWait_(LS_HANDLE_T, LS_WAIT_T*,  int);

extern int getProcPrioById_(LS_ID_T, int*);
extern int putProcPrioById_(LS_ID_T, int*);
extern int hdInherit_(LS_HANDLE_T *h);
extern int hdNotInherit_(LS_HANDLE_T *h);

#define procLsExit_     exit         	    
#define thrdLsExit_     exit          	    
#define getProcPrioByHd_ getProcPrioById_   
#define putProcPrioByHd_ putProcPrioById_   
#define getThrdPrioByHd_ getProcPrioById_   
#define putThrdPrioByHd_ putProcPrioById_   

#define ERR_BASE_NUM    (4096)              
#define ERR_BAD_ARG     (ERR_BASE_NUM + 0)  
#define ERR_NO_MEM      (ERR_BASE_NUM + 1)  
#define ERR_SYSERR      (ERR_BASE_NUM + 2)  

#define PC_LSF_ANYSTR	"Unknown"	    
#define PC_LSF_CUGID    (-11)               
#define PC_LSF_ANYUGID  (-12)               
#define PC_LSF_FMAPNAME	"!1j8Gp6o$(*7&$@"   
#define PC_LSF_FMAPSIZE	(4096)		    

#endif   

