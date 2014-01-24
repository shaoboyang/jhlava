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
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "echkpnt.lib.h"

extern char *putstr_(const char *);
extern int  initenv_(struct config_param *, char *);

static FILE  *m_pLogFile = NULL;
static char  *m_pMessageHeader = NULL;
static char  logMesgBuf[MAXLINELEN];

char *
getEchkpntMethodDir(char *pChkpnt_Cmd,const char *pMethodDir, const char *pProgramName, const char *pMethodName){

	static char fname[] = "getEchkpntMethodDir()";
	char  tmp_chkpnt_cmd[MAXPATHLEN];
	char *pConfigPath = NULL;
	char *pChkpntBuf = NULL;

	
	struct config_param aParamList[] =
	{
	#define LSF_SERVERDIR    0
		{"LSF_SERVERDIR", NULL},
		{NULL, NULL}
	};

	
	if (pProgramName == NULL){
		sprintf(logMesgBuf,"%s : parameter pProgramName is NULL\n",fname);
		fprintf(stderr,"%s",logMesgBuf);
		logMesg(logMesgBuf);
		return(NULL);
	}
	
	if (pChkpnt_Cmd == NULL){
		pChkpntBuf = tmp_chkpnt_cmd;
	}else{
		pChkpntBuf = pChkpnt_Cmd;
	}
 
	
	pConfigPath = getenv("LSF_ENVDIR");
	if ( pConfigPath == NULL){
		pConfigPath = "/etc";
	}
	if (initenv_(aParamList, pConfigPath) < 0){
		sprintf(logMesgBuf,"%s : initenv_() call fail\n",fname);
		logMesg(logMesgBuf);
	}

	if ((pMethodName != NULL)&&(strcmp(pMethodName,ECHKPNT_DEFAULT_METHOD) != 0)
	     &&(pMethodDir != NULL)){
		strcpy(pChkpntBuf,pMethodDir);
	}else{	
		if (aParamList[LSF_SERVERDIR].paramValue != NULL){
			strcpy(pChkpntBuf, aParamList[LSF_SERVERDIR].paramValue);
		}else{
			sprintf(logMesgBuf,"%s : can't get the LSF_SERVERDIR value\n",fname);
			fprintf(stderr,"%s",logMesgBuf);
			logMesg(logMesgBuf);
			return(NULL);
		}
	}

	
	strcat(pChkpntBuf,"/");
	strcat(pChkpntBuf, pProgramName);
	strcat(pChkpntBuf,".");

	
	if (pMethodName != NULL){
   		strcat(pChkpntBuf, pMethodName);
	}else{
		strcat(pChkpntBuf,ECHKPNT_DEFAULT_METHOD);
	}
	
	if (access(pChkpntBuf, X_OK) < 0){
		sprintf(logMesgBuf, "%s : the %s is not executed \n%s\n",
			fname, pChkpntBuf, errno ? strerror(errno) : "");
		fprintf(stderr,"%s",logMesgBuf);
		logMesg(logMesgBuf);
		return NULL;
	}
	
	if (pChkpnt_Cmd == NULL){
		pChkpntBuf = putstr_(pChkpntBuf);
		if (pChkpntBuf == NULL){
			sprintf(logMesgBuf,"%s : %s malloc dynamic space fail \n%s\n",
				fname,"putstr_()",strerror(errno));
			fprintf(stderr,"%s",logMesgBuf);
			logMesg(logMesgBuf);
		}
	}
	if (aParamList[LSF_SERVERDIR].paramValue != NULL){
		free(aParamList[LSF_SERVERDIR].paramValue);
	}
	return(pChkpntBuf);
}


int  
getChkpntDirFile(char *pPathBuf, const char *pFileName){
	
	static char fname[] = "getChkpntDirFile()";
	char *pChkpntPath = NULL;
	int iIndex;

	if (pPathBuf == NULL){
		return (-1);
	}
	strcpy(pPathBuf,"");
	
	pChkpntPath = getenv(ECHKPNT_CHKPNT_DIR);
	
	
	
	if (pChkpntPath != NULL){
		sprintf(pPathBuf, "%s/" , pChkpntPath);

#ifdef DEBUG
		sprintf(logMesgBuf,"%s : the LSB_CHKPNT_DIR is %s\n",fname,pChkpntPath);
#endif

	}else{
		sprintf(logMesgBuf,"%s : LSB_CHKPNT_DIR is not defined\n",fname);
		logMesg(logMesgBuf);
		return(-1);
	}

	
	iIndex = strlen(pPathBuf);
	if (iIndex > 0){
		while(pPathBuf[--iIndex] != '/');
		iIndex += 2;
		pPathBuf[iIndex] = '\0';
	}
	if (pFileName != NULL){
		char tmpFileName[MAXFILENAMELEN];
		char *pFileNameChar = tmpFileName;
		strcpy(tmpFileName,pFileName);
		while(*(pFileNameChar) == '/'){
			pFileNameChar++;
		}
		strcat(pPathBuf, pFileNameChar);
	}
	return(0);
}


int  
redirectFd(const char *pFileName, int iFileNo){

	static char fname[] = "redirectFd()";
	int   iFd;
	int   ireValue = -1;

	if ((pFileName == NULL) || (iFileNo < 0)){
		return(-1);
	}
	
	
	iFd = open(pFileName,O_WRONLY | O_CREAT | O_APPEND, ECHKPNT_FILE_MODE);
	if ( iFd == -1 ){ 
		sprintf(logMesgBuf, "%s : open() can't open %s file\n%s\n",
			fname, pFileName, errno? strerror(errno) : "");
		logMesg(logMesgBuf);
	 	return(-1);
	}
	
	if (dup2(iFd,iFileNo) != -1){
		ireValue = 0;	
	}else{
		sprintf(logMesgBuf, "%s : dup2() fail\n%s\n",fname,strerror(errno));
		logMesg(logMesgBuf);
		ireValue = -1;
	}
	if (iFd != iFileNo){
		close(iFd);
	}
	return(ireValue);
}


int 
initLog(char *pMesgHeader){
	
	char logFileName[MAXPATHLEN];
	
	if (m_pLogFile == NULL){
		if (getChkpntDirFile(logFileName, ECHKPNT_LOG_FILENAME) != 0){
			return(-1);
		}else{
			m_pLogFile = fopen(logFileName,"a");
			if (m_pLogFile != NULL){
				
				if (m_pMessageHeader != NULL){
					free(m_pMessageHeader);
				}
				m_pMessageHeader = putstr_(pMesgHeader);
				fprintf(m_pLogFile,"########### begin to checkpoint ############\n");
				return(0);
			}
			return(-1);
		}
	}else{
		return(-1);
	}
	
}

void
setMesgHeader(char *pMesgHeader){
	if (m_pLogFile == NULL){
		return;
	}
	if (m_pMessageHeader != NULL){
		free(m_pMessageHeader);
	}
	m_pMessageHeader = putstr_(pMesgHeader);
}


void
closeLog(){
	if (m_pLogFile != NULL){
		fprintf(m_pLogFile,"########### %s end checkpoint ###########\n\n",
			m_pMessageHeader != NULL? m_pMessageHeader : "");
		fclose(m_pLogFile);
	}
	if (m_pMessageHeader != NULL){
		free(m_pMessageHeader);
	}
	
}


void
logMesg(const char *pMessage){
	char timeString[MAX_TIME_STRING];

	time_t tTime;
	if ((m_pLogFile == NULL) || (pMessage == NULL)){
		return;
	}
	
	if (m_pMessageHeader != NULL){
		strcpy(timeString,"");
		tTime = time(NULL);
		if (tTime != (time_t)-1){
			int ii;
			strcpy(timeString,ctime(&tTime));
			ii = strlen(timeString);
			if (timeString[ii -1] == '\n'){ 
				timeString[ii - 1] = '\0';
			}
		}	
		fprintf(m_pLogFile,"%s : %s : %s",timeString,m_pMessageHeader,pMessage);
		fflush(m_pLogFile);
	}
	
}

