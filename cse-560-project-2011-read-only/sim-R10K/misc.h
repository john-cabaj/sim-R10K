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
 * $Id: misc.h,v 1.5 1998/08/27 15:45:24 taustin Exp taustin $
 *
 * $Log: misc.h,v $
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

#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include "/filespace/people/j/jtabor/ECE752/sim-R10K-lib/include/zlib.h"

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

/* verbose output flag */
extern int verbose;

#ifdef DEBUG
/* active debug flag */
extern int debugging;
#endif /* DEBUG */

/* register a function to be called when an error is detected */
void
fatal_hook(void (*hook_fn)(FILE *stream));	/* fatal hook function */

#ifdef __GNUC__
/* declare a fatal run-time error, calls fatal hook function */
#define fatal(fmt, args...)	\
  _fatal(__FILE__, __FUNCTION__, __LINE__, fmt, ## args)

void
_fatal(char *file, const char *func, int line, char *fmt, ...)
__attribute__ ((noreturn));
#else /* !__GNUC__ */
void
fatal(char *fmt, ...);
#endif /* !__GNUC__ */

#ifdef __GNUC__
/* declare a panic situation, dumps core */
#define panic(fmt, args...)	\
  _panic(__FILE__, __FUNCTION__, __LINE__, fmt, ## args)

void
_panic(char *file, const char *func, int line, char *fmt, ...)
__attribute__ ((noreturn));
#else /* !__GNUC__ */
void
panic(char *fmt, ...);
#endif /* !__GNUC__ */

#ifdef __GNUC__
/* declare a warning */
#define warn(fmt, args...)	\
  _warn(__FILE__, __FUNCTION__, __LINE__, fmt, ## args)

void
_warn(char *file, const char *func, int line, char *fmt, ...);
#else /* !__GNUC__ */
void
warn(char *fmt, ...);
#endif /* !__GNUC__ */

#ifdef __GNUC__
/* print general information */
#define info(fmt, args...)	\
  _info(__FILE__, __FUNCTION__, __LINE__, fmt, ## args)

void
_info(char *file, char *func, int line, char *fmt, ...);
#else /* !__GNUC__ */
void
info(char *fmt, ...);
#endif /* !__GNUC__ */

#ifdef DEBUG

#ifdef __GNUC__
/* print a debugging message */
#define debug(fmt, args...)	\
    do {                        \
        if (debugging)         	\
            _debug(__FILE__, __FUNCTION__, __LINE__, fmt, ## args); \
    } while(0)

void
_debug(char *file, const char *func, int line, char *fmt, ...);
#else /* !__GNUC__ */
void
debug(char *fmt, ...);
#endif /* !__GNUC__ */

#else /* !DEBUG */

#ifdef __GNUC__
#define debug(fmt, args...)
#else /* !__GNUC__ */
/* the optimizer should eliminate this call! */
static void debug(char *fmt, ...) {}
#endif /* !__GNUC__ */

#endif /* !DEBUG */

/* seed the random number generator */
void
mysrand(unsigned int seed);	/* random number generator seed */

/* get a random number */
int myrand(void);		/* returns random number */

/* copy a string to a new storage allocation (NOTE: many machines are missing
   this trivial function, so I funcdup() it here...) */
char *				/* duplicated string */
mystrdup(const char *s);	/* string to duplicate to heap storage */

/* find the last occurrence of a character in a string */
char *
mystrrchr(char *s, char c);

/* case insensitive string compare (NOTE: many machines are missing this
   trivial function, so I funcdup() it here...) */
int				/* compare result, see strcmp() */
mystricmp(const char *s1, const char *s2);	/* strings to compare, case insensitive */

/* allocate some core, this memory has overhead no larger than a page
   in size and it cannot be released. the storage is returned cleared */
void *getcore(int nbytes);

/* return log of a number to the base 2 */
unsigned int log_base2(unsigned int n);
unsigned int ceil_log_base2(unsigned int n);
unsigned int floor_log_base2(unsigned int n);

/* return string describing elapsed time, passed in SEC in seconds */
char *elapsed_time(long sec);

/* assume bit positions numbered 31 to 0 (31 high order bit), extract num bits
   from word starting at position pos (with pos as the high order bit of those
   to be extracted), result is right justified and zero filled to high order
   bit, for example, extractl(word, 6, 3) w/ 8 bit word = 01101011 returns
   00000110 */
unsigned int
extractl(int word,		/* the word from which to extract */
         int pos,		/* bit positions 31 to 0 */
         int num);		/* number of bits to extract */

#if defined(sparc) && !defined(__svr4__)
#define strtoul strtol
#endif

/* portable 64-bit I/O package */

/* portable vsprintf with quadword support, returns end pointer */
char *myvsprintf(char *obuf, char *format, va_list v);

/* portable sprintf with quadword support, returns end pointer */
char *mysprintf(char *obuf, char *format, ...);

/* portable vfprintf with quadword support, returns end pointer */
void myvfprintf(FILE *stream, char *format, va_list v);

/* portable fprintf with quadword support, returns end pointer */
void myfprintf(FILE *stream, char *format, ...);

/* portable fprintf with quadword support, returns end pointer */
void mygzprintf(gzFile stream, char *format, ...);

#ifdef HOST_HAS_QUAD

/* convert a string to a signed result */
squad_t myatosq(char *nptr, char **endp, int base);

/* convert a string to a unsigned result */
quad_t myatoq(char *nptr, char **endp, int base);

#endif /* HOST_HAS_QUAD */

void *mycalloc(size_t nmemb, size_t size);

#if 0
/* same semantics as fopen() except that filenames ending with a ".gz" or ".Z"
   will be automagically get compressed */
FILE *gzopen(char *fname, char *type);

/* close compressed stream */
void gzclose(FILE *fd);
#endif /* 0 */

#endif /* MISC_H */
