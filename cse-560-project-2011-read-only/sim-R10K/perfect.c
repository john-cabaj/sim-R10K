#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "options.h"
#include "memory.h"
#include "predec.h"
#include "perfect.h"

static struct regs_t pii_regs;
static struct mem_t *pii_mem;

static struct perfect_insn_info_t *pii_buf;
static unsigned int pii_size, pii_num, pii_head, pii_ptr;
static seq_t pii_seq = 0;

static bool_t pii_f_wrong_path = FALSE;

void
perfect_info_init(unsigned int size)
{
   pii_mem = mem_create("pii_mem");

   pii_size = size;
   pii_buf = (struct perfect_insn_info_t*)calloc(pii_size, sizeof(struct perfect_insn_info_t));
   pii_head = pii_ptr = pii_num = 0;
}

void
perfect_info_get(const struct predec_insn_t *pdi,
                 struct perfect_insn_info_t *pii)
{
   md_inst_t inst;  /* actual instruction bits */
   enum md_fault_t fault;
   struct perfect_insn_info_t *piib;

   if (pii_f_wrong_path)
   {
      pii->f_wrong_path = TRUE;
   }
   else if (pdi->poi.PC != pii_regs.PC)
   {
      pii_f_wrong_path = pii->f_wrong_path = TRUE;
   }
   else if (pii_ptr != pii_head)
   {
      piib = &pii_buf[pii_ptr];

      pii->f_wrong_path = FALSE;
      pii->addr = piib->addr;
      pii->dsize = piib->dsize;

      pii->vals[DEP_I1].q = piib->vals[DEP_I1].q;
      pii->vals[DEP_I2].q = piib->vals[DEP_I2].q;
      pii->vals[DEP_I3].q = piib->vals[DEP_I3].q;
      pii->vals[DEP_O1].q = piib->vals[DEP_O1].q;

      pii->PC = piib->PC;
      pii->TPC = piib->TPC;
      pii_regs.PC = pii->NPC = piib->NPC;
      pii->seq = piib->seq;

      pii_ptr = (pii_ptr + 1) % pii_size;
   }
   else /* pii_ptr == pii_head */
   {
      piib = &pii_buf[pii_ptr];

      pii->seq = piib->seq = ++pii_seq;
      pii->f_wrong_path = piib->f_wrong_path = FALSE;
      pii->addr = piib->addr = 0;
      pii->dsize = piib->dsize = 0;

      pii->PC = piib->PC = pii_regs.PC;
      pii->TPC = piib->TPC = 0;
      pii->NPC = piib->NPC = pii_regs.NPC = pii_regs.PC + sizeof(md_inst_t);

      inst = pdi->inst;
      fault = md_fault_none;

      pii_regs.regs[MD_REG_ZERO].q = 0;
      pii_regs.regs[MD_FREG_ZERO].d = 0.0;

      pii->vals[DEP_I1].q = piib->vals[DEP_I1].q = (LREG_ISDEP(pdi->lregnums[DEP_I1])) ? pii_regs.regs[pdi->lregnums[DEP_I1]].q : 0;
      pii->vals[DEP_I2].q = piib->vals[DEP_I2].q = (LREG_ISDEP(pdi->lregnums[DEP_I2])) ? pii_regs.regs[pdi->lregnums[DEP_I2]].q : 0;
      pii->vals[DEP_I3].q = piib->vals[DEP_I3].q = (LREG_ISDEP(pdi->lregnums[DEP_I3])) ? pii_regs.regs[pdi->lregnums[DEP_I3]].q : 0;

#define IR1                     (pdi->lregnums[DEP_I1])
#define IR2                     (pdi->lregnums[DEP_I2])
#define IR3                     (pdi->lregnums[DEP_I3])
#define OR1                     (pdi->lregnums[DEP_O1])

#define CPC                     (pii_regs.PC)
#define SET_NPC(EXPR)           (pii->NPC = piib->NPC = pii_regs.NPC = (EXPR))
#define SET_TPC(EXPR)  (pii->TPC = piib->TPC = pii_regs.TPC = (EXPR))

#define READ_REG_Q(N)           (pii_regs.regs[N].q)
#define WRITE_REG_Q(N,EXPR)     (((N) != DSINK) ? (pii_regs.regs[N].q = (EXPR)) : (0))
#define READ_REG_F(N)           (pii_regs.regs[N].d)
#define WRITE_REG_F(N,EXPR)     (((N) != DSINK) ? (pii_regs.regs[N].d = (EXPR)) : (0))

#define SET_ADDR_DSIZE(ADDR,DSIZE) (pii->addr = piib->addr = (ADDR), pii->dsize = piib->dsize = (DSIZE))

#define READ(ADDR, PVAL, SIZE) mem_access(pii_mem, mc_READ, (ADDR), (PVAL), (SIZE))
#define WRITE(ADDR, PVAL, SIZE) mem_access(pii_mem, mc_WRITE, (ADDR), (PVAL), (SIZE))

      /* system call handler macro */
#define SYSCALL(INST) panic("should not be calling this!")

      /* execute the instruction */
      switch (pdi->poi.op)
      {
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3)  \
 case OP:       \
   SYMCAT(OP,_IMPL);      \
   break;
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)     \
 case OP:       \
   panic("attempted to execute a linking opcode");
#define CONNECT(OP)
#undef DECLARE_FAULT
#define DECLARE_FAULT(FAULT)      \
   { fault = (FAULT); break; }
#include "machine.def"
      default:
         panic("attempted to execute a bogus opcode");
      }

      pii->vals[DEP_O1].q = piib->vals[DEP_O1].q = (pdi->lregnums[DEP_I3] != DSINK) ? pii_regs.regs[pdi->lregnums[DEP_O1]].q : 0;

      /* go to the next instruction */
      pii_regs.PC = pii_regs.NPC;

#undef CPC
#undef SET_NPC
#undef SET_TPC
#undef READ_REG_Q
#undef WRITE_REG_Q
#undef READ_REG_F
#undef WRITE_REG_F
#undef SET_ADDR_DSIZE
#undef READ
#undef WRITE
#undef SYSCALL
#undef IR1
#undef IR2
#undef IR3
#undef OR1
#undef DSINK
#undef DNA

      pii_head = pii_ptr = (pii_ptr + 1) % pii_size;
   }
}

void
perfect_info_skip(const struct predec_insn_t *pdi,
                  md_addr_t PC)
{
   /* skip only correct path nops */
   if (pdi->poi.PC == pii_regs.PC)
      pii_regs.PC = PC;
}

void
mem_sync(struct mem_t *mem_to,
         const struct mem_t *mem_from)
{
  int i;
  struct mem_pte_t *pte_to, *pte_from;

  for (i=0; i < MEM_PTAB_SIZE; i++)
    for (pte_from = mem_from->ptab[i]; pte_from; pte_from = pte_from->next)
      {
        /* find corresponding page in mem_to */
        for (pte_to = mem_to->ptab[i]; pte_to; pte_to = pte_to->next)
          if (pte_to->tag == pte_from->tag)
            break;

        /* allocate new page */
        if (!pte_to)
          {
            pte_to = (struct mem_pte_t *)calloc(1, sizeof(struct mem_pte_t));
            if (!pte_to) fatal("out of virtual memory!");

            pte_to->page = getcore(MD_PAGE_SIZE);
            if (!pte_to->page) fatal("out of virtual memory!");
            pte_to->tag = pte_from->tag;

            pte_to->next = mem_to->ptab[i];
            mem_to->ptab[i] = pte_to;

            mem_to->page_count++;
          }

        /* sync memory contents */
        memmove(pte_to->page, pte_from->page, MD_PAGE_SIZE);
      }
}

void
regs_sync(struct regs_t *regs_to,
          const struct regs_t *regs_from)
{
  int i;
  for (i = 0; i < MD_TOTAL_REGS; i++)
    regs_to->regs[i].q = regs_from->regs[i].q;

  regs_to->PC = regs_from->PC;
  regs_to->NPC = regs_from->NPC;
  regs_to->TPC = regs_from->TPC;

}

void
perfect_info_sync(const struct regs_t *regs,
                  const struct mem_t *mem,
                  seq_t seq)
{
   regs_sync(&pii_regs, regs);
   mem_sync(pii_mem, mem);

   pii_head = pii_ptr = pii_num = 0;
   pii_f_wrong_path = FALSE;

   pii_seq = seq;
}

void
perfect_info_recover(seq_t seq)
{
   int i;

   /* trying to recover to a place for which there is no perfect
      information (i.e., wrong path instruction) */
   if (seq == 0)
      return;

   for (i = 0; i < pii_size; i++)
   {
      pii_ptr = (pii_ptr + pii_size - 1) % pii_size;
      if (pii_buf[pii_ptr].seq == seq)
         break;
   }

   if (i == pii_size)
      panic("unable to recover perfect info to seq %u!", seq);

   pii_ptr = (pii_ptr + 1) % pii_size;

   if (pii_ptr == pii_head)
   {
      pii_f_wrong_path = FALSE;
   }
   else
   {
      pii_f_wrong_path = pii_buf[pii_ptr].f_wrong_path;
      pii_regs.PC = pii_buf[pii_ptr].PC;
   }
}

