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
#include <stdlib.h>
#include <errno.h>

#include "echkpnt.lib.h"
#include "echkpnt.env.h"

extern char *putstr_(const char *);

static int fillTable_();
static int addNewVarToFile_(const char *,const char *);
static int updateVarToFile_();
static int insertVarToTable_(const char *, const char *);


static  VAR_TABLE_ITEM_T  *m_pTableHeader = NULL;      
static  VAR_TABLE_ITEM_T  *m_pTableTail = NULL;	       
static  char  	          *m_pEnvVarFileName = NULL;   

int 
fileIsExist(const char *pWorkPath){

	static char fname[] = "fileIsExist()";
	
	
	static char pFileName[MAXPATHLEN];
	struct stat statbuf;

	if  (m_pEnvVarFileName == NULL)  {
		
		if ( getChkpntDirFile(pFileName, ECHKPNT_VAR_FILE) == -1 ){
			

		}
		m_pEnvVarFileName = pFileName;
	}
	
	if (strlen(m_pEnvVarFileName) == 0){ 
		return(-1);
	}

	
	if (pWorkPath != NULL){
		char  pWorkFileName[MAXPATHLEN];
		
		strcpy(pWorkFileName,"");
		
		if (*pWorkPath != '/'){
			if (getcwd(pWorkFileName,MAXPATHLEN) == NULL){
				return(-1);
			}
		}

		
		if (*pWorkPath != '/'){
			strcat(pWorkFileName,"/");
		}
		strcat(pWorkFileName,pWorkPath);
		strcat(pWorkFileName,ECHKPNT_VAR_FILE);
		if (strcmp(pWorkFileName,m_pEnvVarFileName) != 0){

			
			if ( access(m_pEnvVarFileName, F_OK)== 0 ){
				

				FILE  *pDest = NULL;
				FILE  *pSource = NULL;
				if ((pSource = fopen(m_pEnvVarFileName,"r")) != NULL){
					if ((pDest = fopen(pWorkFileName,"w")) != NULL){
						int c;
						while((c = fgetc(pSource)) != EOF){
							if (fputc(c,pDest) == EOF){
									
								break;
							}
						}
						if (ferror(pSource)){
							
						}

					}else{
						
					}
                
				}else{
					
				}

				
				if (pSource != NULL){
					fclose(pSource);
				}
				if (pDest != NULL){
					fclose(pDest);
				}
			}
			
			
			strcpy(m_pEnvVarFileName, pWorkFileName);
		}
	}
	
	if (stat(m_pEnvVarFileName, &statbuf) < 0){
		
		if (errno == ENOENT){
			return(0);
		}else{
			return(-1);
		}
	}

	
	if (!(S_ISREG(statbuf.st_mode))) {
		fprintf(stderr,"%s : %s is not a regular file\n",fname,m_pEnvVarFileName);
		return(-1);
	}

	
	if (fillTable_() == -1){
		return(-1);
	}else{
		return(1);
	}
}


char *
getEchkpntVar(const char *pVariableName) {

	VAR_TABLE_ITEM_T *pTableItem = NULL;

	
	for (pTableItem = m_pTableHeader; pTableItem != NULL; pTableItem = pTableItem->m_pNextItem) {
		
		if ((pTableItem->m_pVarPair != NULL) 
		    && (pTableItem->m_pVarPair->m_pVariable != NULL) 
		    && (strcmp( pTableItem->m_pVarPair->m_pVariable,pVariableName) == 0)){

		    return(pTableItem->m_pVarPair->m_pValue);
		}
	}
	return(NULL);
}



int 
writeEchkpntVar(const char *pVariableName, const char *pVariableValue){

	
	int iReValue = -1;
	if (( pVariableName == NULL) || ( pVariableValue == NULL )){
		return -1;
	}

	
	iReValue = insertVarToTable_(pVariableName,pVariableValue);
	if (iReValue == 0) {
		return(addNewVarToFile_(pVariableName,pVariableValue));
	}else if (iReValue == 1) {
		return(updateVarToFile_());
    	}else if (iReValue == 2){
		return(0);
	}else{
		
		return(-1);
	}
}



static int 
fillTable_( ){

	FILE   *pFile = NULL;
    	char   line[MAXLINELEN];
	char   *pCurChar = NULL;
	char   *pWord = NULL;

	
	if (m_pEnvVarFileName == NULL){
		
    		return(-1);
	}

	
	pFile = fopen(m_pEnvVarFileName,"r");
    	if (pFile == NULL){
		
		return(-1);
	}

	
	while (fgets(line,MAXLINELEN,pFile) != NULL) {
		pWord = pCurChar = line;
		
        	if (!isalpha(*pCurChar)){
			
			
			goto error;
		}

		
		pCurChar = strchr(pWord,'=');
        	if (pCurChar == NULL){
			
			goto error;
		}
		pWord = pCurChar + 1;
        	*(pCurChar) = '\0';
		
		
		
		if (*(pWord) != '\"'){
			goto error;
		}
		pWord++;
		
		
		pCurChar = strchr(pWord,'\"');
		if (pCurChar == NULL){
			goto error;
		}
		*(pCurChar) = '\0';
		
		
		if (insertVarToTable_(line,pWord) == -1){
			goto error;
		}
	}
	return(0);

error:

	freeTable_();
	return(-1);
}


static int 
addNewVarToFile_(const char *pVariableName, const char *pVariableValue){
	FILE *pFile = NULL;

	if (m_pEnvVarFileName == NULL){
		return(-1);
	}
	
	pFile = fopen(m_pEnvVarFileName,"a");
	if (pFile == NULL){
		return(-1);
	}

	
	fprintf(pFile,"%s=\"%s\"\n",pVariableName,pVariableValue);
	fclose(pFile);
    	return(0);
}


static int 
updateVarToFile_(){

	char tmpFileName[MAXPATHLEN];
	FILE *pTmpFile = NULL;
	VAR_TABLE_ITEM_T *pTableItem = NULL;
	
	if (m_pEnvVarFileName == NULL){
		return(-1);
	}

	
	strcpy(tmpFileName, m_pEnvVarFileName);
	strcat(tmpFileName,".tmp");

	
	pTmpFile = fopen(tmpFileName,"w");
	if (pTmpFile == NULL){
		return(-1);
	}
	
	for (pTableItem = m_pTableHeader; pTableItem != NULL; pTableItem = pTableItem->m_pNextItem){
		VAR_PAIR_T *pVarPair = pTableItem->m_pVarPair;
		
		if ((pVarPair!=NULL)
		    &&(pVarPair->m_pVariable != NULL)
		    &&(pVarPair->m_pValue != NULL)){
        		fprintf(pTmpFile,"%s=\"%s\"\n",pVarPair->m_pVariable,
				pVarPair->m_pValue);
		}
   	}
	fclose(pTmpFile);

	
	if (rename(tmpFileName, m_pEnvVarFileName) == -1){
		return(-1);
	}
	return(0);
}


static int 
insertVarToTable_(const char *pVariableName, const char *pVariableValue){

	VAR_TABLE_ITEM_T *pTableItem = m_pTableHeader;
	
	int iReValue = -1;

	if ((pVariableName == NULL) || (pVariableValue == NULL)){
		return(-1);
	}

	
	for(;pTableItem != NULL; pTableItem = pTableItem->m_pNextItem){
		
		if ((pTableItem->m_pVarPair != NULL)
			&&(pTableItem->m_pVarPair->m_pVariable != NULL)
			&&(strcmp(pTableItem->m_pVarPair->m_pVariable,pVariableName) == 0)){
			break;
		}
	}
	if (pTableItem == NULL){
		
		pTableItem = (VAR_TABLE_ITEM_T *)malloc (sizeof(VAR_TABLE_ITEM_T));
		if (pTableItem == NULL){
			
			return(-1);
		}
		pTableItem->m_pNextItem = NULL;
		
		pTableItem->m_pVarPair = (VAR_PAIR_T *)malloc(sizeof(VAR_PAIR_T));
		if (pTableItem->m_pVarPair == NULL){
			goto error;
		}
		pTableItem->m_pVarPair->m_pVariable = NULL;
		pTableItem->m_pVarPair->m_pValue= NULL;

		
		pTableItem->m_pVarPair->m_pVariable = putstr_(pVariableName);
		if (pTableItem->m_pVarPair->m_pVariable == NULL){
			goto error;
		}
		
		pTableItem->m_pVarPair->m_pValue = putstr_(pVariableValue);
		if (pTableItem->m_pVarPair->m_pValue == NULL){
			goto error;
		}
		if (m_pTableTail != NULL){
			m_pTableTail->m_pNextItem = pTableItem;
		}
		m_pTableTail = pTableItem;
		iReValue = 0;
	}else{
		char *pValue = pTableItem->m_pVarPair->m_pValue;
		if (strcmp(pValue,pVariableValue) == 0){
			return(2);
		}
		pTableItem->m_pVarPair->m_pValue = putstr_(pVariableValue);
		
		if (pTableItem->m_pVarPair->m_pValue == NULL){
			pTableItem->m_pVarPair->m_pValue = pValue;
			return(-1);
		}
		free(pValue);
		iReValue = 1;
	}
	
    	if (m_pTableHeader == NULL){
		m_pTableHeader = pTableItem;
	}
	return(iReValue);

error:
	
	if ((pTableItem != NULL) && (pTableItem->m_pVarPair != NULL)){
		if (pTableItem->m_pVarPair->m_pVariable != NULL){
			free(pTableItem->m_pVarPair->m_pVariable);
		}
		if (pTableItem->m_pVarPair->m_pVariable != NULL){
			free(pTableItem->m_pVarPair->m_pValue);
		}
		free(pTableItem->m_pVarPair);
		free(pTableItem);
	}
	return(-1);
}


void freeTable_(){

	VAR_TABLE_ITEM_T *pTableItem = m_pTableHeader;
	
	while(pTableItem != NULL){
		VAR_TABLE_ITEM_T *pNextTableItem = pTableItem->m_pNextItem;

		
		if (pTableItem->m_pVarPair != NULL){
			if (pTableItem->m_pVarPair->m_pVariable != NULL){
				free(pTableItem->m_pVarPair->m_pVariable);
			}
			if (pTableItem->m_pVarPair->m_pValue != NULL){
				free(pTableItem->m_pVarPair->m_pValue);
			}
			free(pTableItem->m_pVarPair);
		}

		free(pTableItem);
		pTableItem = pNextTableItem;
	}

	
	m_pTableHeader = NULL;
    	m_pTableTail = NULL;
	m_pEnvVarFileName = NULL;
}
