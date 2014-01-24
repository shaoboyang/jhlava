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

#ifndef _INTLIB_LIST_H_INCLUDED
#define _INTLIB_LIST_H_INCLUDED



typedef struct _listEntry          LIST_ENTRY_T;
typedef struct _list               LIST_T;
typedef struct _listEvent          LIST_EVENT_T;
typedef struct _listObserver       LIST_OBSERVER_T; 
typedef struct _listIterator       LIST_ITERATOR_T;

 

struct _listEntry {
    struct _listEntry *           forw;
    struct _listEntry *           back;
};
								       
                     	   	                           
struct _list {	        		                	   
    LIST_ENTRY_T *               forw;
    LIST_ENTRY_T *               back;
    char *                       name;
    int                          numEnts;     
    bool_t                       allowObservers;
    LIST_T *                     observers;   
};


#define LIST_IS_EMPTY(List) ((List)->forw == (LIST_ENTRY_T *)List)

#define LIST_NUM_ENTRIES(List) ((List)->numEnts)

typedef void          (*LIST_ENTRY_DESTROY_FUNC_T)(LIST_ENTRY_T *);
typedef int           (*LIST_ENTRY_EQUALITY_OP_T)(void *entry, 
					       void *subject,
					       int hint);
typedef void          (*LIST_ENTRY_DISPLAY_FUNC_T)(LIST_ENTRY_T *, void *);
typedef char *        (*LIST_ENTRY_CAT_FUNC_T)(LIST_ENTRY_T *, void *);

extern LIST_T *         listCreate(char *name);
extern void             listDestroy(LIST_T *list, 
				    void (*destroy)(LIST_ENTRY_T *));
extern int              listAllowObservers(LIST_T *list);
extern LIST_ENTRY_T *   listGetFrontEntry(LIST_T *list);
extern LIST_ENTRY_T *   listGetBackEntry(LIST_T *list);
extern int              listInsertEntryBefore(LIST_T *list,
					LIST_ENTRY_T *succ, 
					LIST_ENTRY_T *entry);
extern int              listInsertEntryAfter(LIST_T *list,
					LIST_ENTRY_T *pred, 
					LIST_ENTRY_T *entry);
extern int              listInsertEntryAtFront(LIST_T *list,
					LIST_ENTRY_T *entry);
extern int              listInsertEntryAtBack(LIST_T *list,
					LIST_ENTRY_T *entry);
extern LIST_ENTRY_T *   listSearchEntry(LIST_T *list,
					void *subject,
					bool_t (*equal)(void *, void *, int),
					int hint);
extern void             listRemoveEntry(LIST_T *list, LIST_ENTRY_T *entry);
extern int              listNotifyObservers(LIST_T *list, LIST_EVENT_T *event);
#define LIST_TRAVERSE_FORWARD              0x1
#define LIST_TRAVERSE_BACKWARD             0x2

extern void             list2Vector(LIST_T *list, int direction,
				    void *vector, 
				    void (*putVecEnt)(void *vector, int index,
						      LIST_ENTRY_T *entry));

extern void             listDisplay(LIST_T *list, int direction,
				    void (*displayEntry)(LIST_ENTRY_T *, 
							 void *),
				    void *hint);

extern void             listCat(LIST_T *list, int direction,
				char *buffer, int bufferSize,
				char * (*catEntry)(LIST_ENTRY_T *, 
						   void *),
				void *hint);

extern LIST_T*          listDup(LIST_T *, int);
extern void             listDump(LIST_T *);

typedef enum _listEventType {
    LIST_EVENT_ENTER, 
    LIST_EVENT_LEAVE,   
    LIST_EVENT_NULL
} LIST_EVENT_TYPE_T;

struct _listEvent {
    LIST_EVENT_TYPE_T     type;          
    LIST_ENTRY_T          *entry;         
};

typedef bool_t  (*LIST_ENTRY_SELECT_OP_T)(void *extra, 
					  LIST_EVENT_T *);

typedef int     (*LIST_EVENT_CALLBACK_FUNC_T)(LIST_T *list, 
					      void *extra,
					      LIST_EVENT_T *event);

struct _listObserver {
    struct _listObserver *           forw;
    struct _listObserver *           back;
    char *                           name;
    LIST_T *                         list;
    void *                           extra;
    LIST_ENTRY_SELECT_OP_T           select;
    LIST_EVENT_CALLBACK_FUNC_T       enter;
    LIST_EVENT_CALLBACK_FUNC_T       leave_;
};


extern LIST_OBSERVER_T *  listObserverCreate(char *name,
					     void *extra, 
				 	     LIST_ENTRY_SELECT_OP_T select,
					     ...);
extern void               listObserverDestroy(LIST_OBSERVER_T *observer);
extern int                listObserverAttach(LIST_OBSERVER_T *observer,
					     LIST_T *list);
extern void               listObserverDetach(LIST_OBSERVER_T *observer,
					     LIST_T *list);


struct _listIterator {
    char *             name;            
    LIST_T *           list;            
    LIST_ENTRY_T *     curEnt;          
};

#define LIST_ITERATOR_ZERO_OUT(Iter) \
{ \
    memset((void *)(Iter), 0, sizeof(LIST_ITERATOR_T)); \
    (Iter)->name = ""; \
}							   


extern LIST_ITERATOR_T *  listIteratorCreate(char *name);
extern void               listIteratorDestroy(LIST_ITERATOR_T *iter);
extern int                listIteratorAttach(LIST_ITERATOR_T *iter,
					     LIST_T *list);
extern void               listIteratorDetach(LIST_ITERATOR_T *iter);
extern LIST_T *           listIteratorGetList(LIST_ITERATOR_T *iter);
extern LIST_ENTRY_T *     listIteratorGetCurEntry(LIST_ITERATOR_T *iter);
extern int                listIteratorSetCurEntry(LIST_ITERATOR_T *iter,
						  LIST_ENTRY_T *ent,
						  bool_t validateEnt);
extern void               listIteratorNext(LIST_ITERATOR_T *iter, 
					   LIST_ENTRY_T **next);
extern void               listIteratorPrev(LIST_ITERATOR_T *iter,
					   LIST_ENTRY_T **prev);
extern bool_t             listIteratorIsEndOfList(LIST_ITERATOR_T *iter);
   

extern int listerrno;

#undef LIST_ERROR_CODE_ENTRY
#define LIST_ERROR_CODE_ENTRY(Id, Desc) Id,

enum _listErrno {
#   include "listerr.def"
    LIST_ERR_LAST
};

extern enum _listErrno listErrnoType;

extern char *    listStrError(int listerrno);
extern void      listPError(char *usrmsg);

#endif 



