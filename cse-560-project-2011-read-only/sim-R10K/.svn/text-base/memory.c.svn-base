/*
 * memory.c - flat memory space routines
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
 * $Id: memory.c,v 1.6 1998/08/27 15:38:28 taustin Exp taustin $
 *
 * $Log: memory.c,v $
 * Revision 1.6  1998/08/27 15:38:28  taustin
 * implemented host interface description in host.h
 * added target interface support
 * memory module updated to support 64/32 bit address spaces on 64/32
 *       bit machines, now implemented with a dynamically optimized hashed
 *       page table
 * added support for quadword's
 * added fault support
 *
 * Revision 1.5  1997/03/11  01:15:25  taustin
 * updated copyright
 * mem_valid() added, indicates if an address is bogus, used by DLite!
 * long/int tweaks made for ALPHA target support
 *
 * Revision 1.4  1997/01/06  16:00:51  taustin
 * stat_reg calls now do not initialize stat variable values
 *
 * Revision 1.3  1996/12/27  15:52:46  taustin
 * updated comments
 * integrated support for options and stats packages
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
#include "options.h"
#include "stats.h"
/* interface definitions */
#include "memory.h"
/* implementation definitions */
#include "loader.h"

static counter_t sim_num_unaligned_accs = 0;

/* create a flat memory space */
struct mem_t *
mem_create(char *name)			/* name of the memory space */
{
  struct mem_t *mem;

  mem = calloc(1, sizeof(struct mem_t));
  if (!mem)
    fatal("out of virtual memory");

  mem->name = mystrdup(name);
  return mem;
}

/* translate address ADDR in memory space MEM, returns pointer to host page */
byte_t *
mem_translate(struct mem_t *mem,	/* memory space to access */
	      md_addr_t addr)		/* virtual address to translate */
{
  struct mem_pte_t *pte, *prev;

  /* got here via a first level miss in the page tables */
  mem->ptab_misses++; mem->ptab_accesses++;

  /* locate accessed PTE */
  for (prev=NULL, pte=mem->ptab[MEM_PTAB_SET(addr)];
       pte != NULL;
       prev=pte, pte=pte->next)
    {
      if (pte->tag == MEM_PTAB_TAG(addr))
	{
	  /* move this PTE to head of the bucket list */
	  if (prev)
	    {
	      prev->next = pte->next;
	      pte->next = mem->ptab[MEM_PTAB_SET(addr)];
	      mem->ptab[MEM_PTAB_SET(addr)] = pte;
	    }
	  return pte->page;
	}
    }

  /* no translation found, return NULL */
  return NULL;
}

bool_t
mem_arch(struct mem_t *mem,	/* memory space to access */
	 md_addr_t addr)	/* virtual address to translate */
{
  struct mem_pte_t *pte;

  /* locate accessed PTE */
  for (pte = mem->ptab[MEM_PTAB_SET(addr)]; pte; pte = pte->next)
    if (pte->tag == MEM_PTAB_TAG(addr))
      return TRUE;

  return FALSE;
}

/* allocate a memory page */
byte_t *
mem_newpage(struct mem_t *mem,		/* memory space to allocate in */
	    md_addr_t addr)		/* virtual address to allocate */
{
  byte_t *page;
  struct mem_pte_t *pte;

  /* see misc.c for details on the getcore() function */
  page = getcore(MD_PAGE_SIZE);
  if (!page)
    fatal("out of virtual memory");

  /* generate a new PTE */
  pte = calloc(1, sizeof(struct mem_pte_t));
  if (!pte)
    fatal("out of virtual memory");
  pte->tag = MEM_PTAB_TAG(addr);
  pte->page = page;

  /* insert PTE into inverted hash table */
  pte->next = mem->ptab[MEM_PTAB_SET(addr)];
  mem->ptab[MEM_PTAB_SET(addr)] = pte;

  /* one more page allocated */
  mem->page_count++;

  return page;
}

/* generic memory access function, it's safe because alignments and permissions
   are checked, handles any natural transfer sizes; note, faults out if nbytes
   is not a power-of-two or larger then MD_PAGE_SIZE */
enum md_fault_t
mem_access(struct mem_t *mem,		/* memory space to access */
	   enum mem_cmd_t cmd,		/* Read (from sim mem) or Write */
	   md_addr_t addr,		/* target address to access */
	   void *vp,			/* host memory address to access */
	   int nbytes)			/* number of bytes to access */
{
  byte_t *page, *paddr, *p = vp;

  /* check alignments */
  if (!IS_POWEROFTWO(nbytes) || nbytes > MD_PAGE_SIZE)
    return md_fault_access;

  if ((addr & (nbytes-1)) != 0)
    //#define ALIGNMENT_FAULTS
#ifdef ALIGNMENT_FAULTS
    return md_fault_alignment;
#else /* !ALIGNMENT_FAULTS */
    sim_num_unaligned_accs++;
#endif /* ALIGNMENT_FAULTS */

    /* get the page, new page if there isn't an old one */
    page = MEM_PAGE(mem, addr);
    if (!page && cmd == mc_WRITE)
      page = mem_newpage(mem, addr);

    paddr = page + MEM_OFFSET(addr);

    /* perform the copy */
    switch (nbytes)
      {
      case 1:
	if (cmd == mc_READ)
	  *((byte_t *)p) = page ? *((byte_t *)paddr) : 0;
	else
	  *((byte_t *)paddr) = *((byte_t*)p);
	break;
	
      case 2:
	if (cmd == mc_READ)
	  *((half_t *)p) = page ? *((half_t *)paddr) : 0;
	else
	  *((half_t *)paddr) = *((half_t *)p);
	break;
	
      case 4:
	if (cmd == mc_READ)
	  *((word_t *)p) = page ? *((word_t *)paddr) : 0;
	else
	  *((word_t *)paddr) = *((word_t *)p);
	break;
	
#ifdef HOST_HAS_QUAD
      case 8:
	if (cmd == mc_READ)
	  *((quad_t *)p) = page ? *((quad_t *)paddr) : 0;
	else
	  *((quad_t *)paddr) = *((quad_t *)p);
	break;
#endif /* HOST_HAS_QUAD */
	
      default:
	{
	  /* nbytes >= 8/16 and power of two */
	  unsigned words = nbytes >> 2;
	  
	  while (words-- > 0)
	    {
	      if (cmd == mc_READ)
		*((word_t *)p) = page ? *((word_t *)paddr) : 0;
	      else
		*((word_t *)paddr) = *((word_t *)p);

	      p += sizeof(word_t);
	      paddr += sizeof(word_t);
	    }
	}
	break;
      }
    
    /* no fault... */
    return md_fault_none;
}

/* register memory system-specific statistics */
void
mem_print_stats(struct mem_t *mem,	/* memory space to declare */
		FILE *stream)	/* stats data base */
{
  char buf[512];

  sprintf(buf, "%s.page_count", mem->name);
  print_counter(stream, buf, mem->page_count, "memory pages allocated");
  sprintf(buf, "%s.page_mem", mem->name);
  print_counter(stream, buf, mem->page_count * (MD_PAGE_SIZE/1024), "size of memory pages allocated (KB)");
  sprintf(buf, "%s.ptab_misses", mem->name);
  print_counter(stream, buf, mem->ptab_misses, "first level page table misses");
  sprintf(buf, "%s.ptab_accesses", mem->name);
  print_counter(stream, buf, mem->ptab_accesses, "page table accesses");
  sprintf(buf, "%s.ptab_miss_rate", mem->name);
  print_rate(stream, buf, (double)mem->ptab_misses/mem->ptab_accesses, "first level page table miss rate");

  print_counter(stream, "sim_num_unaligned_accs", sim_num_unaligned_accs, "unaligned accesses");
}

/* initialize memory system, call before loader.c */
void
mem_init(struct mem_t *mem)	/* memory space to initialize */
{
  int i;

  /* initialize the first level page table to all empty */
  for (i=0; i < MEM_PTAB_SIZE; i++)
    mem->ptab[i] = NULL;

  mem->page_count = 0;
  mem->ptab_misses = 0;
  mem->ptab_accesses = 0;
}

/* dump a block of memory, returns any faults encountered */
enum md_fault_t
mem_dump(struct mem_t *mem,		/* memory space to display */
	 md_addr_t addr,		/* target address to dump */
	 int len,			/* number bytes to dump */
	 FILE *stream)			/* output stream */
{
  int data;
  enum md_fault_t fault;

  if (!stream)
    stream = stderr;

  addr &= ~sizeof(word_t);
  len = (len + (sizeof(word_t) - 1)) & ~sizeof(word_t);
  while (len-- > 0)
    {
      fault = mem_access(mem, mc_READ, addr, &data, sizeof(word_t));
      if (fault != md_fault_none)
	return fault;

      myfprintf(stream, "0x%08p: %08x\n", addr, data);
      addr += sizeof(word_t);
    }

  /* no faults... */
  return md_fault_none;
}

/* copy a '\0' terminated string to/from simulated memory space, returns
   the number of bytes copied, returns any fault encountered */
enum md_fault_t
mem_strcpy(mem_access_fn mem_fn,	/* user-specified memory accessor */
	   struct mem_t *mem,		/* memory space to access */
	   enum mem_cmd_t cmd,		/* Read (from sim mem) or Write */
	   md_addr_t addr,		/* target address to access */
	   char *s)
{
  int n = 0;
  char c;
  enum md_fault_t fault;

  switch (cmd)
    {
    case mc_READ:
      /* copy until string terminator ('\0') is encountered */
      do {
	fault = mem_fn(mem, mc_READ, addr++, &c, 1);
	if (fault != md_fault_none)
	  return fault;
	*s++ = c;
	n++;
      } while (c);
      break;

    case mc_WRITE:
      /* copy until string terminator ('\0') is encountered */
      do {
	c = *s++;
	fault = mem_fn(mem, mc_WRITE, addr++, &c, 1);
	if (fault != md_fault_none)
	  return fault;
	n++;
      } while (c);
      break;

    default:
      return md_fault_internal;
  }

  /* no faults... */
  return md_fault_none;
}

/* copy NBYTES to/from simulated memory space, returns any faults */
enum md_fault_t
mem_bcopy(mem_access_fn mem_fn,		/* user-specified memory accessor */
	  struct mem_t *mem,		/* memory space to access */
	  enum mem_cmd_t cmd,		/* Read (from sim mem) or Write */
	  md_addr_t addr,		/* target address to access */
	  void *vp,			/* host memory address to access */
	  int nbytes)
{
  byte_t *p = vp;
  enum md_fault_t fault;

  /* copy NBYTES bytes to/from simulator memory */
  while (nbytes-- > 0)
    {
      fault = mem_fn(mem, cmd, addr++, p++, 1);
      if (fault != md_fault_none)
	return fault;
    }

  /* no faults... */
  return md_fault_none;
}

/* copy NBYTES to/from simulated memory space, NBYTES must be a multiple
   of 4 bytes, this function is faster than mem_bcopy(), returns any
   faults encountered */
enum md_fault_t
mem_bcopy4(mem_access_fn mem_fn,	/* user-specified memory accessor */
	   struct mem_t *mem,		/* memory space to access */
	   enum mem_cmd_t cmd,		/* Read (from sim mem) or Write */
	   md_addr_t addr,		/* target address to access */
	   void *vp,			/* host memory address to access */
	   int nbytes)
{
  byte_t *p = vp;
  int words = nbytes >> 2;		/* note: nbytes % 2 == 0 is assumed */
  enum md_fault_t fault;

  while (words-- > 0)
    {
      fault = mem_fn(mem, cmd, addr, p, sizeof(word_t));
      if (fault != md_fault_none)
	return fault;

      addr += sizeof(word_t);
      p += sizeof(word_t);
    }

  /* no faults... */
  return md_fault_none;
}

/* zero out NBYTES of simulated memory, returns any faults encountered */
enum md_fault_t
mem_bzero(mem_access_fn mem_fn,		/* user-specified memory accessor */
	  struct mem_t *mem,		/* memory space to access */
	  md_addr_t addr,		/* target address to access */
	  int nbytes)
{
  byte_t c = 0;
  enum md_fault_t fault;

  /* zero out NBYTES of simulator memory */
  while (nbytes-- > 0)
    {
      fault = mem_fn(mem, mc_WRITE, addr++, &c, 1);
      if (fault != md_fault_none)
	return fault;
    }

  /* no faults... */
  return md_fault_none;
}

bool_t
valid_text_address(struct mem_t *mem,
		   md_addr_t addr)
{
  return (addr >= ld_text_base && 
	  addr < ld_text_base + ld_text_size &&
	  ((addr & (sizeof(md_inst_t) - 1)) == 0));
}

bool_t
valid_data_address(struct mem_t *mem,
		   md_addr_t addr)
{
  return ((addr >= ld_stack_min && addr < ld_stack_base) || 
	  (addr >= ld_data_base));
}

bool_t
valid_word_address(struct mem_t *mem,
		   md_addr_t addr)
{
  return ((addr & MD_DATAPATH_MASK) == 0);
}

bool_t 
valid_arch_address(struct mem_t *mem,
		   md_addr_t addr)
{
  return mem_arch(mem, addr);
}

bool_t 
valid_data_word_address(struct mem_t *mem,
			md_addr_t addr)
{
  return (valid_data_address(mem, addr) && 
	  valid_word_address(mem, addr));
}

int 
valid_data_address_arch(struct mem_t *mem, 
			md_addr_t addr)
{
  return (/* valid address */
	  valid_data_address(mem, addr) && 
	  valid_arch_address(mem, addr));
}

int 
valid_data_word_address_arch(struct mem_t *mem,
			     md_addr_t addr)
{
  return (/* valid data address */
	  valid_data_address(mem, addr) &&
	  valid_word_address(mem, addr) &&
	  valid_arch_address(mem, addr));
}

void
init_hm_stats(struct hhm_stats_t *hhm)
{
  memset((char *)hhm, 0, sizeof(struct hhm_stats_t));
}

#define HHMSUM(CMD) \
   (hhm->hhm[CMD].hit + hhm->hhm[CMD].hiss + hhm->hhm[CMD].miss)
#define CMDSUM(HHM) \
   (hhm->hhm[mc_READ].HHM + hhm->hhm[mc_WRITE].HHM)
void
print_hm_stats(struct hhm_stats_t *hhm, 
	       const char *name, 
	       FILE *stream)
{
  counter_t reads = HHMSUM(mc_READ);
  counter_t writes = HHMSUM(mc_WRITE);
  counter_t hits = CMDSUM(hit);
  counter_t hisses = CMDSUM(hiss);
  counter_t misses = CMDSUM(miss);
  counter_t accs = misses + hisses + hits;

  myfprintf(stream, "%s:accs = %12lu", name, accs);
  if (accs != 0)
    {
      myfprintf(stream, "\tmiss = %12lu", misses); 
      fprintf(stream, " %5.4f", (double)misses/accs);
      myfprintf(stream, "\thiss = %12lu", hisses);
      fprintf(stream, " %5.4f", (double)hisses/accs);
    }
  fprintf(stream, "\n");

  myfprintf(stream, "%s:reads = %12lu", name, reads);
  if (reads != 0)
    {
      myfprintf(stream, "\tmiss = %12lu", hhm->hhm[mc_READ].miss);
      fprintf(stream, " %5.4f", (double)(hhm->hhm[mc_READ].miss)/reads);
      myfprintf(stream, "\thiss = %12lu", hhm->hhm[mc_READ].hiss);
      fprintf(stream, " %5.4f", (double)(hhm->hhm[mc_READ].hiss)/reads);
    }
  fprintf(stream, "\n");

  myfprintf(stream, "%s:writes = %12lu", name, writes);
  if (writes != 0)
    {
      myfprintf(stream, "\tmiss = %12lu", hhm->hhm[mc_WRITE].miss);
      fprintf(stream, " %5.4f", (double)(hhm->hhm[mc_WRITE].miss)/writes);
      myfprintf(stream, "\thiss = %12lu", hhm->hhm[mc_WRITE].hiss);
      fprintf(stream, " %5.4f", (double)(hhm->hhm[mc_WRITE].hiss)/writes);
    }
  fprintf(stream, "\n");
}
