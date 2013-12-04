/*
 * alpha.c - Alpha ISA definition routines
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
 * $Id: alpha.c,v 1.2 1998/08/31 17:15:58 taustin Exp taustin $
 *
 * $Log: alpha.c,v $
 * Revision 1.2  1998/08/31 17:15:58  taustin
 * added register checksuming support
 *
 * Revision 1.1  1998/08/27 16:53:27  taustin
 * Initial revision
 *
 * Revision 1.1  1998/05/06  01:08:39  calder
 * Initial revision
 *
 * Revision 1.4  1997/03/11  01:40:38  taustin
 * updated copyrights
 * long/int tweaks made for ALPHA target support
 * supported added for non-GNU C compilers
 * ss_print_insn() can now tolerate bogus insts (for DLite!)
 *
 * Revision 1.3  1997/01/06  16:07:24  taustin
 * comments updated
 * functional unit definitions moved from ss.def
 *
 * Revision 1.1  1996/12/05  18:52:32  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "host.h"
#include "misc.h"
#include "machine.h"

/* FIXME: currently SimpleScalar/AXP only builds on little-endian... */
#if !defined(BYTES_LITTLE_ENDIAN) || !defined(WORDS_LITTLE_ENDIAN)
#error SimpleScalar/AXP only builds on little-endian machines...
#endif

/* FIXME: currently SimpleScalar/AXP only builds with quadword support... */
#if !defined(HOST_HAS_QUAD)
#error SimpleScalar/AXP only builds on hosts with builtin quadword support...
#error Try building with GNU GCC, as it supports quadwords on most machines.
#endif

int n_processes = 1;

/* preferred nop instruction definition */
md_inst_t MD_NOP_INST = 0x43ff041f;		/* addq r31,r31,r31 */

/* opcode mask -> enum md_opcodem, used by decoder (MD_OP_ENUM()) */
enum md_opcode_t md_mask2op[MD_MAX_MASK+1];
unsigned int md_opoffset[OP_MAX];

/* enum md_opcode -> mask for decoding next level */
unsigned int md_opmask[OP_MAX] = {
  0, /* NA */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3) 0,
#define DEFLINK(OP,MSK,NAME,SHIFT,MASK) MASK,
#define CONNECT(OP)
#include "machine.def"
};

/* enum md_opcode -> shift for decoding next level */
unsigned int md_opshift[OP_MAX] = {
  0, /* NA */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3) 0,
#define DEFLINK(OP,MSK,NAME,SHIFT,MASK) SHIFT,
#define CONNECT(OP)
#include "machine.def"
};

/* enum md_opcode -> description string */
char *md_op2name[OP_MAX] = {
  NULL, /* NA */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3) NAME,
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT) NAME,
#define CONNECT(OP)
#include "machine.def"
};

/* enum md_opcode -> opcode operand format, used by disassembler */
char *md_op2format[OP_MAX] = {
  NULL, /* NA */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3) OPFORM,
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT) NULL,
#define CONNECT(OP)
#include "machine.def"
};

/* enum md_opcode -> enum md_fu_class, used by performance simulators */
enum fuclass_t md_op2fu[OP_MAX] = {
  fuclass_NA, /* NA */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3) RES,
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT) fuclass_NA,
#define CONNECT(OP)
#include "machine.def"
};

char *md_amode_str[md_amode_NUM] =
{
  "(const)",		/* immediate addressing mode */
  "(gp + const)",	/* global data access through global pointer */
  "(sp + const)",	/* stack access through stack pointer */
  "(fp + const)",	/* stack access through frame pointer */
  "(reg + const)",	/* (reg + const) addressing */
  "(reg + reg)"		/* (reg + reg) addressing */
};

/* symbolic register names, parser is case-insensitive */
struct md_reg_names_t md_reg_names[] =
{
  /* name */	/* file */	/* reg */

  /* integer register file */
  { "$r0",	rt_gpr,		0 },
  { "$v0",	rt_gpr,		0 },
  { "$r1",	rt_gpr,		1 },
  { "$r2",	rt_gpr,		2 },
  { "$r3",	rt_gpr,		3 },
  { "$r4",	rt_gpr,		4 },
  { "$r5",	rt_gpr,		5 },
  { "$r6",	rt_gpr,		6 },
  { "$r7",	rt_gpr,		7 },
  { "$err",	rt_gpr,		7 },
  { "$r8",	rt_gpr,		8 },
  { "$r9",	rt_gpr,		9 },
  { "$r10",	rt_gpr,		10 },
  { "$r11",	rt_gpr,		11 },
  { "$r12",	rt_gpr,		12 },
  { "$r13",	rt_gpr,		13 },
  { "$r14",	rt_gpr,		14 },
  { "$r15",	rt_gpr,		15 },
  { "$fp",	rt_gpr,		15 },
  { "$r16",	rt_gpr,		16 },
  { "$a0",	rt_gpr,		16 },
  { "$r17",	rt_gpr,		17 },
  { "$a1",	rt_gpr,		17 },
  { "$r18",	rt_gpr,		18 },
  { "$a2",	rt_gpr,		18 },
  { "$r19",	rt_gpr,		19 },
  { "$a3",	rt_gpr,		19 },
  { "$r20",	rt_gpr,		20 },
  { "$a4",	rt_gpr,		20 },
  { "$r21",	rt_gpr,		21 },
  { "$a5",	rt_gpr,		21 },
  { "$r22",	rt_gpr,		22 },
  { "$r23",	rt_gpr,		23 },
  { "$r24",	rt_gpr,		24 },
  { "$r25",	rt_gpr,		25 },
  { "$r26",	rt_gpr,		26 },
  { "$ra",	rt_gpr,		26 },
  { "$r27",	rt_gpr,		27 },
  { "$r28",	rt_gpr,		28 },
  { "$r29",	rt_gpr,		29 },
  { "$gp",	rt_gpr,		29 },
  { "$r30",	rt_gpr,		30 },
  { "$sp",	rt_gpr,		30 },
  { "$r31",	rt_gpr,		31 },
  { "$zero",	rt_gpr,		31 },

  /* floating point register file - double precision */
  { "$f0",	rt_fpr,		0 },
  { "$f1",	rt_fpr,		1 },
  { "$f2",	rt_fpr,		2 },
  { "$f3",	rt_fpr,		3 },
  { "$f4",	rt_fpr,		4 },
  { "$f5",	rt_fpr,		5 },
  { "$f6",	rt_fpr,		6 },
  { "$f7",	rt_fpr,		7 },
  { "$f8",	rt_fpr,		8 },
  { "$f9",	rt_fpr,		9 },
  { "$f10",	rt_fpr,		10 },
  { "$f11",	rt_fpr,		11 },
  { "$f12",	rt_fpr,		12 },
  { "$f13",	rt_fpr,		13 },
  { "$f14",	rt_fpr,		14 },
  { "$f15",	rt_fpr,		15 },
  { "$f16",	rt_fpr,		16 },
  { "$f17",	rt_fpr,		17 },
  { "$f18",	rt_fpr,		18 },
  { "$f19",	rt_fpr,		19 },
  { "$f20",	rt_fpr,		20 },
  { "$f21",	rt_fpr,		21 },
  { "$f22",	rt_fpr,		22 },
  { "$f23",	rt_fpr,		23 },
  { "$f24",	rt_fpr,		24 },
  { "$f25",	rt_fpr,		25 },
  { "$f26",	rt_fpr,		26 },
  { "$f27",	rt_fpr,		27 },
  { "$f28",	rt_fpr,		28 },
  { "$f29",	rt_fpr,		29 },
  { "$f30",	rt_fpr,		30 },
  { "$f31",	rt_fpr,		31 },

  /* floating point register file - integer precision */
  { "$l0",	rt_lpr,		0 },
  { "$l1",	rt_lpr,		1 },
  { "$l2",	rt_lpr,		2 },
  { "$l3",	rt_lpr,		3 },
  { "$l4",	rt_lpr,		4 },
  { "$l5",	rt_lpr,		5 },
  { "$l6",	rt_lpr,		6 },
  { "$l7",	rt_lpr,		7 },
  { "$l8",	rt_lpr,		8 },
  { "$l9",	rt_lpr,		9 },
  { "$l10",	rt_lpr,		10 },
  { "$l11",	rt_lpr,		11 },
  { "$l12",	rt_lpr,		12 },
  { "$l13",	rt_lpr,		13 },
  { "$l14",	rt_lpr,		14 },
  { "$l15",	rt_lpr,		15 },
  { "$l16",	rt_lpr,		16 },
  { "$l17",	rt_lpr,		17 },
  { "$l18",	rt_lpr,		18 },
  { "$l19",	rt_lpr,		19 },
  { "$l20",	rt_lpr,		20 },
  { "$l21",	rt_lpr,		21 },
  { "$l22",	rt_lpr,		22 },
  { "$l23",	rt_lpr,		23 },
  { "$l24",	rt_lpr,		24 },
  { "$l25",	rt_lpr,		25 },
  { "$l26",	rt_lpr,		26 },
  { "$l27",	rt_lpr,		27 },
  { "$l28",	rt_lpr,		28 },
  { "$l29",	rt_lpr,		29 },
  { "$l30",	rt_lpr,		30 },
  { "$l31",	rt_lpr,		31 },

  /* miscellaneous registers */
  { "$fpcr",	rt_ctrl,	0 },
  { "$uniq",	rt_ctrl,	1 },

  /* program counters */
  { "$pc",	rt_PC,		0 },
  { "$npc",	rt_NPC,		0 },

  /* sentinel */
  { NULL,	rt_gpr,		0 }
};

/* returns a register name string */
char *
md_reg_name(enum md_reg_type rt, int reg)
{
  int i;

  for (i=0; md_reg_names[i].str != NULL; i++)
    {
      if (md_reg_names[i].file == rt && md_reg_names[i].reg == reg)
	return md_reg_names[i].str;
    }

  /* no found... */
  return NULL;
}

/* xor checksum registers */
word_t
md_xor_regs(struct regs_t *regs)
{
  int i;
  quad_t checksum = 0;

  for (i=0; i < MD_NUM_IREGS + MD_NUM_FREGS + 2; i++)
    checksum ^= regs->regs[i].q;

  checksum ^= regs->PC;
  checksum ^= regs->NPC;

  return (word_t)((checksum >> 32) ^ checksum);
}


/* enum md_opcode -> opcode flags, used by simulators */
unsigned int md_op2flags[OP_MAX] = {
  NA, /* NA */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3) FLAGS,
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT) NA,
#define CONNECT(OP)
#include "machine.def"
};

/* intialize the inst decoder, this function builds the ISA decode tables */
void
md_init_decoder(void)
{
  unsigned long max_offset = 0;
  unsigned long offset = 0;

#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3)		\
  if ((MSK)+offset >= MD_MAX_MASK) panic("MASK_MAX is too small");	\
  if (md_mask2op[(MSK)+offset]) fatal("doubly defined opcode");		\
  md_mask2op[(MSK)+offset]=(OP); max_offset=MAX(max_offset,(MSK)+offset);

#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)					\
  if ((MSK)+offset >= MD_MAX_MASK) panic("MASK_MAX is too small");	\
  if (md_mask2op[(MSK)+offset]) fatal("doubly defined opcode");		\
  md_mask2op[(MSK)+offset]=(OP); max_offset=MAX(max_offset,(MSK)+offset);

#define CONNECT(OP)							\
    offset = max_offset+1; md_opoffset[OP] = offset;

#include "machine.def"
}

/* disassemble an Alpha instruction */
void
md_print_insn(md_inst_t inst,		/* instruction to disassemble */
	      md_addr_t pc,		/* addr of inst, used for PC-rels */
	      FILE *stream)		/* output stream */
{
  enum md_opcode_t op;

  /* use stderr as default output stream */
  if (!stream)
    stream = stderr;

  /* decode the instruction, assumes predecoded text segment */
  MD_SET_OPCODE(op, inst);

  /* disassemble the instruction */
  if (op <= OP_NA || op >= OP_MAX)
    {
      /* bogus instruction */
      fprintf(stream, "<invalid inst: 0x%08x>", inst);
    }
  else
    {
      char *s;

#ifdef GET_OUT
      fprintf(stream, "%-10s", MD_OP_NAME(op));
#else /* GET_OUT */
      fprintf(stream, "%s ", MD_OP_NAME(op));
#endif /* GET_OUT */

      s = MD_OP_FORMAT(op);
      while (*s) {
	switch (*s) {
	case 'a':
	  if (DGPRA == MD_REG_ZERO) 
	    fprintf(stream, "zero");
	  else if (DGPRA == MD_REG_SP)
	    fprintf(stream, "sp");
	  else
	    fprintf(stream, "r%d", DGPRA);
	  break;
	case 'b':
	  if (DGPRB == MD_REG_ZERO) 
	    fprintf(stream, "zero");
	  else if (DGPRB == MD_REG_SP)
	    fprintf(stream, "sp");
	  else
	    fprintf(stream, "r%d", DGPRB);
	  break;
	case 'c':
	  if (DGPRC == MD_REG_ZERO) 
	    fprintf(stream, "zero");
	  else if (DGPRC == MD_REG_SP)
	    fprintf(stream, "sp");
	  else
	    fprintf(stream, "r%d", DGPRC);
	  break;
	case 'A':
	  fprintf(stream, "f%d", DGPRA);
	  break;
	case 'B':
	  fprintf(stream, "f%d", DGPRB);
	  break;
	case 'C':
	  fprintf(stream, "f%d", DGPRC);
	  break;
	case 'o':
	  fprintf(stream, "%d", (sword_t)SEXT(OFS));
	  break;
	case 'j':
	  myfprintf(stream, "0x%p", pc + (SEXT(OFS) << 2) + 4);
	  break;
	case 'J':
	  myfprintf(stream, "0x%p", pc + (SEXT21(TARG) << 2) + 4);
	  break;
	case 'i':
	  fprintf(stream, "%d", (word_t)IMM);
	  break;
	default:
	  /* anything unrecognized, e.g., '.' is just passed through */
	  fputc(*s, stream);
	}
	s++;
      }
    }
}

void
md_print_regs(const struct regs_t * regs,
	      FILE * f)
{
  int i;
  for (i = 0; i < MD_NUM_IREGS; i++)
    myfprintf(f, "I%d: %016p\n", i, regs->regs[i].q);

  for (i = 0; i < MD_NUM_FREGS; i++)
    myfprintf(f, "F%d: %016p\n", i, regs->regs[i+MD_NUM_IREGS].q);

  myfprintf(f, "FC: %016p\n", regs->regs[DFPCR].q);
  myfprintf(f, "UQ: %016p\n", regs->regs[DUNIQ].q);
}

void 
md_set_opcode(enum md_opcode_t *op, md_inst_t inst)
{
   *op = md_mask2op[MD_TOP_OP(inst)];
   while (md_opmask[*op])
     *op = md_mask2op[((inst >> md_opshift[*op]) & md_opmask[*op])
		      + md_opoffset[*op]]; 
}

bool_t 
char2bool(const char c)
{
  if (c == 'T' || c == 't') return TRUE;
  else if (c == 'F' || c == 'f') return FALSE;
  else fatal("unknown boolean value: '%c'", c);
}
