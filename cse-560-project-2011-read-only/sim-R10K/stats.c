/* external definitions */
#include <stdio.h>
#include <stdlib.h>

#include "host.h"
#include "machine.h"
#include "misc.h"
/* interface definitions */
#include "stats.h"

void 
print_counter(FILE *stream, 
	      const char *name, 
	      counter_t counter, 
	      const char *desc)
{
  fprintf(stream, "%-30s", name);
  myfprintf(stream, "%12lu", counter);
  fprintf(stream, " # %s\n", desc);
}

void 
print_int(FILE *stream, 
	  const char *name, 
	  int i, 
	  const char *desc)
{
  fprintf(stream, "%-30s%10d # %s\n", name, i, desc);
}

void
print_rate(FILE *stream, 
	   const char *name,
	   double rate,
	   const char *desc)
{
  fprintf(stream, "%-30s%10.4f # %s\n", name, rate, desc);
}

void
print_addr(FILE *stream, 
	   const char *name,
	   md_addr_t addr,
	   const char *desc)
{
  fprintf(stream, "%-30s", name);
  myfprintf(stream, "%10p", addr);
  fprintf(stream, " # %s\n", desc);
}

