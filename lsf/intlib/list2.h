
#ifndef _LIST_
#define _LIST_

struct list_ {
    struct list_   *forw;
    struct list_   *back;
    int            num;
    char           *name;
};

#define LIST_NUM_ENTS(L) ((L)->num)
typedef void (*LIST_DATA_DESTROY_FUNC_T)(void *);

extern struct list_ *listmake(const char *);
extern int  listinsert(struct list_ *,
		       struct list_ *,
		       struct list_ *);
extern int listinsertsort(struct list_ *,
			  struct list_ *,
			  void *,
			  int (*_cmp_)(const void *,
				       const void *,
				       const void *));
extern int listpush(struct list_ *,
		    struct list_ *);
extern int listenque(struct list_ *,
		     struct list_ *);
extern struct list_ * listrm(struct list_ *,
			     struct list_ *);
struct list_ *listpop(struct list_ *);
extern struct list_ *listdeque(struct list_ *);
extern void listfree(struct list_ *, void (*f)(void *));

#endif /* _LIST_ */
