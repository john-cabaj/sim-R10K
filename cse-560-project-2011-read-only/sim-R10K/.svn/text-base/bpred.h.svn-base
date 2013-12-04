/*
 * bpred.h - branch predictor interfaces
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
 * $Id: bpred.h,v 1.1.1.1 1997/05/22 18:04:05 aklauser Exp $
 *
 * $Log: bpred.h,v $
 * Revision 1.1.1.1  1997/05/22 18:04:05  aklauser
 *
 * Revision 1.8  1997/05/01 20:23:06  skadron
 * BTB bug fixes; jumps no longer update direction state; non-taken
 *    branches non longer update BTB
 *
 * Revision 1.7  1997/05/01 00:05:51  skadron
 * Separated BTB from direction-predictor
 *
 * Revision 1.6  1997/04/29  23:50:44  skadron
 * Added r31 info to distinguish between return-JRs and other JRs for bpred
 *
 * Revision 1.5  1997/04/29  22:53:10  skadron
 * Hopefully bpred is now right: bpred now allocates entries only for
 *    branches; on a BTB miss it still returns a direction; and it uses a
 *    return-address stack.  Returns are not yet distinguished among JR's
 *
 * Revision 1.4  1997/04/28  17:37:09  skadron
 * Bpred now allocates entries for any instruction instead of only
 *    branches; also added return-address stack
 *
 * Revision 1.3  1997/04/24  16:57:27  skadron
 * Bpred used to return no prediction if the indexing branch didn't match
 *    in the BTB.  Now it can predict a direction even on a BTB address
 *    conflict
 *
 * Revision 1.2  1997/03/25 16:17:21  skadron
 * Added function called after priming
 *
 * Revision 1.1  1997/02/16  22:23:54  skadron
 * Initial revision
 *
 *
 */

#ifndef BPRED_H
#define BPRED_H

#define dassert(a) assert(a)

/*
 * This module implements a number of branch predictor mechanisms.  The
 * following predictors are supported:
 *
 *	BPred2Level:  two level adaptive branch predictor
 *
 *		It can simulate many prediction mechanisms that have up to
 *		two levels of tables. Parameters are:
 *		     N   # entries in first level (# of shift register(s))
 *		     W   width of shift register(s)
 *		     M   # entries in 2nd level (# of counters, or other FSM)
 *		One BTB entry per level-2 counter.
 *
 *		Configurations:   N, W, M
 *
 *		    counter based: 1, 0, M
 *
 *		    GAg          : 1, W, 2^W
 *		    GAp          : 1, W, M (M > 2^W)
 *		    PAg          : N, W, 2^W
 *		    PAp          : N, W, M (M == 2^(N+W))
 *
 *	BPred2bit:  a simple direct mapped bimodal predictor
 *
 *		This predictor has a table of two bit saturating counters.
 *		Where counter states 0 & 1 are predict not taken and
 *		counter states 2 & 3 are predict taken, the per-branch counters
 *		are incremented on taken branches and decremented on
 *		no taken branches.  One BTB entry per counter.
 *
 *	BPredTaken:  static predict branch taken
 *
 *	BPredNotTaken:  static predict branch not taken
 *
 */

/* branch predictor types */
enum bpclass_t {
  bpclass_TAKEN, 
  bpclass_NOTTAKEN,
  bpclass_DYNAMIC,
  bpclass_NUM
};

struct bpred_dir_opt_t
{
  char *opt;

  unsigned int size;
  unsigned int pbits;
  unsigned int hbits;
  unsigned int pcshift;
};

struct bpred_btb_opt_t 
{
  char *opt;
  
  unsigned int nsets;
  unsigned int assoc;
  unsigned int dbits;
  unsigned int tbits;
};

struct bpred_ras_opt_t 
{
  char *opt;
  unsigned int size;
};

struct bpred_opt_t
{
  char *opt;
  enum bpclass_t bpclass;
  
  struct bpred_dir_opt_t dir1_opt;
  struct bpred_dir_opt_t dir2_opt;
  struct bpred_dir_opt_t chooser_opt;

  struct bpred_btb_opt_t btb_opt;
  struct bpred_ras_opt_t ras_opt;
};

/* Internal declarations, nobody on the outside needs to know what
   these look like */
struct bpred_t;

/* branch predictor update and recovery information */
struct bpred_ras_state_t {
  int bos;
  int tos;
  md_addr_t tos_entry;
};

struct bpred_dir_state_t {
  unsigned int history;
};

struct bpred_state_t {

  int dir1, dir2, chooser;

  /* direction/path histories */
  struct bpred_dir_state_t dir1_s;
  struct bpred_dir_state_t dir2_s;
  struct bpred_dir_state_t chooser_s;

  struct bpred_ras_state_t ras_s;
};

void
bpred_reg_options(struct opt_odb_t *odb,
		  struct bpred_opt_t *opt);

void
bpred_check_options(struct bpred_opt_t *opt);


struct bpred_t *
bpred_create(struct bpred_opt_t *opt);

/* print predictor stats */
void 
bpred_stats_print(const struct bpred_t *bp,
		  FILE *stream);

void
bpred_checkpoint_state(struct bpred_t *bp,
		       struct bpred_state_t *pre_state);

/* probe a predictor for a next fetch address, the predictor is probed
   with branch address BADDR, the branch target is BTARGET (used for
   static predictors), and OP is the instruction opcode (used to simulate
   predecode bits; a pointer to the predictor state entry (or null for jumps)
   is returned in *DIR_UPDATE_PTR (used for updating predictor state),
   and the non-speculative top-of-stack is returned in stack_recover_idx 
   (used for recovering ret-addr stack after mis-predict).  */
md_addr_t				/* predicted branch target addr */
bpred_lookup(struct bpred_t *pred,	/* branch predictor instance */
	     md_addr_t pc,	        /* branch address */
	     enum md_opcode_t op, 
	     struct bpred_state_t *pre_state);

/* Speculative execution can corrupt the ret-addr stack.  So for each
 * lookup we return the top-of-stack (TOS) at that point; a mispredicted
 * branch, as part of its recovery, restores the TOS using this value --
 * hopefully this uncorrupts the stack. */
void
bpred_recover(struct bpred_t *pred,	/* branch predictor instance */
	      md_addr_t pc,	        /* branch address */
	      enum md_opcode_t op, 
	      md_addr_t next_pc,     /* branch outcome */
	      struct bpred_state_t *pre_state);

/* update the branch predictor, only useful for stateful predictors; updates
   entry for instruction type OP at address BADDR.  BTB only gets updated
   for branches which are taken.  Inst was determined to jump to
   address BTARGET and was taken if TAKEN is non-zero.  Predictor 
   statistics are updated with result of prediction, indicated by CORRECT and 
   PRED_TAKEN, predictor state to be updated is indicated by *DIR_UPDATE_PTR 
   (may be NULL for jumps, which shouldn't modify state bits).  Note if
   bpred_update is done speculatively, branch-prediction may get polluted. */
void
bpred_update(struct bpred_t *pred,	/* branch predictor instance */
	     md_addr_t pc,	        /* branch address */
	     enum md_opcode_t op,            /* instruction */
	     md_addr_t next_pc,
	     md_addr_t targ_pc,	/* resolved branch target */
	     md_addr_t pred_pc,	/* predicted branch target */
	     struct bpred_state_t *pre_state);/* predictor state pointer */

typedef void (*bpred_handler_t)(md_addr_t pc,
				enum md_opcode_t op,
				md_addr_t next_pc,
				md_addr_t targ_pc);

#endif /* BPRED_H */

