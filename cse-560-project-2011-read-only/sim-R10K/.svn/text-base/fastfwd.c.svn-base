#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"

#include "options.h"
#include "memory.h"
#include "predec.h"
#include "syscall.h"
#include "eio.h"
#include "sim.h"
#include "fastfwd.h"

static quad_t
regs_value(const struct regs_t *regs, 
	   regnum_t dep)
{
  if (dep == DNA) return 0;
  else if (dep < MD_TOTAL_REGS) return regs->regs[dep].q;
  else
    panic("looking for register %d", dep);
}

bool_t
sim_fastfwd(struct regs_t *regs, 
	    struct mem_t *mem,
	    unsigned long long n_fastfwd,
	    warmup_handler_t warmup_handler)
{
  int icount;
  md_inst_t inst;		/* actual instruction bits */
  enum md_opcode_t op;		/* decoded opcode enum */
  enum md_fault_t fault;
  const struct predec_insn_t *pdi;
  
  /* fast forward simulator loop, performs functional simulation for
     FASTFWD_COUNT insts, then turns on performance (timing) simulation */
  
  for (icount=0; icount < n_fastfwd; )
    {
      /* maintain $r0 semantics */
      regs->regs[MD_REG_ZERO].q = 0;
      regs->regs[MD_FREG_ZERO].q = 0.0;
      
      pdi = predec_lookup(regs->PC);
      if (!pdi)
	{
	  mem_access(mem, mc_READ, regs->PC, &inst, sizeof(md_inst_t));
	  pdi = predec_enter(regs->PC, inst);
	}

      inst = pdi->inst;
      op = pdi->poi.op;
      
      /* nops are not counted */
      if (pdi->iclass != ic_nop)
	{
	  icount++;
	  sim_num_insn++;
	}
      
      /* set default reference address */
      regs->addr = 0, regs->dsize = 0; 
      
      /* set default fault - none */
      fault = md_fault_none;
      
      /* next program counter */
#define IR1                     (pdi->lregnums[DEP_I1])
#define IR2                     (pdi->lregnums[DEP_I2])
#define IR3                     (pdi->lregnums[DEP_I3])
#define OR1                     (pdi->lregnums[DEP_O1])
      
#define CPC                     (regs->PC)
#define SET_NPC(EXPR)           (regs->NPC = (EXPR))
#define SET_TPC(EXPR)		(regs->TPC = (EXPR))

#define READ_REG_Q(N)           (regs->regs[N].q)
#define WRITE_REG_Q(N,EXPR)     (((N) != DSINK) ? (regs->regs[N].q = (EXPR)) : (0))
#define READ_REG_F(N)           (regs->regs[N].d)
#define WRITE_REG_F(N,EXPR)     (((N) != DSINK) ? (regs->regs[N].d = (EXPR)) : (0))

#define SET_ADDR_DSIZE(ADDR,DSIZE) (regs->addr = (ADDR), regs->dsize = (DSIZE))

#define READ(ADDR, PVAL, SIZE) mem_access(mem, mc_READ, (ADDR), (PVAL), (SIZE))
#define WRITE(ADDR, PVAL, SIZE) mem_access(mem, mc_WRITE, (ADDR), (PVAL), (SIZE))

/* system call handler macro */
#define SYSCALL(INST) \
{ \
  if (fdump && sim_num_insn >= insn_dumpbegin && sim_num_insn < insn_dumpend) \
     md_print_regs(regs, fdump);                                    \
  sys_syscall(regs, mem_access, mem, INST, TRUE); \
  if (fdump && sim_num_insn >= insn_dumpbegin && sim_num_insn < insn_dumpend) \
     md_print_regs(regs, fdump);                                    \
}

      /* execute the instruction */
      switch (op)
	{
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3)		\
	case OP:							\
	  SYMCAT(OP,_IMPL);						\
	  break;
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)					\
	case OP:							\
	  panic("attempted to execute a linking opcode");
#define CONNECT(OP)
#undef DECLARE_FAULT
#define DECLARE_FAULT(FAULT)						\
	  { fault = (FAULT); break; }
#include "machine.def"
	default:
	  panic("attempted to execute a bogus opcode");
	}

      if (fault != md_fault_none)
	fatal("fault (%d) detected @ 0x%08p", fault, regs->PC);
	  
      /* Simulator specific warmup function */
      if (warmup_handler)
	warmup_handler(pdi);

      if (fdump)
	{
	  if (pdi->iclass != ic_nop && sim_num_insn >= insn_dumpbegin && sim_num_insn < insn_dumpend)
	    {
	      fprintf(fdump, "%-9u: 0x%08x ",
		      (word_t)sim_num_insn, 
		      (word_t)regs->PC);
	      if (LREG_ISDEP(pdi->lregnums[DEP_O1]))
		myfprintf(fdump, " O1: %016p", regs_value(regs, pdi->lregnums[DEP_O1]));
	      if (pdi->iclass == ic_load || pdi->iclass == ic_store)
		myfprintf(fdump, ", addr: %016p", regs->addr);
	      fprintf(fdump, "\n");
	      fflush(fdump);
	    }
	  else if (sim_num_insn == insn_dumpend)
	    {
	      fclose(fdump);
	    }
	}
	      
      /* go to the next instruction */
      regs->PC = regs->NPC;
      regs->NPC = regs->PC + sizeof(md_inst_t);
    }

  /* unconfigure instruction execution */
#undef SET_NPC
#undef SET_TPC
#undef CPC
#undef GPR
#undef SET_GPR

#undef FPR_Q
#undef SET_FPR_Q
#undef FPR
#undef SET_FPR
#undef FPCR
#undef SET_FPCR
#undef UNIQ
#undef SET_UNIQ

#undef SET_ADDR_MASK
#undef READ
#undef WRITE
#undef SYSCALL

#undef IR1
#undef IR2
#undef IR3
#undef OR1

  return (icount == n_fastfwd);
}



