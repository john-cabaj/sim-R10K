/*
 * sim-safe.c - sample functional simulator implementation
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
 * $Id: sim-safe.c,v 1.7 1998/08/31 17:11:01 taustin Exp taustin $
 *
 * $Log: sim-safe.c,v $
 * Revision 1.7  1998/08/31 17:11:01  taustin
 * added register checksuming support, viewable with "-v" flag
 *
 * Revision 1.6  1998/08/27 16:38:25  taustin
 * implemented host interface description in host.h
 * added target interface support
 * added support for register and memory contexts
 * instruction predecoding moved to loader module
 * Alpha target support added
 * added support for quadword's
 * added fault support
 * added option ("-max:inst") to limit number of instructions analyzed
 * added target-dependent myprintf() support
 *
 * Revision 1.5  1997/03/11  17:14:57  taustin
 * updated copyright
 * long/int tweaks made for ALPHA target support
 * supported added for non-GNU C compilers
 *
 * Revision 1.4  1997/01/06  16:06:28  taustin
 * updated comments
 * opt_reg_header() call now prints simulator overview message
 * access variable now generalized to is_write boolean flag
 *
 * Revision 1.3  1996/12/27  15:54:04  taustin
 * updated comments
 * integrated support for options and stats packages
 * added sim_init() code
 *
 * Revision 1.1  1996/12/05  18:52:32  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"

#ifndef TARGET_ALPHA 
#error This simulator is targeted to ALPHA ISA only
#endif

#include "memory.h"
#include "loader.h"
#include "syscall.h"
#include "options.h"
#include "stats.h"
#include "sim.h"
#include "predec.h"

#include "fastfwd.h"

/*
 * This file implements a functional simulator.  This functional simulator is
 * the simplest, most user-friendly simulator in the simplescalar tool set.
 * Unlike sim-fast, this functional simulator checks for all instruction
 * errors, and the implementation is crafted for clarity rather than speed.
 */

/* simulated registers */
static struct regs_t regs;

/* simulated memory */
static struct mem_t *mem = NULL;

counter_t sim_num_insn = 0;
static counter_t sim_sample_insn = 0;
static counter_t sim_sample_insn_split[ic_NUM];

static bool_t fdumpinsn = FALSE;

/* register simulator-specific options */
void
sim_reg_options(struct opt_odb_t *odb)
{
  opt_reg_header(odb, 
"sim-safe: This simulator implements a functional simulator.  This\n"
"functional simulator is the simplest, most user-friendly simulator in the\n"
"simplescalar tool set.  Unlike sim-fast, this functional simulator checks\n"
"for all instruction errors, and the implementation is crafted for clarity\n"
"rather than speed.\n"
		 );
}

/* check simulator-specific option values */
void
sim_check_options(void)
{
  /* nada */
}

/* register simulator-specific statistics */
void
sim_reg_stats(struct stat_sdb_t *sdb)
{
  /* nada */
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

  /* init predecoded instruction cache */
  predec_init();
}

/* print simulator-specific configuration information */
void
sim_aux_config(FILE *stream)		/* output stream */
{
  /* nothing currently */
}

/* dump simulator-specific auxiliary simulator statistics */
void
sim_aux_stats(FILE *stream)		/* output stream */
{
  /* simulation time */
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

  /* YOUR CODE GOES HERE */
  // print the statistics you've gathered here
  
}

/* un-initialize simulator-specific state */
void
sim_uninit(void)
{
  /* nada */
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
#define SET_TPC(EXPR)

/* general purpose registers */
#define READ_REG_Q(N)		(regs.regs[N].q)
#define WRITE_REG_Q(N,EXPR)	(regs.regs[N].q = (EXPR))

#define READ_REG_F(N)		(regs.regs[N].d)
#define WRITE_REG_F(N,EXPR)	(regs.regs[N].d = (EXPR))

#define SET_ADDR_DSIZE(ADDR,DSIZE)   (regs.addr = (ADDR), regs.dsize = (DSIZE))
/* precise architected memory state accessor macros */
#define READ(ADDR, PVAL, SIZE) mem_access(mem, mc_READ, (ADDR), (PVAL), (SIZE))
#define WRITE(ADDR, PVAL, SIZE) mem_access(mem, mc_WRITE, (ADDR), (PVAL), (SIZE))

/* system call handler macro */
#define SYSCALL(INST)	\
   {                                                                    \
      if (fdumpinsn) md_print_regs(&regs, fdump);                       \
      sys_syscall(&regs, mem_access, mem, INST, TRUE);                  \
   }

static quad_t
regs_value(regnum_t dep)
{
  if (dep == DNA) return 0;
  else if (dep >= 0 && dep < MD_TOTAL_REGS) return regs.regs[dep].q;
  else panic("looking for register %d", dep);
}

bool_t 
sim_sample_on(unsigned long long n_insn)
{
  enum md_fault_t fault;
  const struct predec_insn_t *pdi;
  md_inst_t inst;
  counter_t sim_num_insn_begin = sim_num_insn;

  sample_mode = sample_ON;
  if (n_insn != 0)
      fprintf(stderr, "** sim -- sampling %llu instructions **\n", n_insn);
  else
      fprintf(stderr, "** sim -- sampling until program completion**\n");

  /* main simulator loop, NOTE: the pipe stages are traverse in reverse order
     to eliminate this/next state synchronization and relaxation problems */
  while (n_insn == 0 || sim_num_insn < sim_num_insn_begin + n_insn)
    {
      /* maintain $r0 semantics */
      regs.regs[MD_REG_ZERO].q = 0;
      regs.regs[MD_FREG_ZERO].d = 0.0;

      /* set default reference address and access mode */
      regs.addr = regs.dsize = 0; 

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

      switch (pdi->poi.op) {
      case CMPEQ:
      case CMPLT:
      case CMPLE:
      case CMPULT:
      case CMPULE:
      case CMPBGE:
      case CMPEQI:
      case CMPLTI:
      case CMPLEI:
      case CMPULTI:
      case CMPULEI:
      case CMPBGEI:
        /* YOUR CODE GOES HERE */
        /* The cases above match on all of the Alpha's relational instructions. */
        break;
      default: // this matches all other instructions
        if ( MD_OP_FLAGS(pdi->poi.op) & F_COND ) { // insn is a conditional branch
            /* YOUR CODE GOES HERE */
        } else if ( MD_OP_FLAGS(pdi->poi.op) & (F_ICOMP | F_FCOMP | F_LOAD | F_STORE) ) { // insn sets CC's
          /* YOUR CODE GOES HERE */
        }
        break;
      } // end switch

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

      if (fault != md_fault_none)
	fatal("fault (%d) detected @ 0x%08p", fault, regs.PC);

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

      /* go to the next instruction */
      regs.PC = regs.NPC;
      regs.NPC += sizeof(md_inst_t);

      if (insn_limit != 0 && sim_sample_insn >= insn_limit)
	{
	  myfprintf(stderr, "Reached instruction limit: %u\n", insn_limit);
	  return FALSE;
	}
  
      /* dump progress stats */
      if (insn_progress > 0 && sim_sample_insn >= insn_progress)
	{
	  sim_print_stats(stderr);
	  fflush(stderr);
	  while (sim_sample_insn >= insn_progress)
	    insn_progress += insn_progress_update;
	}
      
    } // end main simulator loop

  /* have we executed enough instructions? */
  return (sim_num_insn - sim_num_insn_begin >= n_insn);
}


bool_t
sim_sample_off(unsigned long long n_insn)
{
  sample_mode = sample_OFF; 
  fprintf(stderr, "** sim -- fast-forwarding %u instructions **\n", n_insn);
  return sim_fastfwd(&regs, mem, n_insn, NULL);
}

bool_t
sim_sample_warmup(unsigned long long n_insn)
{
  sample_mode = sample_WARM;
  fprintf(stderr, "** sim -- fast-forwarding %u instructions **\n", n_insn);
  return sim_fastfwd(&regs, mem, n_insn, NULL);
}

/* start simulation, program loaded, processor precise state initialized */
void
sim_start(void)
{
  /* set up program entry state */
  regs.PC = ld_prog_entry;
  regs.NPC = regs.PC + sizeof(md_inst_t);

  /* YOUR CODE GOES HERE */
  // put your initialization code (if any) here

}



