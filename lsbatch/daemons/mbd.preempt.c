/***********************************************************************
 *	Preemption Policy Routines --
 *
 *		Routines for preemption policy
 *
 ***********************************************************************/

/*
 * Routines related with preemption policy
 */

#include "mbd.h"
#include "mbd.preempt.h"

/*
 * internal data
 */
static R_VECTOR_T *g_preemptive_V = NULL;
static R_VECTOR_T *g_preempted_V = NULL;
static R_VECTOR_T *g_preempting_V = NULL;

