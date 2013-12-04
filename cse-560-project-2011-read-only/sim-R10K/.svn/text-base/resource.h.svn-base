/*
 * resource.h - resource manager interfaces
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
 * $Id: resource.h,v 1.3 1997/03/11 01:26:49 taustin Exp taustin $
 *
 * $Log: resource.h,v $
 * Revision 1.3  1997/03/11  01:26:49  taustin
 * updated copyright
 *
 * Revision 1.1  1996/12/05  18:50:23  taustin
 * Initial revision
 *
 *
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdio.h>

struct fuclass_opt_t {
  char *name;
  unsigned int execlat;
  unsigned int issuelat;
};

struct fuclass_opt_t fuclass_opt[fuclass_NUM];

/* maximum number of resource classes supported */
#define MAX_RES_CLASSES		16

/* maximum number of resource instances for a class supported */
#define MAX_INSTS_PER_CLASS	8

enum resclass_t {
  resclass_NA,
  resclass_IALU,
  resclass_IMULT,
  resclass_FPALU,
  resclass_FPMULT,
  resclass_MEMPORT,
  resclass_NUM
};

struct res_t {
  int execlat;
  tick_t ready;
  struct res_t *next;
  struct res_t *prev;
};

/* resource pool: one entry per resource instance */
struct respool_t;

struct respool_opt_t 
{
  int res_num[resclass_NUM];
};

void
fuclass_reg_options(struct opt_odb_t *odb);

void 
fuclass_check_options();

/* get a free resource from resource pool POOL that can execute a
   operation of class CLASS, returns a pointer to the resource template,
   returns NULL, if there are currently no free resources available,
   follow the MASTER link to the master resource descriptor;
   NOTE: caller is responsible for reseting the busy flag in the beginning
   of the cycle when the resource can once again accept a new operation */
struct res_t *
respool_get_res(struct respool_t *pool, 
		enum fuclass_t fuclass, 
		tick_t now);


int 
respool_free_res(struct respool_t *pool, 
		 enum fuclass_t fuclass, 
		 tick_t now);

void
respool_dump(struct respool_t *pool,
	     FILE *stream);

void
respool_reg_options(struct opt_odb_t *odb,
		    struct respool_opt_t *opt);

void
respool_check_options(struct respool_opt_t *opt);

struct respool_t *
respool_create(struct respool_opt_t *opt);

#endif /* RESOURCE_H */
