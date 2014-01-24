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
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>

#include "echkpnt.lib.h"
#include "echkpnt.env.h"

#define LSTMPDIR        lsTmpDir_

static void usage(const char *);

static char logMesgBuf[MAXLINELEN];
extern char *lsTmpDir_;
static void 
usage (const char *pCmd)
{
    fprintf(stderr, "Usage: %s [-c] [-f] [-k | -s] [-x] [-d chkdir] [-h] [-V] pid\n", pCmd);
}


int 
main(int argc, char **argv){

	static char fname[] = "main()";
   	 
	char  *pMethodName = NULL;
	char  *pMethodDir = NULL;
	char  *pIsOutput = NULL;
	char  *pJobID = NULL;

        char  *presAcctFN = NULL;
	char  resAcctFN[MAXFILENAMELEN];

	char  echkpntProgPath[MAXPATHLEN];     	
	char  *pChkpntDir = NULL;		 
	pid_t   iChildPid;
	LS_WAIT_T  iStatus;			 
    	char *cargv[MAX_ARGS];
	int  argIndex = 0;
	int  cc;
	int iReValue; 
	char  *jf, *jfn;

	 
	while ((cc = getopt(argc, argv, "cfksd:Vhx")) != EOF ){ 
		switch(cc){
		case 'c':
		case 'f':
		case 'k':
		case 'x':
		case 's':
			break;
		case 'V':
 			fputs(_LS_VERSION_, stderr);
			exit(0);
		case 'd': 
			pChkpntDir = optarg;
			break;
		case 'h':
			usage(argv[0]);
			exit(0);
		default:
			usage(argv[0]);
			exit(-1);
		}
	} 

	
	if (pChkpntDir == NULL){ 
		usage(argv[0]);
		exit(-1); 
	} 
	
	if (access(pChkpntDir,W_OK | X_OK) < 0){
		fprintf(stderr,"%s : the chkpnt dir %s can not be accessed\n",
			fname,pChkpntDir);
		exit(-1);
	}

	 
	iReValue = fileIsExist(pChkpntDir);
	
	if (iReValue == 1){

		
		pMethodName = getEchkpntVar(ECHKPNT_METHOD);
		
		if  ((pMethodName == NULL)||(strcmp(pMethodName,"") == 0)){
			pMethodName = ECHKPNT_DEFAULT_METHOD;
		}

		pMethodDir = getEchkpntVar(ECHKPNT_METHOD_DIR);
		pIsOutput = getEchkpntVar(ECHKPNT_KEEP_OUTPUT);
	
	}else if (iReValue == 0) { 
		
		
		initenv_(NULL,NULL);
		pMethodName = getenv(ECHKPNT_METHOD);
		
		if  ((pMethodName == NULL)||(strcmp(pMethodName,"") == 0)){
			pMethodName = ECHKPNT_DEFAULT_METHOD;
		}
		pMethodDir = getenv(ECHKPNT_METHOD_DIR);
		pIsOutput = getenv(ECHKPNT_KEEP_OUTPUT);
	
	        {
		
		jf = getenv("LSB_JOBFILENAME");
		if (jf == NULL) {
		    fprintf(stderr, "%s : getenv of LSB_JOBFILENAME failed", fname);
                } else {
		    jfn = strchr (jf, '/');
		    if (jfn) {
		        sprintf(resAcctFN, "%s/.%s.acct",LSTMPDIR,jfn+1);
			presAcctFN = (char *) resAcctFN;
                    }
                }
                }
		 
		writeEchkpntVar(ECHKPNT_METHOD, pMethodName);
		writeEchkpntVar(ECHKPNT_METHOD_DIR, pMethodDir);
		writeEchkpntVar(ECHKPNT_KEEP_OUTPUT,pIsOutput);
		writeEchkpntVar(ECHKPNT_ACCT_FILE,presAcctFN);
	}else{
		fprintf(stderr, "%s : the .echkpnt file content occurs error: %s\n",fname,strerror(errno));
		exit(-1);
	}
					  
	
        if ((pIsOutput != NULL)&&((strcmp(pIsOutput,ECHKPNT_OPEN_OUTPUT) == 0)
            ||(strcmp(pIsOutput,ECHKPNT_OPEN_OUTPUT_L) == 0))) {
                initLog("Echkpnt");
        }

#ifdef DEBUG
	sprintf(logMesgBuf,"%s : the LSB_ECHKPNT_METHOD = %s\n",fname,pMethodName);
	logMesg(logMesgBuf);
	sprintf(logMesgBuf,"%s : the LSB_ECHKPNT_METHOD_DIR = %s\n",fname,pMethodDir != NULL?pMethodDir : "");
	logMesg(logMesgBuf);
#endif

	 
	if (getEchkpntMethodDir(echkpntProgPath, pMethodDir, ECHKPNT_PROGRAM, pMethodName) == NULL){
		sprintf(logMesgBuf, "%s : the echkpnt method(%s) path is not correct\n",fname,pMethodName);
		goto Error;
	}

#ifdef DEBUG
        sprintf(logMesgBuf,"%s : the echkpntProgPath is : %s\n",fname,echkpntProgPath);
        logMesg(logMesgBuf);
#endif

	for(argIndex = 0;argIndex < argc; argIndex++){
		cargv[argIndex] = argv[argIndex];
	}
	cargv[argIndex] = NULL;
		
	 
    	if (strcmp(pMethodName, ECHKPNT_DEFAULT_METHOD) == 0){
		cargv[0] = "echkpnt.default";
		freeTable_();

#ifdef DEBUG
                logMesg("the echkpnt.default will be executed\n");
#endif
		closeLog();
		execv(echkpntProgPath, cargv);
		sprintf(logMesgBuf,"%s : execute the echkpnt.default fail\n%s\n",fname,
                        (errno? strerror(errno):""));
                fprintf(stderr,"%s",logMesgBuf);
		exit(-1);
	} 
	
	 
	iChildPid = fork();
	if (iChildPid < 0){
		sprintf(logMesgBuf, "%s : fork() fork a child process fail...\n",fname);
		goto Error;
	
	
	}else if (iChildPid == 0){
		long lMaxfds;
		int ii;
		char progName[MAXFILENAMELEN];
	
		
		sprintf(logMesgBuf,"erestart.%s",pMethodName);	
		setMesgHeader(logMesgBuf);

		
		if ((pIsOutput == NULL) || ((strcmp(pIsOutput,ECHKPNT_OPEN_OUTPUT) != 0)
		    &&(strcmp(pIsOutput,ECHKPNT_OPEN_OUTPUT_L) != 0))){
			if (redirectFd(ECHKPNT_DEFAULT_OUTPUT_FILE, 1) == -1){
				sprintf(logMesgBuf, "%s : redirect stdout to %s file\n%s\n", 
					fname, ECHKPNT_DEFAULT_OUTPUT_FILE, 
					errno? strerror(errno) : "");
				goto Error;
			}
			if (redirectFd(ECHKPNT_DEFAULT_OUTPUT_FILE, 2) == -1){
				sprintf(logMesgBuf, "%s : redirect stderr to %s file\n%s\n", 
					fname, ECHKPNT_DEFAULT_OUTPUT_FILE, 
					errno? strerror(errno) : "");
				goto Error;
			}
		
		}else{
			char aFileName[MAXPATHLEN];
			

			
                        if ((getChkpntDirFile(aFileName, ECHKPNT_STDOUT_FILE) == -1)
                            ||  (redirectFd(aFileName,1) == -1)){
				sprintf(logMesgBuf,"%s : redirect the stdout to %s fail\n",
					fname,ECHKPNT_STDOUT_FILE);
                                logMesg(logMesgBuf);
				
				if (redirectFd(ECHKPNT_DEFAULT_OUTPUT_FILE, 1) == -1){
					sprintf(logMesgBuf, "%s : redirect stdout to %s file fail\n%s\n", 
						fname, ECHKPNT_DEFAULT_OUTPUT_FILE, 
						errno? strerror(errno) : "");
					goto Error;
				}
			}

			if ((getChkpntDirFile(aFileName, ECHKPNT_STDERR_FILE) == -1)
                            ||  (redirectFd(aFileName,2) == -1)){
				sprintf(logMesgBuf,"%s : redirect the stderr to %s fail\n",
					fname,ERESTART_STDERR_FILE);
                                logMesg(logMesgBuf);
					
				if (redirectFd(ECHKPNT_DEFAULT_OUTPUT_FILE, 2) == -1){
					sprintf(logMesgBuf, "%s : redirect stderr to %s file fail\n%s\n", 
						fname, ECHKPNT_DEFAULT_OUTPUT_FILE, 
						errno? strerror(errno) : "");
					goto Error;
				}
			}
		}
		
		lMaxfds = sysconf(_SC_OPEN_MAX);
	        for (ii = 3; ii < lMaxfds; ii++){
			close(ii);
		}

		sprintf(progName,"%s.%s",ECHKPNT_PROGRAM,pMethodName);
		cargv[0] = progName;
		freeTable_();

		execv(echkpntProgPath,cargv);
 		sprintf(logMesgBuf, "%s : the child process execute the %s fail\n",fname,progName);
                fprintf(stderr,"%s",logMesgBuf);
                logMesg(logMesgBuf);
                closeLog();
		exit(-1);
	} 
  
	

	
	while ((iChildPid = waitpid(iChildPid, &iStatus, 0)) < 0 && errno == EINTR);

	if (iChildPid < 0 ){
		sprintf(logMesgBuf, "%s : %s fail, \n%s\n", 
				fname, "waitpid", errno? strerror(errno) : "");
		goto Error;
	}else{
		if (WEXITSTATUS(iStatus) != 0) {
			sprintf(logMesgBuf, "%s : the echkpnt.%s fail,the exit value is %d\n", 
				fname, pMethodName,WEXITSTATUS(iStatus));
			fprintf(stderr,"%s",logMesgBuf);
                        logMesg(logMesgBuf);
                        freeTable_();
                        closeLog();
			exit(WEXITSTATUS(iStatus));
		}
	} 

   	 
	pJobID = getenv(ECHKPNT_JOBID);
	if (pJobID == NULL){
		writeEchkpntVar(ECHKPNT_OLD_JOBID,"");
		sprintf(logMesgBuf,"%s : getenv() can not get the env variable LSB_JOBID\n",fname);
		logMesg(logMesgBuf);
	}else{
		writeEchkpntVar(ECHKPNT_OLD_JOBID, pJobID);
	}
	freeTable_(); 
	closeLog();
	exit(0);

Error:

	fprintf(stderr,"%s",logMesgBuf);
        logMesg(logMesgBuf);
        freeTable_();
        closeLog();
        exit(-1);

} 
  
