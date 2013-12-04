/*
 * misc.h - miscellaneous interfaces
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
 * $Id: cacti_misc.h,v 1.1 2004/08/20 23:05:27 amir Exp $
 *
 * $Log: cacti_misc.h,v $
 * Revision 1.1  2004/08/20 23:05:27  amir
 * New cacti3 -AR
 *
 * Revision 1.1  2004/06/27 15:04:23  amir
 * Needed to allow compilation of standalone CACTI  -AR
 *
 * Revision 1.1  2003/10/24 03:11:49  vladp
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2003/09/16 17:58:14  bracy
 * staring functional simulator to do initial dise + ri diagnostics
 *
 * Revision 1.1.1.1  2003/02/21 07:50:29  vladp
 *
 *
 * Revision 1.1.1.1  2002/09/25 14:01:03  vladp
 *
 *
 * Revision 1.2  2002/08/22 03:20:58  vlad
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2002/08/01 13:30:32  vlad
 *
 *
 * Revision 1.2  2002/07/25 16:14:48  vlad
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2002/05/08 02:43:36  vlad
 *
 *
 * Revision 1.5  1998/08/27 15:45:24  taustin
 * implemented host interface description in host.h
 * added target interface support
 * implemented a more portable random() interface
 * disabled calls to sbrk() under malloc(), this breaks some
 *       malloc() implementation (e.g., newer Linux releases)
 * added myprintf() and myatoq() routines for printing and reading
 *       quadword's, respectively
 * added gzopen() and gzclose() routines for reading and writing
 *       compressed files, updated sysprobe to search for GZIP, if found
 *       support is enabled
 * moved host-dependent definitions to host.h
 *
 * Revision 1.4  1997/03/11  01:18:24  taustin
 * updated copyright
 * supported added for non-GNU C compilers
 * ANSI C compiler check added
 * long/int tweaks made for ALPHA target support
 * hacks added to make SYMCAT() portable
 *
 * Revision 1.3  1997/01/06  16:02:01  taustin
 * comments updated
 * system prototypes deleted (-Wall flag no longer a clean compile)
 *
 * Revision 1.1  1996/12/05  18:50:23  taustin
 * Initial revision
 *
 *
 */

#ifndef CACTI_MISC_H
#define CACTI_MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

/* boolean value defs */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef BOOL_DOM
#define BOOL_DOM 2
#endif 

typedef int bool_t;

/* various useful macros */
#ifndef MAX
#define MAX(a, b)    (((a) < (b)) ? (b) : (a))
#endif
#ifndef MIN
#define MIN(a, b)    (((a) < (b)) ? (a) : (b))
#endif

/* flags */
#define FCLEAR(FLAGS) (FLAGS = 0)
#define FSET(FLAGS, F) (FLAGS |= (F))
#define FUNSET(FLAGS, F) (FLAGS &= ~(F))
#define FTEST(FLAGS, F) ((FLAGS & (F)) == (F))
#define FANYTEST(FLAGS, F) ((FLAGS & (F)) != 0)
#define FEMPTY(FLAGS) (FLAGS == 0)

/* Mod optimization */
#define MOD(V, M) ((V) % (M))
#undef MOD
#define MOD(V, M) ((V) & ((M) - 1))

#define INC_MOD(V, M) (V) = MOD((V) + 1, (M))
#define DEC_MOD(V, M) (V) = MOD((V) + (M) - 1, (M))
#define INC_VAR_MOD(V, I, M) (V) = MOD((V) + (I), (M))

#define IS_POWEROFTWO(P) (((P) & (P - 1)) == 0)

/* for printing out "long long" vars */
#define LLHIGH(L)		((int)(((L)>>32) & 0xffffffff))
#define LLLOW(L)		((int)((L) & 0xffffffff))

/* size of an array, in elements */
#define N_ELT(ARR)   (sizeof(ARR)/sizeof((ARR)[0]))

/* rounding macros, assumes ALIGN is a power of two */
#define ROUND_UP(N,ALIGN)	(((N) + ((ALIGN)-1)) & ~((ALIGN)-1))
#define ROUND_DOWN(N,ALIGN)	((N) & ~((ALIGN)-1))

#define DIV_ROUND_UP(N,V) ((N) + (V) - 1)/(V)
#define IS_IEEEFP_DBL_NAN(DBL) (FALSE)

/* return log of a number to the base 2 */
unsigned int cacti_floor_log_base2(unsigned int n);
unsigned int cacti_ceil_log_base2(unsigned int n);

#endif /* CACTI_MISC_H */
