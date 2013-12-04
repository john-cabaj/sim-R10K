/*
 * sim-cache.c - sample cache simulator implementation
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
 * $Id: sim-cache.c,v 1.7 1998/08/27 15:55:11 taustin Exp taustin $
 *
 * $Log: sim-cache.c,v $
 * Revision 1.7  1998/08/27 15:55:11  taustin
 * implemented host interface description in host.h
 * added target interface support
 * added support for register and memory contexts
 * added option ("-max:inst") to limit number of instructions analyzed
 * instruction predecoding moved to loader module
 * Alpha target support added
 * added support for quadword's
 *
 * Revision 1.6  1997/04/16  22:09:45  taustin
 * fixed "bad l2 D-cache parms" fatal string
 *
 * Revision 1.5  1997/03/11  01:27:08  taustin
 * updated copyright
 * `-pcstat' option support added
 * long/int tweaks made for ALPHA target support
 * better defaults defined for caches/TLBs
 * "mstate" command supported added for DLite!
 * supported added for non-GNU C compilers
 *
 * Revision 1.4  1997/01/06  16:03:07  taustin
 * comments updated
 * supported added for 2-level cache memory system
 * instruction TLB supported added
 * -icompress now compresses 64-bit instruction addresses to 32-bit equiv
 * main loop simplified
 *
 * Revision 1.3  1996/12/30  17:14:11  taustin
 * updated to support options and stats packages
 *
 * Revision 1.1  1996/12/05  18:52:32  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "options.h"
#include "memory.h"
#include "predec.h"
#include "cache.h"
#include "bpred.h"
#include "loader.h"
#include "syscall.h"
#include "sim.h"
#include "fastfwd.h"
#include "resource.h"

/*
 * This file implements a functional cache simulator.  Cache statistics are
 * generated for a user-selected cache and TLB configuration, which may include
 * up to two levels of instruction and data cache (with any levels unified),
 * and one level of instruction and data TLBs.  No timing information is
 * generated (hence the distinction, "functional" simulator).
 */

/* simulated registers */
static struct regs_t regs;

/* simulated memory */
static struct mem_t *mem = NULL;

counter_t sim_num_insn = 0;
static counter_t sim_sample_insn = 0;
static counter_t sim_sample_insn_split[ic_NUM];

/* level 1 instruction cache, entry level instruction cache */
static struct cache_t *cache_il1 = NULL;
/* instruction TLB */
static struct cache_t *itlb = NULL;

/* level 1 data cache, entry level data cache */
static struct cache_t *cache_dl1 = NULL;
/* data TLB */
static struct cache_t *dtlb = NULL;

/* level 2 cache (unified) */
static struct cache_t *cache_l2 = NULL;

/* branch predictor */
static struct bpred_t *bpred = NULL;

/* cycle counter */
static tick_t sim_cycle = 0;

static unsigned int mem_hlat;
static unsigned int tlb_mlat;

/* cache/TLB options */
static struct cache_opt_t cache_dl1_opt;
static struct cache_opt_t dtlb_opt;
static struct cache_opt_t cache_il1_opt;
static struct cache_opt_t itlb_opt;
static struct cache_opt_t cache_l2_opt;

/* branch predictor options */
static struct bpred_opt_t bpred_opt;

/* Register simulator-specific options */
void
sim_reg_options(struct opt_odb_t *odb)	/* options database */
{
  dtlb_opt.ct = ct_TLB;
  dtlb_opt.name = "dtlb";
  dtlb_opt.opt = "32:4096:4:l";
  dtlb_opt.hlat = 2;
  cache_reg_options(odb, &dtlb_opt);

  cache_dl1_opt.ct = ct_L1;
  cache_dl1_opt.name = "dl1";
  cache_dl1_opt.opt = "256:32:1:l";
  cache_dl1_opt.hlat = 2;
  cache_reg_options(odb, &cache_dl1_opt);

  itlb_opt.ct = ct_TLB;
  itlb_opt.name = "itlb";
  itlb_opt.opt = "16:4096:4:l";
  itlb_opt.hlat = 1;
  cache_reg_options(odb, &itlb_opt);

  cache_il1_opt.ct = ct_L1;
  cache_il1_opt.name = "il1";
  cache_il1_opt.opt = "256:32:1:l";
  cache_il1_opt.hlat = 1;
  cache_reg_options(odb, &cache_il1_opt);

  cache_l2_opt.ct = ct_L2;
  cache_l2_opt.name = "l2";
  cache_l2_opt.opt = "1024:64:4:l";
  cache_l2_opt.hlat = 10;
  cache_reg_options(odb, &cache_l2_opt);

  opt_reg_uint(odb, "-mem:hlat", "main memory hit latency",
	       &mem_hlat, /* default */100, 
	       /* print */TRUE, /* format */NULL);

  opt_reg_uint(odb, "-tlb:mlat", "TLB miss latency",
	       &tlb_mlat, /* default */30, 
	       /* print */TRUE, /* format */NULL);

  bpred_opt.opt = "dynamic";
  bpred_opt.dir1_opt.opt = "1024:2:0:0";
  bpred_opt.dir2_opt.opt = "1024:2:8:0";
  bpred_opt.chooser_opt.opt = "1024:2";
  bpred_opt.ras_opt.opt = "8";
  bpred_opt.btb_opt.opt = "512:4:16:16";
  bpred_reg_options(odb, &bpred_opt);

  fuclass_reg_options(odb);
}

/* check simulator-specific option values */
void
sim_check_options(void)	/* command line arguments */
{
  /* use a level 1 D-cache? */
  if (mystricmp(cache_dl1_opt.opt, "none"))
    {
      cache_check_options(&cache_dl1_opt);
      cache_dl1 = cache_create(&cache_dl1_opt);
    }

  /* use a level 1 I-cache? */
  if (mystricmp(cache_il1_opt.opt, "none"))
    {
      if (!mystricmp(cache_il1_opt.opt, "dl1"))
	{
	  if (!cache_dl1)
	    fatal("L1 I-cache cannot access L1 D-cache as it's undefined");
	  cache_il1 = cache_dl1;
	}
      else /* il1 is defined */
	{
	  cache_check_options(&cache_il1_opt);
	  cache_il1 = cache_create(&cache_il1_opt);
	}
    }

  if (mystricmp(cache_l2_opt.opt, "none"))
    {
      if (!cache_dl1 && !cache_il1)
	fatal("can't have an L2 D-cache without an L1 D-cache or I-cache!");

      cache_check_options(&cache_l2_opt);
      cache_l2 = cache_create(&cache_l2_opt);
    }

  /* use a D-TLB? */
  if (mystricmp(dtlb_opt.opt, "none"))
    {
      cache_check_options(&dtlb_opt);
      dtlb = cache_create(&dtlb_opt);
    }

  /* use an I-TLB? */
  if (mystricmp(itlb_opt.opt, "none"))
    {
      if (!mystricmp(itlb_opt.opt, "dtlb"))
	{
	  if (!dtlb)
	    fatal("I-TLB cannot use D-TLB as it is undefined");
	  itlb = dtlb;
	}
      else
	{
	  cache_check_options(&itlb_opt);
	  itlb = cache_create(&itlb_opt);
	}
    }

  if (mystricmp(bpred_opt.opt, "none"))
    {
      bpred_check_options(&bpred_opt);
      bpred = bpred_create(&bpred_opt);
    }

  fuclass_check_options();
}

/* initialize the simulator */
void
sim_init(void)
{
  /* allocate and initialize memory space */
  mem = mem_create("mem");
  mem_init(mem);
}

/* load program into simulated state */
void
sim_load_prog(char *fname,		/* program to load */
	      int argc, char **argv,	/* program arguments */
	      char **envp)		/* program environment */
{
  /* load program text and data, set up environment, memory, and regs */
  ld_load_prog(fname, argc, argv, envp, &regs, mem, TRUE);

  predec_init();
}

/* print simulator-specific configuration information */
void
sim_aux_config(FILE *stream)		/* output stream */
{
  /* nada */
}

/* dump simulator-specific auxiliary simulator statistics */
void
sim_aux_stats(FILE *stream)		/* output stream */
{
  /* nada */
  print_counter(stream, "sim_elapsed_time", sim_elapsed_time, "simulation time in seconds");
  print_rate(stream, "sim_insn_rate", (double)sim_num_insn/sim_elapsed_time, "simulation speed (insts/sec)");

  print_counter(stream, "sim_num_insn", sim_num_insn, "instructions simulated (fast-forwarding included)");
  print_counter(stream, "sim_sample_insn", sim_sample_insn, "instructions (in sample)");
  print_counter(stream, "sim_sample_int", sim_sample_insn_split[ic_icomp] + sim_sample_insn_split[ic_icomplong], "integer operations");
  print_counter(stream, "sim_sample_load", sim_sample_insn_split[ic_load], "loads");
  print_counter(stream, "sim_sample_store", sim_sample_insn_split[ic_store], "stores");
  print_counter(stream, "sim_sample_branch", sim_sample_insn_split[ic_ctrl], "branches");
  print_counter(stream, "sim_sample_fp", sim_sample_insn_split[ic_fcomp] + sim_sample_insn_split[ic_fcomplong], "floating point operations");
  print_counter(stream, "sim_sample_prefetch", sim_sample_insn_split[ic_prefetch], "prefetches");
  print_counter(stream, "sim_sample_sys", sim_sample_insn_split[ic_sys], "syscalls");

  print_counter(stream, "sim_cycle", sim_cycle, "cycles simulated");
  print_rate(stream, "sim_CPI", (double)sim_cycle/sim_sample_insn, "CPI");
  print_rate(stream, "sim_IPC", (double)sim_sample_insn/sim_cycle, "IPC");

  /* register cache stats */
  if (cache_dl1)
    cache_stats_print(cache_dl1, stream);
  if (dtlb)
    cache_stats_print(dtlb, stream);
  if (cache_il1 && cache_il1 != cache_dl1)
    cache_stats_print(cache_il1, stream);
  if (itlb && itlb != dtlb)
    cache_stats_print(itlb, stream);
  if (cache_l2)
    cache_stats_print(cache_l2, stream);

 if (bpred)
   bpred_stats_print(bpred, stream);
}

void
sim_start(void)
{
  /* set up program entry state */
  regs.PC = ld_prog_entry;
  regs.NPC = regs.PC + sizeof(md_inst_t);
}

/* un-initialize the simulator */
void
sim_uninit(void)
{
  /* nada */
}

/* l2 data cache block miss handler function */
static unsigned int		/* latency of block access */
l2_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		md_addr_t baddr,	/* block address to access */
		unsigned int bsize,	/* size of block to access */
		tick_t now,		/* time of access */
		bool_t miss_info[ct_NUM])
{
  /* this is a miss to the lowest level, so access main memory, which is
     always done in the main simulator loop */
  return /* access latency */mem_hlat;
}

/* l1 data cache l1 block miss handler function */
static unsigned int		/* latency of block access */
l1_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		md_addr_t baddr,	/* block address to access */
		unsigned int bsize,	/* size of block to access */
		tick_t now,		/* time of access */
		bool_t miss_info[ct_NUM])
{
  unsigned int lat = mem_hlat;
  if (cache_l2)
    lat = cache_access(cache_l2, cmd, baddr, bsize, now, miss_info, l2_miss_handler);
  return lat;
}


/* inst cache block miss handler function */
static unsigned int		/* latency of block access */
tlb_miss_handler(enum mem_cmd_t cmd,	/* access cmd, Read or Write */
		 md_addr_t baddr,	/* block address to access */
		 unsigned int bsize,	/* size of block to access */
		 tick_t now,	/* time of access */
		 bool_t miss_info[ct_NUM])
{
  return /* access latency */tlb_mlat;
}

static void
warmup_handler(const struct predec_insn_t *pdi)
{
  bool_t imiss_info[ct_NUM] = {FALSE, FALSE, FALSE, FALSE};
  bool_t dmiss_info[ct_NUM] = {FALSE, FALSE, FALSE, FALSE};
  
  /* get the next instruction to execute */
  if (itlb)
    cache_access(itlb, mc_READ, regs.PC, sizeof(md_inst_t), 0, imiss_info, tlb_miss_handler);
  if (cache_il1)
    cache_access(cache_il1, mc_READ, regs.PC, sizeof(md_inst_t), 0, imiss_info, l1_miss_handler);
  
  if (pdi->iclass == ic_store || pdi->iclass == ic_load || pdi->iclass == ic_prefetch)
    {
      enum mem_cmd_t dl1_cmd = pdi->iclass == ic_store ? mc_WRITE : (pdi->iclass == ic_load ? mc_READ : mc_PREFETCH);
      enum mem_cmd_t dtlb_cmd = (pdi->iclass == ic_store || pdi->iclass == ic_load) ? mc_READ : mc_PREFETCH;

      if (dtlb)
	cache_access(dtlb, dtlb_cmd, regs.addr, regs.dsize, 0, dmiss_info, tlb_miss_handler);
      if (cache_dl1)
	cache_access(cache_dl1, dl1_cmd, regs.addr, regs.dsize, 0, dmiss_info, l1_miss_handler);
    }
  else if (pdi->iclass == ic_ctrl)
    {
      md_addr_t PPC = 0; /* predicted PC => no prediction */
      if (bpred)
	{
	  struct bpred_state_t bp_state;
	  PPC = bpred_lookup(bpred, regs.PC, pdi->poi.op, &bp_state);
	  if (PPC != regs.NPC)
	    bpred_recover(bpred, regs.PC, pdi->poi.op, regs.NPC, &bp_state);
	  bpred_update(bpred, regs.PC, pdi->poi.op, regs.NPC, regs.TPC, PPC, &bp_state);
	}
    }
}

/*
 * configure the execution engine
 */

/*
 * precise architected register accessors
 */

#define IR1 (pdi->lregnums[DEP_I1])
#define IR2 (pdi->lregnums[DEP_I2])
#define IR3 (pdi->lregnums[DEP_I3])
#define OR1 (pdi->lregnums[DEP_O1])

/* program counter */
#define CPC			(regs.PC)
#define SET_NPC(EXPR)		(regs.NPC = (EXPR))
#define SET_TPC(EXPR)           (regs.TPC = (EXPR))

/* general purpose registers */
#define READ_REG_Q(N)		(regs.regs[N].q)
#define WRITE_REG_Q(N,EXPR)	(regs.regs[N].q = (EXPR))
#define READ_REG_F(N)		(regs.regs[N].d)
#define WRITE_REG_F(N,EXPR)	(regs.regs[N].d = (EXPR))

#define SET_ADDR_DSIZE(ADDR,DSIZE)   (regs.addr = (ADDR), regs.dsize = (DSIZE))
/* precise architected memory state accessor macros */
#define READ(ADDR, PVAL, SIZE) (mem_access(mem, mc_READ, (ADDR), (PVAL), (SIZE)))
#define WRITE(ADDR, PVAL, SIZE) (mem_access(mem, mc_WRITE, (ADDR), (PVAL), (SIZE)))

static quad_t
regs_value(regnum_t dep)
{
  if (dep == DNA) return 0;
  else if (dep >= 0 && dep < MD_TOTAL_REGS) return regs.regs[dep].q;
  else
    panic("looking for register %d", dep);
}

/* system call handler macro */
#define SYSCALL(INST)							\
   sys_syscall(&regs, mem_access, mem, INST, TRUE)

/* start simulation, program loaded, processor precise state initialized */
bool_t
sim_sample(unsigned int n_insn)
{
  md_inst_t inst;
  enum md_fault_t fault;
  struct predec_insn_t *pdi;
  counter_t sim_num_insn_begin = sim_num_insn;
  bool_t fdumpinsn;
  int i;

  tick_t reg_ready[MD_TOTAL_REGS];
  for (i = 0; i < MD_TOTAL_REGS; i++)
    reg_ready[i] = sim_cycle;

  while (n_insn == 0 || sim_num_insn < sim_num_insn_begin + n_insn)
    {
      /* get the next instruction to execute */
      mem_access(mem, mc_READ, regs.PC, &inst, sizeof(md_inst_t));

      /* set default reference address and access mode */
      regs.addr = 0; regs.dsize = 0;

      /* set default fault - none */
      fault = md_fault_none;

      /* get the next instruction to execute */
      pdi = predec_lookup(regs.PC);
      if (!pdi)
	{
	  mem_access(mem, mc_READ, regs.PC, &inst, sizeof(md_inst_t));
	  pdi = predec_enter(regs.PC, inst);
	}

      /* keep an instruction count */
      if (pdi->iclass != ic_nop)
	{
	  sim_num_insn++;
	  sim_sample_insn++;
	  sim_sample_insn_split[pdi->iclass]++;
	}

      fdumpinsn = fdump && sim_num_insn >= insn_dumpbegin && sim_num_insn < insn_dumpend; 
      inst = pdi->inst;
      
      /* maintain $r0 semantics */
      regs.regs[MD_REG_ZERO].q = 0;
      regs.regs[MD_FREG_ZERO].d = 0.0;

      /* execute the instruction */
      switch (pdi->poi.op)
	{
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3)		\
	case OP:							\
          SYMCAT(OP,_IMPL);						\
          break;
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)					\
        case OP:							\
          panic("attempted to execute a linking opcode");
#define CONNECT(OP)
#define DECLARE_FAULT(FAULT)						\
	  { fault = (FAULT); break; }
#include "machine.def"
	default:
          panic("attempted to execute a bogus opcode");
	}

      if (fault != md_fault_none)
	fatal("fault (%d) detected @ 0x%08p", fault, regs.PC);

      /* compute CPI (pipeline delay) for this instruction */

      sim_cycle++; /* every instruction has a CPI of at least 1.  A CPI of 1 means no stalls */
      
      /* Add delays for instruction cache misses */
      {
	unsigned int il1_lat = cache_il1_opt.hlat, itlb_lat = cache_il1_opt.hlat, imem_lat = cache_il1_opt.hlat;
	bool_t imiss_info[ct_NUM] = {FALSE, FALSE, FALSE, FALSE};
	
	if (itlb)
	  itlb_lat = cache_access(itlb, mc_READ, regs.PC, sizeof(md_inst_t), sim_cycle, imiss_info, tlb_miss_handler);
	if (cache_il1)
	  il1_lat = cache_access(cache_il1, mc_READ, regs.PC, sizeof(md_inst_t), sim_cycle, imiss_info, l1_miss_handler);
	
	sim_cycle += MAX(itlb_lat, il1_lat) - cache_il1_opt.hlat;
      }

      if ((pdi->lregnums[DEP_I1] != regnum_NONE) && (reg_ready[pdi->lregnums[DEP_I1]] > sim_cycle)) 
	sim_cycle = reg_ready[pdi->lregnums[DEP_I1]];
      if ((pdi->lregnums[DEP_I2] != regnum_NONE) && (reg_ready[pdi->lregnums[DEP_I2]] > sim_cycle))
	sim_cycle = reg_ready[pdi->lregnums[DEP_I2]];
      if ((pdi->lregnums[DEP_I3] != regnum_NONE) && (reg_ready[pdi->lregnums[DEP_I3]] > sim_cycle)) 
	sim_cycle = reg_ready[pdi->lregnums[DEP_I3]];
  
      /* Access the data cache and TLB */
      if (pdi->iclass == ic_store || pdi->iclass == ic_load || pdi->iclass == ic_prefetch)
	{
	  enum mem_cmd_t dl1_cmd = pdi->iclass == ic_store ? mc_WRITE : (pdi->iclass == ic_load ? mc_READ : mc_PREFETCH);
	  enum mem_cmd_t dtlb_cmd = (pdi->iclass == ic_store || pdi->iclass == ic_load) ? mc_READ : mc_PREFETCH;
	  unsigned int dl1_lat = cache_dl1_opt.hlat;
	  unsigned int dtlb_lat = cache_dl1_opt.hlat;
	  bool_t dmiss_info[ct_NUM] = {FALSE, FALSE, FALSE, FALSE};

	  if (dtlb)
	    dtlb_lat = cache_access(dtlb, dtlb_cmd, regs.addr, regs.dsize, sim_cycle, dmiss_info, tlb_miss_handler);
	  if (cache_dl1)
	    dl1_lat = cache_access(cache_dl1, dl1_cmd, regs.addr, regs.dsize, sim_cycle, dmiss_info, l1_miss_handler);

	  /* Track output register ready time for loads */
	  if (pdi->iclass == ic_load)
	    reg_ready[pdi->lregnums[DEP_O1]] = sim_cycle + MAX(dl1_lat, dtlb_lat);

	}
      /* Track output register ready time for other instructions */
      else if (pdi->lregnums[DEP_O1] != regnum_NONE) 
	{
	  reg_ready[pdi->lregnums[DEP_O1]] = sim_cycle + fuclass_opt[MD_OP_FUCLASS(pdi->poi.op)].execlat;
	}
											
      /* Access the branch predictor */
      if (pdi->iclass == ic_ctrl)
	{
	  md_addr_t PPC = 0; /* predicted PC => no prediction */
	  if (bpred)
	    {
	      struct bpred_state_t bp_state;
	      PPC = bpred_lookup(bpred, regs.PC, pdi->poi.op, &bp_state);
	      if (PPC != regs.NPC)
		bpred_recover(bpred, regs.PC, pdi->poi.op, regs.NPC, &bp_state);
	      bpred_update(bpred, regs.PC, pdi->poi.op, regs.NPC, regs.TPC, PPC, &bp_state);
	    }

	  /* Mis-predicted branch: add length of pipe between F and
	     branch-X to sim_cycle.  This is the number of cycles the
	     next instruction will be effectively delayed */
	  if (PPC != regs.NPC)
	    sim_cycle += cache_il1_opt.hlat + /* decode stage */1 + /* execute stage */fuclass_opt[fuclass_IBRANCH].execlat;
	}

      /* go to the next instruction */
      regs.PC = regs.NPC;
      regs.NPC += sizeof(md_inst_t);

      if (verbose)
	{
	  myfprintf(stderr, "%10n [xor: 0x%08x] @ 0x%08p: ",
		    sim_num_insn, md_xor_regs(&regs), regs.PC);
	  md_print_insn(inst, regs.PC, stderr);
	  if (MD_OP_HASFLAGS(pdi->poi.op, F_MEM))
	    myfprintf(stderr, "  mem: 0x%08p", regs.addr);
	  fprintf(stderr, "\n");
	  /* fflush(stderr); */
	}
	  
      if (fdumpinsn)
	{
	  fprintf(fdump, "%-9u: 0x%08x ", (word_t)sim_num_insn, (word_t)regs.PC);
	  if (LREG_ISDEP(pdi->lregnums[DEP_O1]))
	    myfprintf(fdump, " O1: %016p", regs_value(pdi->lregnums[DEP_O1]));
	  fprintf(fdump, "\n");
	}

      if (fdump && sim_num_insn == insn_dumpend)
	{
	  fflush(fdump);
	  fclose(fdump);
	}

      /* finish early? */
      if (insn_limit && sim_sample_insn >= insn_limit)
	{
	  myfprintf(stderr, "Reached instruction limit: %u\n", insn_limit);
	  return FALSE;
	}
      
      if (insn_progress && sim_sample_insn >= insn_progress)
	{
	  sim_print_stats(stderr);
	  fflush(stderr);
	  while (sim_sample_insn >= insn_progress)
	    insn_progress += insn_progress_update;
	}
    }

  return (sim_num_insn - sim_num_insn_begin == n_insn); 
}

bool_t 
sim_sample_off(unsigned long long n_insn)
{
  sample_mode = sample_OFF;
  fprintf(stderr, "** sim -- fast-forwarding %llu instructions **\n", n_insn);
  return sim_fastfwd(&regs, mem, n_insn, NULL);
}

bool_t 
sim_sample_warmup(unsigned long long n_insn)
{
  sample_mode = sample_WARM;
  fprintf(stderr, "** sim -- warming up %llu instructions **\n", n_insn);
  return sim_fastfwd(&regs, mem, n_insn, warmup_handler);
}

bool_t 
sim_sample_on(unsigned long long n_insn)
{
  sample_mode = sample_ON;

  if (n_insn == 0)
    fprintf(stderr, "** sim -- simulating until program completion **\n");
  else
    fprintf(stderr, "** sim -- simulating %llu instructions **\n", n_insn);
  return sim_sample(n_insn);
}






