/*
 * eio.c - external interfaces to external I/O f\iles
 *
 * This file is a part of the SimpleScalar tool suite written by
 * Todd M. Austin as a part of the Multiscalar Research Project.
 *  
 * The tool suite is currently maintained by Doug Burger and Todd M. Austin.
 * 
 * Copyright (C) 1997, 1998 by Todd M. Austin
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
 * $Id: eio.c,v 1.3 1998/09/03 22:23:24 taustin Exp taustin $
 *
 * $Log: eio.c,v $
 * Revision 1.3  1998/09/03 22:23:24  taustin
 * fixed MACRO bug for Intel Reference Compiler (icc)
 *
 * Revision 1.2  1998/08/31 17:07:35  taustin
 * added MS VC++ headers, fixed bug in call to write()
 *
 * Revision 1.1  1998/08/27 08:20:31  taustin
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <io.h>
#else /* !_MSC_VER */
#include <unistd.h>
#endif /* _MSC_VER */

#include "/usr/include/zlib.h"

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "memory.h"
#include "loader.h"
#include "libexo.h"
#include "syscall.h"
#include "sim.h"
#include "endian.h"
#include "eio.h"

#ifdef _MSC_VER
#define write		_write
#endif /* _MSC_VER */

#define EIO_FILE_HEADER							\
  "/* This is a SimpleScalar EIO file - DO NOT MOVE OR EDIT THIS LINE! */\n"
/*
   EIO transaction format:

   (inst_count, pc,
    ... reg inputs ...
    [r2, r3, r4, r5, r6, r7],
    ... mem inputs ...
    ((addr, size, blob), ...)
    ... reg outputs ...
    [r2, r3, r4, r5, r6, r7],
    ... mem outputs ...
    ((addr, size, blob), ...)
   )
*/

/* EIO transaction count, i.e., number of last transaction completed */
static counter_t eio_trans_icnt = -1;

/* number of instructions to execute between writing a register
	file checkpoint into EIO trace. 0 means only write at syscalls */
unsigned chksum_interval = 0;

/* running checksum of register file */
int running_chksum = 0;  

/* if non-zero we signal a fatal error on a register checksum mismatch */
int verify_register_checksum = 0;

/* verify that it is a register checksum just by shape */
int eio_is_chksum(struct exo_term_t *exo);

gzFile 
eio_create(char *fname)
{
  gzFile fd;
  struct exo_term_t *exo;
  int target_big_endian;

  target_big_endian = (endian_host_byte_order() == endian_big);

  fd = gzopen(fname, "w");
  if (!fd)
    fatal("unable to create EIO file `%s'", fname);

  /* emit EIO file header */
  gzprintf(fd, "%s\n", EIO_FILE_HEADER);
  gzprintf(fd, "/* file_format: %d, file_version: %d, big_endian: %d */\n", 
	  MD_EIO_FILE_FORMAT, EIO_FILE_VERSION, ld_target_big_endian);
  exo = exo_new(ec_list,
		exo_new(ec_integer, (exo_integer_t)MD_EIO_FILE_FORMAT),
		exo_new(ec_integer, (exo_integer_t)EIO_FILE_VERSION),
		exo_new(ec_integer, (exo_integer_t)target_big_endian),
#ifdef EXO_CHKSUM					 
		exo_new(ec_integer, (exo_integer_t)chksum_interval), 
#endif /* EXO_CHKSUM */
		NULL);
  exo_print(exo, fd);
  exo_delete(exo);
  gzprintf(fd, "\n\n");

  return fd;
}

gzFile 
eio_open(char *fname)
{
  gzFile fd;
  struct exo_term_t *exo;
  int file_format, file_version, big_endian, target_big_endian;

  target_big_endian = (endian_host_byte_order() == endian_big);

  fd = gzopen(fname, "r");
  if (!fd)
    fatal("unable to open EIO file `%s'", fname);

  /* read and check EIO file header */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_list
      || !exo->as_list.head
      || exo->as_list.head->ec != ec_integer
      || !exo->as_list.head->next
      || exo->as_list.head->next->ec != ec_integer
      || !exo->as_list.head->next->next
      || exo->as_list.head->next->next->ec != ec_integer
#ifdef EXO_CHKSUM					 
      || !exo->as_list.head->next->next->next
      || exo->as_list.head->next->next->next->ec != ec_integer
      || exo->as_list.head->next->next->next->next != NULL
#else /* !EXO_CHKSUM */
      || exo->as_list.head->next->next->next != NULL
#endif /* EXO_CHKSUM */
		)
    fatal("could not read EIO file header");

  file_format = exo->as_list.head->as_integer.val;
  file_version = exo->as_list.head->next->as_integer.val;
  big_endian = exo->as_list.head->next->next->as_integer.val;
#ifdef EXO_CHKSUM					 
  chksum_interval = exo->as_list.head->next->next->next->as_integer.val;
#endif /* EXO_CHKSUM */
  exo_delete(exo);

  if (file_format != MD_EIO_FILE_FORMAT)
    fatal("EIO file `%s' has incompatible format", fname);

  if (file_version != EIO_FILE_VERSION)
    fatal("EIO file `%s' has incompatible version", fname);

#if (!defined(TARGET_ALPHA) || !defined(BYTES_BIG_ENDIAN))
  if (!!big_endian != !!target_big_endian)
    fatal("EIO file `%s' has incompatible endian format", fname);
#endif

  return fd;
}

/* returns non-zero if file FNAME has a valid EIO header */
int
eio_valid(char *fname)
{
  gzFile fd;
  char buf[512];

  /* open possible EIO file */
  fd = gzopen(fname, "r");
  if (!fd)
    return FALSE;

  /* read and check EIO file header */
  gzgets(fd, buf, 512);

  /* check the header */
  if (strcmp(buf, EIO_FILE_HEADER))
    return FALSE;

  /* all done, close up file */
  gzclose(fd);

  /* else, has a valid header, go with it... */
  return TRUE;
}

void
eio_close(gzFile fd)
{
  gzclose(fd);
}

/* check point current architected state to stream FD, returns
   EIO transaction count (an EIO file pointer) */
counter_t
eio_write_chkpt(struct regs_t *regs,		/* regs to dump */
		struct mem_t *mem,		/* memory to dump */
		gzFile fd)			/* stream to write to */
{
  int i;
  struct exo_term_t *exo;
  struct mem_pte_t *pte;

  mygzprintf(fd, "/* ** start checkpoint @ %n... */\n\n", (int)eio_trans_icnt);

  mygzprintf(fd, "/* EIO file pointer: %n... */\n", (int)eio_trans_icnt);
  exo = exo_new(ec_integer, (exo_integer_t)eio_trans_icnt);
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  /* dump misc regs: icnt, PC, NPC, etc... */
  gzprintf(fd, "/* misc regs icnt, PC, NPC, etc... */\n");
  exo = MD_MISC_REGS_TO_EXO(regs);
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  /* dump integer registers */
  gzprintf(fd, "/* integer regs */\n");
  exo = exo_new(ec_list, NULL);
  for (i=0; i < MD_NUM_IREGS; i++)
    exo->as_list.head = exo_chain(exo->as_list.head, MD_IREG_TO_EXO(regs, i));
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  /* dump FP registers */
  gzprintf(fd, "/* FP regs (integer format) */\n");
  exo = exo_new(ec_list, NULL);
  for (i=0; i < MD_NUM_FREGS; i++)
    exo->as_list.head = exo_chain(exo->as_list.head, MD_FREG_TO_EXO(regs, i));
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  gzprintf(fd, "/* writing `%d' memory pages... */\n", (int)mem->page_count);
  exo = exo_new(ec_list,
		exo_new(ec_integer, (exo_integer_t)mem->page_count),
		exo_new(ec_address, (exo_integer_t)ld_brk_point),
		exo_new(ec_address, (exo_integer_t)ld_stack_min),
		NULL);
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  gzprintf(fd, "/* text segment specifiers (base & size) */\n");
  exo = exo_new(ec_list,
		exo_new(ec_address, (exo_integer_t)ld_text_base),
		exo_new(ec_integer, (exo_integer_t)ld_text_size),
		NULL);
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  gzprintf(fd, "/* data segment specifiers (base & size) */\n");
  exo = exo_new(ec_list,
		exo_new(ec_address, (exo_integer_t)ld_data_base),
		exo_new(ec_integer, (exo_integer_t)ld_data_size),
		NULL);
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  gzprintf(fd, "/* stack segment specifiers (base & size) */\n");
  exo = exo_new(ec_list,
		exo_new(ec_address, (exo_integer_t)ld_stack_base),
		exo_new(ec_integer, (exo_integer_t)ld_stack_size),
		NULL);
  exo_print(exo, fd);
  gzprintf(fd, "\n\n");
  exo_delete(exo);

  /* visit all active memory pages, and dump them to the checkpoint file */
  MEM_FORALL(mem, i, pte)
    {
      /* dump this page... */
      exo = exo_new(ec_list,
		    exo_new(ec_address, (exo_integer_t)MEM_PTE_ADDR(pte, i)),
		    exo_new(ec_blob, MD_PAGE_SIZE, pte->page),
		    NULL);
      exo_print(exo, fd);
      gzprintf(fd, "\n\n");
      exo_delete(exo);
    }

  mygzprintf(fd, "/* ** end checkpoint @ %n... */\n\n", (int)eio_trans_icnt);

  return eio_trans_icnt;
}

/* read check point of architected state from stream FD, returns
   EIO transaction count (an EIO file pointer) */
counter_t
eio_read_chkpt(struct regs_t *regs,		/* regs to dump */
		struct mem_t *mem,		/* memory to dump */
		gzFile fd)			/* stream to read */
{
  int i, page_count;
  counter_t trans_icnt;
  struct exo_term_t *exo, *elt;

  /* read the EIO file pointer */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_integer)
    fatal("could not read EIO file pointer");
  trans_icnt = exo->as_integer.val;
  exo_delete(exo);

  /* read misc regs: icnt, PC, NPC, HI, LO, FCC */
  exo = exo_read(fd);
#ifdef EXO_CHKSUM					 
  MD_EXO_TO_MISC_REGS(exo, sim_num_insn, regs, running_chksum);
#else /* !EXO_CHKSUM */
  MD_EXO_TO_MISC_REGS(exo, sim_num_insn, regs);
#endif /* EXO_CHKSUM */
  exo_delete(exo);

  /* read integer registers */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_list)
    fatal("could not read EIO integer regs");
  elt = exo->as_list.head;
  for (i=0; i < MD_NUM_IREGS; i++)
    {
      if (!elt)
	fatal("could not read EIO integer regs (too few)");

      if (elt->ec != ec_address)
	fatal("could not read EIO integer regs (bad value)");

      MD_EXO_TO_IREG(elt, regs, i);
      elt = elt->next;
    }
  if (elt != NULL)
    fatal("could not read EIO integer regs (too many)");
  exo_delete(exo);

  /* read FP registers */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_list)
    fatal("could not read EIO FP regs");
  elt = exo->as_list.head;
  for (i=0; i < MD_NUM_FREGS; i++)
    {
      if (!elt)
	fatal("could not read EIO FP regs (too few)");

      if (elt->ec != ec_address)
	fatal("could not read EIO FP regs (bad value)");

      MD_EXO_TO_FREG(elt, regs, i);
      elt = elt->next;
    }
  if (elt != NULL)
    fatal("could not read EIO FP regs (too many)");
  exo_delete(exo);

  /* read the number of page defs, and memory config */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_list
      || !exo->as_list.head
      || exo->as_list.head->ec != ec_integer
      || !exo->as_list.head->next
      || exo->as_list.head->next->ec != ec_address
      || !exo->as_list.head->next->next
      || exo->as_list.head->next->next->ec != ec_address
      || exo->as_list.head->next->next->next != NULL)
    fatal("could not read EIO memory page count");
  page_count = exo->as_list.head->as_integer.val;
  ld_brk_point = (md_addr_t)exo->as_list.head->next->as_integer.val;
  ld_stack_min = (md_addr_t)exo->as_list.head->next->next->as_integer.val;
  exo_delete(exo);

  /* read text segment specifiers */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_list
      || !exo->as_list.head
      || exo->as_list.head->ec != ec_address
      || !exo->as_list.head->next
      || exo->as_list.head->next->ec != ec_integer
      || exo->as_list.head->next->next != NULL)
    fatal("count not read EIO text segment specifiers");
  ld_text_base = (md_addr_t)exo->as_list.head->as_integer.val;
  ld_text_size = (unsigned int)exo->as_list.head->next->as_integer.val;
  exo_delete(exo);

  /* read data segment specifiers */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_list
      || !exo->as_list.head
      || exo->as_list.head->ec != ec_address
      || !exo->as_list.head->next
      || exo->as_list.head->next->ec != ec_integer
      || exo->as_list.head->next->next != NULL)
    fatal("count not read EIO data segment specifiers");
  ld_data_base = (md_addr_t)exo->as_list.head->as_integer.val;
  ld_data_size = (unsigned int)exo->as_list.head->next->as_integer.val;
  exo_delete(exo);

  /* read stack segment specifiers */
  exo = exo_read(fd);
  if (!exo
      || exo->ec != ec_list
      || !exo->as_list.head
      || exo->as_list.head->ec != ec_address
      || !exo->as_list.head->next
      || exo->as_list.head->next->ec != ec_integer
      || exo->as_list.head->next->next != NULL)
    fatal("count not read EIO stack segment specifiers");
  ld_stack_base = (md_addr_t)exo->as_list.head->as_integer.val;
  ld_stack_size = (unsigned int)exo->as_list.head->next->as_integer.val;
  exo_delete(exo);

  for (i=0; i < page_count; i++)
    {
      int j;
      md_addr_t page_addr;
      struct exo_term_t *blob;

      /* read the page */
      exo = exo_read(fd);
      if (!exo
	  || exo->ec != ec_list
	  || !exo->as_list.head
	  || exo->as_list.head->ec != ec_address
	  || !exo->as_list.head->next
	  || exo->as_list.head->next->ec != ec_blob
	  || exo->as_list.head->next->next != NULL)
	fatal("could not read EIO memory page");
      page_addr = (md_addr_t)exo->as_list.head->as_integer.val;
      blob = exo->as_list.head->next;

      /* write data to simulator memory */
      for (j=0; j < blob->as_blob.size; j++)
	{
	  byte_t val;

	  val = blob->as_blob.data[j];
	  /* unchecked access... */
	  mem_access(mem, mc_WRITE, page_addr, &val, 1);
	  page_addr++;
	}
      exo_delete(exo);
    }

  return trans_icnt;
}

struct mem_rec_t {
  md_addr_t addr;
  unsigned size, maxsize;
  struct exo_term_t *exo;
  struct exo_term_t *blob;
};

/* reg recs */
static struct exo_term_t *input_regs;
static struct exo_term_t *output_regs;

/* input memory recs */
static struct exo_term_t *input_mem;
static struct mem_rec_t input_mem_rec;

/* output memory recs */
static struct exo_term_t *output_mem;
static struct mem_rec_t output_mem_rec;

static int seen_write;
static mem_access_fn local_mem_fn;

/* size of padding that can be filled on the end of a blob tail */
#define BLOB_TAIL_SIZE		256

/* tracing memory access function */
static enum md_fault_t
my_mem_fn(struct mem_t *mem,		/* memory space to access */
	  enum mem_cmd_t cmd,		/* Read (from sim mem) or Write */
	  md_addr_t addr,		/* target address to access */
	  void *vp,			/* host memory address to access */
	  int nbytes)			/* number of bytes to access */
{
  int i;
  unsigned char *p = vp;
  struct mem_rec_t *mem_rec = NULL;
  struct exo_term_t *mem_list = NULL;
  enum md_fault_t fault = md_fault_none;

  if (cmd == mc_READ && seen_write)
    fatal("Read after Write in eio_syscall()");

  if (cmd == mc_WRITE)
    seen_write = TRUE;

  /* record the memory access */
  if (cmd == mc_READ)
    {
      mem_rec = &input_mem_rec;
      mem_list = input_mem;
    }
  else if (cmd == mc_WRITE)
    {
      mem_rec = &output_mem_rec;
      mem_list = output_mem;
    }
  else
    panic("bogus memory access command");

  /* perform the memory access, Read's first so we can probe *p for data */
  if (cmd == mc_READ/* simulator memory */)
    fault = (*local_mem_fn)(mem, cmd, addr, p, nbytes);

  /* the following freakish code simply coalesces subsequent reads and
     writes to memory into the same EXO blob structure, this greatly
     reduces the size of the EIO output files... */
  if (mem_rec->exo != NULL
      && (mem_rec->addr + mem_rec->size == addr)
      && (mem_rec->size + nbytes < mem_rec->maxsize))
    {
      /* add to last blob */
      for (i=0; i < nbytes; i++)
	mem_rec->blob->as_blob.data[mem_rec->size + i] = p[i];
      mem_rec->size += nbytes;
      mem_rec->blob->as_blob.size = mem_rec->size;
    }
  else
    {
      /* add to a new blob */
      mem_list->as_list.head =
	exo_chain(mem_list->as_list.head,
		  (mem_rec->exo =
		   exo_new(ec_list,
			   exo_new(ec_address, (exo_integer_t)addr),
			   (mem_rec->blob =
			    exo_new(ec_blob, nbytes + BLOB_TAIL_SIZE, NULL)),
			   NULL)));
      for (i=0; i < nbytes; i++)
	mem_rec->blob->as_blob.data[i] = p[i];
      mem_rec->addr = addr;
      mem_rec->size = nbytes;
      mem_rec->maxsize = nbytes + BLOB_TAIL_SIZE;
      mem_rec->blob->as_blob.size = mem_rec->size;
    }

  /* perform the memory access */
  if (cmd == mc_WRITE /* simulator memory */)
    fault = (*local_mem_fn)(mem, cmd, addr, p, nbytes);

  return fault;
}

/* syscall proxy handler, with EIO tracing support, architect registers
   and memory are assumed to be precise when this function is called,
   register and memory are updated with the results of the sustem call */
void
eio_write_trace(gzFile eio_fd,			/* EIO stream file desc */
		counter_t icnt,			/* instruction count */
		struct regs_t *regs,		/* registers to update */
		mem_access_fn mem_fn,		/* generic memory accessor */
		struct mem_t *mem,		/* memory to update */
		md_inst_t inst)			/* system call inst */
{
  int i;
  struct exo_term_t *exo;

  /* write syscall register inputs ($r2..$r7) */
  input_regs = exo_new(ec_list, NULL);
  for (i=MD_FIRST_IN_REG; i <= MD_LAST_IN_REG; i++)
    {
      input_regs->as_list.head =
	exo_chain(input_regs->as_list.head, MD_IREG_TO_EXO(regs, i));
    }

  /* initialize memory inputs */
  input_mem = exo_new(ec_list, NULL); input_mem_rec.exo = NULL;
  output_mem = exo_new(ec_list, NULL); output_mem_rec.exo = NULL;

  /* perform the system call, record inputs and outputs */
  seen_write = FALSE;
  local_mem_fn = mem_fn;

  if (sim_eio_fd != NULL)
    eio_read_trace(sim_eio_fd, icnt, regs, my_mem_fn, mem, inst);
  else
    {
      sys_syscall(regs, my_mem_fn, mem, inst, FALSE);
    }

  /* write syscall breakpoint and register outputs ($r2..$r7) */
  output_regs = exo_new(ec_list, NULL);
  output_regs->as_list.head =
	exo_chain(output_regs->as_list.head,
		  exo_new(ec_address, (exo_integer_t)ld_brk_point));
  for (i=MD_FIRST_OUT_REG; i <= MD_LAST_OUT_REG; i++)
    {
      output_regs->as_list.head =
	exo_chain(output_regs->as_list.head, MD_IREG_TO_EXO(regs, i));
    }

  /* write the whole enchalada to output stream */
  exo = exo_new(ec_list,
		/* icnt */exo_new(ec_integer, (exo_integer_t)icnt),
		/* PC */exo_new(ec_address, (exo_integer_t)regs->PC),
#ifdef EXO_CHKSUM					 
		/* CHKSUM */exo_new(ec_integer, (exo_integer_t)running_chksum),
#endif /* EXO_CHKSUM */
		input_regs, input_mem,
		output_regs, output_mem,
		NULL);
  exo_print(exo, eio_fd);
  gzprintf(eio_fd, "\n\n");

  /* release input storage */
  exo_delete(exo);
}

/* syscall proxy handler from an EIO trace, architect registers
   and memory are assumed to be precise when this function is called,
   register and memory are updated with the results of the sustem call */
void
eio_read_trace(gzFile eio_fd,			/* EIO stream file desc */
	       counter_t icnt,			/* instruction count */
	       struct regs_t *regs,		/* registers to update */
	       mem_access_fn mem_fn,		/* generic memory accessor */
	       struct mem_t *mem,		/* memory to update */
	       md_inst_t inst)			/* system call inst */
{
  int i;
  struct exo_term_t *exo, *exo_icnt, *exo_pc;
#ifdef EXO_CHKSUM
  struct exo_term_t *exo_chksum;
#endif /* EXO_CHKSUM */
  struct exo_term_t *exo_inregs, *exo_inmem, *exo_outregs, *exo_outmem;
  struct exo_term_t *brkrec, *regrec, *memrec;

  /* exit() system calls get executed for real... */
  if (MD_EXIT_SYSCALL(regs))
    {
      sys_syscall(regs, mem_fn, mem, inst, FALSE);
      panic("returned from exit() system call");
    }

  /* else, read the external I/O (EIO) transaction */
  exo = exo_read(eio_fd);

  while (eio_is_chksum(exo)) {
	 /* there was a register checksum -- this is okay as long as we
       aren't verify register state. */
	 if (verify_register_checksum) {
		panic("unverified register checksum");
	 }
	 /* release the register checksum */
	 exo_delete(exo);
	 
	 /* grab another eio transaction */
	 exo = exo_read(eio_fd);
  }

  /* one more transaction processed */
  eio_trans_icnt = icnt;

  /* pull apart the EIO transaction (EXO format) */
  if (!exo
      || exo->ec != ec_list
      || !(exo_icnt = exo->as_list.head)
      || exo_icnt->ec != ec_integer
      || !(exo_pc = exo_icnt->next)
      || exo_pc->ec != ec_address
#ifdef EXO_CHKSUM
      || !(exo_chksum = exo_pc->next)
      || exo_chksum->ec != ec_integer
      || !(exo_inregs = exo_chksum->next)
#else /* !EXO_CHKSUM */
		|| !(exo_inregs = exo_pc->next)
#endif /* EXO_CHKSUM */
      || exo_inregs->ec != ec_list
      || !(exo_inmem = exo_inregs->next)
      || exo_inmem->ec != ec_list
      || !(exo_outregs = exo_inmem->next)
      || exo_outregs->ec != ec_list
      || !(exo_outmem = exo_outregs->next)
      || exo_outmem->ec != ec_list
      || exo_outmem->next != NULL)
    fatal("cannot read EIO transaction");

  /*
   * check the system call inputs
   */

  /* check ICNT input */
  if (icnt != (counter_t)exo_icnt->as_integer.val)
    fatal("EIO trace inconsistency: ICNT mismatch");

  /* check PC input */
  if (regs->PC != (md_addr_t)exo_pc->as_integer.val)
    fatal("EIO trace inconsistency: PC mismatch");

#ifdef EXO_CHKSUM					 
  /* check register checksum input */
  if (verify_register_checksum && 
		(running_chksum != (counter_t)exo_chksum->as_integer.val))
    fatal("EIO trace inconsistency: register checksum mismatch");
#endif /* EXO_CHKSUM */

  /* check integer register inputs */
  for (i=MD_FIRST_IN_REG, regrec=exo_inregs->as_list.head;
       i <= MD_LAST_IN_REG; i++, regrec=regrec->next)
    {
      if (!regrec || regrec->ec != ec_address)
	fatal("EIO trace inconsistency: missing input reg");

      if (MD_EXO_CMP_IREG(regrec, regs, i))
	fatal("EIO trace inconsistency: R[%d] input mismatch", i);
#ifdef VERBOSE
      fprintf(stderr, "** R[%d] checks out...\n", i);
#endif /* VERBOSE */
    }
  if (regrec != NULL)
    fatal("EIO trace inconsistency: too many input regs");

  /* check memory inputs */
  for (memrec=exo_inmem->as_list.head; memrec != NULL; memrec=memrec->next)
    {
      md_addr_t loc;
      struct exo_term_t *addr, *blob;

      /* check the mem transaction format */
      if (!memrec
	  || memrec->ec != ec_list
	  || !(addr = memrec->as_list.head)
	  || addr->ec != ec_address
	  || !(blob = addr->next)
	  || blob->ec != ec_blob
	  || blob->next != NULL)
	fatal("EIO trace inconsistency: bad memory transaction");

      for (loc=addr->as_integer.val, i=0; i < blob->as_blob.size; loc++,i++)
	{
	  unsigned char val;

	  /* was: val = MEM_READ_BYTE(loc); */
	  (*mem_fn)(mem, mc_READ, loc, &val, sizeof(unsigned char));

	  if (val != blob->as_blob.data[i])
	    fatal("EIO trace inconsistency: addr 0x%08p input mismatch", loc);

#ifdef VERBOSE
	  myprintf(stderr, "** 0x%08p checks out...\n", loc);
#endif /* VERBOSE */
	}

      /* simulate view'able I/O */
      if (MD_OUTPUT_SYSCALL(regs))
	{
	  if (sim_progfd)
	    {
	      /* redirect program output to file */
	      fwrite(blob->as_blob.data, 1, blob->as_blob.size, sim_progfd);
	    }
	  else
	    {
	      /* write the output to stdout/stderr */
	      write(MD_STREAM_FILENO(regs),
		    blob->as_blob.data, blob->as_blob.size);
	    }
	}
    }

  /*
   * write system call outputs
   */

  /* adjust breakpoint */
  brkrec = exo_outregs->as_list.head;
  if (!brkrec || brkrec->ec != ec_address)
    fatal("EIO trace inconsistency: missing memory breakpoint");
  ld_brk_point = (md_addr_t)brkrec->as_integer.val;

  /* write integer register outputs */
  for (i=MD_FIRST_OUT_REG, regrec=exo_outregs->as_list.head->next;
       i <= MD_LAST_OUT_REG; i++, regrec=regrec->next)
    {
      if (!regrec || regrec->ec != ec_address)
	fatal("EIO trace inconsistency: missing output reg");

      MD_EXO_TO_IREG(regrec, regs, i);

#ifdef VERBOSE
      fprintf(stderr, "** R[%d] written...\n", i);
#endif /* VERBOSE */
    }
  if (regrec != NULL)
    fatal("EIO trace inconsistency: too many output regs");

  /* write memory outputs */
  for (memrec=exo_outmem->as_list.head; memrec != NULL; memrec=memrec->next)
    {
      md_addr_t loc;
      struct exo_term_t *addr, *blob;

      /* check the mem transaction format */
      if (!memrec
	  || memrec->ec != ec_list
	  || !(addr = memrec->as_list.head)
	  || addr->ec != ec_address
	  || !(blob = addr->next)
	  || blob->ec != ec_blob
	  || blob->next != NULL)
	fatal("EIO trace icnonsistency: bad memory transaction");

      for (loc=addr->as_integer.val, i=0; i < blob->as_blob.size; loc++,i++)
	{
	  /* was: MEM_WRITE_BYTE(loc, blob->as_blob.data[i]); */
	  (*mem_fn)(mem, mc_WRITE,
		    loc, &blob->as_blob.data[i], sizeof(unsigned char));

#ifdef VERBOSE
	  fprintf(stderr, "** 0x%08p written...\n", loc);
#endif /* VERBOSE */
	}
    }

  /* release the EIO EXO node */
  exo_delete(exo);
}

/* fast forward EIO trace EIO_FD to the transaction just after ICNT */
void
eio_fast_forward(gzFile eio_fd, counter_t icnt)
{
  struct exo_term_t *exo, *exo_icnt;

  do
    {
      /* read the next external I/O (EIO) transaction */
      exo = exo_read(eio_fd);

      if (!exo)
	fatal("could not fast forward to EIO checkpoint");

      /* one more transaction processed */
      eio_trans_icnt = icnt;

      /* pull apart the EIO transaction (EXO format) */
      if (!exo
	  || exo->ec != ec_list
	  || !(exo_icnt = exo->as_list.head)
	  || exo_icnt->ec != ec_integer)
	fatal("cannot read EIO transaction (during fast forward)");
    }
  while ((counter_t)exo_icnt->as_integer.val != icnt);

  /* found it! */
}

/* write a register checksum into the trace */
void
eio_write_chksum(gzFile eio_fd,			/* EIO stream file desc */
					  counter_t icnt,			/* instruction count */
					  int chksum)  			/* register checksum */
{
  struct exo_term_t *exo;

  exo = exo_new(ec_list,
	  /*icnt*/exo_new(ec_integer, (exo_integer_t)sim_num_insn),	
	  /*CHKSUM*/exo_new(ec_integer, (exo_integer_t)chksum),
	  NULL);

  /* write out the chksum */
  exo_print(exo, eio_fd);
  gzprintf(eio_fd, "\n\n");

  /* release input storage */
  exo_delete(exo);
}

/* verify a register checksum in the EIO trace */
void
eio_verify_chksum(gzFile eio_fd,			/* EIO stream file desc */
						counter_t icnt,			/* instruction count */
						int chksum)  			/* register checksum */
{
  struct exo_term_t *exo;

  /* else, read the external I/O (EIO) transaction */
  exo = exo_read(eio_fd);

  if (!eio_is_chksum(exo)) 
	 panic("expected register checksum at icnt: ", icnt);

  /* check ICNT input */
  if (icnt != (counter_t)exo->as_list.head->as_integer.val)
    fatal("EIO trace inconsistency: ICNT mismatch");

  /* check the checksum input */
  if (verify_register_checksum && 
		(chksum != (counter_t)exo->as_list.head->next->as_integer.val))
		panic("register checksum does not match exo trace at icnt:", icnt);

  /* one more transaction processed */
  eio_trans_icnt = icnt;

  /* release input storage */
  exo_delete(exo);
}

/* verify that it is a register checksum just by shape */
int 
eio_is_chksum(struct exo_term_t *exo) 
{
  if (!exo
      || exo->ec != ec_list
      || !exo->as_list.head
      || exo->as_list.head->ec != ec_integer
      || !exo->as_list.head->next
      || exo->as_list.head->next->ec != ec_integer
      || exo->as_list.head->next->next != NULL) {
	 return  FALSE;
  } 
  return TRUE;
}

