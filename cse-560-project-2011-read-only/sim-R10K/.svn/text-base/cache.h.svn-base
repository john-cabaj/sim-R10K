/*
 * cache.h - cache module interfaces
 *
 * This file is a part of the SimpleScalar tool suite written by
 * Todd M. Austin as a part of the Multiscalar Research Project.
 *  
 * The tool suite is currently maintained by Doug Burger and Todd M. Austin.
 * 
 * Copyright (C) 1994, 1995, 1996, 1997, 1998 by Todd M. Austin
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
 * $Id: cache.h,v 1.5 1998/08/27 08:09:13 taustin Exp $
 *
 * $Log: cache.h,v $
 * Revision 1.5  1998/08/27 08:09:13  taustin
 * implemented host interface description in host.h
 * added target interface support
 *
 * Revision 1.4  1997/03/11  01:09:45  taustin
 * updated copyright
 * long/int tweaks made for ALPHA target support
 *
 * Revision 1.3  1997/01/06  15:57:55  taustin
 * comments updated
 * cache_reg_stats() now works with stats package
 * cp->writebacks stat added to cache
 *
 * Revision 1.1  1996/12/05  18:50:23  taustin
 * Initial revision
 *
 *
 */

#ifndef CACHE_H
#define CACHE_H

/*
 * This module contains code to implement various cache-like structures.  The
 * user instantiates caches using cache_new().  When instantiated, the user
 * may specify the geometry of the cache (i.e., number of set, line size,
 * associativity), and supply a block access function.  The block access
 * function indicates the latency to access lines when the cache misses,
 * accounting for any component of miss latency, e.g., bus acquire latency,
 * bus transfer latency, memory access latency, etc...  In addition, the user
 * may allocate the cache with or without lines allocated in the cache.
 * Caches without tags are useful when implementing structures that map data
 * other than the address space, e.g., TLBs which map the virtual address
 * space to physical page address, or BTBs which map text addresses to
 * branch prediction state.  Tags are always allocated.  User data may also be
 * optionally attached to cache lines, this space is useful to storing
 * auxilliary or additional cache line information, such as predecode data,
 * physical page address information, etc...
 *
 * The caches implemented by this module provide efficient storage management
 * and fast access for all cache geometries.  When sets become highly
 * associative, a hash table (indexed by address) is allocated for each set
 * in the cache.
 *
 * This module also tracks latency of accessing the data cache, each cache has
 * a hit latency defined when instantiated, miss latency is returned by the
 * cache's block access function, the caches may service any number of hits
 * under any number of misses, the calling simulator should limit the number
 * of outstanding misses or the number of hits under misses as per the
 * limitations of the particular microarchitecture being simulated.
 *
 * Due to the organization of this cache implementation, the latency of a
 * request cannot be affected by a later request to this module.  As a result,
 * reordering of requests in the memory hierarchy is not possible.
 */

/* cache replacement policy */
enum cache_policy_t 
{
  cp_LRU,	/* replace least recently used block (perfect LRU) */
  cp_RANDOM,	/* replace a random block */
  cp_FIFO	/* replace the oldest block in the set */
};

enum cache_type_t 
{
  ct_L1,
  ct_L2,
  ct_L3,
  ct_TLB,
  ct_NUM
};

struct cache_opt_t
{
  enum cache_type_t ct;

  char *name;

  char *opt;
  char *vb_opt;

  unsigned int nsets;
  unsigned int bsize;
  unsigned int assoc;
  enum cache_policy_t policy;
  
  unsigned int hlat;

  bool_t f_wthru;

  unsigned int nvictims;
  unsigned int prefetch_nblock;
  bool_t f_prefetch_ref;
};

void
cache_reg_options(struct opt_odb_t *odb,
		  struct cache_opt_t *opt);

void 
cache_check_options(struct cache_opt_t *opt);

/* forward declaration */
struct cache_t;

typedef unsigned int (* miss_handler_t)(enum mem_cmd_t cmd,
					md_addr_t baddr, 
					unsigned int bsize,
					tick_t now,
					bool_t miss_info[ct_NUM]);

/* create and initialize a general cache structure */
struct cache_t *			/* pointer to cache created */
cache_create(struct cache_opt_t *opt);

/* register cache stats */
void
cache_stats_print(struct cache_t *cp,	/* cache instance */
		  FILE *stream);

/* access a cache, perform a CMD operation on cache CP at address ADDR,
   places NBYTES of data at *P, returns latency of operation if initiated
   at NOW, places pointer to block user data in *UDATA, *P is untouched if
   cache blocks are not allocated (!CP->BALLOC), UDATA should be NULL if no
   user data is attached to blocks */
unsigned int				/* latency of access in cycles */
cache_access(struct cache_t *cp,	/* cache to access */
	     enum mem_cmd_t cmd,	/* access type, Read or Write */
	     md_addr_t addr,		/* address of access */
	     int nbytes,		/* number of bytes to access */
	     tick_t now,		/* time of access */
	     bool_t miss_info[ct_NUM],  /* get miss info through here */
	     miss_handler_t miss_handler);        /* miss handler */

/* flush the entire cache, returns latency of the operation */
unsigned int				/* latency of the flush operation */
cache_flush(struct cache_t *cp,		/* cache instance to flush */
	    tick_t now,		        /* time of cache flush */
	    miss_handler_t miss_handler);/* pointer to miss handler */

/* flush the block containing ADDR from the cache CP, returns the latency of
   the block flush operation */
unsigned int				/* latency of flush operation */
cache_flush_addr(struct cache_t *cp,	/* cache instance to flush */
		 md_addr_t addr,	/* address of block to flush */
		 tick_t now,		/* time of cache flush */
		 miss_handler_t miss_handler); /* pointer to miss handler */

#endif /* CACHE_H */
