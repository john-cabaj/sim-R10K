/*
 * alpha.h - Alpha ISA definitions
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
 * $Id: alpha.h,v 1.2 1998/08/31 17:15:23 taustin Exp taustin $
 *
 * $Log: alpha.h,v $
 * Revision 1.2  1998/08/31 17:15:23  taustin
 * fixed non-portable SEXT definitions
 *
 * Revision 1.1  1998/08/27 16:53:36  taustin
 * Initial revision
 *
 * Revision 1.1  1998/05/06  01:09:18  calder
 * Initial revision
 *
 * Revision 1.5  1997/03/11  01:38:10  taustin
 * updated copyrights
 * long/int tweaks made for ALPHA target support
 * IFAIL() hook now allows simulators to declare instruction faults
 * IDIV()/IMOD()/FDIV() hooks now support simulator fault masking
 * supported added for non-GNU C compilers
 *
 * Revision 1.4  1997/01/06  16:08:10  taustin
 * comments updated
 * functional unit definitions moved from ss.def
 *
 * Revision 1.3  1996/12/27  15:55:37  taustin
 * fixed system header collision with MAXINT
 *
 * Revision 1.1  1996/12/05  18:50:23  taustin
 * Initial revision
 *
 */

// #define GDB_BULLSHIT

#ifndef ALPHA_H
#define ALPHA_H

#include <stdio.h>

#include "host.h"
#include "misc.h"

/*
 * This file contains various definitions needed to decode, disassemble, and
 * execute Alpha AXP instructions.
 */

/* build for Alpha AXP target */
#define TARGET_ALPHA

/* not applicable/available, usable in most definition contexts */
#define NA		0

/*
 * target-dependent type definitions
 */

/* define MD_QUAD_ADDRS if the target requires 64-bit (quadword) addresses */
#define MD_QUAD_ADDRS

/* address type definition */
typedef quad_t md_addr_t;

#define MD_ADDR_COLLISION(ADDR1,DSIZE1,ADDR2,DSIZE2) \
   ((((ADDR1) >= (ADDR2)) && ((ADDR1) < ((ADDR2) + (DSIZE2)))) || \
    (((ADDR2) >= (ADDR1)) && ((ADDR2) < ((ADDR1) + (DSIZE1)))))

#define MD_ADDR_MATCH(ADDR1,DSIZE1,ADDR2,DSIZE2) \
   (((ADDR1) == (ADDR2)) && ((DSIZE1) == (DSIZE2)))

/* Unique sequence and tag numbers */
typedef quad_t seq_t;
typedef quad_t tag_t;

#define MD_DATAPATH_WIDTH 8
#define MD_DATAPATH_MASK (MD_DATAPATH_WIDTH - 1)
#define MD_ADDR_OFFSET(ADDR) ((ADDR) & (MD_DATAPATH_MASK))
#define MD_ALIGN_ADDR(ADDR) ((ADDR) & ~(MD_DATAPATH_MASK))

#define READ_QUAD(PV,PQ,OFF,SZ) \
  switch ((SZ)) { \
    case 1: *(byte_t*)(PV) = *(byte_t*)((byte_t*)(PQ) + (OFF)); break; \
    case 2: *(half_t*)(PV) = *(half_t*)((byte_t*)(PQ) + (OFF)); break; \
    case 4: *(word_t*)(PV) = *(word_t*)((byte_t*)(PQ) + (OFF)); break; \
    case 8: *(quad_t*)(PV) = *(quad_t*)((byte_t*)(PQ) + (OFF)); break; }

#define WRITE_QUAD(PV,PQ,OFF,SZ) \
  switch ((SZ)) { \
    case 1: *(byte_t*)((byte_t*)(PQ) + (OFF)) = *(byte_t*)(PV); break; \
    case 2: *(half_t*)((byte_t*)(PQ) + (OFF)) = *(half_t*)(PV); break; \
    case 4: *(word_t*)((byte_t*)(PQ) + (OFF)) = *(word_t*)(PV); break; \
    case 8: *(quad_t*)((byte_t*)(PQ) + (OFF)) = *(quad_t*)(PV); break; }


/*
 * target-dependent memory module configuration
 */

#define MD_VADDR_SIZE 64
#define MD_PADDR_SIZE 40

/* physical memory page size (must be a power-of-two) */
#define MD_PAGE_SIZE		8192
#define MD_LOG_PAGE_SIZE	13

/* memory access command */
enum mem_cmd_t {
  mc_READ,		/* read memory from target (simulated prog) to host */
  mc_WRITE,		/* write memory from host (simulator) to target */
  mc_PREFETCH,
  mc_NUM
};

/*
 * target-dependent instruction faults
 */

enum md_fault_t {
  md_fault_none = 0,		/* no fault */
  md_fault_invalid,
  md_fault_access,		/* storage access fault */
  md_fault_alignment,		/* storage alignment fault */
  md_fault_overflow,		/* signed arithmetic overflow fault */
  md_fault_div0,		/* division by zero fault */
  md_fault_break,		/* BREAK instruction fault */
  md_fault_unimpl,		/* unimplemented instruction fault */
  md_fault_internal		/* internal S/W fault */
};

#define DEP_INUM 3
#define DEP_ONUM 1
#define DEP_NUM 4

/* dependences in terms of inputs and outputs */
#define DEP_I1 0
#define DEP_I2 1 
#define DEP_I3 2
#define DEP_O1 3

#define DEP_STORE_DATA          DEP_I1
#define DEP_ADDR                DEP_I2

union val_t 
{
  quad_t q;
  dfloat_t d;
};

struct hhm_t
{
  counter_t inst;
  counter_t lookup;
  counter_t hit;
  counter_t hiss;
  counter_t miss;
};

/*
 * target-dependent register file definitions, used by regs.[hc]
 */

typedef shalf_t regnum_t;
#define regnum_NONE -1

/* number of integer registers */
#define MD_NUM_IREGS 32

/* number of floating point registers */
#define MD_NUM_FREGS 32

/* number of control registers */
#define MD_NUM_CREGS 3

/* total number of registers, excluding PC and NPC */
#define MD_TOTAL_REGS (MD_NUM_IREGS + MD_NUM_FREGS + MD_NUM_CREGS)

#define DNA -1
/* register specifiers */
#define DGPRA		((inst >> 21) & 0x1f)		/* reg source #1 */
#define DGPRB		((inst >> 16) & 0x1f)		/* reg source #2 */
#define DGPRC		(inst & 0x1f)			/* reg dest */

#define DFPRA		(((inst >> 21) & 0x1f) + MD_NUM_IREGS)	/* reg source #1 */
#define DFPRB		(((inst >> 16) & 0x1f) + MD_NUM_IREGS)	/* reg source #2 */
#define DFPRC		((inst & 0x1f) + MD_NUM_IREGS)		/* reg dest */

#define DFPCR		(MD_NUM_IREGS+MD_NUM_FREGS+0)
#define DUNIQ		(MD_NUM_IREGS+MD_NUM_FREGS+1)
#define DSINK           (MD_NUM_IREGS+MD_NUM_FREGS+2)

/* returns 8-bit unsigned immediate field value */
#define IMM		((quad_t)((inst >> 13) & 0xff))

/* returns 21-bit unsigned absolute jump target field value */
#define TARG		(inst & 0x1fffff)

/* load/store 16-bit unsigned offset field value */
#define OFS		(inst & 0xffff)

/* sign-extend operands */
#define SEXT(X)								\
  (((X) & 0x8000) ? ((squad_t)(X) | LL(0xffffffffffff0000)) : (squad_t)(X))

#define SEXT21(X)							\
  (((X) & 0x100000) ? ((squad_t)(X) | LL(0xffffffffffe00000)) : (squad_t)(X))

#define SEXT32(X)							\
  (((X) & 0x80000000) ? ((squad_t)(X) | LL(0xffffffff00000000)) : (squad_t)(X))

/* register file */
struct regs_t {
  union val_t regs[MD_TOTAL_REGS];
  md_addr_t PC;		/* program counter */
  md_addr_t NPC;	/* pseudo-register: next-cycle program counter */
  md_addr_t TPC;        /* pseudo-register: target program counter */
  md_addr_t addr;       /* pseudo-register: load-store address */
  unsigned int dsize;   /* pseudo-register: load-store data size */
};

/* well known registers */
enum md_reg_names {
  MD_REG_V0 = 0,	/* return value reg */
  MD_REG_ERR = 7,
  MD_REG_FP = 15,	/* frame pointer */
  MD_REG_A0 = 16,	/* argument regs */
  MD_REG_A1 = 17,
  MD_REG_A2 = 18,
  MD_REG_A3 = 19,
  MD_REG_A4 = 20,
  MD_REG_A5 = 21,
  MD_REG_RA = 26,	/* return address reg */
  MD_REG_GP = 29,	/* global data section pointer */
  MD_REG_SP = 30,	/* stack pointer */
  MD_REG_ZERO = 31,	/* zero register */
  MD_FREG_ZERO = 63
};

#define REG_ISZERO(R) ((R) == MD_REG_ZERO || (R) == MD_FREG_ZERO)
#define REG_ISDEP(R) ((R) != DNA && !REG_ISZERO(R))
#define LREG_ISDEP(R) ((R) != DNA && (R) != MD_REG_ZERO && (R) != MD_FREG_ZERO && (R) != DSINK)


/*
 * target-dependent instruction format definition
 */

/* instruction formats */
typedef word_t md_inst_t;
#define md_inst_size sizeof(md_inst_t)

/* preferred nop instruction definition */
extern md_inst_t MD_NOP_INST;

#define SHIFT_PC(PC) ((PC) >> 2)
#define UNSHIFT_PC(PC) ((PC) << 2)
#define SHIFT_XOR(PC, SHIFT) (((PC) >> (SHIFT)) ^ (PC))
#define SHIFT_ADDR(ADDR) ((ADDR) >> 3)
/*
 * target-dependent loader module configuration
 */

/* maximum size of argc+argv+envp environment */
#define MD_MAX_ENVIRON		16384


/*
 * machine.def specific definitions
 */

/* inst -> enum md_opcode mapping, use this macro to decode insts */
#define MD_TOP_OP(INST)		(((INST) >> 26) & 0x3f)
#define MD_SET_OPCODE(OP, INST)						\
  { OP = md_mask2op[MD_TOP_OP(INST)];					\
    while (md_opmask[OP])						\
      OP = md_mask2op[((INST >> md_opshift[OP]) & md_opmask[OP])	\
		      + md_opoffset[OP]]; }


/* largest opcode field value (currently upper 8-bit are used for pre/post-
    incr/decr operation specifiers */
#define MD_MAX_MASK		2048 /* was: 1024 */

/* global opcode names, these are returned by the decoder (MD_OP_ENUM()) */
enum md_opcode_t {
  OP_NA = 0,	/* NA */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3) OP,
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT) OP,
#define CONNECT(OP)
#include "machine.def"
  OP_MAX	/* number of opcodes + NA */
};

/* internal decoder state */
extern enum md_opcode_t md_mask2op[];
extern unsigned int md_opoffset[];
extern unsigned int md_opmask[];
extern unsigned int md_opshift[];

void md_set_opcode(enum md_opcode_t *op, md_inst_t inst);

/* enum md_opcode -> description string */
#define MD_OP_NAME(OP)		(md_op2name[OP])
extern char *md_op2name[];

/* enum md_opcode -> opcode operand format, used by disassembler */
#define MD_OP_FORMAT(OP)	(md_op2format[OP])
extern char *md_op2format[];

/* function unit classes, update md_fu2name if you update this definition */
enum fuclass_t {
  fuclass_NA = 0, 
  fuclass_IALU, 
  fuclass_ISHIFT,
  fuclass_IBRANCH,
  fuclass_IMULT, 
  fuclass_IDIV,
  fuclass_FADD, 
  fuclass_FBRANCH,
  fuclass_FCVT,
  fuclass_FMULT,
  fuclass_FDIV,
  fuclass_FSQRT,
  fuclass_NUM
};

enum sched_class_t
{
  sclass_INT,
  sclass_FP,
  sclass_STORE,
  sclass_LOAD,
  sclass_TOTAL,
  sclass_NUM
};

/* enum md_opcode -> enum md_fu_class, used by performance simulators */
#define MD_OP_FUCLASS(OP)	(md_op2fu[OP])
extern enum fuclass_t md_op2fu[];

/* instruction flags */
#define F_ICOMP		0x00000001	/* integer computation */
#define F_FCOMP		0x00000002	/* FP computation */
#define F_CTRL		0x00000004	/* control inst */
#define F_UNCOND	0x00000008	/*   unconditional change */
#define F_COND		0x00000010	/*   conditional change */
#define F_MEM		0x00000020	/* memory access inst */
#define F_LOAD		0x00000040	/*   load inst */
#define F_STORE		0x00000080	/*   store inst */
#define F_DISP		0x00000100	/*   displaced (R+C) addr mode */
#define F_RR		0x00000200	/*   R+R addr mode */
#define F_DIRECT	0x00000400	/*   direct addressing mode */
#define F_TRAP		0x00000800	/* traping inst */
#define F_LONGLAT	0x00001000	/* long latency inst (for sched) */
#define F_DIRJMP	0x00002000	/* direct jump */
#define F_INDIRJMP	0x00004000	/* indirect jump */
#define F_CALL		0x00008000	/* function call */
#define F_FPCOND	0x00010000	/* FP conditional branch */
#define F_IMM		0x00020000	/* instruction has immediate operand */
#define F_OFS		0x00040000	/* instruction has immediate operand */
#define F_TARG		0x00080000	/* instruction has immediate operand */

/* enum md_opcode -> opcode flags, used by simulators */
#define MD_OP_FLAGS(OP)		(md_op2flags[OP])
extern unsigned int md_op2flags[];

#define MD_OP_HASFLAGS(OP, FLAGS) \
    ((MD_OP_FLAGS(OP) & (FLAGS)) == (FLAGS))

#define MD_OP_HASANYFLAGS(OP, FLAGS) \
    ((MD_OP_FLAGS(OP) & (FLAGS)) != 0)

/* test for arithmetic overflow */
#define ARITH_OVFL(RESULT, OP1, OP2) ((RESULT) < (OP1) || (RESULT) < (OP2))

/* default target PC handling */
#ifdef GET_OUT
#ifndef SET_TPC
#define SET_TPC(PC)	(void)0
#endif /* SET_TPC */
#endif /* GET_OUT */

/*
 * various other helper macros/functions
 */

/* non-zero if system call is an exit() */
#define OSF_SYS_exit			1
#define MD_EXIT_SYSCALL(REGS)						\
  ((REGS)->regs[MD_REG_V0].q == OSF_SYS_exit)

/* non-zero if system call is a write to stdout/stderr */
#define OSF_SYS_write			4
#define MD_OUTPUT_SYSCALL(REGS)						\
  ((REGS)->regs[MD_REG_V0].q == OSF_SYS_write				\
   && ((REGS)->regs[MD_REG_A0].q == /* stdout */1			\
       || (REGS)->regs[MD_REG_A0].q == /* stderr */2))

/* returns stream of an output system call, translated to host */
#define MD_STREAM_FILENO(REGS)		((REGS)->regs[MD_REG_A0].q)

/* returns non-zero if instruction is a function call */
#define MD_IS_CALL(OP)			((OP) == JSR || (OP) == BSR)

/* returns non-zero if instruction is a function return */
#define MD_IS_RETURN(OP)		((OP) == RETN)

/* returns non-zero if instruction is an indirect jump */
#define MD_IS_INDIR(OP)							\
  ((OP) == JMP || (OP) == JSR || (OP) == RETN || (OP) == JSR_COROUTINE)

/* addressing mode probe, enums and strings */
enum md_amode_type {
  md_amode_imm,		/* immediate addressing mode */
  md_amode_gp,		/* global data access through global pointer */
  md_amode_sp,		/* stack access through stack pointer */
  md_amode_fp,		/* stack access through frame pointer */
  md_amode_disp,	/* (reg + const) addressing */
  md_amode_rr,		/* (reg + reg) addressing */
  md_amode_NUM
};
extern char *md_amode_str[md_amode_NUM];

/* addressing mode pre-probe FSM, must see all instructions */
#define MD_AMODE_PREPROBE(OP, FSM)		{ (FSM) = 0; }

/* compute addressing mode, only for loads/stores */
#define MD_AMODE_PROBE(AM, OP, FSM)					\
  {									\
    if (MD_OP_FLAGS(OP) & F_DISP)					\
      {									\
	if ((RB) == MD_REG_GP)						\
	  (AM) = md_amode_gp;						\
	else if ((RB) == MD_REG_SP)					\
	  (AM) = md_amode_sp;						\
	else if ((RB) == MD_REG_FP) /* && bind_to_seg(addr) == seg_stack */\
	  (AM) = md_amode_fp;						\
	else								\
	  (AM) = md_amode_disp;						\
      }									\
    else if (MD_OP_FLAGS(OP) & F_RR)					\
      (AM) = md_amode_rr;						\
    else								\
      panic("cannot decode addressing mode");				\
  }

/* addressing mode pre-probe FSM, after all loads and stores */
#define MD_AMODE_POSTPROBE(FSM)			/* nada... */


/*
 * EIO package configuration/macros
 */

/* expected EIO file format */
#define MD_EIO_FILE_FORMAT		EIO_ALPHA_FORMAT

#define MD_MISC_REGS_TO_EXO(REGS)					\
  exo_new(ec_list,							\
	  /*icnt*/exo_new(ec_integer, (exo_integer_t)sim_num_insn),	\
	  /*PC*/exo_new(ec_address, (exo_integer_t)(REGS)->PC),	        \
	  /*NPC*/exo_new(ec_address, (exo_integer_t)(REGS)->NPC),	\
	  /*FPCR*/exo_new(ec_integer, (exo_integer_t)(REGS)->regs[DFPCR].q),\
	  /*UNIQ*/exo_new(ec_integer, (exo_integer_t)(REGS)->regs[DUNIQ].q),\
	  NULL)

#define MD_IREG_TO_EXO(REGS, IDX)					\
  exo_new(ec_address, (exo_integer_t)(REGS)->regs[IDX].q)

#define MD_FREG_TO_EXO(REGS, IDX)					\
  exo_new(ec_address, (exo_integer_t)(REGS)->regs[IDX+MD_NUM_IREGS].q)

#define MD_EXO_TO_MISC_REGS(EXO, ICNT, REGS)				\
  /* check EXO format for errors... */					\
  if (!exo								\
      || exo->ec != ec_list						\
      || !exo->as_list.head						\
      || exo->as_list.head->ec != ec_integer				\
      || !exo->as_list.head->next					\
      || exo->as_list.head->next->ec != ec_address			\
      || !exo->as_list.head->next->next					\
      || exo->as_list.head->next->next->ec != ec_address		\
      || !exo->as_list.head->next->next->next				\
      || exo->as_list.head->next->next->next->ec != ec_integer		\
      || !exo->as_list.head->next->next->next->next			\
      || exo->as_list.head->next->next->next->next->ec != ec_integer	\
      || exo->as_list.head->next->next->next->next->next != NULL)	\
    fatal("could not read EIO misc regs");				\
  (ICNT) = (counter_t)exo->as_list.head->as_integer.val;		\
  (REGS)->PC = (md_addr_t)exo->as_list.head->next->as_integer.val;	\
  (REGS)->NPC =  							\
    (md_addr_t)exo->as_list.head->next->next->as_integer.val;		\
  (REGS)->regs[DFPCR].q =							\
    (quad_t)exo->as_list.head->next->next->next->as_integer.val;	\
  (REGS)->regs[DUNIQ].q =							\
    (quad_t)exo->as_list.head->next->next->next->next->as_integer.val;

#define MD_EXO_TO_IREG(EXO, REGS, IDX)					\
  ((REGS)->regs[IDX].q = (quad_t)(EXO)->as_integer.val)

#define MD_EXO_TO_FREG(EXO, REGS, IDX)					\
  ((REGS)->regs[IDX+MD_NUM_IREGS].q = (quad_t)(EXO)->as_integer.val)

#define MD_EXO_CMP_IREG(EXO, REGS, IDX)					\
  ((REGS)->regs[IDX].q != (quad_t)(EXO)->as_integer.val)

#define MD_FIRST_IN_REG			0
#define MD_LAST_IN_REG			21

#define MD_FIRST_OUT_REG		0
#define MD_LAST_OUT_REG			21


/*
 * configure the EXO package
 */

/* EXO pointer class */
typedef quad_t exo_address_t;

/* EXO integer class, 64-bit encoding */
typedef quad_t exo_integer_t;

/* EXO floating point class, 64-bit encoding */
typedef double exo_float_t;

/*
 * configure the stats package
 */

/* counter stats */
#define stat_reg_counter		stat_reg_squad
#define sc_counter			sc_squad
#define for_counter			for_squad

/* address stats */
#define stat_reg_addr			stat_reg_quad


/*
 * configure the DLite! debugger
 */

/* register bank specifier */
enum md_reg_type {
  rt_gpr,		/* general purpose register */
  rt_lpr,		/* integer-precision floating pointer register */
  rt_fpr,		/* single-precision floating pointer register */
  rt_dpr,		/* double-precision floating pointer register */
  rt_ctrl,		/* control register */
  rt_PC,		/* program counter */
  rt_NPC,		/* next program counter */
  rt_NUM
};

/* register name specifier */
struct md_reg_names_t {
  char *str;			/* register name */
  enum md_reg_type file;	/* register file */
  int reg;			/* register index */
};

/* symbolic register names, parser is case-insensitive */
extern struct md_reg_names_t md_reg_names[];

/* returns a register name string */
char *md_reg_name(enum md_reg_type rt, int reg);

void md_print_regs(const struct regs_t * regs, FILE *stream);

/* xor checksum registers */
word_t md_xor_regs(struct regs_t *regs);

int char2bool(char c);

/*
 * configure sim-outorder specifics
 */

enum insn_class_t
{
  ic_nop = 0,
  ic_prefetch,
  ic_load,
  ic_store,
  ic_ctrl,
  ic_icomp,
  ic_icomplong,
  ic_fcomp,
  ic_fcomplong,
  ic_sys,
  ic_other,
  ic_NUM
};

extern char *md_pm2str[];

/* primitive operation used to compute addresses within pipeline */
#define MD_AGEN_OP		ADDQ

/* NOP operation when injected into the pipeline */
#define MD_NOP_OP		OP_NA

/* non-zero for a valid address, used to determine if speculative accesses
   should access the DL1 data cache */
#define MD_VALID_ADDR(ADDR)						\
  (((ADDR) >= ld_text_base && (ADDR) < (ld_text_base + ld_text_size))	\
   || ((ADDR) >= ld_data_base && (ADDR) < ld_brk_point)			\
   || ((ADDR) >= (ld_stack_base - 16*1024*1024) && (ADDR) < ld_stack_base))

/*
 * configure branch predictors
 */

/* shift used to ignore branch address least significant bits, usually
   log2(sizeof(md_inst_t)) */
#define MD_BR_SHIFT		2	/* log2(4) */


/*
 * target-dependent routines
 */

/* intialize the inst decoder, this function builds the ISA decode tables */
void md_init_decoder(void);

/* disassemble a SimpleScalar instruction */
void
md_print_insn(md_inst_t inst,		/* instruction to disassemble */
	      md_addr_t pc,		/* addr of inst, used for PC-rels */
	      FILE *stream);		/* output stream */


#endif /* ALPHA_H */







#if 0

/* instruction/address formats */
typedef qword_t MD_ADDR_TYPE;
typedef qword_t MD_PTR_TYPE;
typedef word_t MD_INST_TYPE;
#define MD_INST_SIZE		sizeof(MD_INST_TYPE)

/* virtual memory segment limits */
#define MD_TEXT_BASE		0x20000000ULL
#define MD_STACK_BASE 		(MD_TEXT_BASE - (409600+4096))

/* well known registers */
enum { REG_V0, REG_A0=16, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_ERR=7,
       REG_GP=29, REG_SP, REG_ZERO, REG_RA=26 };

/* total number of register in processor 32I+32F+HI+LO+FCC+TMP+MEM+CTRL */
#define MD_TOTAL_REGS							\
  (MD_NUM_IREGS+MD_NUM_FREGS+/*FPCR*/1+/*UNIQ*/1+/*MEM*/1+/*CTRL*/1)

/* inst check macros, activated if NO_ICHECKS is not defined (default) */
#ifndef NO_ICHECKS

/* instruction failure notification macro, this can be defined by the
   target simulator if, for example, the simulator wants to handle the
   instruction fault in a machine specific fashion; a string describing
   the instruction fault is passed to the IFAIL() macro */
#ifndef IFAIL
#define IFAIL(S)	fatal(S)
#endif /* IFAIL */

/* check for overflow in X+Y, both signed */
#define OVER(X,Y)	(((((X) > 0) && ((Y) > 0)			\
			   && (MAXINT_VAL - (X) < (Y)))			\
			  ? IFAIL("+ overflow") : (void)0),		\
			 ((((X) < 0) && ((Y) < 0)			\
			   && (-MAXINT_VAL - (X) > (Y)))		\
			  ? IFAIL("+ underflow") : (void)0))

/* check for underflow in X-Y, both signed */
#define UNDER(X,Y)	(((((X) > 0) && ((Y) < 0)			\
			   && (MAXINT_VAL + (Y) < (X)))			\
			  ? IFAIL("- overflow") : (void)0),		\
			 ((((X) < 0) && ((Y) > 0)			\
			   && (-MAXINT_VAL + (Y) > (X)))		\
			  ? IFAIL("- underflow") : (void)0))

/* check for divide by zero error, N is denom */
#define DIV0(N)		(((N) == 0) ? IFAIL("divide by 0") : (void)0)

/* check reg specifier N for required double integer word alignment */
#define INTALIGN(N)	(((N) & 01)					\
			 ? IFAIL("bad INT register alignment") : (void)0)

/* check reg specifier N for required double FP word alignment */
#define FPALIGN(N)	(((N) & 01)					\
			 ? IFAIL("bad FP register alignment") : (void)0)

/* check target address TARG for required jump target alignment */
#define TALIGN(TARG)	(((TARG) & 0x7)					\
			 ? IFAIL("bad jump alignment") : (void)0)

#else /* NO_ICHECKS */

/* inst checks disables, change all checks to NOP expressions */
#define OVER(X,Y)	((void)0)
#define UNDER(X,Y)	((void)0)
#define DIV0(N)		((void)0)
#define INTALIGN(N)	((void)0)
#define FPALIGN(N)	((void)0)
#define TALIGN(TARG)	((void)0)

#endif /* NO_ICHECKS */

/* default division operator semantics, this operation is accessed through a
   macro because some simulators need to check for divide by zero faults
   before executing this operation */
#define IDIV(A, B)	((A) / (B))
#define IMOD(A, B)	((A) % (B))
#define FDIV(A, B)	((A) / (B))
#define FINT(A)		((int)A)

#endif /* 0 */

#define ITOFS(IQ,SQ)                                                    \
{                                                                       \
    squad_t _longhold = (IQ), _e1, _e2;					\
									\
    _e1 = _longhold & 0x40000000;					\
    _e2 = (_longhold >> 23) & ULL(0x7f);				\
    if (_e1)								\
      {									\
	if (_e2 == ULL(0x3f800000))					\
	  _e2 = ULL(0x7ff);						\
	else								\
	  _e2 |= ULL(0x400);						\
      }									\
    else								\
      {									\
	if (_e2 == 0)							\
	  _e2 = 0;							\
	else								\
	  _e2 |= ULL(0x380);						\
      }									\
    (SQ) = ((_longhold & ULL(0x80000000)) << 32			        \
		   | _e2 << 52 | (_longhold & ULL(0x7fffff)) << 29);	\
}

#define IS_POWEROF2(N) (((N) & ((N) - 1)) == 0)
