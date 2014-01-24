
#include "list2.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

struct list_ *
listmake(const char *name)
{
    struct list_ *L;

    L = calloc(1, sizeof(struct list_));
    if (!L)
        return NULL;
    L->forw = L->back = L;

    L->name = strdup(name);

    return L;
}

/* Insert e2 after e
 *
 *       forw
 * List -------> e
 *      <------
 *        back
 *  end          front
 *
 * listpush() pushes e2 at front
 *
 * List -->e --> e2
 *
 * listenqueue() enqueues it at the end
 *
 * List --> e2 --> e
 *
 */
int
listinsert(struct list_ *h,
           struct list_ *e,
           struct list_ *e2)
{
    e->back->forw = e2;
    e2->back = e->back;
    e->back = e2;
    e2->forw = e;

    h->num++;
    return h->num;
}

/* Push at front...
 */
int
listpush(struct list_ *h,
         struct list_ *e)
{
    listinsert(h, h, e);
    return 0;
}

/* Enqueue at end
 */
int
listenque(struct list_ *h,
          struct list_ *e)
{
    listinsert(h, h->forw, e);
    return 0;
}

struct list_ *
listrm(struct list_ *h,
       struct list_ *e)
{
    if (h->num == 0)
        return NULL;

    e->back->forw = e->forw;
    e->forw->back = e->back;
    h->num--;

    return e;
}

/* pop from front
 */
struct list_ *
listpop(struct list_ *h)
{
    struct list_ *e;

    if (h->forw == h)
        return NULL;

    e = listrm(h, h->back);

    return e;
}

/* dequeue from the end
 */
struct list_ *
listdeque(struct list_ *h)
{
    struct list_   *e;

    if (h->forw == h) {
        assert(h->back == h);
        return NULL;
    }

    e = listrm(h, h->forw);
    return e;
}

void
listfree(struct list_ *L,
         void (*f)(void *))
{
    struct list_   *l;

    if (L == NULL)
        return;
    while ((l = listpop(L))) {
        if (f == NULL)
            free(l);
        else
            (*f)(l);
    }

    free(L->name);
    free(L);

} /* listfree()*/

/* Insert in the list by increasing priority
 * _cmp_() is supposed to behave like compare
 * finction for qsort()
 */
int
listinsertsort(struct list_ *L,
               struct list_ *e,
               void  *extra,
               int (*_cmp_)(const void *,
                            const void *,
                            const void *))
{
    struct list_   *l;
    int            r;

    if (!L)
        return(-1);

    if (LIST_NUM_ENTS(L) == 0) {
        listpush(L, e);
        return 0;
    }

    for (l = L->back;
         l != L;
         l = l->back) {

        r = (*_cmp_)(e, l, extra);
        if (r <= 0) {
            /*
             * Found an element l smaller than e.
             *
             *              l
             *              v
             * head->back->back->back
             *           ^
             *           | e here...
             */
            listinsert(L, l->forw, e);
            return 0;
        }
    } /* for (l = L->back; ...; ...) */

    listenque(L, e);

    return 0;

} /* listinsertsort() */
