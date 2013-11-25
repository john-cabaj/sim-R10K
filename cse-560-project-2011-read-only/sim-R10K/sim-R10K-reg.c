/*
 * sim-outorder.c - sample out-of-order issue perf simulator implementation
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
 * $Id: sim-outorder.c,v 1.5 1998/08/27 16:27:48 taustin Exp taustin $
 *
 * $Log: sim-outorder.c,v $
 * Revision 1.5  1998/08/27 16:27:48  taustin
 * implemented host interface description in host.h
 * added target interface support
 * added support for register and memory contexts
 * instruction predecoding moved to loader module
 * Alpha target support added
 * added support for quadword's
 * added fault support
 * added option ("-max:inst") to limit number of instructions analyzed
 * explicit BTB sizing option added to branch predictors, use
 *       "-btb" option to configure BTB
 * added queue statistics for IFQ, RUU, and LSQ; all terms of Little's
 *       law are measured and reports; also, measures fraction of cycles
 *       in which queue is full
 * added fast forward option ("-fastfwd") that skips a specified number
 *       of instructions (using functional simulation) before starting timing
 *       simulation
 * sim-outorder speculative loads no longer allocate memory pages,
 *       this significantly reduces memory requirements for programs with
 *       lots of mispeculation (e.g., cc1)
 * branch predictor updates can now optionally occur in ID, WB,
 *       or CT
 * added target-dependent myprintf() support
 * fixed speculative quadword store bug (missing most significant word)
 * sim-outorder now computes correct result when non-speculative register
 *       operand is first defined speculative within the same inst
 * speculative fault handling simplified
 * dead variable "no_ea_dep" removed
 *
 * Revision 1.4  1997/04/16  22:10:23  taustin
 * added -commit:width support (from kskadron)
 * fixed "bad l2 D-cache parms" fatal string
 *
 * Revision 1.3  1997/03/11  17:17:06  taustin
 * updated copyright
 * `-pcstat' option support added
 * long/int tweaks made for ALPHA target support
 * better defaults defined for caches/TLBs
 * "mstate" command supported added for DLite!
 * supported added for non-GNU C compilers
 * buglet fixed in speculative trace generation
 * multi-level cache hierarchy now supported
 * two-level predictor supported added
 * I/D-TLB supported added
 * many comments added
 * options package supported added
 * stats package support added
 * resource configuration options extended
 * pipetrace support added
 * DLite! support added
 * writeback throttling now supported
 * decode and issue B/W now decoupled
 * new and improved (and more precise) memory scheduler added
 * cruft for TLB paper removed
 *
 * Revision 1.1  1996/12/05  18:52:32  taustin
 * Initial revision
 *
 *
 */

// #define RENAME_DEBUG 
#define INLINE 
#define STATIC 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <signal.h>

#include "host.h"
#include "misc.h"
#include "machine.h"

#ifndef TARGET_ALPHA
#error This simulator is targeted to ALPHA ISA only
#endif /* TARGET_ALPHA */

#include "stats.h"
#include "options.h"
#include "memory.h"
#include "cache.h"
#include "loader.h"
#include "syscall.h"
#include "resource.h"
#include "sim.h"
#include "predec.h"
#include "bpred.h"
#include "adisambig.h"
#include "fastfwd.h"

/* simulated registers */
static struct regs_t regs;

/* simulated memory */
static struct mem_t *mem = NULL;

/* simulator options */

/* instruction fetch parameters */
static int fetch_width;
static int fetch_lat;

/* rename parameters */
static int rename_lat;
static int rename_width;

/* Physical Register Scheme */
static bool_t l1_pregfile_cache;

/* L1 Physical Register */
static unsigned int l1_pregfile_size;
static unsigned int l1_pregfile_rwidth;
static unsigned int l1_pregfile_wwidth;
static unsigned int l1_pregfile_lat;

/* L2 Physical Register */
static unsigned int l2_pregfile_size;
static unsigned int l2_pregfile_rwidth;
static unsigned int l2_pregfile_wwidth;
static unsigned int l2_pregfile_lat;

/* scheduling paramters */
static bool_t sched_inorder;
static bool_t sched_spec;
static int sched_lat;
static int sched_agen_lat;
static int sched_fwd_lat;
static int sched_width[sclass_NUM];
static int sched_rs_num;

/* memory disambiguation */
static struct adisambig_opt_t sched_adisambig_opt;

/* commit parameters */
static int commit_width;
static int commit_store_width;

/* recover paramters */
static int recover_width;

/* functional unit parameters (internal to module) */
static struct respool_opt_t respool_opt;

/* memory hierarchy parameters */
static struct cache_opt_t cache_dl1_opt;
static struct cache_opt_t cache_il1_opt;
static struct cache_opt_t cache_l2_opt;
static struct cache_opt_t dtlb_opt;
static struct cache_opt_t itlb_opt;
static int tlb_miss_lat;
static int mem_lat;

/* branch predictor parameters */
static struct bpred_opt_t bpred_opt;

/* counters and statistics */
tick_t sim_cycle = 1;
counter_t sim_num_insn = 0;

int l1_preg_readNum = 0, l2_preg_readNum = 0;
int l1_preg_writeNum = 0, l2_preg_writeNum = 0;

static counter_t n_insn_commit_sum;
static counter_t n_insn_commit[ic_NUM];

static counter_t n_insn_fetch;
static counter_t n_insn_rename;
static counter_t n_insn_exec[ic_NUM];

static counter_t n_branch_misp;
static counter_t n_load_squash;

static counter_t n_reg_read          = 0;
static counter_t n_reg_writes       = 0;
static counter_t n_reg_read_miss    = 0;
static counter_t n_reg_writes_miss  = 0;

/* simulator structures */

/* PREG_link_t: link to physical register.  Used to create transient
   event and dependence lists.  Tag is used to invalidate a link on
   the fly without checking the lists.  When a link is created,
   link->tag is set equal to link->preg->tag.  If preg->tag is
   incremented, all links to that preg become invalid. */
struct PREG_link_t {
	struct PREG_link_t *next;
	struct preg_t *preg;
	tag_t tag;

	union {
		tick_t when;
		seq_t seq;
		int opnum;
	} x;
};

/* physical register: holds R10000 renamed values and dependence links
   for register scheduling.  preg_np_t and preg_list_t are structure
   for managing lists of pregs */
struct preg_np_t
{
	struct preg_t *next, *prev;
};

struct preg_list_t
{
	struct preg_t *head, *tail;
	int num;
};

enum reg_file_enum {
	L1_PREG_FILE = 1,
	L2_PREG_FILE = 2
};

struct preg_t
{
	/* free list */
	struct preg_np_t flist;

	regnum_t pregnum;
	tag_t tag;

	bool_t f_allocated;

	/* these are the values */
	union val_t val;

	enum md_fault_t fault;

	struct INSN_station_t *is;

	tick_t when_written;

        bool_t             dirty;
	bool_t             bypassValue;
	int                latency;
	tick_t             lru;
	enum reg_file_enum regFile;
	regnum_t           regCache;

	struct PREG_link_t *odeps_head;
	struct PREG_link_t *odeps_tail;
};

/* INSN_station_t are instruction descriptors, which are used as slot
   occupiers in the IFQ and ROB */
struct INSN_station_t
{
	struct INSN_station_t *prev;
	struct INSN_station_t *next;
	struct INSN_station_t *fnext;

	const struct predec_insn_t *pdi;      /* decoded instruction */
	md_addr_t PC;			        /* inst PC  */
	md_addr_t NPC;		        /* next PC */
	md_addr_t TPC;                        /* target PC */
	md_addr_t PPC;		        /* predicted PC */

	bool_t f_wrong_path;

	struct LDST_station_t *ls;            /* LSQ entry */

	bool_t f_rs;                          /* has a reservation station? */

	bool_t f_bmisp;			/* mis-speculated branch */
	struct bpred_state_t bp_pre_state;	/* bpred direction update info */

	bool_t idep_ready[DEP_INUM];		/* input operand ready? */
	tick_t time_ready[DEP_INUM];		/* input operand time */
	regnum_t pregnums[DEP_NUM];         /* physical register holding result */
	regnum_t fregnum;                   /* physical register to free or commit */

	tag_t tag;			        /* RUU slot tag, increment to squash */
	seq_t seq;			        /* used to sort the ready list
                                           and tag inst */

        int    insnStall;
	int    regReadLatency[DEP_INUM];
	int    regWriteLatency[DEP_NUM];

	struct
	{
		tick_t predicted;          /* branch predicted */
		tick_t fetched;            /* fetched */
		tick_t renamed;            /* put into window */
		tick_t regread;
		tick_t ready;
		tick_t issued;
		tick_t completed;
		tick_t resolved;          /* resolved */
		tick_t committed;          /* committed */
	} when;         /* when did each of the timestamp  */
};

/* Queue of INSN_station_t: used to implement the ROB and IFQ */
struct INSN_queue_t
{
	struct INSN_station_t *head, *tail;
	int num, size;
	counter_t count;
};

/* LDST_station_t are the entries in the load store queue (LSQ).  They
   hold information used in scheduling memory operations. */
struct LDST_station_t
{
	struct LDST_station_t *next, *prev, *fnext;

	struct INSN_station_t *is;    /* contains the address and opcode */
	union val_t val;              /* value, for forwarding */
	tag_t tag;                    /* tag used to squash entry */

	md_addr_t addr;
	int dsize;

	bool_t f_stall;
};

/* Queue of LDST_station_t: used to implement the LSQ */
struct LDST_queue_t
{
	struct LDST_station_t *head, *tail;
	int snum, ssize, lnum, lsize;
	counter_t scount, lcount;
};

/* Simulator state */

/* INSN_station_t freelist (simulator only, does not exist in actual procesor) */
static struct INSN_station_t *INSN_flist = NULL;
static int INSN_num = 0;

/* LDST_station_t freelist (simulator only, does not exist in actual processor) */
static struct LDST_station_t *LDST_flist = NULL;
static int LDST_num = 0;

/* PREG_link_t freelist (simulator only, does not exist in actual processor) */
#define MAX_PREG_LINKS                    4096
static struct PREG_link_t *plink_free_list;
static int n_plink = 0;
static int plink_num = 0;
static int n_plink_asserts = 0;

/* fetch state */
static md_addr_t commit_NPC;
static bool_t f_wrong_path;
static md_addr_t fetch_PC;
static tick_t fetch_resume;
static tick_t rename_resume;

/* logical registers (map table) */
static regnum_t *lregs;

/* physical register array and freelist (mirrors the one in the actual microarchitecture) */
static struct preg_t *pregs;
static struct preg_list_t pregs_flist;
static struct preg_list_t l1_pregs_flist;
static struct preg_list_t l2_pregs_flist;

/* reservation station tracker */
static int rs_num;

/* instruction fetch queue (IFQ) */
static struct INSN_queue_t IFQ;
/* reorder buffer (ROB) */
static struct INSN_queue_t ROB;
/* load-store queue (LSQ) */
static struct LDST_queue_t LSQ;

/* the ready instruction queue (queue from which instructions are scheduled) */
static struct PREG_link_t *scheduler_queue = NULL;
/* pending writeback event queue, sorted from soonest to latest event (in time), NOTE:
   PREG_link nodes are used for the list so that it need not be updated during squash events */
static struct PREG_link_t *writeback_queue = NULL;

/* global sequence counter */
static seq_t seq;

/* caches and tlbs */
static struct cache_t *cache_il1 = NULL;
static struct cache_t *cache_dl1 = NULL;
static struct cache_t *cache_l2 = NULL;
static struct cache_t *itlb = NULL;
static struct cache_t *dtlb = NULL;

/* branch predictor */
static struct bpred_t *bpred = NULL;

static struct respool_t *respool = NULL;

/* address disambiguation collision history table */
static struct cht_t *cht = NULL;

/* simulator implementation */

/* INSN_station_t freelist and queue management functions */
STATIC void
INSN_init(void)
{
	int i;
	int INSN_size = IFQ.size + ROB.size + 1;

	INSN_flist = (struct INSN_station_t *)mycalloc(INSN_size, sizeof(struct INSN_station_t));

	for (i = 0; i < INSN_size - 1; i++)
		INSN_flist[i].fnext = &INSN_flist[i+1];
}

STATIC INLINE struct INSN_station_t *
INSN_alloc(void)
{
	struct INSN_station_t *is = INSN_flist;

	if (is)
	{
		INSN_flist = is->fnext;
		is->fnext = NULL;
		INSN_num++;
	}

	return is;
}

STATIC INLINE void
INSN_free(struct INSN_station_t *is)
{
	tag_t tag = is->tag;
	assert(is->prev == NULL && is->next == NULL);
	memset((byte_t*)is, 0, sizeof(struct INSN_station_t));
	is->tag = tag+1; /* squash */

	is->fnext = INSN_flist;
	INSN_flist = is;
	INSN_num--;
}

STATIC INLINE void
INSN_enqueue(struct INSN_queue_t *q,
		struct INSN_station_t *is)
{
	is->prev = q->tail;
	if (q->tail) q->tail->next = is;
	else q->head = is;
	q->tail = is;

	q->num++;
}

STATIC INLINE void
INSN_remove(struct INSN_queue_t *q,
		struct INSN_station_t *is)
{
	if (is->prev) is->prev->next = is->next;
	if (is->next) is->next->prev = is->prev;

	if (q->head == is) q->head = is->next;
	if (q->tail == is) q->tail = is->prev;

	is->next = is->prev = NULL;

	q->num--;
}

/* LDST_station_t freelist management functions */
STATIC void
LDST_init(void)
{
	int i;
	int LDST_size = LSQ.ssize + LSQ.lsize;

	LDST_flist = (struct LDST_station_t *)mycalloc(LDST_size, sizeof(struct LDST_station_t));
	for (i = 0; i < LDST_size - 1; i++)
		LDST_flist[i].fnext = &LDST_flist[i+1];
}

STATIC INLINE struct LDST_station_t *
LDST_alloc(void)
{
	struct LDST_station_t *ls = LDST_flist;

	if (ls)
	{
		LDST_flist = ls->fnext;
		ls->fnext = NULL;
		LDST_num++;
	}

	ls->addr = ls->dsize = 0;
	return ls;
}

STATIC INLINE void
LDST_free(struct LDST_station_t *ls)
{
	tag_t tag = ls->tag;
	assert(ls->next == NULL && ls->prev == NULL);
	memset((byte_t *)ls, 0, sizeof(struct LDST_station_t));
	ls->tag = tag+1; /* squash */
	ls->fnext = LDST_flist;
	LDST_flist = ls;
	LDST_num--;
}

STATIC INLINE void
LDST_enqueue(struct LDST_queue_t *q,
		struct LDST_station_t *ls,
		bool_t f_store)
{
	ls->prev = q->tail;
	if (q->tail) q->tail->next = ls;
	else         q->head = ls;

	q->tail = ls;

	if (f_store) q->snum++;
	else q->lnum++;
}

STATIC INLINE void
LDST_remove(struct LDST_queue_t *q,
		struct LDST_station_t *ls,
		bool_t f_store)
{
	if (ls->prev) ls->prev->next = ls->next;
	if (ls->next) ls->next->prev = ls->prev;

	if (q->head == ls) q->head = ls->next;
	if (q->tail == ls) q->tail = ls->prev;

	ls->prev = ls->next = NULL;

	if (f_store) q->snum--;
	else q->lnum--;
}


/* PREG_link_t management functions */
#define PLINK_set(LINK, PREG)                                       \
		{ (LINK)->next = NULL; (LINK)->preg = (PREG); if (PREG) { (LINK)->tag = (PREG)->tag; } }

#define PLINK_valid(LINK)                                          \
		((LINK)->preg && (LINK)->tag == (LINK)->preg->tag)

STATIC void
PLINK_assert(void)
{
	int n_free_link = 0;
	int n_reg_link = 0, n_valid_reg_link = 0;
	int n_writeback_link = 0, n_valid_writeback_link = 0;
	int n_scheduler_link = 0, n_valid_scheduler_link = 0;

	regnum_t pregnum;
	struct PREG_link_t *l;

	n_plink_asserts++;

	for (l = plink_free_list; l; l = l->next, n_free_link++);
	if (n_free_link != n_plink)
		panic("IS_link screwup!");

	for (pregnum = 0; pregnum < (l1_pregfile_size+l2_pregfile_size); pregnum++)
		for (l = pregs[pregnum].odeps_head; l; l = l->next, n_reg_link++)
			if (PLINK_valid(l)) n_valid_reg_link++;

	for (l = writeback_queue; l; l = l->next, n_writeback_link++)
		if (PLINK_valid(l)) n_valid_writeback_link++;

	for (l = scheduler_queue; l; l = l->next, n_scheduler_link++)
		if (PLINK_valid(l)) n_valid_scheduler_link++;

	if (n_reg_link + n_writeback_link + n_scheduler_link + n_plink != MAX_PREG_LINKS)
		panic("leaking IS_links");
}

/* free an IS link record */
STATIC INLINE void
PLINK_free(struct PREG_link_t *l)
{
	l->preg = NULL; l->tag = 0;
	l->next = plink_free_list;
	plink_free_list = l;
	n_plink++;
	plink_num--;
}

/* free an IS link list */
STATIC INLINE void
PLINK_free_list(struct PREG_link_t *l)
{
	struct PREG_link_t *lf;
	while (l)
	{
		lf = l;
		l = l->next;
		PLINK_free(lf);
	}
}

STATIC void
PLINK_purge(void)
{
	regnum_t pregnum;
	struct  PREG_link_t *l, *pl, *nl;

	for (pregnum = 0; pregnum < (l1_pregfile_size+l2_pregfile_size); pregnum++)
		for (pl = NULL, l = pregs[pregnum].odeps_head; l; l = nl)
		{
			nl = l->next;
			if (!PLINK_valid(l))
			{
				if (pl) pl->next = nl;
				else pregs[pregnum].odeps_head = nl;

				if (l == pregs[pregnum].odeps_tail)
					pregs[pregnum].odeps_tail = pl;

				PLINK_free(l);
			}
			else
			{
				/* advance trailing pointer */
				pl = l;
			}
		}

	for (pl = NULL, l = writeback_queue; l; l = nl)
	{
		nl = l->next;
		if (!PLINK_valid(l))
		{
			if (pl) pl->next = nl;
			else writeback_queue = nl;
			PLINK_free(l);
		}
		else
		{
			pl = l;
		}
	}

	for (pl = NULL, l = scheduler_queue; l; l = nl)
	{
		nl = l->next;
		if (!PLINK_valid(l))
		{
			if (pl) pl->next = nl;
			else scheduler_queue = nl;
			PLINK_free(l);
		}
		else
		{
			pl = l;
		}
	}

	PLINK_assert();
}

/* get a new IS link record */
STATIC INLINE struct PREG_link_t *
PLINK_new(void)
{
	struct PREG_link_t *l;

	if (!plink_free_list)
	{
		PLINK_purge();
		if (!plink_free_list)
			panic("out of is links");
	}
	l = plink_free_list;

	plink_free_list = l->next;
	l->next = NULL;
	n_plink--;
	plink_num++;
	return l;
}

/* initialize the free IS_LINK pool */
STATIC void
PLINK_init(int nlinks)			/* total number of IS_LINK available */
{
	int i;

	plink_free_list = (struct PREG_link_t *) mycalloc (nlinks, sizeof(struct PREG_link_t));
	if (!plink_free_list)
		fatal("out of virtual memory");

	for (i=0; i < nlinks - 1; i++)
		plink_free_list[i].next = &plink_free_list[i+1];

	n_plink = nlinks;
}




/* a reservation station link: this structure links elements of a RUU
   reservation station list; used for ready instruction queue, event queue, and
   output dependency lists; each RS_LINK node contains a pointer to the RUU
   entry it references along with an instance tag, the RS_LINK is only valid if
   the instruction instance tag matches the instruction RUU entry instance tag;
   this strategy allows entries in the RUU can be squashed and reused without
   updating the lists that point to it, which significantly improves the
   performance of (all to frequent) squash events */

#define LE_IN_LIST(LE, LNAME, LIST) \
		((LIST)->num > 0 && ((LE)->LNAME.prev || (LE)->LNAME.next || (LE) == (LIST)->head || (LE) == (LIST)->tail))

#define LE_UNCHAIN(LE, LNAME, LIST) \
		{ \
	if (!LE_IN_LIST(LE, LNAME, LIST)) panic("not in ht"); \
	if ((LE)->LNAME.prev) (LE)->LNAME.prev->LNAME.next = (LE)->LNAME.next; \
	if ((LE)->LNAME.next) (LE)->LNAME.next->LNAME.prev = (LE)->LNAME.prev; \
	if ((LE) == (LIST)->head) (LIST)->head = (LE)->LNAME.next; \
	if ((LE) == (LIST)->tail) (LIST)->tail = (LE)->LNAME.prev; \
	(LE)->LNAME.next = (LE)->LNAME.prev = NULL; \
	(LIST)->num--; \
	if ((LIST)->num < 0) panic("list->num < 0"); \
		}

#define LE_CHAIN(LE, LNAME, LIST) \
		{ \
	if (LE_IN_LIST(LE, LNAME, LIST)) panic("le already in list"); \
	if ((LIST)->tail) (LIST)->tail->LNAME.next = (LE); \
	(LE)->LNAME.prev = (LIST)->tail; \
	(LIST)->tail = (LE); \
	if (!(LIST)->head) (LIST)->head = (LE); \
	(LIST)->num++; \
		}



/*
 * cache/TLB miss handlers
 */

STATIC unsigned int		        /* latency of block access */
null_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		md_addr_t baddr,	/* block address to access */
		unsigned int bsize,	/* size of block to access */
		tick_t when,
		bool_t miss_info[ct_NUM])
{
	return 0;
}

STATIC unsigned int		        /* latency of block access */
tlb_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		md_addr_t baddr,	/* block address to access */
		unsigned int bsize,	/* size of block to access */
		tick_t when,
		bool_t miss_info[ct_NUM])
{
	/* fake translation, for now.  Return tlb miss latency */
	return tlb_miss_lat;
}

STATIC unsigned int		        /* latency of block access */
l2_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		md_addr_t baddr,	/* block address to access */
		unsigned int bsize,	/* size of block to access */
		tick_t when,
		bool_t miss_info[ct_NUM])
{
	return cmd == mc_READ ? mem_lat : 0;
}

STATIC unsigned int		        /* latency of block access */
l1_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		md_addr_t baddr,	/* block address to access */
		unsigned int bsize,	/* size of block to access */
		tick_t when,
		bool_t miss_info[ct_NUM])
{
	int lat = 0;

	if (cache_l2)
		lat = cache_access(cache_l2, cmd, baddr, bsize, when + lat, miss_info, l2_miss_handler);
	else
		lat = mem_lat;

	return cmd == mc_READ ? lat : 0;
}


/* miss handlers for the cache warmup phases */
STATIC unsigned int		        /* latency of block access */
warmup_l1_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		md_addr_t baddr,	/* block address to access */
		unsigned int bsize,	/* size of block to access */
		tick_t when,
		bool_t miss_info[ct_NUM])
{
	if (cache_l2)
		cache_access(cache_l2, cmd, baddr, bsize, when, NULL, null_miss_handler);

	return 0;
}

STATIC void
warmup_handler(const struct predec_insn_t *pdi)
{
	if (cache_il1)
		cache_access(cache_il1, mc_READ, regs.PC, sizeof(md_inst_t), 0, NULL, warmup_l1_miss_handler);

	if (itlb)
		cache_access(itlb, mc_READ, regs.PC, sizeof(md_inst_t), 0, NULL, null_miss_handler);

	if (pdi->iclass == ic_load || pdi->iclass == ic_store || pdi->iclass == ic_prefetch)
	{
		enum mem_cmd_t dl1_cmd = (pdi->iclass == ic_load) ? mc_READ : (pdi->iclass == ic_store ? mc_WRITE : mc_PREFETCH);
		enum mem_cmd_t dtlb_cmd = (pdi->iclass == ic_load || pdi->iclass == ic_store) ? mc_READ : mc_PREFETCH;
		if (cache_dl1)
			cache_access(cache_dl1, dl1_cmd, regs.addr, regs.dsize, 0, NULL, warmup_l1_miss_handler);
		if (dtlb)
			cache_access(dtlb, dtlb_cmd, regs.addr, regs.dsize, 0, NULL, null_miss_handler);
	}
	else if (pdi->iclass == ic_ctrl)
	{
		if (bpred)
		{
			struct bpred_state_t bpred_pre_state;
			md_addr_t ppc;
			ppc = bpred_lookup(bpred, regs.PC, pdi->poi.op, &bpred_pre_state);
			if (ppc != regs.NPC)
				bpred_recover(bpred, regs.PC, pdi->poi.op, regs.NPC, &bpred_pre_state);

			bpred_update(bpred, regs.PC, pdi->poi.op, regs.NPC, regs.TPC, ppc, &bpred_pre_state);
		}
	}
}

/* register simulator-specific options */
void
sim_reg_options(struct opt_odb_t *odb)
{
	opt_reg_header(odb,
			"sim-R10K: This simulator implements an out-of-order issue\n"
			"superscalar processor with a two-level memory system and speculative\n"
			"execution.  This simulator is a performance simulator, tracking the\n"
			"latency of all pipeline operations.\n"
	);

	/* branch predictor options */
	bpred_opt.opt = "dynamic";
	bpred_opt.dir1_opt.opt = "1024:2:0:0";
	bpred_opt.dir2_opt.opt = "1024:2:8:0";
	bpred_opt.chooser_opt.opt = "1024:2";
	bpred_opt.ras_opt.opt = "8";
	bpred_opt.btb_opt.opt = "512:4:16:8";
	bpred_reg_options(odb, &bpred_opt);

	/* memory hierarchy options */

	/* caches & tlbs */
	cache_dl1_opt.ct = ct_L1;
	cache_dl1_opt.name = "dl1";
	cache_dl1_opt.opt = "512:32:2:l";
	cache_reg_options(odb, &cache_dl1_opt);

	dtlb_opt.ct = ct_TLB;
	dtlb_opt.name = "dtlb";
	dtlb_opt.opt = "32:4096:4:l";
	cache_reg_options(odb, &dtlb_opt);

	cache_il1_opt.ct = ct_L1;
	cache_il1_opt.name = "il1";
	cache_il1_opt.opt = "512:32:2:l";
	cache_reg_options(odb, &cache_il1_opt);

	itlb_opt.ct = ct_TLB;
	itlb_opt.name = "itlb";
	itlb_opt.opt = "32:4096:4:l";
	cache_reg_options(odb, &itlb_opt);

	cache_l2_opt.ct = ct_L2;
	cache_l2_opt.name = "l2";
	cache_l2_opt.opt = "1024:128:8:l";
	cache_reg_options(odb, &cache_l2_opt);

	/* TLB options */
	opt_reg_int(odb, "-tlb:mlat",
			"inst/data TLB miss latency (in cycles)",
			&tlb_miss_lat, /* default */30,
			/* print */TRUE, /* format */NULL);

	/* memory latency */
	opt_reg_int(odb, "-mem:hlat", "memory access latency",
			&mem_lat, /* default */70,
			/* print */TRUE, /* format */NULL);

	/* fetch options */
	opt_reg_int(odb, "-fetch:width", "instruction fetch queue size (in insts)",
			&fetch_width, /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-fetch:ifq:size", "instruction fetch queue size (in insts)",
			&IFQ.size, /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-fetch:lat",
			"number of pipeline stages in fetch",
			&fetch_lat, /* default */1,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-rename:lat",
			"rename latency",
			&rename_lat, /* default */1,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-rename:width",
			"rename width",
			&rename_width, /* default */4,
			/* print */TRUE, /* format */NULL);

	/* sched options */

	opt_reg_int(odb, "-sched:rob:size",
			"re-order buffer size",
			&ROB.size, /* default */64,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:ldq:size",
			"load queue size",
			&LSQ.lsize, /* default */16,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:stq:size",
			"store queue size",
			&LSQ.ssize, /* default */8,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:rs:size",
			"number of reservation stations",
			&sched_rs_num, /* default */40,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:int:width",
			"instruction issue B/W (insts/cycle)",
			&sched_width[sclass_INT], /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:fp:width",
			"instruction issue B/W (insts/cycle)",
			&sched_width[sclass_FP], /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:store:width",
			"instruction issue B/W (insts/cycle)",
			&sched_width[sclass_STORE], /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:load:width",
			"instruction issue B/W (insts/cycle)",
			&sched_width[sclass_LOAD], /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:total:width",
			"instruction issue B/W (insts/cycle)",
			&sched_width[sclass_TOTAL], /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:lat",
			"number of pipeline stages in issue",
			&sched_lat, /* default */1,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:agen:lat",
			"address generation latency",
			&sched_agen_lat, /* default */1,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-sched:fwd:lat",
			"address generation latency",
			&sched_fwd_lat, /* default */1,
			/* print */TRUE, /* format */NULL);

	opt_reg_flag(odb, "-sched:inorder", "schedule in-order",
			&sched_inorder, /* default */FALSE,
			/* print */TRUE, /* format */NULL);

	opt_reg_flag(odb, "-sched:spec",
			"schedule instructions down wrong execution paths",
			&sched_spec, /* default */TRUE,
			/* print */TRUE, /* format */NULL);

	opt_reg_string(odb, "-sched:adisambig",
			"load address disambiguation policy {conservative|opportunistic|perfect|<cht_sets>:<cht_ways>}",
			&sched_adisambig_opt.opt, /* default */"conservative",
			/* print */TRUE, /* format */NULL);

	/* scheduler options */
	respool_reg_options(odb, &respool_opt);

	/* Pregfile Scheme Options */
	opt_reg_flag(odb, "-pregfile:cache",
			"L1 Physical Register Cache Scheme",
			&l1_pregfile_cache, /* default */FALSE,
			/* print */TRUE, /* format */NULL);

	/* L1 Pregfile Options */
	opt_reg_uint(odb, "-l1_pregfile:size",
			"Number of L1 Physical Registers",
			&l1_pregfile_size, /* default */256,
			/* print */TRUE, /* format */NULL);

	opt_reg_uint(odb, "-l1_pregfile:rwidth",
			"Number of L1 Physical Register Read Ports",
			&l1_pregfile_rwidth, /* default */ 4,
			/* print */TRUE, /* format */NULL);

	opt_reg_uint(odb, "-l1_pregfile:wwidth",
			"Number of L1 Physical Registers Write Ports",
			&l1_pregfile_wwidth, /* default */ 4,
			/* print */TRUE, /* format */NULL);

	opt_reg_uint(odb, "-l1_pregfile:lat",
			"L1 Physical Register Latency",
			&l1_pregfile_lat, /* default */ 1,
			/* print */TRUE, /* format */NULL);

	/* L2 Pregfile Options */
	opt_reg_uint(odb, "-l2_pregfile:size",
			"Number of L2 Physical Registers",
			&l2_pregfile_size, /* default */0,
			/* print */TRUE, /* format */NULL);

	opt_reg_uint(odb, "-l2_pregfile:rwidth",
			"Number of L2 Physical Register Read Ports",
			&l2_pregfile_rwidth, /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_uint(odb, "-l2_pregfile:wwidth",
			"Number of L2 Physical Registers Write Ports",
			&l2_pregfile_wwidth, /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_uint(odb, "-l2_pregfile:lat",
			"L2 Physical Register Latency",
			&l2_pregfile_lat, /* default */1,
			/* print */TRUE, /* format */NULL);

	/* commit options */
	opt_reg_int(odb, "-commit:width",
			"instruction commit B/W (insts/cycle)",
			&commit_width, /* default */4,
			/* print */TRUE, /* format */NULL);

	opt_reg_int(odb, "-commit:store:width",
			"commit cache B/W (re-executed load + store/cycle)",
			&commit_store_width, /* default */1,
			/* print */TRUE, /* format */NULL);

	/* recovery options */
	opt_reg_int(odb, "-recover:width",
			"instruction recovery B/W (insns/cycle)",
			&recover_width, /* default */4,
			/* print */TRUE, /* format */NULL);

	/* pre-decode options */
	predec_reg_options(odb);
}

/* check simulator-specific option values */
void
sim_check_options(void)        /* command line arguments */
{

	if (fetch_width < 1) fatal("fetch width must be positive");
	if (fetch_lat < 1) fatal("fetch must be at least 1 pipe stage");

	if (rename_width < 1) fatal("rename width must be positive");
	if (rename_lat < 1) fatal("rename must be at least 1 pipe stage");

	if (sched_width[sclass_TOTAL] < 1) fatal("total scheduling width must be positive");
	if (sched_width[sclass_INT] < 1 || sched_width[sclass_INT] > sched_width[sclass_TOTAL])
		fatal("int scheduling width must be positive and less than total scheduling width");
	if (sched_width[sclass_FP] < 1 || sched_width[sclass_FP] > sched_width[sclass_TOTAL])
		fatal("fp scheduling width must be positive and less than total scheduling width");
	if (sched_width[sclass_LOAD] < 1 || sched_width[sclass_LOAD] > sched_width[sclass_TOTAL])
		fatal("load scheduling width must be positive and less than total scheduling width");
	if (sched_width[sclass_STORE] < 1 || sched_width[sclass_STORE] > sched_width[sclass_TOTAL])
		fatal("store scheduling width must be positive and less than total scheduling width");

	if (sched_lat < 1) fatal("schedling must be at least 1 pipe stage");
	if (sched_agen_lat < 1) fatal("agen must be at least 1 pipe stage");
	if (sched_fwd_lat < 1) fatal("store-forward must be at least 1 pipe stage");

	if (sched_rs_num < 1) fatal("need at least 1 reservation station");


	if (IFQ.size < 1)  fatal("inst fetch queue size must be positive");
	if (ROB.size < 1) fatal("ROB size must be a positive number > 1");
	if (LSQ.lsize < 1) fatal("LSQ size must be a positive number > 1");
	if (LSQ.ssize < 1) fatal("LSQ size must be a positive number > 1");

	if (commit_width < 1) fatal("commit width must be positive non-zero");
	if (commit_store_width < 1 || commit_store_width > commit_width) fatal("commit store width must be positive and less than commit width");

	if (recover_width < 1) fatal("recover width must be positive");


	adisambig_check_options(&sched_adisambig_opt);
	if (sched_adisambig_opt.strategy == adisambig_CHT)
		cht = cht_create(&sched_adisambig_opt);

	/* memory */
	if (mem_lat < 1)
		fatal("all memory access latencies must be greater than zero");

	if (mystricmp(bpred_opt.opt, "none"))
	{
		bpred_check_options(&bpred_opt);
		bpred = bpred_create(&bpred_opt);
	}

	/* use a level 1 D-cache? */
	if (mystricmp(cache_dl1_opt.opt, "none"))
	{
		cache_check_options(&cache_dl1_opt);
		cache_dl1 = cache_create(&cache_dl1_opt);
	}

	/* use a level 1 I-cache? */
	if (!mystricmp(cache_il1_opt.opt, "dl1"))
	{
		if (!cache_dl1)
			fatal("L1 I-cache cannot access L1 D-cache as it's undefined");
		cache_il1 = cache_dl1;
	}
	else if (mystricmp(cache_il1_opt.opt, "none"))
	{
		cache_check_options(&cache_il1_opt);
		cache_il1 = cache_create(&cache_il1_opt);
	}

	if (mystricmp(cache_l2_opt.opt, "none"))
	{
		if (!cache_il1 && !cache_dl1)
			fatal("can't have an L2 without an L1 D-cache or I-cache!");

		cache_check_options(&cache_l2_opt);
		cache_l2 = cache_create(&cache_l2_opt);
	}

	if (mystricmp(dtlb_opt.opt, "none"))
	{
		cache_check_options(&dtlb_opt);
		dtlb = cache_create(&dtlb_opt);
	}

	/* use an I-TLB? */
	if (!mystricmp(itlb_opt.opt, "dtlb"))
	{
		if (!dtlb)
			fatal("I-TLB cannot use D-TLB as it is undefined");
		itlb = dtlb;
	}
	else if (mystricmp(itlb_opt.opt, "none"))
	{
		cache_check_options(&itlb_opt);
		itlb = cache_create(&itlb_opt);
	}

	/* resource pool */
	respool_check_options(&respool_opt);
	respool = respool_create(&respool_opt);

	/* check pre-decode options */
	predec_check_options();
}

/* print simulator-specific configuration information */
void
sim_aux_config(FILE *stream)            /* output stream */
{
	/* nada */
}

/* register simulator-specific statistics */
void
sim_stats(FILE *stream)   /* stats database */
{
	/* simulation time */
	print_counter(stream, "sim_elapsed_time", sim_elapsed_time, "simulation time in seconds");
	print_rate(stream, "sim_insn_rate", (double)n_insn_commit_sum/sim_elapsed_time, "simulation speed (insts/sec)");

	/* register baseline stats */
	print_counter(stream, "sim_num_insn_sim", sim_num_insn, "instructions simulated (functional and timing)");

	print_counter(stream, "sim_num_insn", n_insn_commit_sum, "instructions committed");
	print_counter(stream, "sim_num_load", n_insn_commit[ic_load], "loads committed");
	print_counter(stream, "sim_num_store", n_insn_commit[ic_store], "stores committed");
	print_counter(stream, "sim_num_branch", n_insn_commit[ic_ctrl], "branches committed");
	print_counter(stream, "sim_num_fp", n_insn_commit[ic_fcomp] + n_insn_commit[ic_fcomplong], "floating point operations commmitted");
	print_counter(stream, "sim_num_prefetch", n_insn_commit[ic_prefetch], "prefetches committed (encountered)");
	print_counter(stream, "sim_num_sys", n_insn_commit[ic_sys], "system-calls committed");

	print_counter(stream, "sim_fetch_insn",   n_insn_fetch, "instructions fetched");
	print_counter(stream, "sim_rename_insn",   n_insn_rename, "instructions fetched");
	print_counter(stream, "sim_exec_insn", ICVEC_ICSUM(n_insn_exec), "instructions executed");

	/* performance stats */
	print_counter(stream, "sim_cycle", sim_cycle, "cycles");
	print_rate(stream, "sim_IPC", (double)n_insn_commit_sum/sim_cycle, "committed instructions per cycle");

	print_counter(stream, "sim_num_branch_misp", n_branch_misp, "branch mispredictions");
	print_counter(stream, "sim_load_squash", n_load_squash, "load squashes");

	print_counter(stream, "n_reg_read", n_reg_read, "reads");
        print_counter(stream, "n_reg_read_miss", n_reg_read_miss, "misses");
	print_rate(stream, "sim_reg_read_miss_rate", (double)n_reg_read_miss/n_reg_read, "rate of read register cache miss");

	print_counter(stream, "n_reg_writes", n_reg_writes, "writes");
	print_counter(stream, "n_reg_writes_miss", n_reg_writes_miss, "misses");
	print_rate(stream, "sim_reg_write_miss_rate", (double)n_reg_writes_miss/n_reg_writes, "rate of wrote register cache miss");
}

/* forward declarations */
STATIC void regs_init(void);
STATIC void INSN_init(void);
STATIC void LDST_init(void);

/* initialize the simulator */
void
sim_init(void)
{
	/* allocate and initialize register file */
	regs_init();

	/* allocate and initialize memory space */
	mem = mem_create("mem");
	mem_init(mem);

	PLINK_init(MAX_PREG_LINKS);
	INSN_init();
	LDST_init();
}

/* load program into simulated state */
void
sim_load_prog(char *fname,		/* program to load */
		int argc, char **argv,	/* program arguments */
		char **envp)		/* program environment */
{
	/* load program text and data, set up environment, memory, and regs */
	ld_load_prog(fname, argc, argv, envp, &regs, mem, TRUE);

	/* init predecoded instruction cache */
	predec_init();
}

/* dump simulator-specific auxiliary simulator statistics */
void
sim_aux_stats(FILE *stream)             /* output stream */
{
	sim_stats(stream);

	if (bpred)
		bpred_stats_print(bpred, stream);

	if (cache_dl1)
		cache_stats_print(cache_dl1, stream);
	if (cache_l2)
		cache_stats_print(cache_l2, stream);
	if (dtlb)
		cache_stats_print(dtlb, stream);
	if (cache_il1 && cache_il1 != cache_dl1)
		cache_stats_print(cache_il1, stream);
	if (itlb && itlb != dtlb)
		cache_stats_print(itlb, stream);
}

/* un-initialize the simulator */
void
sim_uninit(void)
{
	/* nada */
}


/* a register update unit (RUU) station, this record is contained in the
   processors RUU, which serves as a collection of ordered reservations
   stations.  The reservation stations capture register results and await
   the time when all operands are ready, at which time the instruction is
   issued to the functional units; the RUU is an order circular queue, in which
   instructions are inserted in fetch (program) order, results are stored in
   the RUU buffers, and later when an RUU entry is the oldest entry in the
   machines, it and its instruction's value is retired to the architectural
   register file in program order, NOTE: the RUU and LSQ share the same
   structure, this is useful because loads and stores are split into two
   operations: an effective address add and a load/store, the add is inserted
   into the RUU and the load/store inserted into the LSQ, allowing the add
   to wake up the load/store when effective address computation has finished */

#define OPERANDS_READY(IS)                                              \
		(((IS)->idep_ready[DEP_I1] && (IS)->time_ready[DEP_I1] != 0 && (IS)->time_ready[DEP_I1] <= sim_cycle) && \
				((IS)->idep_ready[DEP_I2] && (IS)->time_ready[DEP_I2] != 0 && (IS)->time_ready[DEP_I2] <= sim_cycle) && \
				((IS)->idep_ready[DEP_I3] && (IS)->time_ready[DEP_I3] != 0 && (IS)->time_ready[DEP_I3] <= sim_cycle))


/* physical register and renaming management functions */

/* assert that all logical registers are mapped */
STATIC void
regs_assert(void)
{
	regnum_t lreg;
	for (lreg = 0; lreg < MD_TOTAL_REGS; lreg++)
		assert(lregs[lreg] >= 0 && lregs[lreg] < (l1_pregfile_size+l2_pregfile_size));
}

/* allocate a new physical register from the free list */
STATIC INLINE regnum_t
regs_alloc(void)
{
	struct preg_t *preg = NULL;

	/* Remove from free list */
	if(l1_pregs_flist.num > 0 && !l1_pregfile_cache){
		preg = l1_pregs_flist.head;
		LE_UNCHAIN(preg, flist, &l1_pregs_flist);
	}
	else if(l2_pregs_flist.num > 0){
		preg = l2_pregs_flist.head;
		LE_UNCHAIN(preg, flist, &l2_pregs_flist);
	}
	else {
		panic("regs_alloc() - Out of registers!");
	}

	if (preg->f_allocated)
		panic("register already allocated!");

	preg->fault        = md_fault_none;
	preg->f_allocated  = TRUE;
	preg->when_written = 0;
	preg->bypassValue  = FALSE;
	preg->regCache     = -1;
	preg->lru          = sim_cycle;
	preg->dirty        = FALSE;

	return preg->pregnum;
}

/* allocate a new physical register from the free list */
STATIC INLINE void
cache_regs_alloc(struct preg_t* l2_preg, struct INSN_station_t *parent_is)
{
	struct preg_t *l1_preg = NULL;

	if(l2_preg == NULL)
		panic("NULL Register L2!");

	/* Remove from free list */
	if(l1_pregs_flist.num > 0){
		l1_preg = l1_pregs_flist.head;
		LE_UNCHAIN(l1_preg, flist, &l1_pregs_flist);
	}
	else{
		regnum_t pregnum;
		struct PREG_link_t *link;
		struct preg_t *victim;
		struct preg_t *lru_preg = &pregs[0];


		for (pregnum = 0; pregnum < l1_pregfile_size; pregnum++)
		{
			if((&pregs[pregnum])->lru < lru_preg->lru){
				lru_preg = &pregs[pregnum];
			}
		}

		victim =  &pregs[lru_preg->regCache];
				
		 if((victim->dirty && l2_preg_writeNum  >= l2_pregfile_wwidth || l1_preg_readNum >= l2_pregfile_rwidth))
		 {
			l2_preg->regCache = -1;
			return;
		 }

		if(victim->pregnum < l1_pregfile_size){
			panic("Register Victim from L1!");
		}
		else {
			victim->regCache = -1;
			victim->latency  = l1_pregfile_lat + l2_pregfile_lat;
			victim->regFile  = L2_PREG_FILE;

			if(victim->is != NULL)
				victim->is->regWriteLatency[DEP_O1] = l2_pregfile_lat;

			if(victim->dirty){
				victim->dirty = FALSE;
				parent_is->insnStall = l2_pregfile_lat;
				l2_preg_writeNum++;
				l1_preg_readNum++;
			}

			for (link = victim->odeps_head; link; link = link->next)
			{
				struct preg_t *opreg;
				struct INSN_station_t *ois;

				if (!PLINK_valid(link))
					continue;

				opreg = link->preg;
				ois = opreg->is;
				ois->regReadLatency[link->x.opnum] = victim->latency;
			}

			l1_preg = lru_preg;
		}
	}

	if(l1_preg != NULL){
		struct PREG_link_t *link;

		if(l1_preg->pregnum >= l1_pregfile_size){
			panic("RegisterCache from L2!");
		}
			
		l1_preg->lru                         = sim_cycle;
		l1_preg->regCache                    = l2_preg->pregnum;

		l2_preg->regCache                    = l1_preg->pregnum;
		l2_preg->latency                     = l1_pregfile_lat;
		l2_preg->regFile                     = L1_PREG_FILE;

		if(l2_preg->is != NULL)
			l2_preg->is->regWriteLatency[DEP_O1] = l2_preg->latency;

		for (link = l2_preg->odeps_head; link; link = link->next)
		{
			struct preg_t *opreg;
			struct INSN_station_t *ois;

			if (!PLINK_valid(link))
				continue;

			opreg = link->preg;
			ois = opreg->is;
			ois->regReadLatency[link->x.opnum] = l2_preg->latency;
		}
	}
}

/* return register to free list */
STATIC INLINE void
regs_free(regnum_t fregnum)
{
	struct preg_t *preg = &pregs[fregnum];

	if (!preg->f_allocated) panic("freeing an unallocated register!");
	if (preg->is) panic("preg has an IS attached!");

	preg->f_allocated = FALSE;

	/* free output dependence tree */
	PLINK_free_list(preg->odeps_head);
	preg->odeps_head = preg->odeps_tail = NULL;

	if(l1_pregfile_cache){
		if(preg->pregnum >= l1_pregfile_size && preg->regFile == L1_PREG_FILE){
			preg->regCache = -1;
			preg->latency  = l1_pregfile_lat + l2_pregfile_lat;
			preg->regFile  = L2_PREG_FILE;
		}
	}
	/* Add to free list */
	if(preg->regFile == L1_PREG_FILE){
		LE_CHAIN(preg, flist, &l1_pregs_flist);
	}
	else if(preg->regFile == L2_PREG_FILE){
		LE_CHAIN(preg, flist, &l2_pregs_flist);
	}
	else
		panic("regs_free() - register not in a register type!");

	preg->tag++;
}

/* set a mapping in the map table, return the previous mapping (used
   later in recovery and freeing) */
STATIC INLINE regnum_t
regs_connect(regnum_t lregnum,
		regnum_t pregnum)
{
	regnum_t fregnum;

	if (!REG_ISDEP(lregnum))
		panic("shouldn't happen anymore!");

	fregnum = lregs[lregnum];
	lregs[lregnum] = pregnum;

	return fregnum;
}

STATIC INLINE void
regs_unlink(regnum_t pregnum,
		regnum_t pregnum_unlink)
{
	struct PREG_link_t *link, *plink;
	struct preg_t *preg, *preg_unlink;

	if (pregnum == regnum_NONE || pregnum_unlink == regnum_NONE)
		return;

	preg = &pregs[pregnum];
	preg_unlink = &pregs[pregnum_unlink];

	for (plink = NULL, link = preg->odeps_head; link; plink = link, link = link->next)
		if (link->preg == preg_unlink)
			break;

	if (!link)
		return;

	if (!plink)
		preg->odeps_head = link->next;
	else
		plink->next = link->next;

	if (link == preg->odeps_tail)
		preg->odeps_tail = plink;

	PLINK_free(link);
}

STATIC INLINE regnum_t
regs_rename(regnum_t lregnum)
{
	if (lregnum == regnum_NONE)
		panic("shouldn't be renaming this register!");

	return lregs[lregnum];
}

STATIC INLINE void
regs_commit(regnum_t pregnum)
{
	struct preg_t *preg = &pregs[pregnum];

	if (!preg->f_allocated)
		panic("committing an unallocated register!");

	preg->bypassValue = FALSE;
	/* free output dependence tree */
	PLINK_free_list(preg->odeps_head);
	preg->odeps_head = preg->odeps_tail = NULL;
}

STATIC INLINE void
regs_recover(regnum_t lregnum,
		regnum_t pregnum,
		regnum_t rregnum)
{
	struct preg_t *rreg = &pregs[rregnum];
	/* these are constant mappings */
	/* roll back mapping */
	if (!rreg->f_allocated) panic("rolling back an unallocated register!");
	lregs[lregnum] = rregnum;
}

STATIC void
regs_tosyscall(void)
{
	int i;

	/* Copy values from physical registers to architectural registers */
	for (i = 0; i < MD_TOTAL_REGS; i++)
	{
		regs.regs[i].q = pregs[lregs[i]].val.q;
		regs_free(lregs[i]);
		lregs[i] = regnum_NONE;
	}
}

STATIC void
regs_fromsyscall(void)
{
	int i;

	/* Allocated brand new registers */
	for (i = 0; i < MD_TOTAL_REGS; i++)
	{
		lregs[i] = regs_alloc();
		pregs[lregs[i]].when_written = sim_cycle;
		pregs[lregs[i]].val.q = regs.regs[i].q;
	}
}

STATIC void
regs_func2timing(void)
{
	regs_fromsyscall();

	fetch_PC = regs.PC;
	assert(valid_text_address(mem, fetch_PC));
}

STATIC void
regs_timing2func(void)
{
	regs_tosyscall();

	regs.PC = commit_NPC;
	regs.NPC = regs.PC + sizeof(md_inst_t);
}


STATIC void
regs_init(void)
{
	regnum_t pregnum;

	int maxPregs = l1_pregfile_size + l2_pregfile_size;

	/* allocate physical registers */
	pregs = (struct preg_t *)mycalloc(maxPregs, sizeof(struct preg_t));

	for (pregnum = 0; pregnum < l1_pregfile_size; pregnum++)
	{
		struct preg_t *preg = &pregs[pregnum];
		preg->pregnum = pregnum;
		preg->latency= l1_pregfile_lat;
		preg->regFile = L1_PREG_FILE;
		LE_CHAIN(preg, flist, &l1_pregs_flist);
	}

	for (pregnum = l1_pregfile_size; pregnum < maxPregs; pregnum++)
	{
		struct preg_t *preg = &pregs[pregnum];
		preg->pregnum = pregnum;
		if(l1_pregfile_cache)
			preg->latency = l1_pregfile_lat + l2_pregfile_lat;
		else
			preg->latency = l2_pregfile_lat;

		preg->regFile = L2_PREG_FILE;
		LE_CHAIN(preg, flist, &l2_pregs_flist);
	}

	/* allocate logical registers */
	lregs = (regnum_t *)mycalloc(MD_TOTAL_REGS, sizeof(regnum_t));
}

bool_t
address_collision(const struct LDST_station_t *ls1,
		const struct LDST_station_t *ls2)
{
	return
			((ls1->addr >= ls2->addr) && (ls1->addr < (ls2->addr + ls2->dsize))) ||
			((ls2->addr >= ls1->addr) && (ls2->addr < (ls1->addr + ls1->dsize)));
}

#define LOAD_ADDR_READY(RS)             ((RS)->idep_ready[DEP_ADDR] && (RS)->time_ready[DEP_ADDR] != 0 && (RS)->time_ready[DEP_ADDR] <= sim_cycle)
#define STORE_ADDR_READY(RS)            ((RS)->idep_ready[DEP_ADDR] && (RS)->time_ready[DEP_ADDR] != 0 && (RS)->time_ready[DEP_ADDR] <= sim_cycle)
#define STORE_DATA_READY(RS)            ((RS)->idep_ready[DEP_STORE_DATA] && (RS)->time_ready[DEP_STORE_DATA] != 0 && (RS)->time_ready[DEP_STORE_DATA] <= sim_cycle))

/* Instruction execution functions */

STATIC enum md_fault_t
stq_load(struct mem_t *mem,	/* memory space to access */
		enum mem_cmd_t cmd,	/* Read or Write access cmd */
		md_addr_t addr,		/* virtual address of access */
		void *p,			/* input/output buffer */
		int nbytes)		/* number of bytes to access */
{
	struct LDST_station_t *store;

	/* check alignments, even speculative this test should always pass */
	if (!IS_POWEROFTWO(nbytes) != 0 || (addr & (nbytes-1)) != 0)
		return md_fault_alignment;

	/* check permissions */
	if (!((addr >= ld_text_base && addr < (ld_text_base+ld_text_size)
			&& cmd == mc_READ)
			|| MD_VALID_ADDR(addr)))
	{
#ifdef MD_ACCESS_FAULTS
		return md_fault_access;
#else /* !MD_ACCESS_FAULTS */
		*(quad_t*)p = 0;
		return md_fault_none;
#endif /* MD_ACCESS_FAULTS */
	}

	for (store = LSQ.tail; store; store = store->prev)
	{
		if (store->is->pdi->iclass != ic_store)
			continue;

		/* not the same address */
		if (MD_ALIGN_ADDR(addr) != MD_ALIGN_ADDR(store->addr))
			continue;

		/* same address => bypass */
		*(quad_t*)p = 0;
		READ_QUAD(p, &store->val.q, MD_ADDR_OFFSET(addr), nbytes);
		return md_fault_none;
	}

	return mem_access(mem, cmd, addr, p, nbytes);
}

STATIC enum md_fault_t
stq_store(struct mem_t *mem,	/* memory space to access */
		enum mem_cmd_t cmd,	/* Read or Write access cmd */
		md_addr_t addr,		/* virtual address of access */
		void *p,			/* input/output buffer */
		int nbytes)		/* number of bytes to access */
{
	struct LDST_station_t *store = NULL;
	bool_t partial = FALSE;

	/* check alignments, even speculative this test should always pass */
	if (!IS_POWEROFTWO(nbytes) != 0 || (addr & (nbytes-1)) != 0)
		return md_fault_alignment;

	/* check permissions */
	if (!((addr >= ld_text_base && addr < (ld_text_base+ld_text_size)
			&& cmd == mc_READ)
			|| MD_VALID_ADDR(addr)))
	{
#ifdef MD_ACCESS_FAULTS
		return md_fault_access;
#else /* !MD_ACCESS_FAULTS */
		return md_fault_none;
#endif /* MD_ACCESS_FAULTS */
	}

	for (store = LSQ.tail->prev; store; store = store->prev)
	{
		if (store->is->pdi->iclass != ic_store)
			continue;

		/* combine partials */
		if (MD_ALIGN_ADDR(addr) == MD_ALIGN_ADDR(store->addr))
		{
			partial = TRUE;
			LSQ.tail->val.q = store->val.q;
			break;
		}
	}

	/* read the entire line so that we can merge partials */
	if (!partial)
		mem_access(mem, mc_READ, MD_ALIGN_ADDR(addr),
				&(LSQ.tail->val.q), MD_DATAPATH_WIDTH);

	/* merge partrial */
	WRITE_QUAD(p, &LSQ.tail->val.q, MD_ADDR_OFFSET(addr), nbytes);
	return md_fault_none;
}


#define IR1 DEP_I1
#define IR2 DEP_I2
#define IR3 DEP_I3
#define OR1 DEP_O1

/* program counters */
#define CPC                     (is->pdi->poi.PC)
#define SET_NPC(EXPR)           (is->NPC = (EXPR))
#define SET_TPC(EXPR)		(is->TPC = (EXPR))

/* general purpose register accessors */
#define READ_REG_Q(N)           (pregs[is->pregnums[(N)]].val.q)
#define WRITE_REG_Q(N,EXPR)     (pregs[is->pregnums[(N)]].val.q = (EXPR))
#define READ_REG_F(N)           (pregs[is->pregnums[(N)]].val.d)
#define WRITE_REG_F(N,EXPR)     (pregs[is->pregnums[(N)]].val.d = (EXPR))

/* set address/address mask */
#define SET_ADDR_DSIZE(ADDR,DSIZE)   \
		(is->ls->addr = (ADDR), is->ls->dsize = (DSIZE))

#define READ(ADDR, PVAL, SIZE) stq_load(mem, mc_READ, (ADDR), (PVAL), (SIZE))
#define WRITE(ADDR, PVAL, SIZE) stq_store(mem, mc_WRITE, (ADDR), (PVAL), (SIZE))

/* system call handler macro */
#define SYSCALL(INST)							\
		{/* only execute system calls in non-speculative mode */		\
		regs_tosyscall();                                                 \
		if (fdump && sim_num_insn >= insn_dumpbegin && sim_num_insn < insn_dumpend) \
		md_print_regs(&regs, fdump);                                    \
		sys_syscall(&regs, mem_access, mem, INST, TRUE);                   \
		if (fdump && sim_num_insn >= insn_dumpbegin && sim_num_insn < insn_dumpend) \
		md_print_regs(&regs, fdump);                                    \
		regs_fromsyscall();                                               \
		is->NPC = regs.NPC; is->TPC = regs.TPC;                        \
		}

STATIC void
exec_insn(struct INSN_station_t *is)
{
	md_inst_t inst = is->pdi->inst;

	regs.PC = is->pdi->poi.PC;
	/* compute default next PC */
	regs.NPC = is->NPC = is->pdi->poi.PC + sizeof(md_inst_t);

	/* maintain $r0 semantics (in spec and non-spec space) */
	pregs[lregs[MD_REG_ZERO]].val.q = 0;
	pregs[lregs[MD_FREG_ZERO]].val.d = 0.0;

	/* set default fault - none */
	pregs[is->pregnums[DEP_O1]].fault = md_fault_none;

#undef DECLARE_FAULT
#define DECLARE_FAULT(FAULT) pregs[is->pregnums[DEP_O1]].fault = (FAULT)

	/* execute the instruction */
	switch (is->pdi->poi.op)
	{
#define DEFINST(OP,MSK,NAME,OPFORM,RES,CLASS,O1,O2,I1,I2,I3)		\
		case OP:							\
		/* execute the instruction */					\
		SYMCAT(OP,_IMPL);						\
		break;
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)					\
		case OP:							\
		/* could speculatively decode a bogus inst, convert to NOP */	\
		/* no EXPR */							\
		break;
#define CONNECT(OP)	/* nada... */
#include "machine.def"
	default:
		break;
		/* can speculatively decode a bogus inst, convert to a NOP */
	}
	/* operation sets next PC */
}

/* undef instruction execution engine */
#undef SET_NPC
#undef SET_TPC
#undef SET_TPC
#undef CPC
#undef READ_REG_Q
#undef WRITE_REG_Q
#undef READ_REG_F
#undef WRITE_REF_F
#undef SET_ADDR_DSIZE
#undef READ
#undef WRITE
#undef SYSCALL
#undef IR1
#undef IR2
#undef IR3
#undef OR1

/* recover instruction trace generator state to precise state state immediately
   before the first mis-predicted branch; this is accomplished by resetting
   all register value copied-on-write bitmasks are reset, and the speculative
   memory hash table is cleared */
STATIC void
IFQ_recover(struct INSN_station_t *recover_is)
{
	struct INSN_station_t *is;

	/* reset trace generation mode */
	f_wrong_path = recover_is->f_wrong_path;

	/* squash everything in IFQ */
	while ((is = IFQ.head))
	{
		INSN_remove(&IFQ, is);
		INSN_free(is);
	}

	if (IFQ.num)
		panic("should have cleaned this guy out!");

	/* reset IFETCH state */
	fetch_PC = recover_is->NPC;
	fetch_resume = sim_cycle + 1;

	/* special case: recovering to a mispredicted branch which has
     not yet resolved */
	if (recover_is->f_bmisp)
	{
		f_wrong_path = TRUE;
		fetch_PC = recover_is->PPC;
	}

	assert(recover_is->f_wrong_path || valid_text_address(mem, fetch_PC));
}

void
IFQ_cleanup(void)
{
	struct INSN_station_t *is;

	/* squash everything in IFQ */
	while ((is = IFQ.head))
	{
		INSN_remove(&IFQ, is);
		INSN_free(is);
	}

	f_wrong_path = 0;
	fetch_PC = 0;
	fetch_resume = 0;
	rename_resume = 0;
}

/* recover processor microarchitecture state back to point of the
   mis-predicted branch at RUU[BRANCH_INDEX] */
STATIC void
ROB_recover(struct INSN_station_t *recover_is,
		bool_t f_bmisp)
{
	int n_recover = 0;
	/* recover from the tail of the RUU towards the head until the branch index
     is reached, this direction ensures that the LSQ can be synchronized with
     the RUU */

	if (!ROB.tail)
		panic("empty RUU");

	/* traverse to older insts until the mispredicted branch is encountered */
	while (ROB.tail && ROB.tail != recover_is)
	{
		struct INSN_station_t *is = ROB.tail;
		struct preg_t *preg = &pregs[is->pregnums[DEP_O1]];

		assert(preg->pregnum == is->pregnums[DEP_O1] && preg->is == is);

		/* is this operation an effective addr calc for a load or store? */
		if (is->pdi->iclass == ic_store || is->pdi->iclass == ic_load || is->pdi->iclass == ic_prefetch)
		{
			struct LDST_station_t *ls = LSQ.tail;
			assert(is->ls == ls && ls->is == is);
			LDST_remove(&LSQ, ls, is->pdi->iclass == ic_store);
			LDST_free(ls);
		}

		if (is->f_rs)
		{
			is->f_rs = FALSE;
			rs_num++;
		}

		preg->is = NULL;
		regs_free(is->pregnums[DEP_O1]);
		regs_recover(is->pdi->lregnums[DEP_O1], is->pregnums[DEP_O1], is->fregnum);

		INSN_remove(&ROB, is);
		INSN_free(is);

		n_recover++;
	}

	rename_resume = sim_cycle + DIV_ROUND_UP(n_recover, recover_width);
}

STATIC void
ROB_cleanup(void)
{
	/* recover from the tail of the RUU towards the head until the branch index
     is reached, this direction ensures that the LSQ can be synchronized with
     the RUU */

	/* traverse to older insts until the mispredicted branch is encountered */
	while (ROB.tail)
	{
		struct INSN_station_t *is = ROB.tail;
		struct preg_t *preg = &pregs[is->pregnums[DEP_O1]];

		assert(preg->is == is);

		/* is this operation an effective addr calc for a load or store? */
		if (is->pdi->iclass == ic_store || is->pdi->iclass == ic_load)
		{
			struct LDST_station_t *ls = LSQ.tail;
			assert(is->ls == ls && ls->is == is);
			LDST_remove(&LSQ, ls, is->pdi->iclass == ic_store);
			LDST_free(ls);
		}

		if (is->f_rs)
		{
			is->f_rs = FALSE;
			rs_num++;
		}

		preg->is = NULL;
		regs_free(is->pregnums[DEP_O1]);
		regs_recover(is->pdi->lregnums[DEP_O1], is->pregnums[DEP_O1], is->fregnum);

		INSN_remove(&ROB, is);
		INSN_free(is);
	}
}

/*
 * the ready instruction queue implementation follows, the ready instruction
 * queue indicates which instruction have all of there *register* dependencies
 * satisfied, instruction will issue when 1) all memory dependencies for
 * the instruction have been satisfied (see lsq_refresh() for details on how
 * this is accomplished) and 2) resources are available; ready queue is fully
 * constructed each cycle before any operation is issued from it -- this
 * ensures that instruction issue priorities are properly observed; NOTE:
 * IS_LINK nodes are used for the event queue list so that it need not be
 * updated during squash events
 */

/* insert ready node into the ready list using ready instruction scheduling
   policy; currently the following scheduling policy is enforced:

     memory and long latency operands, and branch instructions first

   then

     all other instructions, oldest instructions first

  this policy works well because branches pass through the machine quicker
  which works to reduce branch misprediction latencies, and very long latency
  instructions (such loads and multiplies) get priority since they are very
  likely on the program's critical path */
STATIC void
scheduler_enqueue(struct preg_t *preg) 		/* IS to enqueue */
{
	struct PREG_link_t *pnode, *node, *nnode, *new_node;

	if (!OPERANDS_READY(preg->is))
		return;

	/* locate insertion point */
	for (pnode = NULL, node = scheduler_queue;
			node;
			node = nnode)
	{
		nnode = node->next;

		/* Deal with invalid nodes */
		if (!PLINK_valid(node))
		{
			if (pnode) pnode->next = nnode;
			else scheduler_queue = nnode;

			PLINK_free(node);
			continue;
		}

		/* already on scheduler's list */
		if (preg->is->seq == node->preg->is->seq)
			return;

		/* put it here */
		if (preg->is->seq < node->preg->is->seq)
			break;

		pnode = node;
	}

	/* get a free ready list node */
	new_node = PLINK_new();
	PLINK_set(new_node, preg);
	new_node->x.seq = preg->is->seq;

	if (pnode)
	{
		/* insert middle or end */
		new_node->next = pnode->next;
		pnode->next = new_node;
	}
	else
	{
		/* insert at beginning */
		new_node->next = scheduler_queue;
		scheduler_queue = new_node;
	}

	preg->is->when.ready = MAX(preg->is->when.regread, sim_cycle);
}

STATIC void
scheduler_cleanup(void)
{
	while (scheduler_queue)
	{
		struct PREG_link_t *plink = scheduler_queue;
		scheduler_queue = plink->next;
		PLINK_free(plink);
	}
}

/* commit store to data cache if there are free ports, used in commit_stage */

STATIC bool_t
commit_store(struct INSN_station_t *is)
{
	struct LDST_station_t *store = LSQ.head;

	assert(is->ls == store);

	/* go to the data cache */
	if (cache_dl1)
		cache_access(cache_dl1, mc_WRITE,
				MD_ALIGN_ADDR(store->addr), MD_DATAPATH_WIDTH,
				sim_cycle, NULL, l1_miss_handler);

	/* all loads and stores must access D-TLB */
	if (dtlb)
		cache_access(dtlb, mc_READ,
				MD_ALIGN_ADDR(store->addr), MD_DATAPATH_WIDTH,
				sim_cycle, NULL, tlb_miss_handler);

	/* Write store value to memory */
	mem_access(mem, mc_WRITE, MD_ALIGN_ADDR(store->addr),
			(byte_t *)&store->val.q, MD_DATAPATH_WIDTH);

	return TRUE;
}

/* this function commits the results of the oldest completed entries from the
   RUU and LSQ. Stores in the LSQ commit their data to the data cache */
STATIC void
commit_stage(void)
{
	static unsigned int registerStall = 0;
	static bool_t calcStall  = TRUE;

	int commit_n = 0, commit_store_n = 0;

	int i = 0, maxLatency = 1;
	struct INSN_station_t *t_is = ROB.head;
	struct preg_t *t_preg       = NULL;

	for(i=0; i<commit_width; i++){
		if(t_is == NULL)
			break;

		t_preg = &pregs[t_is->pregnums[DEP_O1]];
		if(l1_pregfile_cache && t_preg->regFile == L2_PREG_FILE){ 
			cache_regs_alloc(t_preg, t_is);
			if(t_preg->regCache < 0){
				break;
			}
			else {
				n_reg_writes_miss++;
			}
		}
		else if(l1_pregfile_cache && t_preg->pregnum < l1_pregfile_size)
			panic("Using Pure L1 Register!");

		if(t_is->regWriteLatency[DEP_O1] > 0){
			if(t_preg->regFile == L1_PREG_FILE){
				if(l1_preg_writeNum >= l1_pregfile_wwidth){
					break;
				}
				else {
					l1_preg_writeNum++;
				}
			}
			else if(t_preg->regFile == L2_PREG_FILE){
				if(l2_preg_writeNum >= l2_pregfile_wwidth){
					break;
				}
				else {
					l2_preg_writeNum++;
				}
			}

			t_is->regWriteLatency[DEP_O1]--;
		}

		t_is = t_is->next;
	}

	/* all values must be retired to the architected reg file in
     program order */
	while (ROB.head &&
			commit_n < commit_width)
	{
		struct INSN_station_t *is = ROB.head;
		struct preg_t *preg = &pregs[is->pregnums[DEP_O1]];
		struct preg_t *freg = &pregs[is->fregnum];

		/* at least RUU entry must be complete.  BTW, complete
	 means complete last cycle */
		if (is->f_wrong_path)
			panic("committing a wrong-path insn!");

		if (preg->fault != md_fault_none)
			panic("committing faulting instruction!");

		if (/* prefetches are automatically complete */
				is->pdi->iclass != ic_prefetch &&
				is->pdi->iclass != ic_sys &&
				preg->when_written == 0)
			break;

		if (is->pdi->iclass == ic_store)
		{
			if (commit_store_n == commit_store_width)
				break;

			if (!commit_store(is))
				break;

			commit_store_n++;
		}

		if(is->regWriteLatency[DEP_O1] > 0){
			break;
		}

		/* all right, we're committing this guy */
		if(l1_pregfile_cache){
			t_preg->dirty = TRUE;
			n_reg_writes++;
		}

		n_insn_commit[is->pdi->iclass]++;
		n_insn_commit_sum++;
		sim_num_insn++;

		if (is->pdi->iclass == ic_sys)
		{
			/* This preg will be freed.  We will need to allocate a new one */
			preg->is = NULL;
			/* Do the syscall */
			exec_insn(is);
			/* Allocate new physical register */
			is->pregnums[DEP_O1] = lregs[is->pdi->lregnums[DEP_O1]];
			preg = &pregs[is->pregnums[DEP_O1]];

			preg->is = is;
			preg->when_written = sim_cycle;
		}

		else if (is->pdi->iclass == ic_ctrl)
		{
			/* Update branch predictor */
			if (bpred)
				bpred_update(bpred,
						/* branch address */is->PC,
						/* instruction */is->pdi->poi.op,
						/* actual target address */is->NPC,
						/* target address */is->TPC,
						/* predicted target address */is->PPC,
						/* update info */&is->bp_pre_state);

			if (is->NPC != is->PPC)
				n_branch_misp++;
		}

		if (fdump)
		{
			if (sim_num_insn >= insn_dumpbegin && sim_num_insn < insn_dumpend)
			{
				fprintf(fdump, "%-9u: 0x%08x ",
						(word_t)sim_num_insn,
						(word_t)is->PC);
				if (LREG_ISDEP(is->pdi->lregnums[DEP_O1]))
					myfprintf(fdump, " O1: %016p", pregs[is->pregnums[DEP_O1]].val.q);
				if (is->pdi->iclass == ic_load || is->pdi->iclass == ic_store || is->pdi->iclass == ic_prefetch)
					myfprintf(fdump, ", addr: %016p", is->ls->addr);
				fprintf(fdump, "\n");
				fflush(fdump);
			}
			else if (sim_num_insn == insn_dumpend)
			{
				fclose(fdump);
			}
		}

		if (is->pdi->iclass == ic_load || is->pdi->iclass == ic_store || is->pdi->iclass == ic_prefetch)
		{
			/* remove from LSQ */
			struct LDST_station_t *ls = LSQ.head;
			assert(ls == is->ls && is == ls->is);
			LDST_remove(&LSQ, ls, is->pdi->iclass == ic_store);
			LDST_free(ls);
		}

		if (is->f_rs)
		{
			if (is->pdi->iclass != ic_prefetch)
				panic("non-prefetch with RS at retirement!");

			is->f_rs = FALSE;
			rs_num++;
		}

		regs_commit(is->pregnums[DEP_O1]);

		/* committing now */
		is->when.committed = sim_cycle;

		/* free over-written register */
		if (freg->is) panic("what is this guy still doing with an IS?");

		regs_free(is->fregnum);

		/* Reclaim resources of committing register */
		/* Only for "original instance" */
		if (!preg->is)
			panic("what is this guy doing without an IS?");
		preg->is = NULL;

		/* external per-instruction instance counter, occasionally useful */
		commit_NPC = is->NPC;

		INSN_remove(&ROB, is);
		INSN_free(is);

		/* one more instruction committed to architected state */
		commit_n++;
	}
	calcStall = TRUE;
}

/* writeback stage implementation */


/* insert an event for PREG into the writeback queue, event queue is sorted from
   earliest to latest event, event and associated side-effects will be
   apparent at the start of cycle WHEN */
STATIC void
writeback_enqueue(struct preg_t *preg,
		tick_t when)
{
	struct PREG_link_t *prev, *ev, *new_ev;

	if (when <= sim_cycle)
		panic("event occurred in the past");

	for (ev = writeback_queue; ev; ev = ev->next)
		if (PLINK_valid(ev) && ev->preg == preg)
			panic("already a writeback event for this register!");

	/* get a free event record */
	new_ev = PLINK_new();
	PLINK_set(new_ev, preg);
	new_ev->x.when = when;

	/* locate insertion point */
	for (prev=NULL, ev=writeback_queue;
			ev && ev->x.when < when;
			prev=ev, ev=ev->next);

	if (prev)
	{
		/* insert middle or end */
		new_ev->next = prev->next;
		prev->next = new_ev;
	}
	else
	{
		/* insert at beginning */
		new_ev->next = writeback_queue;
		writeback_queue = new_ev;
	}
}

/* return the next event that has already occurred, returns NULL when no
   remaining events or all remaining events are in the future */
STATIC struct preg_t *
writeback_next(void)
{
	while (writeback_queue && writeback_queue->x.when <= sim_cycle)
	{
		struct PREG_link_t *ev = writeback_queue;
		struct preg_t *preg = ev->preg;
		bool_t valid = PLINK_valid(ev);

		/* unlink and return first event on priority list */
		writeback_queue = ev->next;

		PLINK_free(ev);

		if (valid)
			return preg;
	}

	return NULL;
}

STATIC void
writeback_cleanup(void)
{
	while (writeback_queue)
	{
		struct PREG_link_t *plink = writeback_queue;
		writeback_queue = plink->next;
		PLINK_free(plink);
	}
}

/* writeback completed operation results from the functional units to RUU,
   at this point, the output dependency chains of completing instructions
   are also walked to determine if any dependent instruction now has all
   of its register operands, if so the (nearly) ready instruction is inserted
   into the ready instruction queue */
STATIC void
writeback_stage(void)
{
	struct preg_t *preg;
	int l1_preg_writeNum = 0, l2_preg_writeNum = 0;

	/* service all completed events */
	while (writeback_queue && writeback_queue->x.when <= sim_cycle) {
		struct PREG_link_t *link;
		struct PREG_link_t *ev = writeback_queue;
		struct preg_t *preg = ev->preg;
		struct INSN_station_t *is = preg->is;

		// Non-valid entry - go to next and free
		if (!(PLINK_valid(ev))) {
			writeback_queue = ev->next;
			PLINK_free(ev);
			continue;
		}

		/* IS has completed execution and (possibly) produced a result */
		if (!is->when.regread || !is->when.ready || !is->when.issued)
			panic("written back insn !regread, !ready, or !issued");

		// Changed - is can't be WB before its completed
		// Was causing aborts - being too restrictive -WB now takes into account ports so may
		// not always be able to be completed the same cycle.
		if (is->when.completed != 0 && is->when.completed > sim_cycle)
			panic("insn completion timing mismatch!");

		if (!preg->f_allocated)
			panic("physical register not allocated!");

		//Used by commit stage to commit value when at ROB.head
		preg->when_written = is->when.completed;
		//Used to determine if the value is accessed via the bypass network or via the register file
		preg->bypassValue = TRUE;

		/* Are we resolving a mis-predicted branch? */
		if (is->f_bmisp)
		{
			if (is->pdi->iclass != ic_ctrl && is->pdi->poi.op != PAL_CALLSYS)
				panic("mis-predicted non-branch?!?!?");

			/* Mark this instruction as no longer mispredicting */
			is->f_bmisp = FALSE;
			is->when.resolved = sim_cycle;

			/* recover ROB and IFQ, and steer fetch to correct path */
			ROB_recover(is, /* f_bmisp */TRUE);
			IFQ_recover(is);

			/* recover branch predictor state */
			if (bpred)
				bpred_recover(bpred, is->PC, is->pdi->poi.op, is->NPC, &is->bp_pre_state);
		}

		/* wakeup ready instructions */
		/* walk output list, queue up ready operations */
		for (link = preg->odeps_head; link; link = link->next)
		{
			struct preg_t *opreg;
			struct INSN_station_t *ois;

			if (!PLINK_valid(link))
				continue;

			opreg = link->preg;
			ois = opreg->is;

			if (ois->idep_ready[link->x.opnum])
				panic("output dependence already satisfied");

			/* input is now ready */
			ois->idep_ready[link->x.opnum] = TRUE;
			ois->time_ready[link->x.opnum] = sim_cycle; //Assume bypassing so value is ready this cycle

			/* try and schedule this instruction the next time around */
			if (!ois->pdi->iclass == ic_nop)
				scheduler_enqueue(opreg);
		}

		//Grab next entry and free current
		writeback_queue = ev->next;
		PLINK_free(ev);
	}  /* for all writeback events */
}

STATIC bool_t
schedule_load(struct INSN_station_t *is)
{
	int cache_lat = 1, tlb_lat = 1;

	/* invalid load */
	if (!MD_VALID_ADDR(is->ls->addr))
	{
		cache_lat = tlb_lat = sched_agen_lat + sched_fwd_lat;
	}
	else /* valid */
	{
		enum mem_cmd_t cmd = is->pdi->iclass == ic_load ? mc_READ : mc_PREFETCH;
		cache_lat = tlb_lat = sched_agen_lat + sched_fwd_lat;

		if (cache_dl1)
			cache_lat = sched_agen_lat +
			cache_access(cache_dl1, cmd, MD_ALIGN_ADDR(is->ls->addr), MD_DATAPATH_WIDTH,
					sim_cycle + sched_agen_lat, NULL, l1_miss_handler);

		/* access the D-DLB, NOTE: this code will
	 initiate speculative TLB misses */
		if (dtlb)
			tlb_lat = sched_agen_lat +
			cache_access(dtlb, cmd, MD_ALIGN_ADDR(is->ls->addr), MD_DATAPATH_WIDTH,
					sim_cycle + sched_agen_lat, NULL, tlb_miss_handler);
	}

	/* This guy has issued */
	is->when.issued = sim_cycle;
	is->when.completed = sim_cycle + MAX(cache_lat, tlb_lat);

	return TRUE;
}


/* register and memory scheduler */
STATIC void
schedule_stage(void)
{
	struct PREG_link_t *node = NULL, *pnode = NULL, *nnode = NULL;
	int sched_n[sclass_NUM];

	memset((byte_t*)sched_n, 0, sclass_NUM * sizeof(int));

	/* walk over list of ready un-scheduled instructions, issue the N
     oldest possible ones */
	int count = 0;
	for (pnode = NULL, node = scheduler_queue;
			node && sched_n[sclass_TOTAL] < sched_width[sclass_TOTAL];
			node = nnode)
	{
		int i;
		struct preg_t *preg = node->preg;
		struct INSN_station_t *is;
		int inst_l1_preg_readNum = 0, inst_l2_preg_readNum = 0;
		bool_t regStall  = 0;

		nnode = node->next;

		count++;
		/* if link is not valid (instruction has been squashed), delete and skip */
		if (!PLINK_valid(node) || !preg->is)
		{
			if (pnode) pnode->next = nnode;
			else scheduler_queue = nnode;
			PLINK_free(node);
			continue;
		}

		is = preg->is;

		/* Enforce in-order issue? */
		if (sched_inorder && is->prev && !is->prev->when.issued)
			break;

		/* Instruction is not ready yet => skip */
		if (is->when.ready > sim_cycle) {
			pnode = node;
			continue;
		}

		for(i=DEP_I1; i<=DEP_I3; i++){
			if(!LREG_ISDEP(is->pdi->lregnums[i]))
				continue;

			if(!(&pregs[is->pregnums[i]])->bypassValue && is->time_ready[i] < sim_cycle && is->regReadLatency[i] > 0) {
				if((&pregs[is->pregnums[i]])->regFile == L1_PREG_FILE &&  l1_preg_readNum  < l1_pregfile_rwidth){
					l1_preg_readNum++;
					is->regReadLatency[i]--;

				}
				else if((&pregs[is->pregnums[i]])->regFile == L2_PREG_FILE &&  l2_preg_readNum  < l2_pregfile_rwidth){
					if(l1_pregfile_cache && l1_preg_writeNum < l1_pregfile_wwidth){
						l1_preg_writeNum++;
						l2_preg_readNum++;
						is->regReadLatency[i]--;
					}
					else {
						l2_preg_readNum++;
						is->regReadLatency[i]--;
					}
				}
			}
		}

		if(LREG_ISDEP(is->pdi->lregnums[DEP_I1]) && !(&pregs[is->pregnums[DEP_I1]])->bypassValue && is->regReadLatency[DEP_I1] > 0){
			pnode = node;
			continue;
		}
		if(LREG_ISDEP(is->pdi->lregnums[DEP_I2]) && !(&pregs[is->pregnums[DEP_I2]])->bypassValue && is->regReadLatency[DEP_I2] > 0){
			pnode = node;
			continue;
		}
		if(LREG_ISDEP(is->pdi->lregnums[DEP_I3]) && !(&pregs[is->pregnums[DEP_I3]])->bypassValue && is->regReadLatency[DEP_I3] > 0){
			pnode = node;
			continue;
		}

		if(l1_pregfile_cache){
			for(i=DEP_I1; i<=DEP_I3; i++) {
				if(!LREG_ISDEP(is->pdi->lregnums[i]) || (&pregs[is->pregnums[i]])->bypassValue)
					continue;

				struct preg_t *t_preg = &pregs[is->pregnums[i]];
				if(t_preg->regFile == L2_PREG_FILE){
					cache_regs_alloc(t_preg, is);
					if(t_preg->regCache < 0){
						regStall = TRUE;
					}
					else{
						n_reg_read++;
						n_reg_read_miss++;
					}
				}
				else {
					n_reg_read++;
					pregs[t_preg->regCache].lru = sim_cycle;
				}
			}
		}
	
		if(regStall){
			pnode = node;
			continue;
		}

		/* Special actions for stores */
		if (is->pdi->iclass == ic_store)
		{
			bool_t f_shadow_store = FALSE;
			struct LDST_station_t *store = is->ls;
			struct LDST_station_t *load = NULL;
			unsigned int store_dist = 1;

			/* No store scheduling slot => skip */
			if (sched_n[sclass_STORE] == sched_width[sclass_STORE])
			{
				pnode = node;
				continue;
			}

			is->when.issued = is->when.completed = preg->when_written = sim_cycle;
			preg->bypassValue = TRUE;

			/* consume store scheduling slot */
			n_insn_exec[is->pdi->iclass]++;
			sched_n[sclass_STORE]++;
			sched_n[sclass_TOTAL]++;

			/* free reservation station */
			is->f_rs = FALSE;
			rs_num++;

			/* remove node from scheduler queue */
			if (pnode) pnode->next = nnode;
			else scheduler_queue = nnode;
			PLINK_free(node);

			/* the current store is store #1 */
			store_dist = 1;
			f_shadow_store = FALSE;
			for (load = store->next; load; load = load->next)
			{
				quad_t qs, ql;
				struct INSN_station_t *lis = load->is;
				struct INSN_station_t *lis_prev = lis->prev;
				struct preg_t *lpreg = &pregs[lis->pregnums[DEP_O1]];

				if (lis->pdi->iclass == ic_store)
				{
					/* shadow store */
					if (address_collision(load, store))
					{
						f_shadow_store = TRUE;
						break;
					}

					store_dist++;
					continue;
				}

				if (lis->pdi->iclass == ic_prefetch)
					continue;

				if (lis->when.issued == 0)
					continue;

				/* Address collision */
				if (!address_collision(load, store))
					continue;

				/* Compare values before we squash */
				qs = ql = 0;

				/* some messed up thing with LDS (which loads a word
		 and converts it to a floating-point quad) */
				if (lis->pdi->poi.op == LDS)
				{
					READ_QUAD(&qs, &store->val.q, MD_ADDR_OFFSET(load->addr), load->dsize);
					ITOFS(qs,qs);
					ql = lpreg->val.q;
				}
				else
				{
					READ_QUAD(&qs, &store->val.q, MD_ADDR_OFFSET(load->addr), load->dsize);
					READ_QUAD(&ql, &lpreg->val.q, 0, load->dsize);
				}

				/* Everything is cool */
				if (qs == ql)
					continue;

				/* Load mis-specualtion => normal squash */
				n_load_squash++;

				/* Try not to do this squash again */
				if (sched_adisambig_opt.strategy == adisambig_CHT)
					cht_enter(cht, lis->PC, store_dist);

				/* recover ROB, IFQ, and branch predictor starting from lis */
				ROB_recover(lis_prev, /* f_bmisp */FALSE);
				IFQ_recover(lis_prev);

				if (bpred)
					bpred_recover(bpred, lis_prev->PC, lis_prev->pdi->poi.op,
							lis_prev->NPC, &lis_prev->bp_pre_state);

				/* one recovery (the first) per store */
				break;
			}
		}
		/* Special scheduling for loads */
		else if (is->pdi->iclass == ic_load)
		{
			struct LDST_station_t *load = is->ls;
			struct LDST_station_t *store = NULL;
			int store_dist = 0;

			/* no load scheduling slot => skip */
			if (sched_n[sclass_LOAD] == sched_width[sclass_LOAD])
			{
				pnode = node;
				continue;
			}

			/* check for conservative/cht stalls */
			if (sched_adisambig_opt.strategy == adisambig_CONSERVATIVE)
			{
				for (store = load->prev; store; store = store->prev)
				{
					struct INSN_station_t *sis = store->is;

					if (sis->pdi->iclass != ic_store)
						continue;

					/* Store has not issued and address is not ready */
					if (sis->f_rs && !STORE_ADDR_READY(sis))
						break;
				}

				if (store)
				{
					load->f_stall = TRUE;
					pnode = node;
					continue;
				}
			}

			else if (sched_adisambig_opt.strategy == adisambig_CHT)
			{
				int collision_dist = cht_lookup(cht, is->PC);
				if (collision_dist > 0)
				{
					store_dist = 0;
					for (store = load->prev; store; store = store->prev)
					{
						struct INSN_station_t *sis = store->is;

						if (sis->pdi->iclass != ic_store)
							continue;

						store_dist++;

						if (store_dist < collision_dist)
							continue;

						/* store has not issued */
						if (sis->f_rs)
							break;
					}

					if (store)
					{
						load->f_stall = TRUE;
						pnode = node;
						continue;
					}
				}
			}

			store_dist = 0;
			for (store = load->prev; store; store = store->prev)
			{
				struct INSN_station_t *sis = store->is;

				if (sis->pdi->iclass != ic_store)
					continue;

				store_dist++;

				if (!address_collision(store, load))
					continue;

				/* store address and data both known => bypass with no penalty */
				if (!sis->f_rs)
				{
					READ_QUAD(&load->val.q, &store->val.q, MD_ADDR_OFFSET(load->addr), load->dsize);

					is->when.issued = sim_cycle;
					is->when.completed = sim_cycle + sched_agen_lat + sched_fwd_lat;

					n_insn_exec[is->pdi->iclass]++;

					is->f_rs = FALSE;
					rs_num++;

					writeback_enqueue(preg, is->when.completed);

					sched_n[sclass_LOAD]++;
					sched_n[sclass_TOTAL]++;

					/* load has been scheduled */
					if (pnode) pnode->next = nnode;
					else scheduler_queue = nnode;
					PLINK_free(node);
					break;
				}
				/* store address known, but data not ready => wait */
				else if (STORE_ADDR_READY(sis))
				{
					load->f_stall = TRUE;
					pnode = node;
					break;
				}
				/* perfect memory disambiguation => stall */
				else if (sched_adisambig_opt.strategy == adisambig_PERFECT)
				{
					load->f_stall = TRUE;
					pnode = node;
					break;
				}
				/* either address or data of colliding store is not
		 known */
				else
				{
					continue;
				}
			}

			if (store)
				continue;

			/* no collisions => cache access is valid */
			if (schedule_load(is))
			{
				mem_access(mem, mc_READ, load->addr, &load->val.q, load->dsize);

				writeback_enqueue(preg, is->when.completed);

				n_insn_exec[is->pdi->iclass]++;
				sched_n[sclass_LOAD]++;
				sched_n[sclass_TOTAL]++;

				/* free reservation station */
				is->f_rs = FALSE;
				rs_num++;

				/* remove from scheduling queue */
				if (pnode) pnode->next = nnode;
				else scheduler_queue = nnode;
				PLINK_free(node);
			}
			else
			{
				/* stall load, try to schedule next instruction */
				load->f_stall = TRUE;
				pnode = node;
				continue;
			}
		}
		else if (is->pdi->iclass == ic_prefetch)
		{
			if (schedule_load(is))
			{
				n_insn_exec[is->pdi->iclass]++;
				sched_n[sclass_LOAD]++;
				sched_n[sclass_TOTAL]++;

				/* free reservation station */
				is->f_rs = FALSE;
				rs_num++;

				/* remove from scheduling queue */
				if (pnode) pnode->next = nnode;
				else scheduler_queue = nnode;
				PLINK_free(node);
			}
			else
			{
				pnode = node;
				continue;
			}
		}

		/* not a load or a store */
		else
		{
			int execlat = 1;
			enum fuclass_t fuclass = MD_OP_FUCLASS(is->pdi->poi.op);
			struct res_t *fu = NULL;
			enum sched_class_t sclass = sclass_NUM;

			if (fuclass >= fuclass_IALU && fuclass <= fuclass_IDIV)
				sclass = sclass_INT;
			else if (fuclass >= fuclass_FADD && fuclass <= fuclass_FSQRT)
				sclass = sclass_FP;

			if (sclass != sclass_NUM)
			{
				if (sched_n[sclass] == sched_width[sclass])
				{
					pnode = node;
					continue;
				}

				fu = respool_get_res(respool, fuclass, sim_cycle);
				if (!fu)
				{
					pnode = node;
					continue;
				}
				execlat = fu->execlat;
			}

			is->when.issued = sim_cycle;
			is->when.completed = sim_cycle + execlat;
			n_insn_exec[is->pdi->iclass]++;

			/* free reservation station */
			is->f_rs = FALSE;
			rs_num++;

			writeback_enqueue(preg, is->when.completed);

			/* remove from scheduling queue */
			if (pnode) pnode->next = nnode;
			else scheduler_queue = nnode;
			PLINK_free(node);

			if (sclass != sclass_NUM)
				sched_n[sclass]++;

			sched_n[sclass_TOTAL]++;
		}
	}
}

static void
preg_connect_deps(struct preg_t *preg)
{
	struct INSN_station_t *is = preg->is;
	int dep;

	for (dep = DEP_I1; dep <= DEP_I3; dep++)
	{
		struct preg_t *preg_dep;
		struct PREG_link_t *plink;

		is->idep_ready[dep] = TRUE;
		is->time_ready[dep]= sim_cycle;

		if (!LREG_ISDEP(is->pdi->lregnums[dep]))
			continue;

		preg_dep = &pregs[is->pregnums[dep]];
		if (preg_dep->when_written)
			continue;

		/* dependence not ready */
		is->idep_ready[dep] = FALSE;
		is->time_ready[dep]= 0;

		plink = PLINK_new();
		PLINK_set(plink, preg);
		plink->x.opnum = dep;

		/* link these in program order */
		if (preg_dep->odeps_tail)
			preg_dep->odeps_tail->next = plink;
		else
			preg_dep->odeps_head = plink;

		preg_dep->odeps_tail = plink;
	}
}

/* dispatch instructions from the IFETCH -> DISPATCH queue: instructions are
   first decoded, then they allocated RUU (and LSQ for load/stores) resources
   and input and output dependence chains are updated accordingly */

STATIC void
rename_stage(void)
{
	int rename_n = 0;

	while (/* instruction decode B/W left? */
			rename_n < rename_width
			/* insts still available from fetch unit? */
			&& IFQ.head)
	{
		/* get the next instruction from the IFETCH -> DISPATCH queue */
		struct INSN_station_t *is = IFQ.head;
		struct preg_t *preg = NULL;
		int dep = 0;

		/* un-acceptable path */
		if (!sched_spec && f_wrong_path)
			break;

		/* ROB full */
		if (ROB.num == ROB.size)
			break;

		/* LDQ full */
		if ((is->pdi->iclass == ic_load || is->pdi->iclass == ic_prefetch) && LSQ.lnum == LSQ.lsize)
			break;

		/* STQ full */
		if (is->pdi->iclass == ic_store && LSQ.snum == LSQ.ssize)
			break;

		/* no more reservation stations */
		if (is->pdi->iclass != ic_sys && sched_rs_num == rs_num)
			break;

		/* don't let anyone come in if a syscall is in the machine */
		if (ROB.num > 0 && ROB.tail->pdi->iclass == ic_sys)
			break;

		rename_n++;
		n_insn_rename++;

		/* move insn from IFQ to ROB */
		INSN_remove(&IFQ, is);
		INSN_enqueue(&ROB, is);

		/* timing stats */
		is->f_wrong_path = f_wrong_path;
		is->when.renamed = sim_cycle + 1;
		is->when.regread = is->when.renamed + sched_lat;
		is->when.ready = is->when.issued = is->when.completed = is->when.resolved = is->when.committed = 0;
		is->f_bmisp = FALSE;
		is->ls = NULL;

		/* allocate LSQ entry for ld/st */
		if (is->pdi->iclass == ic_store || is->pdi->iclass == ic_load || is->pdi->iclass == ic_prefetch)
		{
			/* Create LSQ entry, link to ROB, and vice versa */
			is->ls = LDST_alloc();
			is->ls->is = is;
			LDST_enqueue(&LSQ, is->ls, is->pdi->iclass == ic_store);
		}

		/* rename the input registers */
		struct preg_t *input_preg = NULL;
		for (dep = DEP_I1; dep < DEP_INUM; dep++) {
			if (is->pdi->lregnums[dep] != regnum_NONE){
				is->pregnums[dep] = regs_rename(is->pdi->lregnums[dep]);
				input_preg = &(pregs[is->pregnums[dep]]);
				is->regReadLatency[dep] = input_preg->latency;
			}
		}

		/* allocate a new output physical register */
		is->pregnums[DEP_O1] = regs_alloc();
		/* register previously mapped to lregnums[DEP_O1] must be freed when this instruction retires */
		is->fregnum = regs_connect(is->pdi->lregnums[DEP_O1], is->pregnums[DEP_O1]);

		preg = &pregs[is->pregnums[DEP_O1]];
		preg->is = is;
		is->regWriteLatency[DEP_O1] = preg->latency;

		if (is->pdi->iclass == ic_sys)
		{
			is->when.ready = is->when.issued = is->when.resolved = is->when.completed = preg->when_written = sim_cycle;
			continue;
		}

		/* Allocate a reservation station */
		is->f_rs = TRUE;
		rs_num--;

		/* execute the instruction. Actual instruction execution happens
         here, schedule stage only computes latencies */
		exec_insn(is);

		/* connect register dependences.  Put on scheduling queue if instruction is ready */
		preg_connect_deps(preg);
		scheduler_enqueue(preg);

		/* this may be a mispredicted branch or jump.  Note,
	 must test this for F_TRAP also because of longjmp */
		if (!is->f_wrong_path &&
				MD_OP_HASANYFLAGS(is->pdi->poi.op, F_CTRL|F_TRAP))
		{
			/* is the trace generator trasitioning into
	     mis-speculation mode? */
			if (is->PPC != is->NPC)
			{
				/* entering mis-speculation mode, save PC */
				f_wrong_path = TRUE;
				is->f_bmisp = TRUE;
			}
		}
	}
}

/* fetch up as many instruction as one branch prediction and one cache line
   acess will support without overflowing the IFETCH -> DISPATCH QUEUE */
STATIC void
fetch_stage(void)
{
	int fetch_n;

	if (fetch_resume > sim_cycle)
		return;

	for (fetch_n = 0;
			/* fetch up to as many instruction as the DISPATCH
	  stage can decode */
			fetch_n < fetch_width &&
			/* fetch until IFETCH -> DISPATCH queue fills */
			INSN_flist &&
			IFQ.num < IFQ.size  &&
			/* valid text address */
			valid_text_address(mem, fetch_PC);
	)
	{
		md_inst_t inst;
		struct INSN_station_t *is = NULL;
		struct predec_insn_t *pdi = NULL;
		int cache_lat = 1, tlb_lat = 1;

		pdi = predec_lookup(fetch_PC);
		if (!pdi)
		{
			mem_access(mem, mc_READ, fetch_PC, &inst, sizeof(md_inst_t));
			pdi = predec_enter(fetch_PC, inst);
		}
		inst = pdi->inst;

		/* pretend like we are not fetching nops */
		if (pdi->iclass == ic_nop)
		{
			fetch_PC += sizeof(md_inst_t);
			continue;
		}

		/* address is within program text, read instruction from memory */
		if (cache_il1)
			cache_lat =
					cache_access(cache_il1, mc_READ, fetch_PC, sizeof(md_inst_t),
							sim_cycle, NULL, l1_miss_handler);

		if (itlb)
			tlb_lat =
					cache_access(itlb, mc_READ, fetch_PC, sizeof(md_inst_t),
							sim_cycle, NULL, tlb_miss_handler);

		/* I-cache/I-TLB miss? assumes I-cache hit >= I-TLB hit */
		if (MAX(tlb_lat, cache_lat) != 1)
		{
			/* I-cache miss, block fetch until it is resolved */
			fetch_resume = sim_cycle + MAX(tlb_lat, cache_lat) - 1;
			break;
		}

		/* I-cache and I-tlb hit here */
		is = INSN_alloc();

		is->PC = fetch_PC;
		is->seq = ++seq;
		is->pdi = pdi;
		is->when.fetched = sim_cycle + fetch_lat;

		/* How many cycles is fetch supposed to take? */

		/* adjust instruction fetch queue */
		INSN_enqueue(&IFQ, is);

		/* get the next predicted fetch address; only use branch predictor
	 result for branches (assumes pre-decode bits) */
		is->PPC = fetch_PC = is->PC + sizeof(md_inst_t);
		if (bpred)
		{
			is->PPC = fetch_PC =
					bpred_lookup(bpred, is->PC, is->pdi->poi.op, &is->bp_pre_state);

			is->when.predicted = sim_cycle;

			/* discontinuous fetch => break until next cycle */
			if (is->PPC != is->PC + sizeof(md_inst_t))
				break;
		}

		fetch_n++;
		n_insn_fetch++;
	}

	/* helps with managing fetch policy */
	fetch_resume = sim_cycle + 1;
}

STATIC void
cleanup_assert(void)
{
	if (INSN_num != 0)
		panic("%d INSN's left unreclaimed!", INSN_num);

	if (LDST_num != 0)
		panic("%d LDST's left unreclaimed!", LDST_num);

	if (plink_num != 0)
		panic("%d plink's left unreclaimed!", plink_num);
}

bool_t
sim_sample_off(unsigned long long n_insn)
{
	sample_mode = sample_OFF;
	fprintf(stderr, "sim: ** starting functional simulation -- fast-forwarding %llu instructions **\n", n_insn);

	return sim_fastfwd(&regs, mem, n_insn, NULL);
}

bool_t
sim_sample_warmup(unsigned long long n_insn)
{
	sample_mode = sample_WARM;
	fprintf(stderr, "sim: ** starting functional simulation -- warming up for %llu instructions **\n", n_insn);

	return sim_fastfwd(&regs, mem, n_insn, warmup_handler);
}

bool_t
sim_sample_on(unsigned long long n_insn)
{
	counter_t n_insn_commit_sum_beg = n_insn_commit_sum;

	sample_mode = sample_ON;

	fprintf(stderr, "sim: ** starting timing simulation");

	if (n_insn != 0)
	{
		fprintf(stderr, " -- simulating %llu instructions", n_insn);
	}

	fprintf(stderr, " **\n");

	regs_func2timing();

	/* set up timing simulation entry state */
	fetch_PC = regs.PC;

	/* main simulator loop, NOTE: the pipe stages are traverse in reverse order
     to eliminate this/next state synchronization and relaxation problems */
	while (n_insn == 0 ||  n_insn_commit_sum < n_insn_commit_sum_beg + n_insn)
	{
		/* commit entries from RUU/LSQ to architected register file */
		commit_stage();

		/* service result completions, also readies dependent operations */
		/* ==> inserts operations into ready queue --> register deps resolved */
		writeback_stage();

		/* invoke scheduler to schedule ready or partially ready events.
         The two schedulers act in parallel but are written separately
         for clarity */
		schedule_stage();

		/* decode and dispatch new operations */
		/* ==> insert ops w/ no deps or all regs ready --> reg deps resolved */
		rename_stage();

		/* call instruction fetch unit if it is not blocked */
		fetch_stage();

		if (insn_limit != 0 && n_insn_commit_sum >= insn_limit)
		{
			myfprintf(stderr, "Reached instruction limit: %u\n", insn_limit);
			return FALSE;
		}

		/* dump progress stats */
		if (insn_progress > 0 && n_insn_commit_sum >= insn_progress)
		{
			sim_print_stats(stderr);
			fflush(stderr);
			while (n_insn_commit_sum >= insn_progress)
				insn_progress += insn_progress_update;
		}

		/* go to next cycle */
		sim_cycle++;
		l1_preg_readNum = 0; l2_preg_readNum = 0;
		l1_preg_writeNum = 0; l2_preg_writeNum = 0;
	}

	/* blow away all transient state */
	ROB_cleanup();
	IFQ_cleanup();
	PLINK_purge();

	cleanup_assert();

	regs_timing2func();

	/* have we executed enough instructions? */
	return (n_insn_commit_sum - n_insn_commit_sum_beg >= n_insn);
}

/* start simulation, program loaded, processor precise state initialized */
void
sim_start(void)
{
	/* ignore any floating point exceptions, they may occur on mis-speculated
     execution paths */
	signal(SIGFPE, SIG_IGN);

	/* set up program entry state */
	regs.PC = ld_prog_entry;
	regs.NPC = regs.PC + sizeof(md_inst_t);
}
