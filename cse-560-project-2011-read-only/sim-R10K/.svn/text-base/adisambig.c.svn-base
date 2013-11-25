#include <stdio.h>
#include <stdlib.h>

#include "machine.h"
#include "options.h"
#include "adisambig.h"

void
adisambig_check_options(struct adisambig_opt_t *opt)
{
  if (!mystricmp(opt->opt, "perfect")) opt->strategy = adisambig_PERFECT;
  else if (!mystricmp(opt->opt, "conservative")) opt->strategy = adisambig_CONSERVATIVE;
  else if (!mystricmp(opt->opt, "opportunistic")) opt->strategy = adisambig_OPPORTUNISTIC;
  else if (sscanf(opt->opt, "%d:%d", &opt->n_sets, &opt->n_ways) == 2)
    {
      opt->strategy = adisambig_CHT;
      if (opt->n_sets < 1 || opt->n_ways < 1)
	fatal("cht sets and ways must be positive!");
    }
  else fatal("unknown memory disambiguation model: '%s'", opt);
}

struct cht_ent_t 
{
  struct cht_ent_t *hnext, *hprev;
  struct cht_ent_t *next, *prev;
  struct cht_ent_t *fnext;
  
  md_addr_t PC;
  unsigned int store_dist;
};

struct cht_set_t
{
  int hsize, occ;
  
  struct cht_ent_t **ht, **lht;
  struct cht_ent_t *head, *tail;
  
  struct cht_ent_t *flist;
};

struct cht_t
{
  struct adisambig_opt_t *opt;
  
  int hsize;
  int h1mod;
  int h2shift, h2mod;
  
  /* itable sets */
  struct cht_set_t *sets;
};

struct cht_t *
cht_create(struct adisambig_opt_t *opt)
{
  int i;
  struct cht_t *cht;
  
  cht = (struct cht_t *)calloc(1, sizeof(struct cht_t));
  cht->opt = opt;
  
  cht->hsize = cht->opt->n_ways * 4;
  
  cht->sets = (struct cht_set_t *)calloc(cht->opt->n_sets, 
					 sizeof(struct cht_set_t));
  
  cht->h1mod = cht->opt->n_sets;
  cht->h2shift = log_base2(cht->h1mod);
  cht->h2mod = cht->hsize;
  
  for (i = 0; i < cht->opt->n_sets; i++)
    {
      int j;
      struct cht_set_t *set = &cht->sets[i];
      
      /* initialize the free list for each set */
      set->flist = (struct cht_ent_t *)calloc(cht->opt->n_ways, 
					      sizeof(struct cht_ent_t));
      for (j = 0; j < cht->opt->n_ways - 1; j++)
	set->flist[j].fnext = &set->flist[j+1];
      
      set->ht = (struct cht_ent_t **)calloc(cht->hsize, sizeof(struct cht_ent_t *));
      set->lht = (struct cht_ent_t **)calloc(cht->hsize, sizeof(struct cht_ent_t *));
		
      set->head = set->tail = NULL;
      set->occ = 0;
    }
  
  return cht;
}

static void
cht_set_unchain_lru(struct cht_set_t *set,
		    struct cht_ent_t *che)
{
  /* unchain from global order */
  if (che->prev) che->prev->next = che->next;
  if (che->next) che->next->prev = che->prev;
  if (che == set->head) set->head = che->next;
  if (che == set->tail) set->tail = che->prev;
  che->next = che->prev = NULL;
}

void
cht_set_chain_lru(struct cht_set_t *set,
		  struct cht_ent_t *che)
{
  /* add to global order */
  che->next = set->head;
  if (set->head) set->head->prev = che;
  else set->tail = che;
  set->head = che;
}

static void 
cht_remove(struct cht_t *cht, 
	   struct cht_ent_t *che)
{
  int hash1, hash2;
  struct cht_set_t *set;
  
  hash1 = MOD(SHIFT_PC(che->PC), cht->h1mod);
  hash2 = MOD(SHIFT_PC(che->PC) >> cht->h2shift, cht->h2mod);
  set = &cht->sets[hash1];
  
  /* unchain from PC hash */
  if (che->hprev) che->hprev->hnext = che->hnext;
  if (che->hnext) che->hnext->hprev = che->hprev;
  if (che == set->ht[hash2]) set->ht[hash2] = che->hnext;
  if (che == set->lht[hash2]) set->lht[hash2] = che->hprev;
  che->hnext = che->hprev = NULL;
  
  /* unchain from LRU */
  cht_set_unchain_lru(set, che);
  
  /* reclaim to free list */
  set->occ--;
  
  che->fnext = set->flist;
  set->flist = che;
}

/* insert integration table entry */
void
cht_enter(struct cht_t *cht,
	  md_addr_t PC,
	  unsigned int store_dist)
{
  int hash1, hash2;
  struct cht_set_t *set;
  struct cht_ent_t *che;
  
  hash1 = MOD(SHIFT_PC(PC), cht->h1mod);
  hash2 = MOD(SHIFT_PC(PC) >> cht->h2shift, cht->h2mod);
  
  set = &cht->sets[hash1];
  
  for (che = set->ht[hash2]; che; che = che->hnext)
    if (che->PC == PC)
      break;
  
  /* Update LRU */
  if (che && che != set->head)
    {
      cht_set_unchain_lru(set, che);
      cht_set_chain_lru(set, che);
    }
  
  if (!che)
    {
      /* potentially evict */
      if (!set->flist)
	cht_remove(cht, set->tail);
      
      che = set->flist;
      set->flist = che->fnext;
      che->fnext = NULL;
      
      che->PC = PC;
      che->store_dist = 1000;

      /* add to PC bucket */
      if (set->lht[hash2]) set->lht[hash2]->hnext = che;
      che->hprev = set->lht[hash2];
      set->lht[hash2] = che;
      if (!set->ht[hash2]) set->ht[hash2] = che;
      
      /* update LRU */
      cht_set_chain_lru(set, che);
      
      set->occ++;
    }
  
#ifdef GET_OUT
  if (che->store_dist > 0 && store_dist >= che->store_dist)
    panic("How did this happen?");
#endif /* GET_OUT */

  if (che->store_dist > 0 && store_dist < che->store_dist)
    che->store_dist = store_dist;
}

unsigned int
cht_lookup(struct cht_t *cht,
	   md_addr_t PC)
{
  int hash1, hash2;
  struct cht_set_t *set;
  struct cht_ent_t *che;

  hash1 = MOD(SHIFT_PC(PC), cht->h1mod);
  hash2 = MOD(SHIFT_PC(PC) >> cht->h2shift, cht->h2mod);

  set = &cht->sets[hash1];

  for (che = set->ht[hash2]; che; che = che->hnext)
    if (che->PC == PC)
      return che->store_dist;

  return 0;
}
