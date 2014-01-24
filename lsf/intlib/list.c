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


#include "intlibout.h"

LIST_T *
listCreate(char *name)
{
    LIST_T *list;

    list = calloc(1, sizeof(LIST_T));
    if (list == NULL)
        return NULL;

    list->name = putstr_(name);
    list->forw = list->back = (LIST_ENTRY_T *)list;

    return list;
}

void
listDestroy(LIST_T *list, void (*destroy)(LIST_ENTRY_T *))
{
    LIST_ENTRY_T *entry;

    while (! LIST_IS_EMPTY(list)) {
        entry = list->forw;

        listRemoveEntry(list, entry);
        if (destroy)
            (*destroy)(entry);
        else
            free(entry);
    }

    if (list->allowObservers) {

        listDestroy(list->observers,
                    (LIST_ENTRY_DESTROY_FUNC_T)&listObserverDestroy);
    }

    free(list->name);
    free(list);
}

int
listAllowObservers(LIST_T *list)
{
    if (list->allowObservers)

        return 0;

    list->allowObservers = TRUE;
    list->observers = listCreate("Observer list");

    if (! list->observers)

        return(-1);

    return(0);
}

LIST_ENTRY_T *
listGetFrontEntry(LIST_T *list)
{
    if (LIST_IS_EMPTY(list))
        return NULL;

    return list->forw;
}

LIST_ENTRY_T *
listGetBackEntry(LIST_T *list)
{
    if (LIST_IS_EMPTY(list))
        return NULL;

    return list->back;
}

int
listInsertEntryBefore(LIST_T * list,
                      LIST_ENTRY_T *succ,
                      LIST_ENTRY_T *entry)
{
    entry->forw = succ;
    entry->back = succ->back;
    succ->back->forw = entry;
    succ->back = entry;

    if (list->allowObservers && ! LIST_IS_EMPTY(list->observers)) {
        LIST_EVENT_T event;

        event.type = LIST_EVENT_ENTER;
        event.entry = entry;

        listNotifyObservers(list, &event);
    }

    list->numEnts++;

    return (0);
}

int
listInsertEntryAfter(LIST_T * list, LIST_ENTRY_T *pred, LIST_ENTRY_T *entry)
{
    return listInsertEntryBefore(list, pred->forw, entry);

}

int
listInsertEntryAtFront(LIST_T * list, LIST_ENTRY_T *entry)
{
    return listInsertEntryBefore(list, list->forw, entry);
}

int
listInsertEntryAtBack(LIST_T * list, LIST_ENTRY_T *entry)
{
    return listInsertEntryBefore(list, (LIST_ENTRY_T *)list, entry);
}


LIST_ENTRY_T *
listSearchEntry(LIST_T *list, void *subject,
                bool_t (*equal)(void *, void *, int),
                int hint)
{
    LIST_ITERATOR_T iter;
    LIST_ENTRY_T *ent;

    LIST_ITERATOR_ZERO_OUT(&iter);
    listIteratorAttach(&iter, list);

    for (ent = listIteratorGetCurEntry(&iter);
         ent != NULL;
         listIteratorNext(&iter, &ent)) {
        if ((*equal)((void *)ent, subject, hint) == TRUE)
            return ent;
    }

    return NULL;
}

void
listRemoveEntry(LIST_T *list, LIST_ENTRY_T *entry)
{
    if (entry->back == NULL || entry->forw == NULL)
        return;

    entry->back->forw = entry->forw;
    entry->forw->back = entry->back;

    if (list->allowObservers && ! LIST_IS_EMPTY(list->observers)) {
        LIST_EVENT_T event;

        event.type = LIST_EVENT_LEAVE;
        event.entry = entry;

        (void) listNotifyObservers(list, &event);
    }

    list->numEnts--;
}

int
listNotifyObservers(LIST_T *list, LIST_EVENT_T *event)
{
    LIST_OBSERVER_T *observer;
    LIST_ITERATOR_T iter;

    listIteratorAttach(&iter, list->observers);

    for (observer = (LIST_OBSERVER_T *)listIteratorGetCurEntry(&iter);
         ! listIteratorIsEndOfList(&iter);
         listIteratorNext(&iter, (LIST_ENTRY_T **)&observer))
    {
        if (observer->select != NULL) {
            if (! (*observer->select)(observer->extra, event))
                continue;
        }

        switch (event->type) {
        case (int) LIST_EVENT_ENTER:
            if (observer->enter)
                (*observer->enter)(list, observer->extra, event);
            break;
        case (int) LIST_EVENT_LEAVE:
            if (observer->leave_)
                (*observer->leave_)(list, observer->extra, event);
            break;
        default:
            listerrno = LIST_ERR_BADARG;
            return -1;
        }
    }

    return 0;
}

void
list2Vector(LIST_T *list, int direction, void *vector,
            void (*putVecEnt)(void *vector, int index, LIST_ENTRY_T *entry))
{
    LIST_ITERATOR_T iter;
    LIST_ENTRY_T *entry;
    int entIdx;

    if (direction == 0)
        direction = LIST_TRAVERSE_FORWARD;

    LIST_ITERATOR_ZERO_OUT(&iter);
    listIteratorAttach(&iter, list);

    if (direction & LIST_TRAVERSE_BACKWARD)
        listIteratorSetCurEntry(&iter, listGetBackEntry(list), FALSE);

    entIdx = 0;
    for (entry = listIteratorGetCurEntry(&iter);
         entry != NULL;
         (direction & LIST_TRAVERSE_FORWARD) ?
         listIteratorNext(&iter, &entry) : listIteratorPrev(&iter, &entry))
    {
        if (putVecEnt != NULL)
            (*putVecEnt)(vector, entIdx, entry);
        else
            *(void **)((long)vector + entIdx * sizeof(void *)) = (void *)entry;

        entIdx++;
    }
}

void
listDisplay(LIST_T *list,
            int direction,
            void (*displayFunc)(LIST_ENTRY_T *, void *),
            void *hint)
{
    LIST_ITERATOR_T iter;
    LIST_ENTRY_T *entry;

    if (direction == 0)
        direction = LIST_TRAVERSE_FORWARD;

    LIST_ITERATOR_ZERO_OUT(&iter);
    listIteratorAttach(&iter, list);

    if (direction & LIST_TRAVERSE_BACKWARD)
        listIteratorSetCurEntry(&iter, listGetBackEntry(list), FALSE);

    for (entry = listIteratorGetCurEntry(&iter);
         entry != NULL;
         (direction & LIST_TRAVERSE_FORWARD) ?
         listIteratorNext(&iter, &entry) : listIteratorPrev(&iter, &entry))
    {
        (*displayFunc)(entry, hint);
    }
}

void
listCat(LIST_T *list,
        int direction,
        char *buffer,
        int bufferSize,
        char * (*catFunc)(LIST_ENTRY_T *, void *),
        void *hint)
{
    LIST_ITERATOR_T iter;
    LIST_ENTRY_T *entry;
    int curSize;

    buffer[0] = '\000';
    if (direction == 0)
        direction = LIST_TRAVERSE_FORWARD;

    LIST_ITERATOR_ZERO_OUT(&iter);
    listIteratorAttach(&iter, list);

    if (direction & LIST_TRAVERSE_BACKWARD)
        listIteratorSetCurEntry(&iter, listGetBackEntry(list), FALSE);

    curSize = 0;
    for (entry = listIteratorGetCurEntry(&iter);
         entry != NULL;
         (direction & LIST_TRAVERSE_FORWARD) ?
         listIteratorNext(&iter, &entry) : listIteratorPrev(&iter, &entry))
    {
        char *str;

        str = (*catFunc)(entry, hint);
        if (! str) {
            continue;
        }

        if (curSize + strlen(str) > bufferSize - 1) {
            break;
        }

        strcat(buffer, str);
        curSize += strlen(str);
    }

}


LIST_T*
listDup(LIST_T* list, int sizeOfEntry)
{
    LIST_T *newList;
    LIST_ENTRY_T *listEntry;
    LIST_ENTRY_T *newListEntry;
    LIST_ITERATOR_T iter;

    newList = listCreate(list->name);
    if (! newList) {
        return NULL;
    }

    LIST_ITERATOR_ZERO_OUT(&iter);
    listIteratorAttach(&iter, list);

    for (listEntry = listIteratorGetCurEntry(&iter);
         listEntry != NULL;
         listIteratorNext(&iter, &listEntry)) {

        newListEntry = (LIST_ENTRY_T *)calloc(1, sizeOfEntry);

        memcpy(newListEntry, listEntry, sizeOfEntry);

        listInsertEntryAtBack(newList, newListEntry);

    }

    listIteratorDetach(&iter);

    return newList;
}

void
listDump(LIST_T* list)
{
    LIST_ITERATOR_T iter;
    LIST_ENTRY_T *listEntry;

    LIST_ITERATOR_ZERO_OUT(&iter);
    listIteratorAttach(&iter, list);

    for (listEntry = listIteratorGetCurEntry(&iter);
         ! listIteratorIsEndOfList(&iter);
         listIteratorNext(&iter, &listEntry)) {

        ls_syslog(LOG_DEBUG,"\
%s: Entry=<%x> is in list=<%s>", __func__, listEntry, list->name);

    }

    listIteratorDetach(&iter);

}

LIST_OBSERVER_T *
listObserverCreate(char *name, void *extra, LIST_ENTRY_SELECT_OP_T select, ...)
{
    LIST_OBSERVER_T *observer;
    LIST_EVENT_TYPE_T etype;
    LIST_EVENT_CALLBACK_FUNC_T callback;
    va_list ap;

    observer = calloc(1, sizeof(LIST_OBSERVER_T));
    if (observer == NULL) {
        listerrno = LIST_ERR_NOMEM;
        goto Fail;
    }

    observer->name = putstr_(name);
    observer->select = select;
    observer->extra = extra;

    va_start(ap, select);

    for (;;) {
        etype = va_arg(ap, LIST_EVENT_TYPE_T);

        if (etype == LIST_EVENT_NULL)
            break;

        callback = va_arg(ap, LIST_EVENT_CALLBACK_FUNC_T);

        switch (etype) {
        case (int) LIST_EVENT_ENTER:
            observer->enter = callback;
            break;

        case (int) LIST_EVENT_LEAVE:
            observer->leave_ = callback;
            break;

        default:
            listerrno = LIST_ERR_BADARG;
            goto Fail;
        }
    }

    return observer;

  Fail:
    FREEUP(observer);
    return NULL;
}


void
listObserverDestroy(LIST_OBSERVER_T *observer)
{
    free(observer->name);
    free(observer);
}


int
listObserverAttach(LIST_OBSERVER_T *observer, LIST_T *list)
{
    int cc;

    if (! list->allowObservers) {
        listerrno = (int) LIST_ERR_NOOBSVR;
        return -1;
    }

    cc = listInsertEntryBefore(list->observers,
                               (LIST_ENTRY_T *)list->observers,
                               (LIST_ENTRY_T *)observer);
    if (cc < 0)
        return cc;

    observer->list = list;

    return 0;
}


void
listObserverDetach(LIST_OBSERVER_T *observer, LIST_T *list)
{
    if (observer->list)
        listRemoveEntry(observer->list, (LIST_ENTRY_T *)observer);

    observer->list = NULL;
}

LIST_ITERATOR_T *
listIteratorCreate(char *name)
{
    LIST_ITERATOR_T *iter;

    iter = calloc(1, sizeof(LIST_ITERATOR_T));
    if (! iter) {
        listerrno = (int)LIST_ERR_NOMEM;
        return NULL;
    }

    iter->name = putstr_(name);
    return iter;

}

void
listIteratorDestroy(LIST_ITERATOR_T *iter)
{
    free(iter->name);
    free(iter);
}

int
listIteratorAttach(LIST_ITERATOR_T *iter, LIST_T *list)
{
    iter->list = list;
    iter->curEnt = list->forw;

    return 0;
}

void
listIteratorDetach(LIST_ITERATOR_T *iter)
{
    iter->list = NULL;
    iter->curEnt = NULL;
}


LIST_T *
listIteratorGetSubjectList(LIST_ITERATOR_T *iter)
{
    return iter->list;
}


LIST_ENTRY_T *
listIteratorGetCurEntry(LIST_ITERATOR_T *iter)
{
    if (iter->curEnt == (LIST_ENTRY_T *)iter->list)
        return NULL;

    return iter->curEnt;
}


int
listIteratorSetCurEntry(LIST_ITERATOR_T *iter,
                        LIST_ENTRY_T *entry,
                        bool_t validateEnt)
{
    LIST_ENTRY_T *savedCurEnt;
    LIST_ENTRY_T *ent;

    if (validateEnt) {
        bool_t found = FALSE;

        savedCurEnt = iter->curEnt;

        iter->curEnt = listGetFrontEntry(iter->list);
        for (ent = listIteratorGetCurEntry(iter);
             ! listIteratorIsEndOfList(iter);
             listIteratorNext(iter, (LIST_ENTRY_T **)&ent))
        {
            if (ent == entry) {
                found = TRUE;
                break;
            }
        }

        if (! found) {
            listerrno = LIST_ERR_BADARG;
            iter->curEnt = savedCurEnt;
            return -1;
        }
    }

    iter->curEnt = entry;
    return (0);

}

void
listIteratorNext(LIST_ITERATOR_T *iter,
                 LIST_ENTRY_T **next)
{
    iter->curEnt = iter->curEnt->forw;
    *next = listIteratorGetCurEntry(iter);
}

void
listIteratorPrev(LIST_ITERATOR_T *iter, LIST_ENTRY_T **prev)
{
    iter->curEnt = iter->curEnt->back;
    *prev = listIteratorGetCurEntry(iter);
}


bool_t
listIteratorIsEndOfList(LIST_ITERATOR_T *iter)
{
    return (iter->curEnt == (LIST_ENTRY_T *)iter->list);

}

int listerrno;

#undef LIST_ERROR_CODE_ENTRY
#define LIST_ERROR_CODE_ENTRY(Id, Desc) Desc,

static char *listErrList[] = {

/* catgets 6510  */         "No Error",
/* catgets 6511  */         "Bad arguments",
/* catgets 6512  */         "Memory allocation failed",
/* catgets 6513  */         "Permission denied for attaching observers",
                            "Last Error (no error)"
};

#ifdef  I18N_COMPILE
static int listErrListID[] = {
       6510,
       6511,
       6512,
       6513
};
#endif

char *
listStrError(int errnum)
{
    static char buf[216];

    if (errnum < 0 || errnum > (int) LIST_ERR_LAST) {
        sprintf(buf, "Unknown error number %d", errnum);
        return (buf);
    }

    return listErrList[errnum];

}

void
listPError(char *usrmsg)
{
    if (usrmsg) {
        fputs(usrmsg, stderr);
        fputs(": ", stderr);
    }
    fputs(listStrError(listerrno), stderr);
    putc('\n', stderr);

}

void
inList(struct listEntry *pred, struct listEntry *entry)
{
    entry->forw = pred;
    entry->back = pred->back;
    pred->back->forw = entry;
    pred->back = entry;
}

void
offList(struct listEntry *entry)
{
    entry->back->forw = entry->forw;
    entry->forw->back = entry->back;
}

struct listEntry *
mkListHeader (void)
{
    struct listEntry *q;

    q = calloc(1, sizeof (struct listEntry));
    if (q == NULL)
        return NULL;

    q->forw = q->back = q;
    q->entryData = 0;

    return q;
}
