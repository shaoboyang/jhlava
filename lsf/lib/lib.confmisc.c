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
#include <pwd.h>
#include <grp.h>
#include "lib.h"
#include "lproto.h"

#define NL_SETN   23    
char *
getNextValue(char **line)
{
    return (getNextValueQ_(line, '(', ')'));
} 

int
keyMatch (struct keymap *keyList, char *line, int exact)
{
    int pos = 0;
    int i;
    char *sp = line;
    char *word;
    int found;

    i = 0;
    while (keyList[i].key != NULL) {      
	keyList[i].position = -1;    
	i++;
    }

    while ((word = getNextWord_(&sp)) != NULL) {
	i = 0;
	found = FALSE;
	while (keyList[i].key != NULL) {
	    if (strcasecmp(word, keyList[i].key) == 0) {
		if (keyList[i].position != -1)   
		    return FALSE;
		found = TRUE;
		keyList[i].position = pos;
		break;     
	    }
            i++;
	}
	if (! found)
	    return FALSE;
	
	pos++;
    }

    if (! exact)
	return TRUE;

    i = 0;
    while (keyList[i].key != NULL) {
	if (keyList[i].position == -1)    
	    return FALSE;
	i++;
    }

    return TRUE;
} 

int 
isSectionEnd(char *linep, char *lsfile, int *LineNum, char *sectionName)
{
    char *word;

    word = getNextWord_(&linep);      
    if (strcasecmp(word, "end") != 0)
        return FALSE;

    word = getNextWord_(&linep);      
    if (! word ) {
	ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5400,
	"%s(%d): section %s ended without section name, ignored"), lsfile, *LineNum, sectionName); /* catgets 5400 */
	return TRUE;
    }

    if (strcasecmp (word, sectionName) != 0) 
	ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5401,
	"%s(%d): section %s ended with wrong section name %s,ignored"), lsfile, *LineNum, sectionName, word); /* catgets 5401 */

    return TRUE;

} 

char *
getBeginLine(FILE *fp, int *LineNum)
{
    char *sp;
    char *wp;
    
    for (;;) {
        sp = getNextLineC_(fp, LineNum, TRUE);
        if (! sp)
            return (NULL);
	
        wp = getNextWord_(&sp);
        if (wp && (strcasecmp(wp, "begin") == 0))
            return sp;
    }
    
} 

int
readHvalues(struct keymap *keyList, char *linep, FILE *fp, char *lsfile, 
	    int *LineNum, int exact, char *section)
{
    static char fname[] = "readHvalues"; 
    char *key;
    char *value;
    char *sp, *sp1;
    char error = FALSE;
    int i=0;

    sp = linep;      
    key = getNextWord_(&linep);
    if ((sp1 = strchr(key, '=')) != NULL)
	*sp1 = '\0';

    value = strchr(sp, '=');
    if (!value) {
	ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5402,
	"%s: %s(%d): missing '=' after keyword %s, section %s ignoring the line"), fname, lsfile, *LineNum, key, section); /* catgets 5402 */
    } else {
        value++; 
        while (*value == ' ')
	    value++;
    
        if (value[0] == '\0') {
	    ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5403,
	    "%s: %s(%d): null value after keyword %s, section %s ignoring the line"), fname, lsfile, *LineNum, key, section); /* catgets 5403 */
        }
    
        if (value[0] == '(') {
            value++;
            if ((sp1 = strrchr(value, ')')) != NULL)
                *sp1 = '\0';
        }
        if (putValue(keyList, key, value) < 0) {
	    ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5404,
	    "%s: %s(%d): bad keyword %s in section %s, ignoring the line"), fname, lsfile, *LineNum, key, section); /* catgets 5404 */
        }
    } 
    if ((linep = getNextLineC_(fp, LineNum, TRUE)) != NULL) {
	if (isSectionEnd(linep, lsfile, LineNum, section)) {
	    if (! exact)
		return 0;

	    i = 0;
	    while (keyList[i].key != NULL) {
		if (keyList[i].val == NULL) {
		    ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5405,
	"%s: %s(%d): required keyword %s is missing in section %s, ignoring the section"), fname,  lsfile, *LineNum, keyList[i].key, section); /* catgets 5405 */
		    error = TRUE;
		} 
                i++;
	    }
	    if (error) {
                i = 0;
		while (keyList[i].key != NULL) {
		    FREEUP(keyList[i].val);
		    i++;
		}
		return -1;
	    }
	    return 0;
	}

        return readHvalues(keyList, linep, fp, lsfile, LineNum, exact, section);
    }
 
    ls_syslog(LOG_ERR, I18N_PREMATURE_EOF, 
	 	fname, lsfile, *LineNum, section);
    return -1;

} 

int
putValue(struct keymap *keyList, char *key, char *value)
{
    int i;

    i=0;
    while (keyList[i].key != NULL) {
	if (strcasecmp(keyList[i].key, key) == 0) {
	    FREEUP (keyList[i].val);
            if (strcmp (value, "-") == 0)
                keyList[i].val = putstr_("");
            else
		keyList[i].val = putstr_(value);
	    return 0;
	}
        i++;
    }

    return -1;
} 

void 
doSkipSection(FILE *fp, int *LineNum, char *lsfile, char *sectionName)
{
    char *word;
    char *cp;
    
    while ((cp = getNextLineC_(fp, LineNum, TRUE)) != NULL) {
	word = getNextWord_(&cp);      
	if (strcasecmp(word, "end") == 0) {
	    word = getNextWord_(&cp);
	    if (! word) {
		ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5407,
	"%s(%d): Section ended without section name, ignored"), lsfile, *LineNum); /* catgets 5407 */
	    } else {
		if (strcasecmp(word, sectionName) != 0)
		    ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5408,
	"%s(%d): Section %s ended with wrong section name: %s, ignored"), lsfile, *LineNum, sectionName, word); /* catgets 5408 */
	    }	
	    return;
	}
    }
    
    ls_syslog(LOG_ERR, I18N_PREMATURE_EOF,
		"doSkipSection", lsfile, *LineNum, sectionName);

} 

int
mapValues(struct keymap *keyList, char *line)
{
    int pos = 0;
    char *value;
    int i = 0;
    int found;
    int numv = 0;

    while (keyList[i].key != NULL) {   
	FREEUP (keyList[i].val);
	if (keyList[i].position != -1)  
	    numv++;
	i++;
    }

    while ((value = getNextValue(&line)) != NULL) {
	i=0;
	found = FALSE;
	while (keyList[i].key != NULL) {
	    if (keyList[i].position != pos) {
	        i++;
		continue;
	    }
            if (strcmp (value, "-") == 0)
                keyList[i].val = putstr_("");
            else {
		if (keyList[i].val != NULL)
		    FREEUP (keyList[i].val);
		keyList[i].val = putstr_(value);
            }
	    found = TRUE;
            break;
	}
	if (! found)     
	    goto fail;
	pos++;
    }

    if (pos != numv)
	goto fail;
	
    return 0;

fail: 
    i=0;
    while (keyList[i].key != NULL)  {
	if (keyList[i].val != NULL) {
	    free(keyList[i].val);
	    keyList[i].val = NULL;
	}

        i++;
    }
    return -1;

} 

int
putInLists (char *word, struct admins *admins, int *numAds, char *forWhat)
{
    static char fname[] = "putInLists"; 
    struct passwd *pw;
    char **tempNames;
    int i, *tempIds, *tempGids;

    if ((pw = getpwlsfuser_(word)) == NULL) { 
        if (logclass & LC_TRACE) {
             ls_syslog(LOG_DEBUG, _i18n_msg_get(ls_catd , NL_SETN, 5410,
                "%s: <%s> is not a valid user on this host"), fname, word); /* catgets 5410 */
        }
    }
    if (isInlist (admins->adminNames, word, admins->nAdmins)) {
        ls_syslog(LOG_WARNING, _i18n_msg_get(ls_catd , NL_SETN, 5411,
	"%s: Duplicate user name <%s> %s; ignored"), fname, word, forWhat); /* catgets 5411 */
        return (0);
    }
    admins->adminIds[admins->nAdmins] = (pw == NULL) ? -1 : pw->pw_uid;
    admins->adminGIds[admins->nAdmins] = (pw == NULL) ? -1 : pw->pw_gid;
    admins->adminNames[admins->nAdmins] = putstr_(word);
    admins->nAdmins += 1;

    if (admins->nAdmins >= *numAds) {
        *numAds = *numAds * 2;
        tempIds = (int *) realloc(admins->adminIds, *numAds * sizeof (int));
        tempGids = (int *) realloc(admins->adminGIds, *numAds * sizeof (int));
        tempNames = (char **) realloc(admins->adminNames, *numAds * sizeof (char *));
        if (tempIds == NULL || tempGids == NULL || tempNames == NULL) {
            ls_syslog(LOG_ERR, I18N_FUNC_FAIL_M, fname, "realloc");
            FREEUP (tempIds);
            FREEUP (tempGids);
            FREEUP (tempNames);

            FREEUP (admins->adminIds);
            FREEUP (admins->adminGIds);
            for (i = 0; i < admins->nAdmins; i++) 
                FREEUP (admins->adminNames[i]);
            FREEUP (admins->adminNames);
            admins->nAdmins = 0;
            lserrno = LSE_MALLOC;
            return (-1);
        } else {
            admins->adminIds = tempIds;
            admins->adminGIds = tempGids;
            admins->adminNames = tempNames;
        }
    }
    return (0);
} 

int
isInlist (char **adminNames, char *userName, int actAds)
{
    int i;

    if (actAds == 0)
        return (FALSE);
    for (i = 0; i < actAds; i++) {
        if (strcmp (adminNames[i], userName) == 0)
            return (TRUE);
    }
    return (FALSE);

} 

char *
getBeginLine_conf(struct lsConf *conf, int *LineNum)
{
    char *sp;
    char *wp;
    
    if (conf == NULL)
	return (NULL);

    for (;;) {
        sp = getNextLineC_conf(conf, LineNum, TRUE);
        if (sp == NULL)
            return (NULL);
	
        wp = getNextWord_(&sp);
        if (wp && (strcasecmp(wp, "begin") == 0))
            return sp;
    }
    
} 

void 
doSkipSection_conf(struct lsConf *conf, int *LineNum, char *lsfile, char *sectionName)
{
    char *word;
    char *cp;
    
    if (conf == NULL)
	return;

    while ((cp = getNextLineC_conf(conf, LineNum, TRUE)) != NULL) {
	word = getNextWord_(&cp);      
	if (strcasecmp(word, "end") == 0) {
	    word = getNextWord_(&cp);
	    if (! word) {
		ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5419,
	"%s(%d): Section ended without section name, ignored"), lsfile, *LineNum); /* catgets 5419 */
	    } else {
		if (strcasecmp(word, sectionName) != 0)
		    ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5420,
	"%s(%d): Section %s ended with wrong section name: %s, ignored"), lsfile, *LineNum, sectionName, word); /* catgets 5420 */
	    }	
	    return;
	}
    }
    
    ls_syslog(LOG_ERR, I18N_PREMATURE_EOF, 
		"doSkipSection_conf", lsfile, *LineNum, sectionName);

} 
