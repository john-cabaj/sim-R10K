#ifndef ADISAMBIG_H
#define ADISAMBIG_H

enum adisambig_t
{
  adisambig_PERFECT,
  adisambig_CONSERVATIVE,
  adisambig_OPPORTUNISTIC,
  adisambig_CHT,
  adisambig_NUM
};

struct adisambig_opt_t
{
  char *opt;
  enum adisambig_t strategy;

  unsigned int n_ways;
  unsigned int n_sets;
};

struct cht_t;

void 
adisambig_check_options(struct adisambig_opt_t *opt);

struct cht_t *
cht_create(struct adisambig_opt_t *opt);

unsigned int
cht_lookup(struct cht_t *cht,
	   md_addr_t PC);

void
cht_enter(struct cht_t *cht,
	  md_addr_t PC,
	  unsigned int store_dist);

#endif /* ADISAMBIG_H */
