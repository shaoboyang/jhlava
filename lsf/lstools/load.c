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
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "../lsf.h"

#include "../intlib/intlibout.h"
#define MAXFIELDSIZE 20
int num_loadindex;

struct indexFmt {
    char *name;
    char *hdr;
    char *busy;
    char *ok;
    float scale;
    int  dispLen;
    char *normFmt;
    char *expFmt;
};

 
struct indexFmt fmt[] = {
{ "r15s", "%6s", "*%4.1", "%5.1", 1.0 ,  6, "f",   "g"},
{ "r1m",  "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g" },
{ "r15m", "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g" },
{ "ut",   "%5s", "*%3.0", "%3.0", 100.0, 5, "f%%", "g%%"},
{ "pg",   "%6s", "*%4.1", "%4.1", 1.0,   5, "f",   "g"},
{ "io",   "%6s", "*%4.0", "%4.0", 1.0,   4, "f",   "g"},
{ "ls",   "%4s", "*%2.0", "%3.0", 1.0,   3, "f",   "g"},
{ "it",   "%6s", "*%4.0", "%5.0", 1.0,   5, "f",   "g"},
{ "tmp",  "%6s", "*%3.0", "%4.0", 1.0,   5, "fM",  "fG"},
{ "swp",  "%6s", "*%3.0", "%4.0", 1.0,   5, "fM",  "fG"},
{ "mem",  "%6s", "*%4.0", "%5.0", 1.0,   5, "fM",  "fG"},
{ "dflt", "%7s", "*%6.1", "%6.1", 1.0,   6, "f",   "g" },
{  NULL,  "%7s", "*%6.1", "%6.1", 1.0,   6, "f",   "g" }
 };

struct indexFmt widefmt[] = {
{ "r15s", "%6s", "*%4.1", "%5.1", 1.0 ,  6, "f",   "g"},
{ "r1m",  "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g" },
{ "r15m", "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g" },
{ "ut",   "%5s", "*%3.0", "%3.0", 100.0, 5, "f%%", "g%%"},
{ "pg",   "%6s", "*%4.1", "%4.1", 1.0,   5, "f",   "g"},
{ "io",   "%6s", "*%4.0", "%4.0", 1.0,   4, "f",   "g"},
{ "ls",   "%4s", "*%2.0", "%3.0", 1.0,   3, "f",   "g"},
{ "it",   "%6s", "*%4.0", "%5.0", 1.0,   5, "f",   "g"},
{ "tmp",  "%6s", "*%3.0", "%4.0", 1.0,   5, "fM",  "fG"},
{ "swp",  "%6s", "*%3.0", "%4.0", 1.0,   5, "fM",  "fG"},
{ "mem",  "%6s", "*%4.0", "%5.0", 1.0,   5, "fM",  "fG"},
{ "dflt", "%14s", "*%6.1", "%6.1", 1.0,   13, "f",   "g" },
{  NULL,  "%14s", "*%6.1", "%6.1", 1.0,   13, "f",   "g" }
 };


#define DEFAULT_FMT  11

#define NL_SETN 27 


static int
nameToFmt( char *indx)
{
    int i;

    if (strcmp(indx,"swap") == 0)
        indx = "swp";
    if (strcmp(indx, "login") == 0)
        indx = "ls";
    if (strcmp(indx, "idle") == 0)
        indx = "it";
    if (strcmp(indx, "cpu") == 0)
        indx = "r1m";
    
    for (i=0; fmt[i].name; i++) {
        if (strcmp(indx, fmt[i].name) == 0)
            return i;
    }
    return (i-1);	
} 

char *
formatHeader(char **dispindex, char longformat)
{
    int i, fmtid, maxMem; 
    char tmpbuf[MAXLSFNAMELEN];
    static char *line = NULL;
    static int first = TRUE;
#define HEADERLEN  132

    if (first) {
	if ((line = (char *)malloc(HEADERLEN)) == NULL) {
	    fprintf(stderr, I18N_NO_MEMORY);
	    exit (-1);
        }
	first = FALSE;
	maxMem = HEADERLEN;
    }

    if (longformat)
        sprintf(line, (_i18n_msg_get(ls_catd,NL_SETN, 1151, "HOST_NAME               status"))); /* catgets 1151 */
    else
        sprintf(line, (_i18n_msg_get(ls_catd,NL_SETN, 1152, "HOST_NAME       status"))); /* catgets 1152 */

    for(i=0; dispindex[i]; i++) {
        fmtid = nameToFmt(dispindex[i]);
        if (fmtid == DEFAULT_FMT){  
	    if ((maxMem - strlen(line)) < HEADERLEN) {
		maxMem = 2 * maxMem;
	        if ((line = (char *)realloc(line, maxMem)) == NULL) {
		    fprintf(stderr,I18N_FUNC_FAIL,"formatHeader","realloc" );
		    fprintf(stderr,"\n");
		    exit(-1);
		}
            }
            if (strlen(dispindex[i]) >= 7) 
		sprintf(tmpbuf, " %s", dispindex[i]);
            else
		sprintf(tmpbuf, fmt[fmtid].hdr, dispindex[i]);
        }
	else
            sprintf(tmpbuf, fmt[fmtid].hdr, dispindex[i]);

        strcat(line, tmpbuf);
    }
    num_loadindex=i;
    return(line);
} 


char *
wideformatHeader(char **dispindex, char longformat)
{
    int i, fmtid, maxMem; 
    char tmpbuf[MAXLSFNAMELEN];
    static char *line = NULL;
    static int first = TRUE;
#define HEADERLEN  132

    if (first) {
	if ((line = (char *)malloc(HEADERLEN)) == NULL) {
	    fprintf(stderr, I18N_NO_MEMORY);
	    exit (-1);
        }
	first = FALSE;
	maxMem = HEADERLEN;
    }

    if (longformat)
        sprintf(line, (_i18n_msg_get(ls_catd,NL_SETN, 1151, "HOST_NAME               status"))); /* catgets 1151 */
    else
        sprintf(line, (_i18n_msg_get(ls_catd,NL_SETN, 1152, "HOST_NAME       status"))); /* catgets 1152 */

    for(i=0; dispindex[i]; i++) {
        fmtid = nameToFmt(dispindex[i]);
        if (fmtid == DEFAULT_FMT){  
	    if ((maxMem - strlen(line)) < HEADERLEN) {
		maxMem = 2 * maxMem;
	        if ((line = (char *)realloc(line, maxMem)) == NULL) {
		    fprintf(stderr,I18N_FUNC_FAIL,"formatHeader","realloc" );
		    fprintf(stderr,"\n");
		    exit(-1);
		}
            }
            if (strlen(dispindex[i]) >= 7) 
		sprintf(tmpbuf, " %s", dispindex[i]);
            else
		sprintf(tmpbuf, widefmt[fmtid].hdr, dispindex[i]);
        }
	else
            sprintf(tmpbuf, widefmt[fmtid].hdr, dispindex[i]);

        strcat(line, tmpbuf);
    }
    num_loadindex=i;
    return(line);
} 



char *
stripSpaces(char *field)
{
    char *np, *cp, *sp;
    int len, i;

    cp = field;
    while (*cp == ' ')
        cp++;

    
    if (*cp == '*') {
        sp = cp;
	for (sp=sp+1; *sp==' '; sp++) {
	    *(sp-1) = ' ';
	    *sp = '*';
	}
        
        np = strchr(cp, '*');
        cp = np;
    }

    
    len = strlen(field);
    i = len - 1;
    while((i > 0) && (field[i] == ' '))
        i--;
    if (i < len-1)
        field[i] = '\0';
    return(cp);
} 


char **
filterToNames(char *filter)
{
    char tmpname[MAXLSFNAMELEN];
    static char **names;
    static int namelen;
    int i,j,k,len;

    if (!names) {
        namelen =16;
        names=(char **)malloc(namelen*sizeof (char *));
        memset(names, 0, namelen*sizeof(char *));
    }
    for(i=0; names[i]; i++)
        free(names[i]);
    len = strlen(filter);
    i=0;
    k=0;
    while (i < len) {
        for(j=0; ((filter[i] != ':') && (filter[i] != '\0')); j++, i++)
            tmpname[j] = filter[i];
        i++;
        tmpname[j] = '\0';
        names[k++] = (char *)putstr_(tmpname); 
        if (!names[k-1])
            fprintf(stderr, I18N_FUNC_FAIL,"filterToNames", "malloc" );
        if (k==namelen) {
            
            if(!(names=(char **)realloc(names, namelen*2*sizeof(char *)))) {
                 lserrno = LSE_MALLOC;
                 ls_perror(NULL);
            }
            memset(names+namelen, 0, namelen*sizeof(char *));
            namelen <<= 1; 
        }     
    }
    names[k] = NULL;
    return(names);
} 

int
makeFields(struct hostLoad *host, char *loadval[], char **dispindex)
{
    int j, id, nf;
    static char first = TRUE;
    char *sp;
    char tmpfield[MAXFIELDSIZE];
    char fmtField[MAXFIELDSIZE];
    char firstFmt[MAXFIELDSIZE];

    if (first) {
	first = FALSE;
	for (j=0; j < num_loadindex;j++)
	    loadval[j] = malloc(MAXFIELDSIZE);
        if (loadval[j-1] == NULL)
	    fprintf(stderr, I18N_FUNC_FAIL ,"makeFields", "malloc" );
    }

    
    nf = 0;
    for(j=0; dispindex[j]; j++, nf++) {
	int newIndexLen;

        id = nameToFmt(dispindex[j]);
	if (id == DEFAULT_FMT)
	    newIndexLen = strlen(dispindex[j]);

        if (host->li[j] >= INFINIT_LOAD) 
            sp = "-";
        else {
            if (LS_ISBUSYON(host->status, j)) {
                strcpy(firstFmt, fmt[id].busy);
                sprintf(fmtField, "%s%s",firstFmt, fmt[id].normFmt);
                sprintf(tmpfield, fmtField, host->li[j] * fmt[id].scale);
            }
            else { 
                strcpy(firstFmt, fmt[id].ok);
                sprintf(fmtField, "%s%s", firstFmt, fmt[id].normFmt);
                sprintf(tmpfield, fmtField, host->li[j] * fmt[id].scale);
            }
            sp = stripSpaces(tmpfield);

            
            if (strlen(sp) > fmt[id].dispLen) {
                if (host->li[j] > 1024)
                    sprintf(fmtField, "%s%s", firstFmt, fmt[id].expFmt);
                else
                    sprintf(fmtField, "%s%s", firstFmt, fmt[id].normFmt);
                if ((host->li[j] > 1024) &&
                    ((!strcmp(fmt[id].name,"mem")) ||
                    (!strcmp(fmt[id].name,"tmp")) ||
                    (!strcmp(fmt[id].name,"swp"))))
                    sprintf(tmpfield,fmtField,(host->li[j]*fmt[id].scale)/1024);
                else 
                    sprintf(tmpfield,fmtField, (host->li[j] * fmt[id].scale));
            }
            sp = stripSpaces(tmpfield);
        }
	if (id == DEFAULT_FMT && newIndexLen >= 7){
	    char newFmt[10];
	    int len;
	    sprintf(newFmt, "%s%d%s", "%", newIndexLen+1, "s");
	    
	    len = (newIndexLen+1) > strlen(sp) ? (newIndexLen+1): strlen(sp); 
	    loadval[j] = realloc(loadval[j],  len+1);
            sprintf(loadval[j], newFmt, sp);
        }
	else 
            sprintf(loadval[j], fmt[id].hdr, sp); 
    }

    return(nf);
} 


int
makewideFields(struct hostLoad *host, char *loadval[], char **dispindex)
{
    int j, id, nf;
    static char first = TRUE;
    char *sp;
    char tmpfield[MAXFIELDSIZE];
    char fmtField[MAXFIELDSIZE];
    char firstFmt[MAXFIELDSIZE];

    if (first) {
	first = FALSE;
	for (j=0; j < num_loadindex;j++)
	    loadval[j] = malloc(MAXFIELDSIZE);
        if (loadval[j-1] == NULL)
	    fprintf(stderr, I18N_FUNC_FAIL ,"makeFields", "malloc" );
    }

    
    nf = 0;
    for(j=0; dispindex[j]; j++, nf++) {
	int newIndexLen;

        id = nameToFmt(dispindex[j]);
	if (id == DEFAULT_FMT)
	    newIndexLen = strlen(dispindex[j]);

        if (host->li[j] >= INFINIT_LOAD) 
            sp = "-";
        else {
            if (LS_ISBUSYON(host->status, j)) {
                strcpy(firstFmt, widefmt[id].busy);
                sprintf(fmtField, "%s%s",firstFmt, widefmt[id].normFmt);
                sprintf(tmpfield, fmtField, host->li[j] * widefmt[id].scale);
            }
            else { 
                strcpy(firstFmt, widefmt[id].ok);
                sprintf(fmtField, "%s%s", firstFmt, widefmt[id].normFmt);
                sprintf(tmpfield, fmtField, host->li[j] * widefmt[id].scale);
            }
            sp = stripSpaces(tmpfield);

            
            if (strlen(sp) > widefmt[id].dispLen) {
                if (host->li[j] > 1024)
                    sprintf(fmtField, "%s%s", firstFmt, widefmt[id].expFmt);
                else
                    sprintf(fmtField, "%s%s", firstFmt, widefmt[id].normFmt);
                if ((host->li[j] > 1024) &&
                    ((!strcmp(widefmt[id].name,"mem")) ||
                    (!strcmp(widefmt[id].name,"tmp")) ||
                    (!strcmp(widefmt[id].name,"swp"))))
                    sprintf(tmpfield,fmtField,(host->li[j]*widefmt[id].scale)/1024);
                else 
                    sprintf(tmpfield,fmtField, (host->li[j] * widefmt[id].scale));
            }
            sp = stripSpaces(tmpfield);
        }
	if (id == DEFAULT_FMT && newIndexLen >= 7){
	    char newFmt[10];
	    int len;
	    sprintf(newFmt, "%s%d%s", "%", newIndexLen+1, "s");
	    
	    len = (newIndexLen+1) > strlen(sp) ? (newIndexLen+1): strlen(sp); 
	    loadval[j] = realloc(loadval[j],  len+1);
            sprintf(loadval[j], newFmt, sp);
        }
	else 
            sprintf(loadval[j], widefmt[id].hdr, sp); 
    }

    return(nf);
} 

