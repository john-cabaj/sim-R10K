/*
 * resource.c - resource manager routines
 *
 * This file is a part of the SimpleScalar tool suite written by
 * Todd M. Austin as a part of the Multiscalar Research Project.
 *  
 * The tool suite is currently maintained by Doug Burger and Todd M. Austin.
 * 
 * Copyright (C) 1994, 1995, 1996, 1997 by Todd M. Austin
 *
 * This source file is distributed "as is" in the hope that it will be
 * useful.  The tool set comes with no warranty, and no author or
 * distributor accepts any responsibility for the consequences of its
 * use. 
 * 
 * Everyone is granted permission to copy, modify and redistribute
 * this tool set under the following conditions:
 * 
 *    This source code is distributed for non-commercial use only. 
 *    Please contact the maintainer for restrictions applying to 
 *    commercial use.
 *
 *    Permission is granted to anyone to make or distribute copies
 *    of this source code, either as received or modified, in any
 *    medium, provided that all copyright notices, permission and
 *    nonwarranty notices are preserved, and that the distributor
 *    grants the recipient permission for further redistribution as
 *    permitted by this document.
 *
 *    Permission is granted to distribute this file in compiled
 *    or executable form under the same conditions that apply for
 *    source code, provided that either:
 *
 *    A. it is accompanied by the corresponding machine-readable
 *       source code,
 *    B. it is accompanied by a written offer, with no time limit,
 *       to give anyone a machine-readable copy of the corresponding
 *       source code in return for reimbursement of the cost of
 *       distribution.  This written offer must permit verbatim
 *       duplication by anyone, or
 *    C. it is distributed by someone who received only the
 *       executable form, and is accompanied by a copy of the
 *       written offer of source code that they received concurrently.
 *
 * In other words, you are welcome to use, share and improve this
 * source file.  You are forbidden to forbid anyone else to use, share
 * and improve what you give them.
 *
 * INTERNET: dburger@cs.wisc.edu
 * US Mail:  1210 W. Dayton Street, Madison, WI 53706
 *
 * $Id: resource.c,v 1.3 1997/03/11 01:19:53 taustin Exp taustin $
 *
 * $Log: resource.c,v $
 * Revision 1.3  1997/03/11  01:19:53  taustin
 * updated copyright
 * check added to ensure number of res insts <= MAX_INSTS_PER_CLASS
 *
 * Revision 1.1  1996/12/05  18:52:32  taustin
 * Initial revision
 *
 *
 */

/* External definitions */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "machine.h"
#include "options.h"
/* Local definitions */
#include "resource.h"
/* Implementation definitions */
#include "misc.h"
  
struct fuclass_opt_t fuclass_opt[fuclass_NUM] = {
  { "na", 1, 1 }, 
  { "ialu", 1, 1 },
  { "ishift", 2, 1 },
  { "ibranch", 1, 1 },
  { "imult", 4, 1 },
  { "idiv", 8, 8 },
  { "fadd", 2, 1 },
  { "fbranch", 1, 1 },
  { "fcvt", 2, 1 },
  { "fmult", 4, 1 },
  { "fdiv", 8, 8 },
  { "fsqrt", 16, 16 }
};

void
fuclass_reg_options(struct opt_odb_t *odb)
{
  char buf[128], buf2[128];
  enum fuclass_t fu;
  for (fu = fuclass_IALU; fu < fuclass_NUM; fu++)
    {
      sprintf(buf, "-%s:execlat", fuclass_opt[fu].name);
      sprintf(buf2, "%s execution latency (<execlat>)", fuclass_opt[fu].name);
      opt_reg_uint(odb, buf, buf2,
		   &fuclass_opt[fu].execlat, /* default */fuclass_opt[fu].execlat,
		     /* print */TRUE, /* format */NULL);
    }
}

void
fuclass_check_options()
{
  enum fuclass_t fu;
  for (fu = fuclass_IALU; fu < fuclass_NUM; fu++)
    if (fuclass_opt[fu].execlat == 0)
      fatal("%s execution latency must be positive");
}

/* resource pool indices */
static enum resclass_t mp_fuclass2resclass[fuclass_NUM] = {
  resclass_NA, /* FUClass_NA */	  
  resclass_IALU, /* IntALU */ 
  resclass_IMULT, /* IntMULT */
  resclass_IMULT, /* IntDIV */
  resclass_FPALU, /* FloatADD */
  resclass_FPALU, /* FloatCMP */
  resclass_FPALU, /* FloatCVT */
  resclass_FPMULT, /* FloatMULT */
  resclass_FPMULT, /* FloatDIV */
  resclass_FPMULT, /* FloatSQRT */
  resclass_MEMPORT, /* RdPort */
  resclass_MEMPORT, /* WrPort */
};

static char *mp_resclass2str[resclass_NUM] = 
{
  "N/A",
  "ialu",
  "imult",
  "fpalu",
  "fpmult",
  "cacheport"
};

void
respool_reg_options(struct opt_odb_t *odb,
		    struct respool_opt_t *opt)
{
  char buf[128], buf2[128];
  enum resclass_t rc;

  /* placeholder */
  opt->res_num[resclass_NA] = 0;

  for (rc = resclass_IALU; rc < resclass_NUM; rc++)
    {
      sprintf(buf, "-respool:%s:num", mp_resclass2str[rc]);
      sprintf(buf2, "total number of %s's available", mp_resclass2str[rc]);
      opt_reg_int(odb, buf, buf2,
		  &opt->res_num[rc], /* default */4,
		  /* print */TRUE, /* format */NULL);
    }
}

void
respool_check_options(struct respool_opt_t *opt)
{
  enum resclass_t rc;
  for (rc = resclass_IALU; rc < resclass_NUM; rc++)
    if (opt->res_num[rc] < 1)
      fatal("need at least 1 '%s'", mp_resclass2str[rc]);
}


struct respool_t
{
  struct respool_opt_t *opt;
  struct res_t *res_heads[resclass_NUM];
  struct res_t *res_tails[resclass_NUM];
};


/* create a resource pool */
struct respool_t *
respool_create(struct respool_opt_t *opt)
{
  enum resclass_t rc;
  int j;
  struct respool_t *pool;

  pool = (struct respool_t *) calloc (1, sizeof(struct respool_t));
  pool->opt = opt;

  for (rc = resclass_IALU; rc < resclass_NUM; rc++)
    {
      for (j = 0; j < pool->opt->res_num[rc]; j++)
	{
	  struct res_t * fu;
	  fu = (struct res_t *) calloc (1, sizeof(struct res_t));

	  /* attach to next element */
	  if (pool->res_heads[rc])
	    pool->res_heads[rc]->prev = fu;
	  fu->next = pool->res_heads[rc];

	  /* Attach to head */
	  pool->res_heads[rc] = fu;

	  /* Attach to tail */
	  if (pool->res_tails[rc] == 0) 
	    pool->res_tails[rc] = fu;
	}
    }

  return pool;
}

/* get a free resource from resource pool POOL that can execute a
   operation of class CLASS, returns a pointer to the resource template,
   returns NULL, if there are currently no free resources available,
   follow the MASTER link to the master resource descriptor;
   NOTE: caller is responsible for reseting the busy flag in the beginning
   of the cycle when the resource can once again accept a new operation */
struct res_t *
respool_get_res(struct respool_t *pool, 
		enum fuclass_t fu_class, 
		tick_t now)
{
  enum resclass_t res_class;
  struct res_t * fu, *pfu, *nfu;

  res_class = mp_fuclass2resclass[fu_class];
  fu = pool->res_heads[res_class];
  
  /* No resource ready */
  if (fu->ready > now)
    return NULL;

  fu->execlat = fuclass_opt[fu_class].execlat;
  fu->ready = now + fuclass_opt[fu_class].issuelat;

  /* Detach from resource list */
  if (fu->next) 
    fu->next->prev = NULL;
  pool->res_heads[res_class] = fu->next;
  if (!pool->res_heads[res_class])
    pool->res_tails[res_class] = NULL;

  /* Reinsert fu into list, from the back, in order of readyness */
  for (nfu = NULL, pfu = pool->res_tails[res_class]; 
       pfu; 
       nfu = pfu, pfu = pfu->prev)
    if (pfu->ready <= fu->ready)
      break;

  /* insert in the back */
  if (!nfu)
    {
      if (pool->res_tails[res_class])
	pool->res_tails[res_class]->next = fu;
      else
	pool->res_heads[res_class] = fu;
      fu->prev = pool->res_tails[res_class];
      pool->res_tails[res_class] = fu;
    }
  /* insert in the front */
  else if (!pfu)
    {
      if (pool->res_heads[res_class])
	pool->res_heads[res_class]->prev = fu;
      else
	pool->res_tails[res_class] = fu;
      fu->next = pool->res_heads[res_class];
      pool->res_heads[res_class] = fu;
    }
  /* insert in the middle */
  else
    {
      pfu->next = fu;
      nfu->prev = fu;
      fu->next = nfu;
      fu->prev = pfu;
    }

  return fu;
}

/* Return number of free units of this class */
int
respool_free_res(struct respool_t *pool, 
		 enum fuclass_t fu_class,
		 tick_t now)
{
  int free = 0;
  enum resclass_t res_class = mp_fuclass2resclass[fu_class];
  struct res_t *fu;

  for (fu = pool->res_heads[res_class]; 
       fu && fu->ready <= now; 
       fu = fu->next, free++) ;
    
  return free;
}

void
respool_dump(struct respool_t *pool,
	     FILE *stream)
{
  int i;
  struct res_t * fu;

  for (i = 0; i < resclass_NUM; i++)
    {
      fprintf(stream, "%s(%d)", mp_resclass2str[i], pool->opt->res_num[i]);
      for (fu = pool->res_heads[i]; fu; fu = fu->next)
	fprintf(stream, " %9u", (unsigned int)fu->ready);
      fprintf(stream, "\n");
    }
}


