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

#ifndef _ECHKPNT_ENV_H
#define _ECHKPNT_ENV_H

#define ECHKPNT_VAR_FILE 		"/.echkpnt"

typedef struct varpair{
	char *m_pVariable; 
	char *m_pValue;    
}VAR_PAIR_T;

typedef struct varTableItem{
	VAR_PAIR_T *m_pVarPair;
	struct varTableItem *m_pNextItem;
}VAR_TABLE_ITEM_T;


char *getEchkpntVar(const char *);
int  writeEchkpntVar(const char *, const char *);
int  fileIsExist(const char *);

void freeTable_();

#endif
