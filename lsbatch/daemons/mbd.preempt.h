/***********************************************************************
 *	Preemption Policy Routines --
 *
 *		Header file
 *
 ***********************************************************************/
 
#ifndef MBD_PREEMPT_H
#define MBD_PREEMPT_H

#if defined(DEBUG_PREEMPTION)
#define ENABLE_PREEMPT_DEBUG 1
#else
#define ENABLE_PREEMPT_DEBUG 0
#endif

#define DEBUG_PREEMPT ((logclass & LC_PREEMPT) || (ENABLE_PREEMPT_DEBUG))

#define PREEMPTION_SLOTS_CANNOT_BE_PREEMPTED(job) \
    (((job)->jFlags & JFLAG_URGENT) \
        ||((job)->jFlags & JFLAG_URGENT_NOSTOP) \
        ||((job)->qPtr->qAttrib & Q_ATTRIB_BACKFILL))

typedef struct key_and_value {
    void *name;
    int value;
} KV_T;

typedef struct ref_vector {
    void **data;
    int size;
    int idx;
} R_VECTOR_T;

typedef void (*R_VECTOR_FREE_FUNC_T)(void *);

#endif /*MBD_PREEMPT_H*/

