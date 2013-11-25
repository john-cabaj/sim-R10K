/*------------------------------------------------------------
 *  Copyright 1994 Digital Equipment Corporation and Steve Wilton
 *                         All Rights Reserved
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Users of this software agree to the terms and conditions set forth herein,
 * and hereby grant back to Digital a non-exclusive, unrestricted, royalty-
 * free right and license under any changes, enhancements or extensions
 * made to the core functions of the software, including but not limited to
 * those affording compatibility with other hardware or software
 * environments, but excluding applications which incorporate this software.
 * Users further agree to use their best efforts to return to Digital any
 * such changes, enhancements or extensions that they make and inform Digital
 * of noteworthy uses of this software.  Correspondence should be provided
 * to Digital at:
 *
 *                       Director of Licensing
 *                       Western Research Laboratory
 *                       Digital Equipment Corporation
 *                       100 Hamilton Avenue
 *                       Palo Alto, California  94301
 *
 * This software may be distributed (but not offered for sale or transferred
 * for compensation) to third parties, provided such third parties agree to
 * abide by the terms and conditions of this notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *------------------------------------------------------------*/

#ifndef __POWER_H
#define __POWER_H

#include "stats.h"
#include "cacti3/cacti_params.h"

/*  The following are things you might want to change
 *  when compiling
 */

/*
 * The output can be in 'long' format, which shows everything, or
 * 'short' format, which is just what a program like 'grap' would
 * want to see
 */

#define LONG 1
#define SHORT 2

#define OUTPUTTYPE LONG

/* Do we want static AFs (STATIC_AF) or Dynamic AFs (DYNAMIC_AF) */
#define DYNAMIC_AF

/*
 * Address bits in a word, and number of output bits from the cache 
 */

/* limits on the various N parameters */

#define MAXN 8            /* Maximum for Ndwl,Ntwl,Ndbl,Ntbl */
#define MAXSUBARRAYS 8    /* Maximum subarrays for data and tag arrays */
#define MAXSPD 8          /* Maximum for Nspd, Ntspd */


/*===================================================================*/

/*
 * The following are things you probably wouldn't want to change.  
 */


#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define BIGNUM 1e30
#define DIVIDE(a,b) ((b)==0)? 0:(a)/(b)
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

/* Used to communicate with the horowitz model */

#define RISE 1
#define FALL 0
#define NCH  1
#define PCH  0

/*===================================================================*/

/*
 * Cache layout parameters and process parameters 
 * Thanks to Glenn Reinman for the technology scaling factors
 */

#define GEN_POWER_FACTOR 1.31

#define VSINV         0.456   
#define VTHINV100x60  0.438   /* inverter with p=100,n=60 */
#define VTHNAND60x90  0.561   /* nand with p=60 and three n=90 */
#define VTHNOR12x4x1  0.503   /* nor with p=12, n=4, 1 input */
#define VTHNOR12x4x2  0.452   /* nor with p=12, n=4, 2 inputs */
#define VTHNOR12x4x3  0.417   /* nor with p=12, n=4, 3 inputs */
#define VTHNOR12x4x4  0.390   /* nor with p=12, n=4, 4 inputs */
#define VTHOUTDRINV    0.437
#define VTHOUTDRNOR   0.431
#define VTHOUTDRNAND  0.441
#define VTHOUTDRIVE   0.425
#define VTHCOMPINV    0.437
#define VTHMUXDRV1    0.437
#define VTHMUXDRV2    0.486
#define VTHMUXDRV3    0.437
#define VTHEVALINV    0.267
#define VTHSENSEEXTDRV  0.437

#define AF    .5
#define POPCOUNT_AF  (23.9/64.0)
#define I_POPCOUNT_AF  (1.0 - POPCOUNT_AF)

/* static double Cmetal 1.222e-15 */

/* fF/um2 at 1.5V */
#define Cndiffarea    0.137e-15		/* FIXME: ??? */

/* fF/um2 at 1.5V */
#define Cpdiffarea    0.343e-15		/* FIXME: ??? */

/* fF/um at 1.5V */
#define Cndiffside    0.275e-15		/* in general this does not scale */

/* fF/um at 1.5V */
#define Cpdiffside    0.275e-15		/* in general this does not scale */

/* fF/um at 1.5V */
#define Cndiffovlp    0.138e-15		/* FIXME: by depth??? */

/* fF/um at 1.5V */
#define Cpdiffovlp    0.138e-15		/* FIXME: by depth??? */

/* fF/um assuming 25% Miller effect */
#define Cnoxideovlp   0.263e-15		/* FIXME: by depth??? */

/* fF/um assuming 25% Miller effect */
#define Cpoxideovlp   0.338e-15		/* FIXME: by depth??? */

/* fF/um2 */
#define Cgate         1.95e-15		/* FIXME: ??? */

/* fF/um2 */
#define Cgatepass     1.45e-15		/* FIXME: ??? */

/*
 * CMOS 0.8um model parameters
 *   - from Appendix II of Cacti tech report
 */


extern bool_t power_model_f;

/* Used to pass values around the program */

enum proc_struct_t
{
  ps_IL1_TAG,
  ps_IL1_DATA,
  ps_ITLB,
  ps_DL1_TAG,
  ps_DL1_DATA,
  ps_DTLB,
  ps_L2_TAG,
  ps_L2_DATA,
  ps_DIRPRED,
  ps_BTB,
  ps_RAS,
  ps_DECODE,
  ps_RENAME,
  ps_FREELIST,
  ps_ROB,
  ps_LQADDR,
  ps_LQDATA,
  ps_SQADDR,
  ps_SQDATA,
  ps_WAKEUP,
  ps_SELECT,
  ps_RSTATION,
  ps_REGFILE,
  ps_IALU,
  ps_FALU,
  ps_AGEN,
  ps_RESULTBUS,
  ps_TOTALNOCLOCK,
  ps_CLOCK,
  ps_TOTAL,
  ps_NUM
};

enum proc_bigstruct_t
{
  pbs_BPRED, /* ps_DIRPRED + ps_BTB + ps_RAS */
  pbs_IMEM, /* ps_IL1 + ps_ITLB */
  pbs_DECODE, /* ps_DECODE */
  pbs_RENAME, /* ps_RENAME + ps_FREELIST + ps_ITCROSSCHECK + ps_REGCOUNT*/
  pbs_ALU, /* ps_IALU + ps_FALU */
  pbs_LSQ, /* ps_SQADDR + ps_SQDATA + ps_LQADDR + ps_LQDATA + ps_DDSQADDR + ps_DDSQDATA + ps_DL1 + ps_DTLB */
  pbs_DMEM, /* ps_DL1 + ps_DTLB */
  pbs_WINDOW, /* ps_WAKEUP + ps_SELECT + ps_RSTATION + ps_RESULTBUS */
  pbs_REGFILE, /* ps_REGFILE */
  pbs_ROB, /* ps_ROB */
  pbs_L2, /* ps_L2 */
  pbs_TOTALNOCLOCK,
  pbs_CLOCK, /* ps_CLOCK */
  pbs_TOTAL, /* ps_TOTAL */
  pbs_NUM
};

void power_count_access(enum proc_struct_t ps,
			bool_t write_f,
			bool_t hard_count_f);

void power_count_dynamic_af(enum proc_struct_t ps, 
			    unsigned int pc,
			    unsigned int width);

void power_count_access_new_cycle(void);
void power_set_power_nports(enum proc_struct_t ps, 
			    double r_power,
			    double w_power,
			    double max_power,
			    unsigned int rports,
			    unsigned int wports,
			    unsigned int rwports);
double logtwo(double x);
double gatecap(double width,double wirelength);
double gatecappass(double width,double wirelength);
double draincap(double width,int nchannel,int stack);
double restowidth(double res,int nchannel);

double ialu_power(void);

double falu_power(void);

double arbiter_power(unsigned int nlines);

double comparator_power(unsigned int nbits);

double array_power(unsigned int rows, 
		   unsigned int cols, 
		   unsigned int rport, 
		   unsigned int wport, 
		   bool_t cache,
		   double *r_power,
		   double *w_power);

double rat_array_power(unsigned int rows, 
		       unsigned int cols, 
		       unsigned int rport, 
		       unsigned int wport,
		       double *r_power,
		       double *w_power);

double cam_array_power(unsigned int rows, 
		       unsigned int cols, 
		       unsigned int rports, 
		       unsigned int wports,
		       double *r_power,
		       double *w_power);

double simple_array_power(unsigned int rows, 
			  unsigned int cols,
			  unsigned int rports,
			  unsigned int wports,
			  bool_t cache,
			  double *r_power,
			  double *w_power);

double array_decoder_power(unsigned int rows,
			   unsigned int cols,
			   unsigned int ports);

double resultbus_power(unsigned int npregs, 
		       unsigned int nports, 
		       unsigned int nalu, 
		       unsigned int width);

double clock_power(unsigned int npiperegs,
		   unsigned int nialu,
		   unsigned int nfpalu,
		   double die_length);

void total_power(void);

double clock_power_factor(void);

int pop_count(quad_t bits);
int pop_count_slow(quad_t bits);

struct objective_params_t
{
   double delay_weight;
   double power_weight;
   double area_weight;
};

void
cache_onebank_power(unsigned int nsets,
		    unsigned int assoc,
		    unsigned int dbits,
		    unsigned int tbits,
		    unsigned int nbanks,
		    unsigned int rwport,
		    unsigned int rport,
		    unsigned int wport,
		    unsigned int asize,
		    unsigned int osize,
		    unsigned int bsize,
		    struct objective_params_t *op,
		    double *power_tag_read,
		    double *power_tag_write,
		    double *power_data_read,
		    double *power_data_write
);

double 
power_single_access(enum proc_struct_t ps);

void
power_reg_options(struct opt_odb_t *odb);

void
power_check_options(void);

void
power_aux_config(FILE *stream);

void 
power_stats_print(tick_t ncycle,
		  FILE *stream);

void
power_set_sample(bool_t f_sample);

/* assume half values to be bypassed instead of read from the regfile*/
#define REGFILE_READ_RATE 0.5
#define CLOCK_FACTOR 1.13 * 1.0
#define RESULTBUS_ACTIVITY_FACTOR 0.9
#define REGFILE_ACTIVITY_FACTOR 0.88

#endif
