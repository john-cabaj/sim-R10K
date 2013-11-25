/*
 * cache.c - cache module routines
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
 * $Id: cache.c,v 1.5 1998/08/27 08:02:01 taustin Exp $
 *
 * $Log: cache.c,v $
 * Revision 1.5  1998/08/27 08:02:01  taustin
 * implemented host interface description in host.h
 * added target interface support
 * implemented a more portable random() interface
 * fixed cache writeback stats for cache flushes
 *
 * Revision 1.4  1997/03/11  01:08:30  taustin
 * updated copyright
 * long/int tweaks made for ALPHA target support
 * double-word interfaces removed
 *
 * Revision 1.3  1997/01/06  15:56:20  taustin
 * comments updated
 * fixed writeback bug when balloc == FALSE
 * strdup() changed to mystrdup()
 * cache_reg_stats() now works with stats package
 * cp->writebacks stat added to cache
 *
 * Revision 1.1  1996/12/05  18:52:32  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "options.h"
#include "cache.h"
#include "stats.h"
#include "sim.h"

void
cache_reg_options(struct opt_odb_t *odb,
		  struct cache_opt_t *opt)
{
  char buf[128];
  sprintf(buf, "-cache:%s", opt->name);
  opt_reg_string(odb, buf,
		 "cache config, i.e., {none|<nsets>:<bsize>:<assoc>:<repl>}",
		 &opt->opt, opt->opt, /* print */TRUE, NULL);

  sprintf(buf, "-cache:%s:hlat", opt->name);
  opt_reg_uint(odb, buf,
	       "cache hit-latency",
	       &opt->hlat, /* default */1, /* print */TRUE, NULL);

  sprintf(buf, "-cache:%s:prefetch:nblock", opt->name);
  opt_reg_uint(odb, buf,
	       "prefetch nth block (n = 0 means no prefetch)",
	       &opt->prefetch_nblock, /* default */0, /* print */TRUE, NULL);

  sprintf(buf, "-cache:%s:prefetch:ref", opt->name);
  opt_reg_flag(odb, buf,
	       "trigger prefetch on hardware reference (false = trigger prefetch on miss only)",
	       &opt->f_prefetch_ref, /* default */FALSE, /* print */TRUE, NULL);

#ifdef GET_OUT
  sprintf(buf, "-cache:%s:vb", opt->name);
  opt_reg_string(odb, buf,
		 "cache victim-buffer config, i.e., {none|<size>}",
		 &opt->vb_opt, "none", /* print */TRUE, NULL);

  sprintf(buf, "-cache:%s:wthru", opt->name);
  opt_reg_flag(odb, buf,
	       "cache write-thru",
	       &opt->f_wthru, /* default */FALSE, /* print */TRUE, NULL);
#endif /* GET_OUT */
}

void
cache_check_options(struct cache_opt_t *opt)
{
  char c;

  if (sscanf(opt->opt, "%d:%d:%d:%c", &opt->nsets, &opt->bsize, &opt->assoc, &c) != 4)
    fatal ("bad cache:%s params!", opt->name);

  if (opt->nsets <= 0 || !IS_POWEROF2(opt->nsets))
    fatal("cache:%s nsets `%d' must be non-zero and power of 2", opt->name, opt->nsets);
  /* blocks must be at least one datum large, i.e., 8 bytes for SS */
  if (opt->bsize < 8 || !IS_POWEROF2(opt->bsize))
    fatal("cache:%s bsize `%d' must be at least 8 and a power of 2", opt->name, opt->bsize);
  if (opt->assoc <= 0)
    fatal("cache:%s assoc `%d' must be positive", opt->name, opt->assoc);

#ifdef GET_OUT
  if (!mystricmp(opt->vb_opt, "none"))
    opt->nvictims = 0;
  else if (sscanf(opt->vb_opt, "%d", &opt->nvictims) != 1)
    fatal ("bad cache:%s:vb params!", opt->name);
#endif /* GET_OUT */
}


/* cache block (or line) definition */
struct cache_blk_t
{
  struct cache_blk_t *way_next;	/* next block in the ordered way chain, used
				   to order blocks for replacement */
  struct cache_blk_t *way_prev;	/* previous block in the order way chain */

  md_addr_t baddr;		/* block address */
  
  unsigned int status;		/* block status, see CACHE_BLK_* defs above */
  tick_t ready;		        /* time when block will be accessible, field is set when a miss fetch is initiated */
};

/* cache set definition (one or more blocks sharing the same set index) */
struct cache_set_t
{
  struct cache_blk_t *way_head;	/* head of way list */
  struct cache_blk_t *way_tail;	/* tail pf way list */
  struct cache_blk_t *blks;	/* cache blocks, allocated sequentially, so
				   this pointer can also be used for random
				   access to cache blocks */
};

/* cache definition */
struct cache_t
{
  struct cache_opt_t *opt;

  /* miss/replacement handler, read/write BSIZE bytes starting at BADDR
     from/into cache block BLK, returns the latency of the operation
     if initiated at NOW, returned latencies indicate how long it takes
     for the cache access to continue (e.g., fill a write buffer), the
     miss/repl functions are required to track how this operation will
     effect the latency of later operations (e.g., write buffer fills),
     if !BALLOC, then just return the latency; BLK_ACCESS_FN is also
     responsible for generating any user data and incorporating the latency
     of that operation */

  /* derived data, for fast decoding */
  md_addr_t blk_mask;
  int set_shift;
  md_addr_t set_mask;		/* use *after* shift */
  int tag_shift;
  md_addr_t tag_mask;		/* use *after* shift */
  md_addr_t tagset_mask;	/* used for fast hit detection */

  /* bus resource */
  tick_t bus_free;		/* time when bus to next level of cache is
				   free, NOTE: the bus model assumes only a
				   single, fully-pipelined port to the next
 				   level of memory that requires the bus only
 				   one cycle for cache line transfer (the
 				   latency of the access to the lower level
 				   may be more than one cycle, as specified
 				   by the miss handler */

  /* per-cache stats */
  counter_t lookups[mc_NUM];/* total number of lookups */
  counter_t misses[mc_NUM]; /* total number of misses */
  counter_t replacements;	 /* total number of replacements at misses */
  counter_t writebacks;		 /* total number of writebacks at misses */
  counter_t invalidations;	 /* total number of external invalidations */

  struct cache_set_t vb;

  /* NOTE: this is a variable-size tail array, this must be the LAST field
     defined in this structure! */
  struct cache_set_t sets[0];	/* each entry is a set */
};

/* block status values */
#define CACHE_BLK_VALID		0x00000001	/* block in valid, in use */
#define CACHE_BLK_DIRTY		0x00000002	/* dirty block */

/* cache access macros */
#define CACHE_TAG(cp, addr)	((addr) >> (cp)->tag_shift)
#define CACHE_SET(cp, addr)	(((addr) >> (cp)->set_shift) & (cp)->set_mask)
#define CACHE_BLK(cp, addr)	((addr) & (cp)->blk_mask)
#define CACHE_TAGSET(cp, addr)	((addr) & (cp)->tagset_mask)

/* extract/reconstruct a block address */
#define CACHE_BADDR(cp, addr)	((addr) & ~(cp)->blk_mask)
#define CACHE_MK_BADDR(cp, tag, set)					\
  (((tag) << (cp)->tag_shift)|((set) << (cp)->set_shift))

/* bound squad_t/dfloat_t to positive int */
#define BOUND_POS(N)		((int)(MIN(MAX(0, (N)), 2147483647)))

/* where to insert a block onto the ordered way chain */
enum list_loc_t { Head, Tail };

/* insert BLK into the order way chain in SET at location WHERE */
static void
update_way_list(struct cache_set_t *set,	/* set contained way chain */
		struct cache_blk_t *blk,	/* block to insert */
		enum list_loc_t where)		/* insert location */
{
  /* already where we're supposed to be */
  if ((where == Head && !blk->way_prev) ||
      (where == Tail && !blk->way_next))
    return;

  /* unlink block from way list */
  if (blk->way_prev) blk->way_prev->way_next = blk->way_next;
  if (blk->way_next) blk->way_next->way_prev = blk->way_prev;
  if (set->way_head == blk) set->way_head = blk->way_next;
  if (set->way_tail == blk) set->way_tail = blk->way_prev;
  blk->way_prev = blk->way_next = NULL;

  /* link BLK back into the list */
  if (where == Head)
    {
      /* link to the head of the way list */
      blk->way_next = set->way_head;
      blk->way_prev = NULL;
      set->way_head->way_prev = blk;
      set->way_head = blk;
    }
  else if (where == Tail)
    {
      /* link to the tail of the way list */
      blk->way_prev = set->way_tail;
      blk->way_next = NULL;
      set->way_tail->way_next = blk;
      set->way_tail = blk;
    }
}

/* create and initialize a general cache structure */
struct cache_t *			/* pointer to cache created */
cache_create(struct cache_opt_t *opt)       /* victim buffer size */
{
  struct cache_t *cp;
  int i, j;

  /* check all cache parameters */
  /* allocate the cache structure */
  cp = (struct cache_t *)
    calloc(1, sizeof(struct cache_t) + opt->nsets * sizeof(struct cache_set_t));
  if (!cp)
    fatal("out of virtual memory");

  /* initialize user parameters */
  cp->opt = opt;

  /* compute derived parameters */
  cp->blk_mask = cp->opt->bsize-1;
  cp->set_shift = log_base2(cp->opt->bsize);
  cp->set_mask = cp->opt->nsets-1;
  cp->tag_shift = cp->set_shift + log_base2(cp->opt->nsets);
  cp->tag_mask = (1 << (32 - cp->tag_shift))-1;
  cp->tagset_mask = ~cp->blk_mask;
  cp->bus_free = 0;

  /* print derived parameters during debug */
  debug("%s: cp->blk_mask  = 0x%08x", cp->blk_mask);
  debug("%s: cp->set_shift = %d", cp->set_shift);
  debug("%s: cp->set_mask  = 0x%08x", cp->set_mask);
  debug("%s: cp->tag_shift = %d", cp->tag_shift);
  debug("%s: cp->tag_mask  = 0x%08x", cp->tag_mask);

  /* initialize cache stats */
  cp->lookups[mc_READ] = cp->lookups[mc_WRITE] = 0;
  cp->misses[mc_READ] = cp->misses[mc_WRITE] = 0;
  cp->replacements = 0;
  cp->writebacks = 0;
  cp->invalidations = 0;

  /* slice up the data blocks */
  for (i=0; i<cp->opt->nsets; i++)
    {
      struct cache_set_t *set = &cp->sets[i];
      set->way_head = set->way_tail = NULL;

      set->blks = (struct cache_blk_t*)calloc(cp->opt->assoc, sizeof(struct cache_blk_t));
      /* link the data blocks into ordered way chain */
      for (j=0; j<cp->opt->assoc; j++)
	{
	  struct cache_blk_t *blk = &set->blks[j];

	  /* invalidate new cache block */
	  blk->status = 0;
	  blk->baddr = 0;
	  blk->ready = 0;

	  /* insert into head of way list, order is arbitrary at this point */
	  blk->way_next = set->way_head;
	  blk->way_prev = NULL;
	  if (set->way_head) set->way_head->way_prev = blk;
	  set->way_head = blk;
	  if (!set->way_tail) set->way_tail = blk;
	}
    }

  if (cp->opt->nvictims)
    {
      cp->vb.blks = (struct cache_blk_t *)calloc(cp->opt->nvictims, sizeof(struct cache_blk_t));
      /* link the data blocks into ordered way chain */
      for (j=0; j<cp->opt->nvictims; j++)
	{
	  struct cache_blk_t *blk = &cp->vb.blks[j];

	  /* invalidate new cache block */
	  blk->status = 0;
	  blk->baddr = 0;
	  blk->ready = 0;

	  /* insert into head of way list, order is arbitrary at this point */
	  blk->way_next = cp->vb.way_head;
	  blk->way_prev = NULL;
	  if (cp->vb.way_head) cp->vb.way_head->way_prev = blk;
	  cp->vb.way_head = blk;
	  if (!cp->vb.way_tail) cp->vb.way_tail = blk;
	}      
    }
  return cp;
}

/* parse policy */
enum cache_policy_t			/* replacement policy enum */
cache_char2policy(char c)		/* replacement policy as a char */
{
  switch (c) {
  case 'l': return cp_LRU;
  case 'r': return cp_RANDOM;
  case 'f': return cp_FIFO;
  default: fatal("bogus replacement policy, `%c'", c);
  }
}

/* register cache stats */
void
cache_stats_print(struct cache_t *cp,	/* cache instance */
		  FILE *stream)	        
{
  counter_t lookups = cp->lookups[mc_READ] + cp->lookups[mc_WRITE];
  counter_t misses = cp->misses[mc_READ] + cp->misses[mc_WRITE];

  char buf[512];

  sprintf(buf, "%s.accesses", cp->opt->name);
  print_counter(stream, buf, lookups, "total number of accesses");
  sprintf(buf, "%s.misses", cp->opt->name);
  print_counter(stream, buf, misses, "total number of misses");
  sprintf(buf, "%s.miss_rate", cp->opt->name);
  print_rate(stream, buf, (double)misses/lookups, "miss rate");
#ifdef GET_OUT
  sprintf(buf, "%s.reads", cp->opt->name);
  print_counter(stream, buf, cp->lookups[mc_READ], "total number of reads");
  sprintf(buf, "%s.read_misses", cp->opt->name);
  print_counter(stream, buf, cp->misses[mc_READ], "total number of read misses");
  sprintf(buf, "%s.read_miss_rate", cp->opt->name);
  print_rate(stream, buf, (double)cp->misses[mc_READ]/cp->lookups[mc_READ], "read miss rate");

  sprintf(buf, "%s.prefetch_accesses", cp->opt->name);
  print_counter(stream, buf, cp->lookups[mc_PREFETCH], "total number of prefetches");
  sprintf(buf, "%s.prefetches", cp->opt->name);
  print_counter(stream, buf, cp->misses[mc_PREFETCH], "total number of prefetches");
  
  sprintf(buf, "%s.replacements", cp->opt->name);
  print_counter(stream, buf, cp->replacements, "total number of replacements");
  sprintf(buf, "%s.writebacks", cp->opt->name);
  print_counter(stream, buf, cp->writebacks, "total number of writebacks");
#endif /* GET_OUT */
}

/* access a cache, perform a CMD operation on cache CP at address ADDR,
   on NBYTES of data, returns latency of operation if initiated
   at NOW */
unsigned int				/* latency of access in cycles */
cache_access(struct cache_t *cp,	/* cache to access */
	     enum mem_cmd_t cmd,	/* access type, mc_READ or mc_WRITE */
	     md_addr_t addr,		/* address of access */
	     int nbytes,		/* number of bytes to access */
	     tick_t now,                /* time of access */
	     bool_t miss_info[ct_NUM],  /* miss info */
	     miss_handler_t miss_handler)/* miss handler */
{
  md_addr_t baddr = CACHE_BADDR(cp, addr);

  struct cache_set_t *set = &cp->sets[CACHE_SET(cp, addr)];
  struct cache_blk_t *blk;
  int lat = 0;

  if (sample_mode == sample_ON) cp->lookups[cmd]++;

#ifdef GET_OUT
  /* check alignments */
  if ((nbytes & (nbytes-1)) != 0 || (addr & (nbytes-1)) != 0)
    fatal("cache: access error: bad size or alignment, addr 0x%08x", addr);

  /* access must fit in cache block */
  if ((addr + nbytes) > ((addr & ~cp->blk_mask) + cp->opt->bsize))
    fatal("cache: access error: access spans block, addr 0x%08x", addr);
#endif /* GET_OUT */
 
  /* permissions are checked on cache misses */
    
  /* search the way list */
  for (blk=set->way_head; blk; blk=blk->way_next)
    if (blk->baddr == baddr && (blk->status & CACHE_BLK_VALID))
      break;

  /* **MISS** */

  if (!blk)
    {
      struct cache_blk_t *vb_blk = NULL;

      /* select the appropriate block to replace */
      switch (cp->opt->policy) {
      case cp_LRU:
      case cp_FIFO:
	blk = set->way_tail;
	break;
      case cp_RANDOM:
	blk = &set->blks[(myrand() % (cp->opt->assoc))];
	break;
      default:
	panic("bogus replacement policy");
      }

      /* Wait until replaced block comes back from MSHR */
      if (blk->status & CACHE_BLK_VALID)
	{
	  if (sample_mode == sample_ON) cp->replacements++;
	  
	  /* don't replace the block until outstanding 
	     misses are satisfied */
	  lat += BOUND_POS(blk->ready - now);
	  
	  /* stall until the bus to next level of memory is available */
	  lat += BOUND_POS(cp->bus_free - (now + lat));
	  
	  /* track bus resource usage (only if timing is on) */
	  if (sample_mode == sample_ON) 
	    cp->bus_free = MAX(cp->bus_free, (now + lat)) + 1;
	}

      /* Try to find the corresponding block in the victim buffer */
      if (cp->opt->nvictims)
	{
	  for (vb_blk = cp->vb.way_head; vb_blk; vb_blk = vb_blk->way_next)
	    if (vb_blk->baddr == baddr && vb_blk->status & CACHE_BLK_VALID)
	      break;
	}

      /* If victim block found, do the swap.  It's OK if replaced block is
         invalid */
      if (vb_blk)
	{
	  struct cache_blk_t temp_blk;
	  temp_blk.baddr = vb_blk->baddr; 
	  temp_blk.status = vb_blk->status; 
	  temp_blk.ready = vb_blk->ready;
	  
	  vb_blk->baddr = blk->baddr;
	  vb_blk->status = blk->status;
	  vb_blk->ready = blk->ready;
	  
	  blk->baddr = temp_blk.baddr;
	  blk->status = temp_blk.status;
	  blk->ready = temp_blk.ready;
	  
	  /* move entry to head of VB */
	  update_way_list(&cp->vb, vb_blk, Head);
	}
      /* no victim buffer swap, this is a real miss */
      else 
	{
	  enum mem_cmd_t rcmd = (cmd == mc_READ || cmd == mc_WRITE) ? mc_READ : mc_PREFETCH;

	  if (miss_info) miss_info[cp->opt->ct] = TRUE;
	  if (sample_mode == sample_ON) cp->misses[cmd]++;

	  if (blk->status & CACHE_BLK_VALID)
	    {
	      /* replaced guy goes into victim buffer */
	      if (cp->opt->nvictims > 0)
		{
		  if (cp->vb.way_tail->status & CACHE_BLK_DIRTY)
		    {
		      if (sample_mode == sample_ON) cp->writebacks++;
		      lat += miss_handler(mc_WRITE, cp->vb.way_tail->baddr, 
					  cp->opt->bsize, now+lat, miss_info);
		      
		      if (sample_mode == sample_ON)
			cp->bus_free = MAX(cp->bus_free, now+lat) + 1;
		    }

		  cp->vb.way_tail->baddr = blk->baddr;
		  cp->vb.way_tail->status = blk->status;
		  cp->vb.way_tail->ready = blk->ready;
		  
		  update_way_list(&cp->vb, cp->vb.way_tail, Head);
		}
	      else
		{
		  /* write back replaced block data */
		  if (blk->status & CACHE_BLK_DIRTY)
		    {
		      /* write back the cache block */
		      if (sample_mode == sample_ON) cp->writebacks++;
		      lat += miss_handler(mc_WRITE, blk->baddr, cp->opt->bsize, now+lat, miss_info);
		      if (sample_mode == sample_ON)
			cp->bus_free = MAX(cp->bus_free, now+lat) + 1;
		    }
		}
	    }
	  
	  /* read data block */
	  lat += miss_handler(rcmd, baddr, cp->opt->bsize, now+lat, miss_info);
	  if (sample_mode == sample_ON)
	    cp->bus_free = MAX(cp->bus_free, now+lat) + 1;

	  blk->baddr = baddr;
	  blk->status = CACHE_BLK_VALID;	/* dirty bit set on update */
	  
	  if (sample_mode == sample_ON)
	    blk->ready = now + lat;
	}
    }
  
  /* update dirty status */
  if (cmd == mc_WRITE)
    {
      if (cp->opt->f_wthru)
	{
	  if (sample_mode == sample_ON)
	    {
	      /* stall until the bus to next level of memory is available */
	      lat += BOUND_POS(cp->bus_free - (now + lat));
	      
	      /* track bus resource usage */
	      if (sample_mode == sample_ON) 
		cp->bus_free = MAX(cp->bus_free, (now + lat)) + 1;
	    }

	  /* write back the cache block */
	  lat += miss_handler(mc_WRITE, blk->baddr, cp->opt->bsize, now+lat, miss_info);
	}
      else
	{
	  blk->status |= CACHE_BLK_DIRTY;
	}
    }

  /* if LRU replacement and this is not the first element of list, reorder */
  if (blk->way_prev && cp->opt->policy == cp_LRU)
    update_way_list(set, blk, Head);

  /* return first cycle data is available to access */
  return (sample_mode == sample_ON) ? (int) MAX(cp->opt->hlat, (blk->ready - now)) : 0;
}

/* flush the entire cache, returns latency of the operation */
unsigned int				/* latency of the flush operation */
cache_flush(struct cache_t *cp,		/* cache instance to flush */
	    tick_t now,                 /* time of cache flush */
	    miss_handler_t miss_handler)
{
  int i, lat = cp->opt->hlat; /* min latency to probe cache */

  /* no way list updates required because all blocks are being invalidated */
  for (i=0; i<cp->opt->nsets; i++)
    {
      struct cache_set_t *set = &cp->sets[i];
      struct cache_blk_t *blk;
      
      for (blk=set->way_head; blk; blk=blk->way_next)
	{
	  if (blk->status & CACHE_BLK_VALID)
	    {
	      if (sample_mode == sample_ON) cp->invalidations++;
	      blk->status &= ~CACHE_BLK_VALID;

	      if (blk->status & CACHE_BLK_DIRTY)
		{
		  /* write back the invalidated block */
          	  if (sample_mode == sample_ON) cp->writebacks++;
		  lat += miss_handler(mc_WRITE, blk->baddr, cp->opt->bsize, now+lat, NULL);
		}
	    }
	}
    }

  /* return latency of the flush operation */
  return lat;
}

/* flush the block containing ADDR from the cache CP, returns the latency of
   the block flush operation */
unsigned int				/* latency of flush operation */
cache_flush_addr(struct cache_t *cp,	/* cache instance to flush */
		 md_addr_t addr,	/* address of block to flush */
		 tick_t now,		/* time of cache flush */
		 miss_handler_t miss_handler)
{
  md_addr_t baddr = CACHE_BADDR(cp, addr);
  struct cache_set_t *set = &cp->sets[CACHE_SET(cp, baddr)];
  struct cache_blk_t *blk;
  int lat = cp->opt->hlat; /* min latency to probe cache */

  /* search the way list */
  for (blk=set->way_head; blk; blk=blk->way_next)
    if (blk->baddr == baddr && (blk->status & CACHE_BLK_VALID))
      break;

  if (blk)
    {
      if (sample_mode == sample_ON) cp->invalidations++;
      blk->status &= ~CACHE_BLK_VALID;

      if (blk->status & CACHE_BLK_DIRTY)
	{
	  /* write back the invalidated block */
          if (sample_mode == sample_ON) cp->writebacks++;
	  lat += miss_handler(mc_WRITE, blk->baddr, cp->opt->bsize, now+lat, NULL);
	}
      /* move this block to tail of the way (LRU) list */
      update_way_list(set, blk, Tail);
    }

  /* return latency of the operation */
  return lat;
}

