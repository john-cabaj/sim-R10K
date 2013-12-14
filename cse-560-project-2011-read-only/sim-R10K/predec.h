#ifndef PREDEC_H
#define PREDEC_H

/* pre-decoded instruction */
struct predec_bits_t
{
  bool_t f_problem;
  bool_t f_trigger;
  bool_t f_preexecute;
};

struct predec_poi_t
{
  md_addr_t PC;
  enum md_opcode_t op;
  int imm;
};

struct predec_insn_t
{
  struct predec_insn_t *next;

  struct predec_poi_t poi;
  md_inst_t inst;
  enum insn_class_t iclass;

  regnum_t lregnums[DEP_NUM];

  /* instance counter, comes in handy occasionally.  must be updated
     externally */
  counter_t n_inst;
};

void
predec_init(void);

struct predec_insn_t *
predec_lookup(md_addr_t pc);

struct predec_insn_t *
predec_enter(md_addr_t PC, md_inst_t inst);

void
predec_reg_options(struct opt_odb_t *odb);

void
predec_check_options(void);

#endif /* PREDEC_H */
