#ifndef _STATS_H
#define _STATS_H

void 
print_counter(FILE *stream, 
	      const char *name, 
	      counter_t counter, 
	      const char *desc);

void 
print_int(FILE *stream, 
	  const char *name, 
	  int i, 
	  const char *desc);

void
print_rate(FILE *stream, 
	   const char *name,
	   double rate,
	   const char *desc);

void
print_addr(FILE *stream, 
	   const char *name,
	   md_addr_t addr,
	   const char *desc);

#define BDVEC_BDSUM(BDVEC) \
   ((BDVEC)[FALSE] + \
    (BDVEC)[TRUE])

#define PMVEC_PMSUM(EMVEC) \
   ((EMVEC)[pm_CTRL_DRIVEN] + \
    (EMVEC)[pm_DATA_DRIVEN])

#define ICVEC_ICSUM(ICVEC) \
   ((ICVEC)[ic_nop] + \
    (ICVEC)[ic_prefetch] + \
    (ICVEC)[ic_load] + \
    (ICVEC)[ic_store] + \
    (ICVEC)[ic_ctrl] + \
    (ICVEC)[ic_icomp] + \
    (ICVEC)[ic_icomplong] + \
    (ICVEC)[ic_fcomp] + \
    (ICVEC)[ic_fcomplong] + \
    (ICVEC)[ic_other])

#define ICPMVEC_ICPMSUM(ICPMVEC) \
   ((ICPMVEC)[ic_nop][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_nop][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_prefetch][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_prefetch][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_load][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_load][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_store][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_store][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_ctrl][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_ctrl][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_icomp][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_icomp][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_icomplong][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_icomplong][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_fcomp][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_fcomp][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_fcomplong][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_fcomplong][pm_DATA_DRIVEN] + \
    (ICPMVEC)[ic_other][pm_CTRL_DRIVEN] + \
    (ICPMVEC)[ic_other][pm_DATA_DRIVEN])

#define ICPMVEC_ICSUM(ICPMVEC, PM) \
   ((ICPMVEC)[ic_nop][(PM)] + \
    (ICPMVEC)[ic_prefetch][(PM)] + \
    (ICPMVEC)[ic_load][(PM)] + \
    (ICPMVEC)[ic_store][(PM)] + \
    (ICPMVEC)[ic_ctrl][(PM)] + \
    (ICPMVEC)[ic_icomp][(PM)] + \
    (ICPMVEC)[ic_icomplong][(PM)] + \
    (ICPMVEC)[ic_fcomp][(PM)] + \
    (ICPMVEC)[ic_fcomplong][(PM)] + \
    (ICPMVEC)[ic_other][(PM)])

#define ICBDVEC_ICBDSUM(ICBDVEC) \
   ((ICBDVEC)[ic_nop][TRUE] + (ICBDVEC)[ic_nop][FALSE] + \
    (ICBDVEC)[ic_prefetch][TRUE] + (ICBDVEC)[ic_prefetch][FALSE] + \
    (ICBDVEC)[ic_load][TRUE] + (ICBDVEC)[ic_load][FALSE] + \
    (ICBDVEC)[ic_store][TRUE] + (ICBDVEC)[ic_store][FALSE] + \
    (ICBDVEC)[ic_ctrl][TRUE] + (ICBDVEC)[ic_ctrl][FALSE] + \
    (ICBDVEC)[ic_icomp][TRUE] + (ICBDVEC)[ic_icomp][FALSE] + \
    (ICBDVEC)[ic_icomplong][TRUE] + (ICBDVEC)[ic_icomplong][FALSE] + \
    (ICBDVEC)[ic_fcomp][TRUE] + (ICBDVEC)[ic_fcomp][FALSE] + \
    (ICBDVEC)[ic_fcomplong][TRUE] + (ICBDVEC)[ic_fcomplong][FALSE] + \
    (ICBDVEC)[ic_other][TRUE] + (ICBDVEC)[ic_other][FALSE])

#define ICBDVEC_ICSUM(ICBDVEC, BD) \
   ((ICBDVEC)[ic_nop][(BD)] + \
    (ICBDVEC)[ic_prefetch][(BD)] + \
    (ICBDVEC)[ic_load][(BD)] + \
    (ICBDVEC)[ic_store][(BD)] + \
    (ICBDVEC)[ic_ctrl][(BD)] + \
    (ICBDVEC)[ic_icomp][(BD)] + \
    (ICBDVEC)[ic_icomplong][(BD)] + \
    (ICBDVEC)[ic_fcomp][(BD)] + \
    (ICBDVEC)[ic_fcomplong][(BD)] + \
    (ICBDVEC)[ic_other][(BD)])

#endif /* _STATS_H */
