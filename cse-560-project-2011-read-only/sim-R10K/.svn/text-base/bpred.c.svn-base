/*
 * bpred.c - branch predictor routines
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
 * $Id: bpred.c,v 1.1.1.1 1997/05/22 00:33:18 aklauser Exp $
 *
 * $Log: bpred.c,v $
 * Revision 1.1.1.1  1997/05/22 00:33:18  aklauser
 *
 * Revision 1.11  1997/05/01 20:23:00  skadron
 * BTB bug fixes; jumps no longer update direction state; non-taken
 *    branches non longer update BTB
 *
 * Revision 1.10  1997/05/01 00:05:42  skadron
 * Separated BTB from direction-predictor
 *
 * Revision 1.9  1997/04/30  01:42:42  skadron
 * 1. Not aggressively returning the BTB target regardless of hit on jump's,
 *    but instead returning just "taken" when it's a BTB miss yields an
 *    apparent epsilon performance improvement for cc1 and perl.
 * 2. Bug fix: if no retstack, treat return's as any other jump
 *
 * Revision 1.8  1997/04/29  23:50:33  skadron
 * Added r31 info to distinguish between return-JRs and other JRs for bpred
 *
 * Revision 1.7  1997/04/29  22:53:04  skadron
 * Hopefully bpred is now right: bpred now allocates entries only for
 *    branches; on a BTB miss it still returns a direction; and it uses a
 *    return-address stack.  Returns are not yet distinguished among JR's
 *
 * Revision 1.6  1997/04/28  17:37:02  skadron
 * Bpred now allocates entries for any instruction instead of only
 *    branches; also added return-address stack
 *
 * Revision 1.5  1997/04/24  16:57:21  skadron
 * Bpred used to return no prediction if the indexing branch didn't match
 *    in the BTB.  Now it can predict a direction even on a BTB address
 *    conflict
 *
 * Revision 1.4  1997/03/27  16:31:52  skadron
 * Fixed bug: sim-outorder calls bpred_after_priming(), even if no bpred
 *    exists.  Now we check for a null ptr.
 *
 * Revision 1.3  1997/03/25  16:16:33  skadron
 * Statistics now take account of priming: statistics report only
 *    post-prime info.
 *
 * Revision 1.2  1997/02/24  18:02:41  skadron
 * Fixed output format of a formula stat
 *
 * Revision 1.1  1997/02/16  22:23:54  skadron
 * Initial revision
 *
 *
 */

/* external definitions */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "machine.h"
#include "misc.h"
#include "options.h"
#include "stats.h"
#include "memory.h"
/* interface definitions */
#include "bpred.h"
#include "sim.h"

enum bpclass_t 
str2bpclass(char *s)
{
  if (!mystricmp(s, "taken")) return bpclass_TAKEN;
  else if (!mystricmp(s, "nottaken")) return bpclass_NOTTAKEN;
  else if (!mystricmp(s, "dynamic")) return bpclass_DYNAMIC;
  else fatal("unknown bpred class");

  return bpclass_NUM;
}

#define IS_TAKEN(P,PBITS) ((P) >= (1<<(PBITS-1)))
#define UPDATE_TAKEN(P,PBITS,TAKEN) \
   { if (TAKEN && P < ((1<<PBITS)-1)) { ++P; } else if (!TAKEN && P > 0) { --P; } }

/* an entry in a BTB */
struct bpred_btb_ent_t {
  md_addr_t pc;		/* address of branch being tracked */
  md_addr_t target;		/* last destination of branch when taken */
  struct bpred_btb_ent_t *prev, *next; /* lru chaining pointers */
};

struct bpred_btb_set_t {
  struct bpred_btb_ent_t *head, *tail;
};

struct bpred_btb_t {
  struct bpred_btb_opt_t *opt;
  md_addr_t tag_mask;
  md_addr_t data_mask;
  struct bpred_btb_set_t *sets;
};

/* direction predictor def */
struct bpred_dir_t 
{
  struct bpred_dir_opt_t *opt;
  /* Used by every predictor */
  unsigned char *table;	/* prediction state table */

  /* Used by two-level predictors */
  unsigned int hmask;
  struct bpred_dir_state_t s;
};

struct bpred_ras_t {
  struct bpred_ras_opt_t *opt;
  md_addr_t *stack;
  struct bpred_ras_state_t s;
};

/* branch predictor def */
struct bpred_t 
{
  struct bpred_opt_t *opt;

  struct bpred_dir_t *dir1;       /* first direction predictor */
  struct bpred_dir_t *dir2;       /* second direction predictor */
  struct bpred_dir_t *chooser;    /* direction predictor chooser */

  struct bpred_btb_t *btb;        /* direct branch btb */

  struct bpred_ras_t *ras;        /* return address stack */

  /* stats */
  counter_t lookups;
  counter_t updates;
  counter_t recovers;

  counter_t addr_hits;
  counter_t dir_hits;

  counter_t cond_updates;
  counter_t cond_dir_hits;
  counter_t cond_addr_hits;
  counter_t uncond_updates;
  counter_t uncond_addr_hits;
  counter_t ret_updates;
  counter_t ret_hits;	
};

void
bpred_stats_print(const struct bpred_t *bp,
		  FILE *stream)
{
  print_counter(stream, "bpred.lookups", bp->updates, "total bpred lookups");
  print_counter(stream, "bpred.hits", bp->addr_hits, "total bpred address hits");
  print_rate(stream, "bpred.hit_rate", (double)bp->addr_hits/bp->updates, "bpred address hit-rate");
  print_counter(stream, "bpred.cond_dir_lookups", bp->cond_updates, "bpred conditional branch direction lookups");
  print_counter(stream, "bpred.cond_dir_hits", bp->cond_dir_hits, "bpred conditional branch direction hits");
  print_rate(stream, "bpred.cond_dir_hit_rate", (double)bp->cond_dir_hits/bp->cond_updates, "bpred conditional branch direction hit-rate");
  print_counter(stream, "bpred.ret_lookups", bp->ret_updates, "bpred return address lookups");
  print_counter(stream, "bpred.ret_hits", bp->ret_hits, "bpred return address hits");
  print_rate(stream, "bpred.ret_hit_rate", (double)bp->ret_hits/bp->ret_updates, "bpred return address hit-rate");
}

/* create a branch direction predictor */
static struct bpred_dir_t *
bpred_dir_create (struct bpred_dir_opt_t *opt)
{
  struct bpred_dir_t *dir;
  int i;

  dir = (struct bpred_dir_t *)calloc(1, sizeof(struct bpred_dir_t));
  if (!dir) 
    fatal("out of virtual memory");

  dir->opt = opt;
  dir->hmask = (1 << dir->opt->hbits) - 1;
  dir->table = (char *) calloc (dir->opt->size, sizeof(unsigned char));
  if (!dir->table)
    fatal("cannot allocate BHT storage");
  
  /* initialize counters to weakly this-or-that */
  for (i = 0; i < dir->opt->size; i++)
    dir->table[i] = (1<<(dir->opt->pbits-1))-(i & 1);

  dir->s.history = 0;

  return dir;
}

struct bpred_btb_t * 
bpred_btb_create(struct bpred_btb_opt_t *opt)
{
  int i,j;
  struct bpred_btb_t *btb;

  btb = (struct bpred_btb_t *) calloc (1, sizeof(struct bpred_btb_t));
  if (!btb)
    fatal("cannot allocate btb");

  btb->opt = opt;
  btb->sets = (struct bpred_btb_set_t *) calloc 
    (btb->opt->nsets, sizeof(struct bpred_btb_set_t)); 
  if (!btb->sets)
    fatal("cannot allocate btb data");

  btb->tag_mask = UNSHIFT_PC((((md_addr_t)1 << btb->opt->tbits)-1) << log_base2(btb->opt->nsets));
  btb->data_mask = UNSHIFT_PC(((md_addr_t)1 << btb->opt->dbits)-1);

  /* Initialize way lists */
  for (i = 0; i < btb->opt->nsets; i++)
    {
      struct bpred_btb_set_t *set = &btb->sets[i];
      set->head = set->tail = NULL;
      
      for (j = 0; j < btb->opt->assoc - 1; j++)
	{
	  struct bpred_btb_ent_t *ent;
	  
	  ent = (struct bpred_btb_ent_t *)calloc(1, sizeof(struct bpred_btb_ent_t));
	  if (!ent)
	    fatal("unable to allocate btb entry!");

	  if (set->head) set->head->prev = ent;
	  else set->tail = ent;
	  ent->next = set->head;
	  set->head = ent;
	}
    }

  return btb;
}

static struct bpred_ras_t * 
bpred_ras_create(struct bpred_ras_opt_t *opt)
{
  struct bpred_ras_t *ras;

  ras = (struct bpred_ras_t *) calloc (1, sizeof(struct bpred_ras_t));
  if (!ras)
    fatal("cannot allocate ras");

  ras->opt = opt;

  ras->s.tos = ras->s.bos = 0;
  ras->stack = (md_addr_t *) calloc (ras->opt->size, sizeof(md_addr_t));
  if (!ras->stack)
    fatal("cannot allocate ras stack");
  
  return ras;
}

void
bpred_reg_options(struct opt_odb_t *odb,
		  struct bpred_opt_t *opt)
{
  opt_reg_string(odb, "-bpred:dirmethod", 
		 "predictor type {nottaken|taken|dynamic}",
                 &opt->opt, /* default */opt->opt,
                 /* print */TRUE, /* format */NULL);
 
  opt_reg_string(odb, "-bpred:dir1", 
		 "first direction predictor config {none|<size>:<pbits>}",
		 &opt->dir1_opt.opt, /* default */opt->dir1_opt.opt,
		 /* print */TRUE, /* format */NULL);

  opt_reg_string(odb, "-bpred:dir2", 
		 "second direction predictor config {none|<size>:<pbits>:<hbits>:<pcshift>}",
		 &opt->dir2_opt.opt, /* default */opt->dir2_opt.opt,
		 /* print */TRUE, /* format */NULL);

  opt_reg_string(odb, "-bpred:dirchooser", 
		 "direction predictor chooser config {<size>}",
		 &opt->chooser_opt.opt, /* default */opt->chooser_opt.opt,
		 /* print */TRUE, /* format */NULL);

  opt_reg_string(odb, "-bpred:ras", 
		 "return address config {none|<size>}",
		 &opt->ras_opt.opt, /* default */opt->ras_opt.opt, 
		 /* print */TRUE, /* format */NULL);

  opt_reg_string(odb, "-bpred:btb", 
		 "BTB config {none|<nsets>:<assoc>:<dbits>:<tbits>}",
		 &opt->btb_opt.opt, /* default */opt->btb_opt.opt,
		 /* print */TRUE, /* format */NULL);
}

void
bpred_check_options(struct bpred_opt_t *opt)
{
  opt->bpclass = str2bpclass(opt->opt);
  
  if (mystricmp(opt->dir1_opt.opt, "none"))
    {
      if (sscanf(opt->dir1_opt.opt, "%d:%d:%d:%d", 
		 &opt->dir1_opt.size, &opt->dir1_opt.pbits, &opt->dir1_opt.hbits, &opt->dir1_opt.pcshift) != 4)
	fatal("bad bpred:dir1 parameters '%s'", opt->dir1_opt.opt);
      if (!IS_POWEROF2(opt->dir1_opt.size))
	fatal("bpred:dir1 <size> '%d' must be positive and a power of 2", opt->dir1_opt.size);
      if (opt->dir1_opt.pbits <= 0 || opt->dir1_opt.pbits > 8)
	fatal("bpred:dir1 <pbits> '%d' must be > 0 and <= 8", opt->dir1_opt.pbits);
      if (opt->dir1_opt.hbits < 0 || opt->dir1_opt.hbits > 30)
	fatal("bpred:dir1 <hbits> '%d' must be >= 0 and <= 30", opt->dir1_opt.hbits);
      if (opt->dir1_opt.pcshift < 0 || opt->dir1_opt.hbits > 31)
	fatal("bpred:dir1 <pcshift> '%d' must be >= 0 and <= 30", opt->dir1_opt.pcshift);
    }
  
  if (mystricmp(opt->dir2_opt.opt, "none"))
    {
      if (sscanf(opt->dir2_opt.opt, "%d:%d:%d:%d", 
		 &opt->dir2_opt.size, &opt->dir2_opt.pbits, &opt->dir2_opt.hbits, &opt->dir2_opt.pcshift) != 4)
	fatal("bad bpred:dir2 parameters '%s'", opt->dir2_opt.opt);
      if (!IS_POWEROF2(opt->dir2_opt.size))
	fatal("bpred:dir2 <size> '%d' must be positive and a power of 2", opt->dir2_opt.size);
      if (opt->dir2_opt.pbits <= 0 || opt->dir2_opt.pbits > 8)
	fatal("bpred:dir2 <pbits> '%d' must be > 0 and <= 8", opt->dir2_opt.pbits);
      if (opt->dir2_opt.hbits < 0 || opt->dir2_opt.hbits > 30)
	fatal("bpred:dir2 <hbits> '%d' must be >= 0 and <= 30", opt->dir2_opt.hbits);
      if (opt->dir2_opt.pcshift < 0 || opt->dir2_opt.hbits > 31)
	fatal("bpred:dir2 <pcshift> '%d' must be >= 0 and <= 30", opt->dir2_opt.pcshift);
    }
  
  if (mystricmp(opt->chooser_opt.opt, "none"))
    {
      if (!mystricmp(opt->dir1_opt.opt, "none") || !mystricmp(opt->dir2_opt.opt, "none"))
	fatal("bpred:chooser must have both bpred:dir1 and bpred:dir2 to choose between");
      if (sscanf(opt->chooser_opt.opt, "%d:%d", &opt->chooser_opt.size, &opt->chooser_opt.pbits) != 2)
	fatal("bad bpred:chooser parameters");
      if (opt->chooser_opt.size <= 0 || !IS_POWEROF2(opt->chooser_opt.size))
	fatal("bpred:chooser <size> '%d' must be positive and a power of 2", opt->chooser_opt.size);
      if (opt->chooser_opt.pbits <= 0 || opt->chooser_opt.pbits > 8)
	fatal("bpred:chooser <pbits> '%d' must be > 0 and <= 8", opt->chooser_opt.pbits);
    }
  else
    {
      if (mystricmp(opt->dir1_opt.opt, "none") && mystricmp(opt->dir2_opt.opt, "none"))
	fatal("bpred:chooser is needed to choose between bpred:dir1 and bpred:dir2");
    }
   
  if (!mystricmp(opt->btb_opt.opt, "none"))
    {
      opt->btb_opt.nsets = opt->btb_opt.assoc = 0;
    }
  else 
    {
      if (sscanf(opt->btb_opt.opt, "%d:%d:%d:%d", 
		 &opt->btb_opt.nsets, &opt->btb_opt.assoc, &opt->btb_opt.dbits, &opt->btb_opt.tbits) != 4)
	fatal("bad bpred:btb parameters");

      if (opt->btb_opt.nsets <= 0 || !IS_POWEROF2(opt->btb_opt.nsets))
	fatal("bpred:btb <nsets> '%d' must be positive and a power of 2", opt->btb_opt.nsets);
      if (opt->btb_opt.assoc <= 0)
	fatal("bpred:btb <assoc> '%d' must be positive", opt->btb_opt.assoc);
      if (opt->btb_opt.assoc > 1 && !opt->btb_opt.tbits)
	fatal("Cannot have a btb that is both associative and untagged");
    }

  if (!mystricmp(opt->ras_opt.opt, "none"))
    opt->ras_opt.size = 0;
  else 
    {
      if (sscanf(opt->ras_opt.opt, "%d", &opt->ras_opt.size) != 1)
	fatal("bad bpred:ras parameters");
      
      if (opt->ras_opt.size <= 0 || !IS_POWEROF2(opt->ras_opt.size))
	fatal("bpred:ras <size> must be positive and a power of 2", opt->ras_opt.size);
    }
}

/* create a branch predictor */
struct bpred_t *			/* branch predictory instance */
bpred_create(struct bpred_opt_t *opt)
{
  struct bpred_t *bp;

  bp = (struct bpred_t *)calloc(1, sizeof(struct bpred_t));
  if (!bp) 
    fatal("out of virtual memory");

  bp->opt = opt;

  /* allocate direction predictors */
  if (mystricmp(bp->opt->dir1_opt.opt, "none"))
    bp->dir1 = bpred_dir_create(&bp->opt->dir1_opt);

  if (mystricmp(bp->opt->dir2_opt.opt, "none"))
    bp->dir2 = bpred_dir_create(&bp->opt->dir2_opt);

  if (mystricmp(bp->opt->chooser_opt.opt, "none"))
    bp->chooser = bpred_dir_create(&bp->opt->chooser_opt);

  /* allocate ras */
  if (bp->opt->ras_opt.size > 0)
    bp->ras = bpred_ras_create(&bp->opt->ras_opt);

  /* allocate btb */
  if (bp->opt->btb_opt.nsets > 0)
    bp->btb = bpred_btb_create(&bp->opt->btb_opt);

  return bp;
}

#define PC_HASH(PC, SIZE) SHIFT_PC(PC) & (SIZE - 1)

/* returns a pointer to counter */
static bool_t			
bpred_dir_lookup(struct bpred_dir_t *dir,	  /* direction predictor */
		 md_addr_t pc)
{
  int index = (dir->s.history & dir->hmask) ^ (SHIFT_PC(pc) << dir->opt->pcshift);
  return IS_TAKEN(dir->table[MOD(index, dir->opt->size)],dir->opt->pbits);
}

static void
bpred_dir_spec_update(struct bpred_dir_t *dir,
		      md_addr_t pc,
		      enum md_opcode_t op,
		      int taken,
		      md_addr_t targ_pc)
{
  if (dir->opt->hbits == 0)
    return;

  if (!MD_OP_HASFLAGS(op, F_COND))
    return;

  dir->s.history <<= 1;
  dir->s.history |= taken;
}

static md_addr_t
bpred_btb_lookup(struct bpred_btb_t *btb,
		 md_addr_t pc)

{
  struct bpred_btb_set_t *set = &btb->sets[MOD(SHIFT_PC(pc), btb->opt->nsets)];
  struct bpred_btb_ent_t *ent;
  for (ent = set->head; ent; ent = ent->next)
    if ((ent->pc & btb->tag_mask) == (pc & btb->tag_mask))
      return (pc & ~btb->data_mask) | (ent->target & btb->data_mask);
  
  return 0;
}

static md_addr_t 
bpred_ras_lookup(struct bpred_ras_t *ras,
		 md_addr_t pc,
		 enum md_opcode_t op)
{
  if (op != RETN)
    fatal("ras consulted on non-return");

  return (ras->s.bos != ras->s.tos) ? (ras->stack[ras->s.tos]) : 0;
}

static void
bpred_ras_spec_update(struct bpred_ras_t *ras,
		      md_addr_t pc,
		      enum md_opcode_t op,
		      int taken,
		      md_addr_t targ_pc)
{
  if (op == JSR || op == BSR)
    {
      INC_MOD(ras->s.tos, ras->opt->size);
      ras->stack[ras->s.tos] = pc + sizeof(md_inst_t);
      if (ras->s.tos == ras->s.bos)
	INC_MOD(ras->s.bos, ras->opt->size);
    }
  else if (op == RETN)
    {
      /* ras is not empty */
      if (ras->s.tos != ras->s.bos)
	DEC_MOD(ras->s.tos, ras->opt->size);
    }

  ras->s.tos_entry = ras->stack[ras->s.tos];
}
		      
/* probe a predictor for a next fetch address, the predictor is probed
   with branch address BADDR, the branch target is BTARGET (used for
   static predictors), and OP is the instruction opcode (used to simulate
   predecode bits; a pointer to the predictor state entry (or null for jumps)
   is returned in *DIR_UPDATE_PTR (used for updating predictor state),
   and the non-speculative top-of-stack is returned in stack_recover_idx 
   (used for recovering ret-addr stack after mis-predict).  */
void
bpred_checkpoint_state(struct bpred_t *bp,
		       struct bpred_state_t *pre_state)
{
  if (bp->dir1) pre_state->dir1_s = bp->dir1->s;
  if (bp->dir2) pre_state->dir2_s = bp->dir2->s;
  if (bp->ras) pre_state->ras_s = bp->ras->s;
}

md_addr_t				/* predicted branch target addr */
bpred_lookup(struct bpred_t *bp,	/* branch predictor instance */
	     md_addr_t pc,	        /* branch address */
	     enum md_opcode_t op, 
	     struct bpred_state_t *pre_state)
{
  bool_t taken = TRUE;
  md_addr_t target;

  bpred_checkpoint_state(bp, pre_state);

  if (!MD_OP_HASFLAGS(op, F_CTRL))
    return pc + sizeof(md_inst_t);

  if (sample_mode == sample_ON) bp->lookups++;

  /* Except for jumps, get a pointer to direction-prediction bits, 
     Taken defaults to true for unconditional jumps */
  if (MD_OP_HASFLAGS(op, F_COND))
    {
      switch (bp->opt->bpclass)
	{
      case bpclass_DYNAMIC:
	{
	  if (bp->chooser)
	    {
	      pre_state->dir1 = bpred_dir_lookup(bp->dir1, pc);
	      pre_state->dir2 = bpred_dir_lookup(bp->dir2, pc);
	      pre_state->chooser = bpred_dir_lookup(bp->chooser, pc);
	      
	      taken = (pre_state->chooser) ? (pre_state->dir2) : (pre_state->dir1);
	    }
	  else if (bp->dir1)
	    {
	      taken = bpred_dir_lookup(bp->dir1, pc);
	    }
	  else
	    {
	      taken = bpred_dir_lookup(bp->dir2, pc);
	    }
	}
      break;
      case bpclass_TAKEN:
	{
	  taken = TRUE;
	}
	break;
      case bpclass_NOTTAKEN:
	{
	  taken = FALSE;
	}
	break;
      default:
	panic("bogus predictor class");
      }
    }

  if (!taken)
    {
      target = pc + sizeof(md_inst_t);
    }
  else 
    {
      /* Returns */
      if (op == RETN && bp->ras)
	target = bpred_ras_lookup(bp->ras, pc, op);
      /* Everything else */
      else if (bp->btb)
	target = bpred_btb_lookup(bp->btb, pc);
      else
	target = 0;
    }

  /* Speculative updates of all state info */
  if (bp->dir1) bpred_dir_spec_update(bp->dir1, pc, op, taken, target);
  if (bp->dir2) bpred_dir_spec_update(bp->dir2, pc, op, taken, target);
  if (bp->ras) bpred_ras_spec_update(bp->ras, pc, op, taken, target);

  return target;
}

/* Speculative execution can corrupt the ret-addr stack.  So for each
 * lookup we return the top-of-stack (TOS) at that point; a mispredicted
 * branch, as part of its recovery, restores the TOS using this value --
 * hopefully this uncorrupts the stack. */
static void
bpred_dir_recover(struct bpred_dir_t *dir,
		  md_addr_t pc,
		  enum md_opcode_t op,
		  md_addr_t next_pc,
		  struct bpred_dir_state_t *dir_s)
{
  if (!dir->opt->hbits)
    return;

  dir->s = *dir_s;

  /* Is this a recover/fixup or just a recover? */
  if (MD_OP_HASFLAGS(op, F_CTRL|F_COND))
    {
      bool_t taken = (next_pc != pc + sizeof(md_inst_t)) ? 1 : 0;

      dir->s.history <<= 1;
      dir->s.history |= taken;
    }
}

static void
bpred_ras_recover(struct bpred_ras_t *ras,
		  md_addr_t pc,
		  enum md_opcode_t op,
		  md_addr_t next_pc,
		  struct bpred_ras_state_t *ras_s)
{
  ras->s = *ras_s;
  ras->stack[ras->s.tos] = ras->s.tos_entry;

  if (op == BSR || op == JSR)
    {
      INC_MOD(ras->s.tos, ras->opt->size);
      ras->stack[ras->s.tos] = pc + sizeof(md_inst_t);
      if (ras->s.tos == ras->s.bos)
	INC_MOD(ras->s.bos, ras->opt->size);
    }
  else if (op == RETN)
    {
      /* ras is not empty */
      if (ras->s.bos != ras->s.tos)
	DEC_MOD(ras->s.tos, ras->opt->size);
    }

  ras->s.tos_entry = ras->stack[ras->s.tos];
}

void
bpred_recover(struct bpred_t *bp,	/* branch predictor instance */
	      md_addr_t pc,	        /* branch address */
	      enum md_opcode_t op,
	      md_addr_t next_pc,
	      struct bpred_state_t *pre_state)	/* recovery info */
{
  if (sample_mode == sample_ON) bp->recovers++; 
  
  if (bp->ras)
    bpred_ras_recover(bp->ras, pc, op, next_pc, &pre_state->ras_s);
  if (bp->dir1)
    bpred_dir_recover(bp->dir1, pc, op, next_pc, &pre_state->dir1_s);
  if (bp->dir2)
    bpred_dir_recover(bp->dir2, pc, op, next_pc, &pre_state->dir2_s);
}

/* update the branch predictor, only useful for stateful predictors; updates
   entry for instruction type OP at address PC.  BTB only gets updated
   for branches which are taken.  Inst was determined to jump to
   address BTARGET and was taken if TAKEN is non-zero.  Predictor 
   statistics are updated with result of prediction, indicated by CORRECT and 
   PRED_TAKEN, predictor state to be updated is indicated by *DIR_UPDATE_PTR 
   (may be NULL for jumps, which shouldn't modify state bits).  Note if
   bpred_update is done speculatively, branch-prediction may get polluted. */

static void
bpred_btb_update(struct bpred_btb_t *btb,
		 md_addr_t pc,
		 md_addr_t targ_pc)
{
  struct bpred_btb_set_t *set = &btb->sets[MOD(SHIFT_PC(pc), btb->opt->nsets)];
  struct bpred_btb_ent_t *ent = NULL;

  if (btb->opt->assoc == 1)
    {
      ent = set->head;
    }
  /* Associative btb, do LRU */
  else 
    {
      for (ent = set->head; ent; ent = ent->next)
	if (ent->pc == pc)
	  break;

      if (!ent)
	ent = set->tail;

      /* LRU */
      if (ent != set->head)
	{
	  /* Reset tail */
	  if (ent == set->tail) set->tail = ent->prev;

	  /* Unlink */
	  if (ent->prev) ent->prev->next = ent->next;
	  if (ent->next) ent->next->prev = ent->prev;
      
	  /* Link to head */
	  ent->prev = NULL;
	  set->head->prev = ent;
	  ent->next = set->head;
	  set->head = ent;
	}
    }

  ent->pc = pc;
  ent->target = targ_pc;
}

static void
bpred_dir_update(struct bpred_dir_t *dir,
		 md_addr_t pc,
		 bool_t taken,
		 struct bpred_dir_state_t *dir_s)
{
  int index = (dir_s->history & dir->hmask) ^ (SHIFT_PC(pc) << dir->opt->pcshift);
  UPDATE_TAKEN(dir->table[MOD(index, dir->opt->size)],dir->opt->pbits,taken);
}

void
bpred_update(struct bpred_t *bp,     /* branch predictor instance */
	     md_addr_t pc,	     /* branch address */
	     enum md_opcode_t op,    /* instruction */
	     md_addr_t next_pc,      /* actual next address */
	     md_addr_t targ_pc,	     /* resolved branch target */
	     md_addr_t pred_pc,      /* predicted branch target */
	     struct bpred_state_t *pre_state)/* predictor state pointer */
{
  bool_t taken, pred_taken, addr_correct, dir_correct;
  md_addr_t fallthru_pc = pc + sizeof(md_inst_t);

  /* don't change bpred state for non-branch instructions or if this
   * is a stateless predictor*/
  if (!MD_OP_HASFLAGS(op, F_CTRL))
    return;

  if (sample_mode == sample_ON) bp->updates++;

  taken = (targ_pc == next_pc) ? TRUE : FALSE;
  pred_taken = (pred_pc != fallthru_pc || pred_pc == targ_pc) ? TRUE : FALSE;
  addr_correct = (next_pc == pred_pc);
  dir_correct = (taken == pred_taken);

  if (sample_mode == sample_ON)
    {
      /* statistics */
      if (addr_correct)
	bp->addr_hits++;
      if (dir_correct)
	bp->dir_hits++;
      
      /* More specific stats for returns and indirect jumps/calls */
      if (op == RETN)
	{
	  bp->ret_updates++;
	  if (addr_correct) bp->ret_hits++; 
	}
      else if (MD_OP_HASFLAGS(op, F_COND))
	{
	  bp->cond_updates++;
	  if (dir_correct)
	    bp->cond_dir_hits++;
	  if (addr_correct)
	    bp->cond_addr_hits++;
	}
      else
	{
	  bp->uncond_updates++;
	  if (addr_correct)
	    bp->uncond_addr_hits++;
	}
    }

  if (MD_OP_HASFLAGS(op, F_COND))
    {
      /* Update direction info, if this was not a conditional branch, we would 
	 not have these pointers */
      if (bp->dir2)
	bpred_dir_update(bp->dir2, pc, taken, &pre_state->dir2_s);
      
      /* combining predictor also updates second predictor and meta predictor */
      /* second direction predictor */
      if (bp->dir1)
	bpred_dir_update(bp->dir1, pc, taken, &pre_state->dir1_s);
      
      /* chooser: only update if directions were different */
      if (bp->chooser && pre_state->dir1 != pre_state->dir2)
	bpred_dir_update(bp->chooser, pc, pre_state->dir2 == taken, &pre_state->chooser_s);

    }

  /* No updates per se for returns, if there is a ras, otherwise may need to
     update BTB */
  if (bp->ras && op == RETN)
    return;

  /* Update BTB for taken branches only */
  if (taken && bp->btb)
    bpred_btb_update(bp->btb, pc, targ_pc);
}




