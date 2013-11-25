#ifndef PERFECT_H
#define PERFECT_H

struct perfect_insn_info_t 
{
  seq_t seq;

  md_addr_t PC;
  md_addr_t NPC;
  md_addr_t TPC;

  union val_t vals[DEP_NUM];
  md_addr_t addr;
  int dsize;
  bool_t f_wrong_path;
};


void
perfect_info_init(unsigned int size);

void
perfect_info_get(const struct predec_insn_t *pdi,
		 struct perfect_insn_info_t *pii);

void
perfect_info_skip(const struct predec_insn_t *pdi,
		  md_addr_t PC);

void
perfect_info_sync(const struct regs_t *regs,
		  const struct mem_t *mem,
		  seq_t seq);


void
perfect_info_recover(seq_t seq);

#endif /* PERFECT_H */
