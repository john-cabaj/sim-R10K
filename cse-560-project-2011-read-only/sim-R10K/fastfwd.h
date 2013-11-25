#ifndef FASTFWD_H
#define FASTFWD_H

typedef
void (* warmup_handler_t)(const struct predec_insn_t *pdi);

bool_t
sim_fastfwd(struct regs_t *regs, 
	    struct mem_t *mem, 
	    unsigned long long n_fastfwd,
	    warmup_handler_t warmup_handler);

#endif /* FASTFWD_H */
