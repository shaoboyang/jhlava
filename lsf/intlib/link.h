/*
 * Elementary single linked list in C.
 * D. Knuth Art of Computer Programming Volume 1. 2.2
 *
 */
#ifndef __LINK__
#define __LINK__

/* Each linked list is made of a head whose ptr is always
 * NULL, a list of following links starting from next and
 * a number num indicating how many elements are in the
 * list.
 */
typedef struct link {
    int           num;
    void          *ptr;
    struct link   *next;
} link_t;

#define LINK_NUM_ENTRIES(L) ((L)->num)

typedef struct linkiter {
    link_t   *pos;
} linkiter_t;

link_t   *ecalloc(void);
void     efree(link_t *);
link_t   *initLink(void);
void     finLink(link_t *);
int      inLink(link_t *,void *);
void     *rmLink(link_t *, void *);
void     *peekLink(link_t *, void *val);
int      pushLink(link_t *, void *);
int      enqueueLink(link_t *, void *);
void     *dequeueLink(link_t *);
int      priorityLink(link_t *,
		      void *,
		      void *,
		      int (*cmp)(const void *,
				 const void *,
				 const void *));
void     *popLink(link_t *);
void     *visitLink(link_t *);
void     traverseInit(const link_t *,
		      linkiter_t *);
void     *traverseLink(linkiter_t *);

#endif /* __LINK__ */
