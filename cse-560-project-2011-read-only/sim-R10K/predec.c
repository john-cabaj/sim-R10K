/* standard includes */
#include <stdio.h>
#include <stdlib.h>
/* external definitions */
#include "host.h"
#include "options.h"
#include "machine.h"
#include "misc.h"
/* interface definitions */
#include "predec.h"
/* implementation definitions */
#include "stats.h"

static int predec_insn_htsize = 1024 * 1024;
static struct predec_insn_t **predec_insn_ht = NULL;

void 
predec_reg_options(struct opt_odb_t *odb)
{
  /* nothing here */
}

void
predec_check_options(void)
{
  /* hence, nothing here */
}

void
predec_init(void)
{
  predec_insn_ht = (struct predec_insn_t **)mycalloc(predec_insn_htsize, sizeof(struct predec_insn_t*));
}


static inline void
predec_decode(struct predec_insn_t *pdi)
{
  md_inst_t inst;

  /* have a valid inst here */
  MD_SET_OPCODE(pdi->poi.op, pdi->inst);

  /* decode the instruction */
  /* need this name to get the decode macros to work */
  inst = pdi->inst;
  
  switch (pdi->poi.op)
    {
#define DEFINST(OP,MSK,NAME,OPFORM,RES,CLASS,O1,O2,I1,I2,I3)\
    case OP:					\
      /* compute dependences */	                \
      pdi->lregnums[DEP_O1] = O1; 		\
      pdi->lregnums[DEP_I1] = I1;               \
      pdi->lregnums[DEP_I2] = I2;               \
      pdi->lregnums[DEP_I3] = I3;               \
      break;
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)		\
    case OP:					\
      /* speculative bogus inst converted to NOP */\
      pdi->poi.op = MD_NOP_OP;			\
      /* compute dependences */	                \
      pdi->lregnums[DEP_O1] = regnum_NONE;      \
      pdi->lregnums[DEP_I1] = regnum_NONE;      \
      pdi->lregnums[DEP_I2] = regnum_NONE;      \
      pdi->lregnums[DEP_I3] = regnum_NONE;      \
      break;
#define CONNECT(OP)	/* nada... */
#include "machine.def"
    default:
      /* can speculatively decode a bogus inst, convert to a NOP */
      pdi->poi.op = MD_NOP_OP;
      /* compute output/input dependencies to out1-2 and in1-3 */	
      pdi->lregnums[DEP_O1] = regnum_NONE; 
      pdi->lregnums[DEP_I1] = regnum_NONE;
      pdi->lregnums[DEP_I2] = regnum_NONE;
      pdi->lregnums[DEP_I3] = regnum_NONE;
      /* no EXPR */
    }

  pdi->poi.imm = 0;
  if (MD_OP_HASFLAGS(pdi->poi.op, F_OFS) && !MD_OP_HASFLAGS(pdi->poi.op, F_CTRL|F_COND))
    pdi->poi.imm = OFS;
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_IMM))
    pdi->poi.imm = IMM;

  /* There are two instructions that ALPHA treats as nops,
     bis zero, zero, zero and ldq_u zero, 0(sp) */
  if (pdi->poi.op == BIS && 
      pdi->lregnums[DEP_I1] == MD_REG_ZERO && 
      pdi->lregnums[DEP_I2] == MD_REG_ZERO && 
      pdi->lregnums[DEP_O1] == MD_REG_ZERO)
    pdi->iclass = ic_nop;

  else if (pdi->poi.op == LDQ_U && 
      pdi->lregnums[DEP_I2] == MD_REG_SP && 
      pdi->lregnums[DEP_O1] == MD_REG_ZERO)
    pdi->iclass = ic_nop;
     
  /* This is a compiler prefetch */
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_LOAD) &&
	   (pdi->lregnums[DEP_O1] == MD_REG_ZERO || pdi->lregnums[DEP_O1] == MD_FREG_ZERO))
    pdi->iclass = ic_prefetch;

  else if (MD_OP_HASFLAGS(pdi->poi.op, F_LOAD))
    pdi->iclass = ic_load;
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_STORE))
    pdi->iclass = ic_store;
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_CTRL))
    pdi->iclass = ic_ctrl;
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_ICOMP|F_LONGLAT))
    pdi->iclass = ic_icomplong;
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_ICOMP))
    pdi->iclass = ic_icomp;
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_FCOMP|F_LONGLAT))
    pdi->iclass = ic_fcomplong;
  else if (MD_OP_HASFLAGS(pdi->poi.op, F_FCOMP))
    pdi->iclass = ic_fcomplong;
  else if (pdi->poi.op == PAL_CALLSYS)
    pdi->iclass = ic_sys;
  else
    pdi->iclass = ic_other;

  if (pdi->lregnums[DEP_O1] == DNA || pdi->lregnums[DEP_O1] == MD_REG_ZERO || pdi->lregnums[DEP_O1] == MD_FREG_ZERO)
    pdi->lregnums[DEP_O1] = DSINK;
}

struct predec_insn_t *
predec_enter(md_addr_t PC, 
	     md_inst_t inst)
{
  unsigned int hash = MOD(SHIFT_PC(PC), predec_insn_htsize);
  struct predec_insn_t *pdi = (struct predec_insn_t *)mycalloc(1, sizeof(struct predec_insn_t));

  pdi->poi.PC = PC;
  pdi->inst = inst;
  pdi->next = predec_insn_ht[hash];
  predec_insn_ht[hash] = pdi;

  predec_decode(pdi);
  return pdi;
}

struct predec_insn_t *
predec_lookup(md_addr_t PC)
{
  unsigned int hash = MOD(SHIFT_PC(PC), predec_insn_htsize);
  struct predec_insn_t *pdi = NULL;

  for (pdi = predec_insn_ht[hash]; pdi; pdi = pdi->next)
    if (pdi->poi.PC == PC)
      return pdi;

  return NULL;
}

