/*
 * main.c - main line routines
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
 * $Id: main.c,v 1.8 1998/08/31 17:09:49 taustin Exp taustin $
 *
 * $Log: main.c,v $
 * Revision 1.8  1998/08/31 17:09:49  taustin
 * dumbed down parts for MS VC++
 * added unistd.h include to remove warnings
 *
 * Revision 1.7  1998/08/27 08:39:28  taustin
 * added support for MS VC++ compilation
 * implemented host interface description in host.h
 * added target interface support
 * added simulator and program output redirection (via "-redir:sim"
 *       and "redir:prog" options, respectively)
 * added "-nice" option (available on all simulator) that resets
 *       simulator scheduling priority to specified level
 * added checkpoint support
 * implemented a more portable random() interface
 * all simulators now emit command line used to invoke them
 *
 * Revision 1.6  1997/04/16  22:09:20  taustin
 * added standalone loader support
 *
 * Revision 1.5  1997/03/11  01:13:36  taustin
 * updated copyright
 * random number generator seed option added
 * versioning format simplified (X.Y)
 * fast terminate (-q) option added
 * RUNNING flag now helps stats routine determine when to spew stats
 *
 * Revision 1.3  1996/12/27  15:52:20  taustin
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
#include <string.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <sys/time.h>
#endif
#ifdef BFD_LOADER
#include <bfd.h>
#endif /* BFD_LOADER */

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "endian.h"
#include "version.h"
#include "options.h"
#include "stats.h"
#include "memory.h"
#include "loader.h"
#include "sim.h"

/* stats signal handler */
static void
signal_sim_stats(int sigtype)
{
  sim_dump_stats = TRUE;
}

/* exit signal handler */
static void
signal_exit_now(int sigtype)
{
  sim_exit_now = TRUE;
}

#if 0 /* not portable... :-( */
/* total simulator (data) memory usage */
unsigned int sim_mem_usage = 0;
#endif

/* execution start/end times */
time_t sim_start_time;
time_t sim_end_time;
int sim_elapsed_time;

/* byte/word swapping required to execute target executable on this host */
int sim_swap_bytes;
int sim_swap_words;

/* exit when this becomes non-zero */
int sim_exit_now = FALSE;

/* longjmp here when simulation is completed */
jmp_buf sim_exit_buf;

/* set to non-zero when simulator should dump statistics */
int sim_dump_stats = FALSE;

/* options database */
struct opt_odb_t *sim_odb;

/* stats database */
struct stat_sdb_t *sim_sdb;

/* EIO interfaces */
char *sim_eio_fname = NULL;
char *sim_chkpt_fname = NULL;
FILE *sim_eio_fd = NULL;

/* track first argument orphan, this is the program to execute */
static int exec_index = -1;

/* random number generator seed */
static int rand_seed;

FILE *sim_progfd;

unsigned long long insn_progress_update;
counter_t insn_progress;

unsigned long long insn_limit;
char *insn_dumpfile;
unsigned long long insn_dumpbegin;
unsigned long long insn_dumpend;
FILE *fdump;

char *insn_sample_str;
unsigned long long insn_sample[sample_NUM];

char *insn_sample_first_str;
unsigned long long insn_sample_first[sample_NUM];

enum sample_mode_t sample_mode;

static void
insn_reg_options(struct opt_odb_t * odb)
{
  opt_reg_ulonglong(odb, "-insn:limit",
		    "instruction limit", &insn_limit,
		    /* default */0, /* print */TRUE, /* format */NULL);
  opt_reg_ulonglong(odb, "-insn:progress",
		    "instruction progress update interval", &insn_progress_update,
		    /* default */100*1000*1000, /* print */TRUE, /* format */NULL);
  opt_reg_string(odb, "-insn:sample",
                 "sampling parameters i.e. {no|<off>:<warm>:<on>}",
		 &insn_sample_str, "no", /* print */TRUE, /* format */NULL);
  opt_reg_string(odb, "-insn:sample:first",
                 "first sample parameters i.e. {no|<off>:<warm>:<on>}",
		 &insn_sample_first_str, "no", /* print */TRUE, /* format */NULL);
  opt_reg_string(odb, "-insn:dumpfile",
                 "rundump file {<filename>|none}",
                 &insn_dumpfile, "none", /* print */TRUE, NULL);
  opt_reg_ulonglong(odb, "-insn:dumpbegin",
	      "rundump begin insn", &insn_dumpbegin,
	      /* default */0, /* print */TRUE, /* format */NULL);
  opt_reg_ulonglong(odb, "-insn:dumpend",
		    "rundump end insn", &insn_dumpend,
		    /* default */0, /* print */TRUE, /* format */NULL);
}

static void
insn_check_options(void)
{
  /* Start this guy */
  insn_progress = insn_progress_update;

  if (!mystricmp(insn_dumpfile, "none"))
    fdump = NULL;
  else
    fdump = fopen(insn_dumpfile, "w+");

  insn_sample[sample_ON] = insn_sample[sample_OFF] = insn_sample[sample_WARM] = 0;
  if (mystricmp(insn_sample_str, "no"))
    {
      if (sscanf(insn_sample_str, "%llu:%llu:%llu",
		 &insn_sample[sample_OFF], &insn_sample[sample_WARM], &insn_sample[sample_ON]) != 3)
	fatal("unrecognized sampling format '%s'", insn_sample);
    }

  insn_sample_first[sample_ON] = insn_sample_first[sample_OFF] = insn_sample_first[sample_WARM] = 0;
  if (mystricmp(insn_sample_first_str, "no"))
    {
      if (sscanf(insn_sample_first_str, "%llu:%llu:%llu",
		 &insn_sample_first[sample_OFF], &insn_sample_first[sample_WARM], &insn_sample_first[sample_ON]) != 3)
	fatal("unrecognized sampling format '%s'", insn_sample_first);
    }
}

static int
orphan_fn(int i, int argc, char **argv)
{
  exec_index = i;
  return /* done */FALSE;
}

static void
banner(FILE *fd, int argc, char **argv)
{
  char *s;

  fprintf(fd,
	  "%s: SimpleScalar/%s Tool Set version %d.%d of %s.\n"
	  "Copyright (c) 1994-1998 by Todd M. Austin.  All Rights Reserved.\n"
	  "\n",
	  ((s = strrchr(argv[0], '/')) ? s+1 : argv[0]),
	  VER_TARGET, VER_MAJOR, VER_MINOR, VER_UPDATE);
}

static void
usage(FILE *fd, int argc, char **argv)
{
  fprintf(fd, "Usage: %s {-options} executable {arguments}\n", argv[0]);
  opt_print_help(sim_odb, fd);
}

static int running = FALSE;

/* print all simulator stats */
void
sim_print_stats(FILE *fd)		/* output stream */
{
  char *s;
  time_t sim_current_time;

#if 0 /* not portable... :-( */
  extern char etext, *sbrk(int);
#endif

  if (!running)
    return;

  if (fdump)
    fclose(fdump);

  /* get stats time */
  sim_end_time = time((time_t *)NULL);
  sim_elapsed_time = MAX(sim_end_time - sim_start_time, 1);

#if 0 /* not portable... :-( */
  /* compute simulator memory usage */
  sim_mem_usage = (sbrk(0) - &etext) / 1024;
#endif

  /* print simulation stats */
  sim_current_time = time((time_t *)NULL);
  s = ctime(&sim_current_time);
  if (s[strlen(s)-1] == '\n')
    s[strlen(s)-1] = '\0';

  fprintf(fd, "\nsim: ** simulation statistics @ %s **\n", s);

  sim_aux_stats(fd);
  fprintf(fd, "\n");
}

/* print stats, uninitialize simulator components, and exit w/ exitcode */
static void
exit_now(int exit_code)
{
  /* print simulation stats */
  sim_print_stats(stderr);

  /* un-initialize the simulator */
  sim_uninit();

  /* all done! */
  exit(exit_code);
}


void
sim_main(void)
{
  sim_start();

  if (insn_sample_first[sample_OFF] && !sim_sample_off(insn_sample_first[sample_OFF]))
    return;

  if (insn_sample_first[sample_WARM] && !sim_sample_warmup(insn_sample_first[sample_WARM]))
    return;

  if (insn_sample_first[sample_ON] && !sim_sample_on(insn_sample_first[sample_ON]))
    return;

  if (insn_sample[sample_OFF] || insn_sample[sample_WARM] || insn_sample[sample_ON])
    {
      for (;;)
        {
          if (insn_sample[sample_OFF] && !sim_sample_off(insn_sample[sample_OFF]))
            return;

          if (insn_sample[sample_WARM] && !sim_sample_warmup(insn_sample[sample_WARM]))
            return;

          if (insn_sample[sample_ON] && !sim_sample_on(insn_sample[sample_ON]))
            return;
        }
    }

  /* simulate whatever is left */
  sim_sample_on(0);
}



int
main(int argc, char **argv, char **envp)
{
  char *s;
  int i, exit_code;

#ifndef _MSC_VER
  /* catch SIGUSR1 and dump intermediate stats */
  signal(SIGUSR1, signal_sim_stats);

  /* catch SIGUSR2 and dump final stats and exit */
  signal(SIGUSR2, signal_exit_now);
#endif /* _MSC_VER */

  /* register an error handler */
  fatal_hook(sim_print_stats);

  /* set up a non-local exit point */
  if ((exit_code = setjmp(sim_exit_buf)) != 0)
    {
      /* special handling as longjmp cannot pass 0 */
      exit_now(exit_code-1);
    }

  /* register global options */
  sim_odb = opt_new(orphan_fn);
  opt_reg_flag(sim_odb, "-v", "verbose operation",
	       &verbose, /* default */FALSE, /* !print */FALSE, NULL);
  opt_reg_int(sim_odb, "-seed",
	      "random number generator seed (0 for timer seed)",
	      &rand_seed, /* default */1, /* print */TRUE, NULL);
  opt_reg_string(sim_odb, "-chkpt", "restore EIO trace execution from <fname>",
		 &sim_chkpt_fname, /* default */NULL, /* !print */FALSE, NULL);

  /* register instruction execution options */
  insn_reg_options(sim_odb);

  /* register all simulator-specific options */
  sim_reg_options(sim_odb);

  /* parse simulator options */
  exec_index = -1;
  opt_process_options(sim_odb, argc, argv);

  /* need at least two argv values to run */
  if (argc < 2)
    {
      banner(stderr, argc, argv);
      usage(stderr, argc, argv);
      exit(1);
    }

  /* opening banner */
  banner(stderr, argc, argv);

  /* seed the random number generator */
  if (rand_seed == 0)
    {
      /* seed with the timer value, true random */
      mysrand(time((time_t *)NULL));
    }
  else
    {
      /* seed with default or user-specified random number generator seed */
      mysrand(rand_seed);
    }

  /* exec_index is set in orphan_fn() */
  if (exec_index == -1)
    {
      /* executable was not found */
      fprintf(stderr, "error: no executable specified\n");
      usage(stderr, argc, argv);
      exit(1);
    }
  /* else, exec_index points to simulated program arguments */

  /* initialize the instruction decoder */
  md_init_decoder();

  /* need options */
  insn_check_options();

  /* check simulator-specific options */
  sim_check_options();

  /* default architected value... */
  sim_num_insn = 0;

#ifdef BFD_LOADER
  /* initialize the bfd library */
  bfd_init();
#endif /* BFD_LOADER */

  /* initialize all simulation modules */
  sim_init();

  /* initialize architected state */
  sim_load_prog(argv[exec_index], argc-exec_index, argv+exec_index, envp);

  /* register all simulator stats */

  /* record start of execution time, used in rate stats */
  sim_start_time = time((time_t *)NULL);

  /* emit the command line for later reuse */
  fprintf(stderr, "sim: command line: ");
  for (i=0; i < argc; i++)
    fprintf(stderr, "%s ", argv[i]);
  fprintf(stderr, "\n");

  /* output simulation conditions */
  s = ctime(&sim_start_time);
  if (s[strlen(s)-1] == '\n')
    s[strlen(s)-1] = '\0';
  fprintf(stderr, "\nsim: simulation started @ %s, options follow:\n", s);
  opt_print_options(sim_odb, stderr, /* short */TRUE, /* notes */TRUE);
  sim_aux_config(stderr);
  fprintf(stderr, "\n");

  /* omit option dump time from rate stats */
  sim_start_time = time((time_t *)NULL);

  running = TRUE;
  sim_main();

  /* simulation finished early */
  exit_now(0);

  return 0;
}
