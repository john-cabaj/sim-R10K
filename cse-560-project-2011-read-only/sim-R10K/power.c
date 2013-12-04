/* I inclued this copyright since we're using Cacti for some stuff */

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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "machine.h"
#include "options.h"
#include "cacti3/cacti_params.h"
#include "power.h"

static double CSCALE;
static double RSCALE;
static double LSCALE;
static double ASCALE;
static double VSCALE;
static double VTSCALE;
static double SSCALE;
static double MSCALE;
static double GEN_POWER_SCALE;

/* corresponds to 8um of m3 @ 225ff/um */
static double Cwordmetal;

/* corresponds to 16um of m2 @ 275ff/um */
static double Cbitmetal;

/* corresponds to 1um of m2 @ 275ff/um */
static double Cmetal;

static double CM3metal;
static double CM2metal;

/* um */
static double Leff;

/* note that the value of Cgatepass will be different depending on 
   whether or not the source and drain are at different potentials or
   the same potential.  The two values were averaged */

/* fF/um */
static double Cpolywire;

/* ohms*um of channel width */
static double Rnchannelstatic;

/* ohms*um of channel width */
static double Rpchannelstatic;

static double Rnchannelon;

static double Rpchannelon;

/* corresponds to 16um of m2 @ 48mO/sq */
static double Rbitmetal;

/* corresponds to  8um of m3 @ 24mO/sq */
static double Rwordmetal;

static double Period;

static double krise;
static double tsensedata;
static double tsensetag;
static double tfalldata;
static double tfalltag;
static double Vbitpre;
static double Vt;
static double Vbitsense;

static double Powerfactor;
static double SensePowerfactor;

/* transistor widths in um (as described in tech report, appendix 1) */
static double Wdecdrivep;
static double Wdecdriven;
static double Wdec3to8n;
static double Wdec3to8p;
static double WdecNORn;
static double WdecNORp;
static double Wdecinvn;
static double Wdecinvp;

static double Wworddrivemax;
static double Wmemcella;
static double Wmemcellr;
static double Wmemcellw;
static double Wmemcellbscale = 2;		/* means 2x bigger than Wmemcella */
static double Wbitpreequ;

static double Wbitmuxn;
static double WsenseQ1to4;
static double Wcompinvp1;
static double Wcompinvn1;
static double Wcompinvp2;
static double Wcompinvn2;
static double Wcompinvp3;
static double Wcompinvn3;
static double Wevalinvp;
static double Wevalinvn;

static double Wcompn;
static double Wcompp;
static double Wcomppreequ;
static double Wmuxdrv12n;
static double Wmuxdrv12p;
static double WmuxdrvNANDn;
static double WmuxdrvNANDp;
static double WmuxdrvNORn;
static double WmuxdrvNORp;
static double Wmuxdrv3n;
static double Wmuxdrv3p;
static double Woutdrvseln;
static double Woutdrvselp;
static double Woutdrvnandn;
static double Woutdrvnandp;
static double Woutdrvnorn;
static double Woutdrvnorp;
static double Woutdrivern;
static double Woutdriverp;

static double Wcompcellpd2;
static double Wcompdrivern;
static double Wcompdriverp;
static double Wcomparen2;
static double Wcomparen1;
static double Wmatchpchg;
static double Wmatchinvn;
static double Wmatchinvp;
static double Wmatchnandn;
static double Wmatchnandp;
static double Wmatchnorn;
static double Wmatchnorp;

static double WSelORn;
static double WSelORprequ;
static double WSelPn;
static double WSelPp;
static double WSelEnn;
static double WSelEnp;

static double Wsenseextdrv1p;
static double Wsenseextdrv1n;
static double Wsenseextdrv2p;
static double Wsenseextdrv2n;


/* bit width of RAM cell in um */
static double BitWidth;

/* bit height of RAM cell in um */
static double BitHeight;

static double Cout;

/* Sizing of cells and spacings */
static double RatCellHeight;
static double RatCellWidth;
static double RatShiftRegWidth;
static double RatNumShift =      4;
static double BitlineSpacing;
static double WordlineSpacing;

static double RegCellHeight;
static double RegCellWidth;

static double CamCellHeight;
static double CamCellWidth;
static double MatchlineSpacing;
static double TaglineSpacing;

/*===================================================================*/

/* ALU POWER NUMBERS for .18um 733Mhz */
/* normalize to cap from W */
static double NORMALIZE_SCALE = 1.0/(733.0e6*1.45*1.45);
/* normalize .18um cap to other gen's cap, then xPowerfactor */
static double POWER_SCALE; 
static double I_ADD;        
static double I_ADD32;      
static double I_MULT16;     
static double I_SHIFT;      
static double I_LOGIC;      
static double F_ADD;        
static double F_MULT;       

static double I_ADD_CLOCK;  
static double I_MULT_CLOCK; 
static double I_SHIFT_CLOCK;
static double I_LOGIC_CLOCK;
static double F_ADD_CLOCK; 
static double F_MULT_CLOCK;

static struct tech_params_t tech_opt;

bool_t f_power_sample;

void
power_set_sample(bool_t f_sample)
{
  f_power_sample = f_sample;
}


void
power_reg_options(struct opt_odb_t *odb)
{
   opt_reg_double(odb, "-power:tech", "technology size",
                  &tech_opt.tech_size, /* default */0.10,
                  /* print */TRUE, /* format */NULL);

   opt_reg_double(odb, "-power:crossover", "crossover (vdd->gnd) current multiplier",
                  &tech_opt.crossover, /* default */1.2,
                  /* print */TRUE, /* format */NULL);

   opt_reg_double(odb, "-power:standby", "standby multiplier",
                  &tech_opt.standby, /* default */0.2,
                  /* print */TRUE, /* format */NULL);

   opt_reg_double(odb, "-power:vdd", "supply voltage (0.7-4.5)",
		  &tech_opt.vdd, /* default */1.2,
		  /* print */TRUE, /* format */NULL);
   
   opt_reg_double(odb, "-power:mhz", "clock frequency (MHz)",
		  &tech_opt.mhz, /* default */2000,
		  /* print */TRUE, /* format */NULL);
};

void
power_check_options(void)
{
   if (tech_opt.tech_size == 0.80)
     {
       CSCALE =	1.0;		/* wire capacitance scaling factor */
       RSCALE =	1.0;		/* wire resistance scaling factor */
       LSCALE =	1.0;		/* length (feature) scaling factor */
       VSCALE =	1.0;		/* voltage scaling factor */
       VTSCALE = 1.0;		/* threshold voltage scaling factor */
       SSCALE =	1.0;		/* sense voltage scaling factor */
       GEN_POWER_SCALE = (GEN_POWER_FACTOR*GEN_POWER_FACTOR*GEN_POWER_FACTOR*GEN_POWER_FACTOR);
     }
   else if (tech_opt.tech_size == 0.40)
     {
       CSCALE =	1.0;		/* wire capacitance scaling factor */
       RSCALE =	1.0;		/* wire resistance scaling factor */
       LSCALE =	0.5;		/* length (feature) scaling factor */
       VSCALE =	1.0;		/* voltage scaling factor */
       VTSCALE = 1.0;		/* threshold voltage scaling factor */
       SSCALE =	1.0;		/* sense voltage scaling factor */
       GEN_POWER_SCALE = (GEN_POWER_FACTOR*GEN_POWER_FACTOR*GEN_POWER_FACTOR);
     }
   else if (tech_opt.tech_size == 0.35)
     {
       CSCALE = 5.2197;	/* wire capacitance scaling factor */
       RSCALE =	5.2571;	/* wire resistance scaling factor */
       LSCALE =	0.4375;		/* length (feature) scaling factor */
       VSCALE =	0.52;		/* voltage scaling factor */
       VTSCALE = 0.6147;		/* threshold voltage scaling factor */
       SSCALE =	0.95;		/* sense voltage scaling factor */
       GEN_POWER_SCALE = (GEN_POWER_FACTOR*GEN_POWER_FACTOR);
     }
   else if (tech_opt.tech_size == 0.25)
     {
       CSCALE = 10.2197;	/* wire capacitance scaling factor */
       RSCALE =	10.2571;	/* wire resistance scaling factor */
       LSCALE =	0.3571;		/* length (feature) scaling factor */
       VSCALE =	0.45;		/* voltage scaling factor */
       VTSCALE = 0.5596;		/* threshold voltage scaling factor */
       SSCALE =	0.90;		/* sense voltage scaling factor */
       GEN_POWER_SCALE = GEN_POWER_FACTOR;
     }
   else if (tech_opt.tech_size == 0.18)
     {
       CSCALE =	19.7172;	/* wire capacitance scaling factor */
       RSCALE =	20.0000;	/* wire resistance scaling factor */
       LSCALE =	0.2250;		/* length (feature) scaling factor */
       VSCALE =	0.4;		/* voltage scaling factor */
       VTSCALE = 0.5046;	/* threshold voltage scaling factor */
       SSCALE =	0.85;		/* sense voltage scaling factor */
       GEN_POWER_SCALE = 1;
     }
   else if (tech_opt.tech_size == 0.10)
     {
       CSCALE =	84.2172;	/* wire capacitance scaling factor, linear: 51.7172, predicted: 84.2172 */
       RSCALE =	80.0000;	/* wire resistance scaling factor */
       LSCALE =	0.1250;		/* length (feature) scaling factor */
       VSCALE =	0.38;		/* voltage scaling factor */
       VTSCALE = 0.49;		/* threshold voltage scaling factor */
       SSCALE = 0.80;		/* sense voltage scaling factor */
       GEN_POWER_SCALE = 1/GEN_POWER_FACTOR;
     }
   else
      fatal("unsupported technology size");

   ASCALE = LSCALE * LSCALE;
   MSCALE = (LSCALE * .624 / .2250);

   if (tech_opt.crossover < 1)
      fatal("crossover current multiplier must be > 1");

   if (tech_opt.standby > 1)
      fatal("standby power multiplier must be < 1");

   tech_opt.fudgefactor = 0.8 / tech_opt.tech_size;

   Period = 1 / (tech_opt.mhz * 1000000);
   krise = (0.4e-9 * LSCALE);
   tsensedata = (5.8e-10 * LSCALE);
   tsensetag = (2.6e-10 * LSCALE);
   tfalldata = (7e-10 * LSCALE);
   tfalltag = (7e-10 * LSCALE);
   Vbitpre = (3.3 * SSCALE);
   Vt = (1.09 * VTSCALE);
   Vbitsense = (0.10 * SSCALE);

   Powerfactor = (tech_opt.mhz * 1000000)*tech_opt.vdd*tech_opt.vdd;   
   SensePowerfactor = (tech_opt.mhz * 1000000)*(tech_opt.vdd/2)*(tech_opt.vdd/2);
     
   /* corresponds to 8um of m3 @ 225ff/um */
   Cwordmetal = (1.8e-15 * (CSCALE * ASCALE));
     
   /* corresponds to 16um of m2 @ 275ff/um */
   Cbitmetal = (4.4e-15 * (CSCALE * ASCALE));

   /* corresponds to 1um of m2 @ 275ff/um */
   Cmetal = Cbitmetal/16;

   CM3metal = Cbitmetal/16; 
   CM2metal = Cbitmetal/16; 

   /* um */
   Leff = (0.8 * LSCALE);
   
   /* note that the value of Cgatepass will be different depending on 
      whether or not the source and drain are at different potentials or
      the same potential.  The two values were averaged */
   
   /* fF/um */
   Cpolywire = (0.25e-15 * CSCALE * LSCALE);
     
   /* ohms*um of channel width */
   Rnchannelstatic = (25800 * LSCALE);
   
   /* ohms*um of channel width */
   Rpchannelstatic = (61200 * LSCALE);
   Rnchannelon = (9723 * LSCALE);
   Rpchannelon = (22400 * LSCALE);

   /* corresponds to 16um of m2 @ 48mO/sq */
   Rbitmetal = (0.320 * (RSCALE * ASCALE));
   
   /* corresponds to  8um of m3 @ 24mO/sq */
   Rwordmetal = (0.080 * (RSCALE * ASCALE));
   
   /* other stuff (from tech report, appendix 1) */


/* Threshold voltages (as a proportion of Vdd)
   If you don't know them, set all values to 0.5 */

/* transistor widths in um (as described in tech report, appendix 1) */
   Wdecdrivep =	(57.0 * LSCALE);
   Wdecdriven = (40.0 * LSCALE);
   Wdec3to8n = (14.4 * LSCALE);
   Wdec3to8p = (14.4 * LSCALE);
   WdecNORn = (5.4 * LSCALE);
   WdecNORp = (30.5 * LSCALE);
   Wdecinvn = (5.0 * LSCALE);
   Wdecinvp = (10.0  * LSCALE);

   Wworddrivemax = (100.0 * LSCALE);
   Wmemcella = (2.4 * LSCALE);
   Wmemcellr = (4.0 * LSCALE);
   Wmemcellw = (2.1 * LSCALE);
   Wbitpreequ =	(10.0 * LSCALE);
 
   Wbitmuxn = (10.0 * LSCALE);
   WsenseQ1to4 = (4.0 * LSCALE);
   Wcompinvp1 = (10.0 * LSCALE);
   Wcompinvn1 = (6.0 * LSCALE);
   Wcompinvp2 = (20.0 * LSCALE);
   Wcompinvn2 = (12.0 * LSCALE);
   Wcompinvp3 = (40.0 * LSCALE);
   Wcompinvn3 =	(24.0 * LSCALE);
   Wevalinvp = (20.0 * LSCALE);
   Wevalinvn = (80.0 * LSCALE);
   
   Wcompn = (20.0 * LSCALE);
   Wcompp = (30.0 * LSCALE);
   Wcomppreequ = (40.0 * LSCALE);
   Wmuxdrv12n = (30.0 * LSCALE);
   Wmuxdrv12p =	(50.0 * LSCALE);
   WmuxdrvNANDn = (20.0 * LSCALE);
   WmuxdrvNANDp = (80.0 * LSCALE);
   WmuxdrvNORn = (60.0 * LSCALE);
   WmuxdrvNORp = (80.0 * LSCALE);
   Wmuxdrv3n = (200.0 * LSCALE);
   Wmuxdrv3p = (480.0 * LSCALE);
   Woutdrvseln = (12.0 * LSCALE);
   Woutdrvselp = (20.0 * LSCALE);
   Woutdrvnandn	= (24.0 * LSCALE);
   Woutdrvnandp	= (10.0 * LSCALE);
   Woutdrvnorn = (6.0 * LSCALE);
   Woutdrvnorp = (40.0 * LSCALE);
   Woutdrivern = (48.0 * LSCALE);
   Woutdriverp = (80.0 * LSCALE);
   
   Wcompcellpd2 = (2.4 * LSCALE);
   Wcompdrivern = (400.0 * LSCALE);
   Wcompdriverp = (800.0 * LSCALE);
   Wcomparen2 = (40.0 * LSCALE);
   Wcomparen1 = (20.0 * LSCALE);
   Wmatchpchg = (10.0 * LSCALE);
   Wmatchinvn = (10.0 * LSCALE);
   Wmatchinvp = (20.0 * LSCALE);
   Wmatchnandn = (20.0 * LSCALE);
   Wmatchnandp = (10.0 * LSCALE);
   Wmatchnorn = (20.0 * LSCALE);
   Wmatchnorp = (10.0 * LSCALE);

   WSelORn = (10.0 * LSCALE);
   WSelORprequ = (40.0 * LSCALE);
   WSelPn = (10.0 * LSCALE);
   WSelPp = (15.0 * LSCALE);
   WSelEnn = (5.0 * LSCALE);
   WSelEnp = (10.0 * LSCALE);

   Wsenseextdrv1p = (40.0*LSCALE);
   Wsenseextdrv1n = (24.0*LSCALE);
   Wsenseextdrv2p = (200.0*LSCALE);
   Wsenseextdrv2n = (120.0*LSCALE);
   

   /* bit width of RAM cell in um */
   BitWidth = (16.0 * LSCALE);
   
   /* bit height of RAM cell in um */
   BitHeight = (16.0 * LSCALE);

   Cout	= (0.5e-12 * LSCALE);

   /* Sizing of cells and spacings */
   RatCellHeight = (40.0 * LSCALE);
   RatCellWidth = (70.0 * LSCALE);
   RatShiftRegWidth = (120.0 * LSCALE);
   BitlineSpacing = (6.0 * LSCALE);
   WordlineSpacing = (6.0 * LSCALE);

   RegCellHeight = (16.0 * LSCALE);
   RegCellWidth = (8.0  * LSCALE);
   CamCellHeight = (40.0 * LSCALE);
   CamCellWidth = (25.0 * LSCALE);
   MatchlineSpacing = (6.0 * LSCALE);
   TaglineSpacing = (6.0 * LSCALE);

/*===================================================================*/

   /* ALU POWER NUMBERS for .18um 733Mhz */
   /* normalize to cap from W */
   NORMALIZE_SCALE = (1.0/(733.0e6*1.45*1.45));
   /* normalize .18um cap to other gen's cap, then xPowerfactor */
   POWER_SCALE = (GEN_POWER_SCALE * NORMALIZE_SCALE * Powerfactor);
   I_ADD = ((.37 - .091) * POWER_SCALE);
   I_ADD32 = (((.37 - .091)/2)*POWER_SCALE);
   I_MULT16 = ((.31-.095)*POWER_SCALE);
   I_SHIFT = ((.21-.089)*POWER_SCALE);
   I_LOGIC = ((.04-.015)*POWER_SCALE);
   F_ADD = ((1.307-.452)*POWER_SCALE);
   F_MULT = ((1.307-.452)*POWER_SCALE);
     
   I_ADD_CLOCK = (.091*POWER_SCALE);
   I_MULT_CLOCK = (.095*POWER_SCALE);
   I_SHIFT_CLOCK = (.089*POWER_SCALE);
   I_LOGIC_CLOCK = (.015*POWER_SCALE);
   F_ADD_CLOCK = (.452*POWER_SCALE);
   F_MULT_CLOCK =  (.452*POWER_SCALE);
}



/*----------------------------------------------------------------------*/

double logfour(double x)
{
   if (x <= 0)
      fprintf(stderr, "%e\n", x);
   return ((double) (log(x) / log(4.0)));
}

/* safer pop count to validate the fast algorithm */
int pop_count_slow(quad_t bits)
{
   int count = 0;
   quad_t tmpbits = bits;
   while (tmpbits)
   {
      if (tmpbits & 1)
         ++count;
      tmpbits >>= 1;
   }
   return count;
}

/* fast pop count */
int pop_count(quad_t bits)
{
#define T unsigned long long
#define ONES ((T)(-1))
#define TWO(k) ((T)1 << (k))
#define CYCL(k) (ONES/(1 + (TWO(TWO(k)))))
#define BSUM(x,k) ((x)+=(x) >> TWO(k), (x) &= CYCL(k))
   quad_t x = bits;
   x = (x & CYCL(0)) + ((x >> TWO(0)) & CYCL(0));
   x = (x & CYCL(1)) + ((x >> TWO(1)) & CYCL(1));
   BSUM(x, 2);
   BSUM(x, 3);
   BSUM(x, 4);
   BSUM(x, 5);
   return x;
}

static char * ps2str[ps_NUM] = {
                                  "il1_tag",
                                  "il1_data",
                                  "itlb",
                                  "dl1_tag",
                                  "dl1_data",
                                  "dtlb",
                                  "l2_tag",
                                  "l2_data",
                                  "dirpred",
                                  "btb",
                                  "ras",
                                  "decode",
                                  "rename",
                                  "freelist",
                                  "rob",
                                  "lqaddr",
                                  "lqdata",
                                  "sqaddr",
                                  "sqdata",
                                  "wakeup",
                                  "select",
                                  "rstation",
                                  "regfile",
                                  "ialu",
                                  "falu",
				  "agen",
                                  "resultbus",
                                  "totalnoclock",
                                  "clock",
                                  "total"
                               };

static char * pbs2str[pbs_NUM] = {
                                    "bpred",
                                    "imem",
                                    "decode",
                                    "rename",
                                    "alu",
                                    "lsq",
                                    "dmem",
                                    "window",
                                    "regfile",
                                    "rob",
                                    "l2",
                                    "totalnoclock",
                                    "clock",
                                    "total"
                                 };

static enum proc_bigstruct_t ps2pbs[ps_NUM] = {
         /* ps_IL1_TAG */pbs_IMEM,
         /* ps_IL1_DATA */pbs_IMEM,
         /* ps_ITLB */pbs_IMEM,
         /* ps_DL1_TAG */pbs_DMEM,
         /* ps_DL1_DATA */pbs_DMEM,
         /* ps_DTLB */pbs_DMEM,
         /* ps_L2_TAG */pbs_L2,
         /* ps_L2_DATA */pbs_L2,
         /* ps_DIRPRED */pbs_BPRED,
         /* ps_BTB */pbs_BPRED,
         /* ps_RAS */pbs_BPRED,
         /* ps_DECODE */pbs_DECODE,
         /* ps_RENAME */pbs_RENAME,
         /* ps_FREELIST */pbs_RENAME,
         /* ps_ROB */pbs_ROB,
         /* ps_LQADDR */pbs_LSQ,
         /* ps_LQDATA */pbs_LSQ,
         /* ps_SQADDR */pbs_LSQ,
         /* ps_SQDATA */pbs_LSQ,
         /* ps_WAKEUP */pbs_WINDOW,
         /* ps_SELECT */pbs_WINDOW,
         /* ps_RSTATION */pbs_WINDOW,
         /* ps_REGFILE */pbs_REGFILE,
         /* ps_IALU */pbs_ALU,
         /* ps_FALU */pbs_ALU,
	 /* ps_AGEN */pbs_LSQ,
         /* ps_RESULTBUS */pbs_WINDOW,
         /* ps_TOTALNOCLOCK */pbs_TOTALNOCLOCK,
         /* ps_CLOCK */pbs_CLOCK,
         /* ps_TOTAL */pbs_TOTAL
      };

static double global_clockcap = 0.0;

struct power_struct_t
{
  /* basic parameters computed by calc_power */
  unsigned int rports;
  unsigned int wports;
  unsigned int rwports;

  double r_power;
  double w_power;
  
  double max_power;

  /* access counts */
  counter_t raccs;
  counter_t waccs;
  counter_t raccs_this_cycle;
  counter_t waccs_this_cycle;
  counter_t rwaccs_this_cycle;
  counter_t acc_cycles;

#ifdef GET_OUT  
  counter_t popcount;
  counter_t popcount_cycles;
  unsigned int popcount_width;
#endif /* GET_OUT */
  
  /* energy totals for different clock gating schemes */

  double energy_cg_none;
#ifdef GET_OUT 
  double energy_cg_allornone;
  double energy_cg_byport;
#endif /* GET_OUT */
  double energy_cg_byportidle;
};

static struct power_struct_t power_structs[ps_NUM];
static struct power_struct_t power_bigstructs[pbs_NUM];

void
power_count_access(enum proc_struct_t ps,
		   bool_t write_f,
		   bool_t hard_count_f)
{
   struct power_struct_t *pws = &power_structs[ps];

   if (!f_power_sample)
      return;

   if (write_f)
     {
       pws->waccs++;
       if (!pws->waccs_this_cycle && !pws->rwaccs_this_cycle)
	 pws->acc_cycles++;
       
       if (pws->waccs_this_cycle < pws->wports && hard_count_f)
	 pws->waccs_this_cycle++;
       else if (pws->waccs_this_cycle == pws->wports && pws->rwaccs_this_cycle < pws->rwports && hard_count_f)
	 pws->rwaccs_this_cycle++;
       else if (hard_count_f)
	 panic("more %s write accesses than number of ports!", ps2str[ps]);
     }
   else
     {
       pws->raccs++;
       if (!pws->raccs_this_cycle && !pws->rwaccs_this_cycle)
	 pws->acc_cycles++;
       
       if (pws->raccs_this_cycle < pws->rports && hard_count_f)
	 pws->raccs_this_cycle++;
       else if (pws->raccs_this_cycle == pws->rports && pws->rwaccs_this_cycle < pws->rwports && hard_count_f)
	 pws->rwaccs_this_cycle++;
       else if (hard_count_f)
	 panic("more %s read accesses than number of ports!", ps2str[ps]);
     }
}

void
power_count_dynamic_af(enum proc_struct_t ps,
                       unsigned int pc,
                       unsigned int width)
{
#ifdef GET_OUT
   struct power_struct_t *pws = &power_structs[ps];
   pws->popcount = pc;
   pws->popcount_cycles++;
   pws->popcount_width = width;
#endif /* GET_OUT */
}

void
power_count_access_new_cycle(void)
{
   enum proc_struct_t ps;
   for (ps = 0; ps < ps_NUM; ps++)
     power_structs[ps].raccs_this_cycle = power_structs[ps].waccs_this_cycle = power_structs[ps].rwaccs_this_cycle = 0;
}

void
power_set_power_nports(enum proc_struct_t ps,
		       double r_power,
		       double w_power,
		       double max_power,
		       unsigned int rports,
		       unsigned int wports,
		       unsigned int rwports)
{
   struct power_struct_t *pws = &power_structs[ps];

   pws->r_power = r_power;
   pws->w_power = w_power;
   pws->max_power = max_power;
   pws->rports = rports;
   pws->wports = wports;
   pws->rwports = rwports;
}

#ifdef GET_OUT
double
power_single_access(enum proc_struct_t ps)
{
   return power_structs[ps].max_power / power_structs[ps].num_ports;
}

/* compute bitline activity factors which we use to scale bitline power
   Here it is very important whether we assume 0's or 1's are
   responsible for dissipating power in pre-charged stuctures. (since
   most of the bits are 0's, we assume the design is power-efficient
   enough to allow 0's to _not_ discharge 
*/
double
power_dynamic_af(enum proc_struct_t ps)
{
   struct power_struct_t *pws = &power_structs[ps];
   double avg_pop_count = (pws->popcount_cycles) ? ((double)pws->popcount / pws->popcount_cycles) : 0.0;
   return 1 - avg_pop_count / pws->popcount_width;
}
#endif /* GET_OUT */

/* compute power statistics on each cycle, for each conditional clocking style.  Obviously
most of the speed penalty comes here, so if you don't want per-cycle power estimates
you could post-process 
 
See README.wattch for details on the various clock gating styles.
 
*/
void power_stats_print(tick_t ncycle,
                       FILE *stream)
{
   struct power_struct_t *pwstnc = &power_structs[ps_TOTALNOCLOCK];
   struct power_struct_t *pwsc = &power_structs[ps_CLOCK];
   struct power_struct_t *pwst = &power_structs[ps_TOTAL];
   enum proc_struct_t ps;
   char buf[128], buf2[128];

   pwstnc->energy_cg_none = 0.0;
#ifdef GET_OUT
   pwstnc->energy_cg_allornone = 0.0;
   pwstnc->energy_cg_byport = 0.0;
#endif /* GET_OUT */
   pwstnc->energy_cg_byportidle = 0.0;

   for (ps = 0; ps < ps_TOTALNOCLOCK; ps++)
   {
      struct power_struct_t *pws = &power_structs[ps];

      if (pws->r_power == 0.0)
         continue;

      pws->energy_cg_none = 
	(tech_opt.standby) * (ncycle * pws->max_power) + 
	(1.0-tech_opt.standby) * ((pws->rports * pws->r_power) + (pws->wports * pws->w_power) + (pws->rwports * pws->r_power));
#ifdef GET_OUT
      pws->energy_cg_allornone = pws->acc_cycles * pws->max_power;
      pws->energy_cg_byport = (pws->raccs * pws->r_power) + (pws->waccs * pws->w_power);
      pws->energy_cg_byportidle = tech_opt.standby * pws->energy_cg_none + (1.0-tech_opt.standby) * pws->energy_cg_byport;
#endif /* GET_OUT */
      pws->energy_cg_byportidle = 
	(tech_opt.standby * ncycle * pws->max_power) + 
	(1.0-tech_opt.standby) * ((pws->raccs * pws->r_power) + (pws->waccs * pws->w_power));

      pwstnc->energy_cg_none += pws->energy_cg_none;
#ifdef GET_OUT
      pwstnc->energy_cg_allornone += pws->energy_cg_allornone;
      pwstnc->energy_cg_byport += pws->energy_cg_byport;
#endif /* GET_OUT */
      pwstnc->energy_cg_byportidle += pws->energy_cg_byportidle;
   }

   /* compute clock power */
   pwsc->energy_cg_none = tech_opt.standby * ncycle * pwsc->max_power + 
     (1.0-tech_opt.standby) * ((pwsc->rports * pwsc->r_power) + (pwsc->wports * pwsc->w_power) + (pwsc->rwports * pwsc->r_power));
#ifdef GET_OUT
   pwsc->energy_cg_allornone = ncycle * pwsc->max_power * (pwstnc->energy_cg_allornone / pwstnc->energy_cg_none);
   pwsc->energy_cg_byport = ncycle * pwsc->max_power * (pwstnc->energy_cg_byport / pwstnc->energy_cg_none);
#endif /* GET_OUT */
   pwsc->energy_cg_byportidle = pwsc->energy_cg_none * (pwstnc->energy_cg_byportidle / pwstnc->energy_cg_none);

   /* compute total power */
   pwst->energy_cg_none = pwstnc->energy_cg_none + pwsc->energy_cg_none;
#ifdef GET_OUT
   pwst->energy_cg_allornone = pwstnc->energy_cg_allornone + pwsc->energy_cg_allornone;
   pwst->energy_cg_byport = pwstnc->energy_cg_byport + pwsc->energy_cg_byport;
#endif /* GET_OUT */
   pwst->energy_cg_byportidle = pwstnc->energy_cg_byportidle + pwsc->energy_cg_byportidle;

#ifdef GET_OUT
   fprintf(stream, "----------------\n");
   print_rate(stream, "total_energy_cg_none", power_structs[ps_TOTAL].energy_cg_none, "");
   for (ps = 0; ps < ps_NUM; ps++)
   {
      struct power_struct_t *pws = &power_structs[ps];
      sprintf(buf, "avg_power_cg_none_%s", ps2str[ps]);
      sprintf(buf2, "per cycle %s power usage", ps2str[ps]);
      print_rate(stream, buf, pws->energy_cg_none/ncycle, buf2);
   }


   fprintf(stream, "----------------\n");
   print_rate(stream, "total_energy_cg_allornone", power_structs[ps_TOTAL].energy_cg_allornone, "");
   for (ps = 0; ps < ps_NUM; ps++)
   {
      struct power_struct_t *pws = &power_structs[ps];
      sprintf(buf, "avg_power_cg_allornone_%s", ps2str[ps]);
      sprintf(buf2, "per cycle %s power usage", ps2str[ps]);
      print_rate(stream, buf, pws->energy_cg_allornone/ncycle, buf2);
   }

   fprintf(stream, "----------------\n");
   print_rate(stream, "total_energy_cg_byport", power_structs[ps_TOTAL].energy_cg_byport, "");
   for (ps = 0; ps < ps_NUM; ps++)
   {
      struct power_struct_t *pws = &power_structs[ps];
      sprintf(buf, "avg_power_cg_byport_%s", ps2str[ps]);
      sprintf(buf2, "per cycle %s power usage", ps2str[ps]);
      print_rate(stream, buf, pws->energy_cg_byport/ncycle, buf2);
   }
#endif /* GET_OUT */
   fprintf(stream, "----------------\n");
   print_rate(stream, "total_energy_cg_byportidle", power_structs[ps_TOTAL].energy_cg_byportidle, "");
   for (ps = 0; ps < ps_NUM; ps++)
   {
      struct power_struct_t *pws = &power_structs[ps];
      if (pws->r_power == 0.0)
	continue;

      sprintf(buf, "avg_power_cg_byportidle_%s", ps2str[ps]);
      sprintf(buf2, "per cycle %s power usage", ps2str[ps]);
      print_rate(stream, buf, pws->energy_cg_byportidle/ncycle, buf2);
   }

   fprintf(stream, "----------------\n");
   for (ps = 0; ps < ps_NUM; ps++)
   {
      struct power_struct_t *pws = &power_structs[ps];
      if (pws->r_power == 0.0)
	continue;

      sprintf(buf, "total_read_access_%s", ps2str[ps]);
      sprintf(buf2, "total %s read accesses", ps2str[ps]);
      print_counter(stream, buf, pws->raccs, buf2);
   }

   fprintf(stream, "----------------\n");
   for (ps = 0; ps < ps_NUM; ps++)
   {
      struct power_struct_t *pws = &power_structs[ps];
      if (pws->w_power == 0.0)
	continue;

      sprintf(buf, "total_write_access_%s", ps2str[ps]);
      sprintf(buf2, "total %s write accesses", ps2str[ps]);
      print_counter(stream, buf, pws->waccs, buf2);
   }

   fprintf(stream, "----------------\n");
}


/* this routine takes the number of rows and cols of an array structure
   and attemps to make it make it more of a reasonable circuit structure
   by trying to make the number of rows and cols as close as possible.
   (scaling both by factors of 2 in opposite directions).  it returns
   a scale factor which is the amount that the rows should be divided
   by and the columns should be multiplied by.
*/
unsigned int squarify(unsigned int rows,
                      unsigned int cols)
{
   unsigned int scale_factor = 1;

   if (rows == cols)
      return 1;

   while (rows > cols)
   {
      rows /= 2;
      cols *= 2;

      if (rows / 2 <= cols)
         return ((int)pow(2.0, (double)scale_factor));
      scale_factor++;
   }

   return 1;
}

void power_aux_config(FILE *stream)
{
   enum proc_bigstruct_t pbs;
   enum proc_struct_t ps;
#ifdef GET_OUT

   double ambient_power = 2.0;
#endif /* GET_OUT */

   fprintf(stderr ,"Fine-grain structure power consumption\n");
   for (ps = 0; ps < ps_NUM; ps++)
     {
       struct power_struct_t *pws = &power_structs[ps];
       if (pws->max_power == 0.0)
	 continue;
       
       fprintf(stderr, "fg_%s (W): %g (%.3g%%)\n",
	       ps2str[ps],
	       pws->max_power,
	       100 * pws->max_power / power_structs[ps_TOTAL].max_power);
     }

   for (pbs = 0; pbs < pbs_NUM; pbs++)
     power_bigstructs[pbs].max_power = 0;

   for (ps = 0; ps < ps_NUM; ps++)
     power_bigstructs[ps2pbs[ps]].max_power += power_structs[ps].max_power;

   fprintf(stderr ,"Coarse-grain structure power consumption\n");
   for (pbs = 0; pbs < pbs_NUM; pbs++)
     {
       struct power_struct_t *pws = &power_bigstructs[pbs];
       if (pws->max_power == 0.0)
	 continue;

       fprintf(stderr, "cg_%s (W): %g (%.3g%%)\n",
	       pbs2str[pbs],
	       pws->max_power,
	       100 * pws->max_power / power_bigstructs[pbs_TOTAL].max_power);
     }
       
   /* clock component   fudge component */
   /* the clock power is a linear scaling */
}

/*======================================================================*/



/*
 * This part of the code contains routines for each section as
 * described in the tech report.  See the tech report for more details
 * and explanations */

/*----------------------------------------------------------------------*/

double driver_size(double driving_cap, double desiredrisetime)
{
   double nsize, psize;
   double Rpdrive;

   Rpdrive = desiredrisetime / (driving_cap * log(VSINV) * -1.0);
   psize = restowidth(Rpdrive, PCH);
   nsize = restowidth(Rpdrive, NCH);
   if (psize > Wworddrivemax)
   {
      psize = Wworddrivemax;
   }

   if (psize < 4.0 * LSCALE)
      psize = 4.0 * LSCALE;

   return (psize);
}

/* Decoder delay:  (see section 6.1 of tech report) */

double array_decoder_power(unsigned int rows,
                           unsigned int cols,
                           unsigned int ports)
{
   double Ctotal = 0;
   double Ceq = 0;
   int numstack;
   int decode_bits = 0;
   double rowsb;

   rowsb = (double)rows;

   /* number of input bits to be decoded */
   decode_bits = ceil((logtwo(rowsb)));

   /* First stage: driving the decoders */

   /* This is the capacitance for driving one bit (and its complement).
      -There are #rowsb 3->8 decoders contributing gatecap.
      - 2.0 factor from 2 identical sets of drivers in parallel
   */
   Ceq = 2.0 * (draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1)) +
         gatecap(Wdec3to8n + Wdec3to8p, 10.0) * rowsb;

   /* There are ports * #decode_bits total */
   Ctotal += ports * decode_bits * Ceq;

   if (verbose)
      fprintf(stderr, "Decoder -- Driving decoders            == %g\n", .3*Ctotal*Powerfactor);

   /* second stage: driving a bunch of nor gates with a nand
      numstack is the size of the nor gates -- ie. a 7-128 decoder has
      3-input NAND followed by 3-input NOR  */

   numstack = ceil((1.0 / 3.0) * logtwo(rows));

   if (numstack <= 0)
      numstack = 1;
   if (numstack > 5)
      numstack = 5;

   /* There are #rowsb NOR gates being driven*/
   Ceq = (3.0 * draincap(Wdec3to8p, PCH, 1) + draincap(Wdec3to8n, NCH, 3) +
          gatecap(WdecNORn + WdecNORp, ((numstack * 40) + 20.0))) * rowsb;

   Ctotal += ports * Ceq;

   if (verbose)
      fprintf(stderr, "Decoder -- Driving nor w/ nand         == %g\n", .3*ports*Ceq*Powerfactor);

   /* Final stage: driving an inverter with the nor
      (inverter preceding wordline driver) -- wordline driver is in the next section*/

   Ceq = (gatecap(Wdecinvn + Wdecinvp, 20.0) +
          numstack * draincap(WdecNORn, NCH, 1) +
          draincap(WdecNORp, PCH, numstack));

   if (verbose)
      fprintf(stderr, "Decoder -- Driving inverter w/ nor     == %g\n", .3*ports*Ceq*Powerfactor);

   Ctotal += ports * Ceq;

   /* assume Activity Factor == .3  */

   return (.3*Ctotal*Powerfactor);
}

double array_rport_wordline_power(unsigned int rows, unsigned int cols,
				  double wordlinelength,
				  unsigned int rports)
{
   double Ctotal = 0;
   double Ceq = 0;
   double Cline = 0;
   double Cliner = 0;
   double desiredrisetime, psize, nsize;
   double colsb;

   colsb = (double)cols;

   /* Calculate size of wordline drivers assuming rise time == Period / 8
      - estimate cap on line 
      - compute min resistance to achieve this with RC 
      - compute width needed to achieve this resistance */

   desiredrisetime = Period / 16;
   Cline = (gatecappass(Wmemcellr, 1.0)) * colsb + wordlinelength * CM3metal;
   psize = driver_size(Cline, desiredrisetime);
   /* how do we want to do p-n ratioing? -- here we just assume the same ratio
      from an inverter pair  */
   nsize = psize * Wdecinvn / Wdecinvp;

   Ceq = draincap(Wdecinvn, NCH, 1) + draincap(Wdecinvp, PCH, 1) +
         gatecap(nsize + psize, 20.0);
   Ctotal += rports * Ceq;


   /* Compute caps of read wordline and write wordlines
      - wordline driver caps, given computed width from above
      - read wordlines have 1 nmos access tx, size ~4
      - write wordlines have 2 nmos access tx, size ~2
      - metal line cap
   */

   Cliner = (gatecappass(Wmemcellr, (BitWidth - 2 * Wmemcellr) / 2.0)) * colsb +
            wordlinelength * CM3metal +
            2.0 * (draincap(nsize, NCH, 1) + draincap(psize, PCH, 1));
   Ctotal += rports * Cliner;

   if (verbose)
   {
      fprintf(stderr, "Wordline Driver Sizes -- nsize == %f, psize == %f\n", nsize, psize);
      fprintf(stderr, "Wordline -- Inverter -> Driver         == %g\n", rports*Ceq*Powerfactor);
      fprintf(stderr, "Wordline -- Line                       == %g\n", 1e12*Cline);
      fprintf(stderr, "Wordline -- Line -- access -- gatecap  == %g\n", 1e12*colsb*2*gatecappass(Wmemcella, (BitWidth - 2*Wmemcella) / 2.0));
      fprintf(stderr, "Wordline -- Line -- driver -- draincap == %g\n", 1e12*draincap(nsize, NCH, 1) + draincap(psize, PCH, 1));
      fprintf(stderr, "Wordline -- Line -- metal              == %g\n", 1e12*wordlinelength*CM3metal);
   }

   /* AF == 1 assuming a different wordline is charged each cycle, but only
      1 wordline (per port) is actually used */
   return (Ctotal*Powerfactor);
}

double array_wport_wordline_power(unsigned int rows, unsigned int cols,
				  double wordlinelength,
				  unsigned int wports)
{
   double Ctotal = 0;
   double Ceq = 0;
   double Cline = 0;
   double Clinew = 0;
   double desiredrisetime, psize, nsize;
   double colsb;

   colsb = (double)cols;

   /* Calculate size of wordline drivers assuming rise time == Period / 8
      - estimate cap on line 
      - compute min resistance to achieve this with RC 
      - compute width needed to achieve this resistance */

   desiredrisetime = Period / 16;
   Cline = (gatecappass(Wmemcellr, 1.0)) * colsb + wordlinelength * CM3metal;
   psize = driver_size(Cline, desiredrisetime);
   /* how do we want to do p-n ratioing? -- here we just assume the same ratio
      from an inverter pair  */
   nsize = psize * Wdecinvn / Wdecinvp;

   Ceq = draincap(Wdecinvn, NCH, 1) + draincap(Wdecinvp, PCH, 1) +
         gatecap(nsize + psize, 20.0);
   Ctotal += wports * Ceq;

   /* Compute caps of read wordline and write wordlines
      - wordline driver caps, given computed width from above
      - read wordlines have 1 nmos access tx, size ~4
      - write wordlines have 2 nmos access tx, size ~2
      - metal line cap
   */

   Clinew = (2.0 * gatecappass(Wmemcellw, (BitWidth - 2 * Wmemcellw) / 2.0)) * colsb +
            wordlinelength * CM3metal +
            2.0 * (draincap(nsize, NCH, 1) + draincap(psize, PCH, 1));
   Ctotal += wports * Clinew;

   if (verbose)
   {
      fprintf(stderr, "Wordline Driver Sizes -- nsize == %f, psize == %f\n", nsize, psize);
      fprintf(stderr, "Wordline -- Inverter -> Driver         == %g\n", wports*Ceq*Powerfactor);
      fprintf(stderr, "Wordline -- Line                       == %g\n", 1e12*Cline);
      fprintf(stderr, "Wordline -- Line -- access -- gatecap  == %g\n", 1e12*colsb*2*gatecappass(Wmemcella, (BitWidth - 2*Wmemcella) / 2.0));
      fprintf(stderr, "Wordline -- Line -- driver -- draincap == %g\n", 1e12*draincap(nsize, NCH, 1) + draincap(psize, PCH, 1));
      fprintf(stderr, "Wordline -- Line -- metal              == %g\n", 1e12*wordlinelength*CM3metal);
   }

   /* AF == 1 assuming a different wordline is charged each cycle, but only
      1 wordline (per port) is actually used */
   return (Ctotal*Powerfactor);
}



double array_rport_bitline_power(unsigned int rows,
				 unsigned int cols,
				 double bitlinelength,
				 unsigned int rports,
				 bool_t cache)
{
   double Ctotal = 0;
   double Ccolmux = 0;
   double Cbitrowr = 0;
   double Cprerow = 0;
   double Cpregate = 0;
   double Cliner = 0;
   double rowsb;
   double colsb;

   double desiredrisetime, Cline, psize;

   rowsb = (double)rows;
   colsb = (double)cols;

   /* Draincaps of access tx's */

   Cbitrowr = draincap(Wmemcellr, NCH, 1);

   /* Cprerow -- precharge cap on the bitline
      -simple scheme to estimate size of pre-charge tx's in a similar fashion
       to wordline driver size estimation.
      -FIXME: it would be better to use precharge/keeper pairs, i've omitted this
       from this version because it couldn't autosize as easily.
   */

   desiredrisetime = Period / 8;

   Cline = rowsb * Cbitrowr + CM2metal * bitlinelength;
   psize = driver_size(Cline, desiredrisetime);
   /* compensate for not having an nmos pre-charging */
   psize = psize + psize * Wdecinvn / Wdecinvp;

   if (verbose)
      printf("Cprerow auto   == %g (psize == %g)\n", draincap(psize, PCH, 1), psize);

   Cprerow = draincap(psize, PCH, 1);

   /* Cpregate -- cap due to gatecap of precharge transistors -- tack this
      onto bitline cap, again this could have a keeper */
   Cpregate = 4.0 * gatecap(psize, 10.0);
   global_clockcap += rports * cols * 2.0 * Cpregate;

   /*
      reg files (cache==0) 
      => single ended bitlines (1 bitline/col)
      => AFs from pop_count
      caches (cache ==1)
      => double-ended bitlines (2 bitlines/col)
      => AFs = .5 (since one of the two bitlines is always charging/discharging)
   */

   if (!cache)
   {
      /* compute the total line cap for read/write bitlines */
      Cliner = rowsb * Cbitrowr + CM2metal * bitlinelength + Cprerow;

      /* Bitline inverters at the end of the bitlines (replaced w/ sense amps
         in cache styles) */
      Ccolmux = gatecap(MSCALE * (29.9 + 7.8), 0.0) + gatecap(MSCALE * (47.0 + 12.0), 0.0);
      Ctotal += rports * cols * (Cliner + Ccolmux + 2.0 * Cpregate);
   }
   else
   {
      Cliner = rowsb * Cbitrowr + CM2metal * bitlinelength + Cprerow + draincap(Wbitmuxn, NCH, 1);
      Ccolmux = (draincap(Wbitmuxn, NCH, 1)) + 2.0 * gatecap(WsenseQ1to4, 10.0);
      Ctotal += .5 * rports * 2.0 * cols * (Cliner + Ccolmux + 2.0 * Cpregate);
   }

   if (verbose)
   {
      fprintf(stderr, "Bitline -- Precharge                   == %g\n", 1e12*Cpregate);
      fprintf(stderr, "Bitline -- Line                        == %g\n", 1e12*Cliner);
      fprintf(stderr, "Bitline -- Line -- access draincap     == %g\n", 1e12*rowsb*Cbitrowr);
      fprintf(stderr, "Bitline -- Line -- precharge draincap  == %g\n", 1e12*Cprerow);
      fprintf(stderr, "Bitline -- Line -- metal               == %g\n", 1e12*bitlinelength*CM2metal);
      fprintf(stderr, "Bitline -- Colmux                      == %g\n", 1e12*Ccolmux);

      fprintf(stderr, "\n");
   }

   if (!cache)
     return (Ctotal*Powerfactor);
   else
     return (Ctotal*SensePowerfactor*.4);
   
}

double array_wport_bitline_power(unsigned int rows,
				 unsigned int cols,
				 double bitlinelength,
				 unsigned int wports,
				 bool_t cache)
{
   double Ctotal = 0;
   double Cbitroww = 0;
   double Cwritebitdrive = 0;
   double Clinew = 0;
   double rowsb;
   double colsb;

   double desiredrisetime, Cline, psize, nsize;

   rowsb = (double)rows;
   colsb = (double)cols;

   /* Draincaps of access tx's */
   Cbitroww = draincap(Wmemcellw, NCH, 1);
   desiredrisetime = Period / 8;

   /* Cwritebitdrive -- write bitline drivers are used instead of the precharge
      stuff for write bitlines
      - 2 inverter drivers within each driver pair */

   Cline = rowsb * Cbitroww + CM2metal * bitlinelength;
   psize = driver_size(Cline, desiredrisetime);
   nsize = psize * Wdecinvn / Wdecinvp;
   Cwritebitdrive = 2.0 * (draincap(psize, PCH, 1) + draincap(nsize, NCH, 1));

   /*
      reg files (cache==0) 
      => single ended bitlines (1 bitline/col)
      => AFs from pop_count
      caches (cache ==1)
      => double-ended bitlines (2 bitlines/col)
      => AFs = .5 (since one of the two bitlines is always charging/discharging)
   */

   if (!cache)
   {
      Clinew = rowsb * Cbitroww + CM2metal * bitlinelength + Cwritebitdrive;
      Ctotal += .3 * wports * cols * (Clinew + Cwritebitdrive);
   }
   else
   {
      Clinew = rowsb * Cbitroww + CM2metal * bitlinelength + Cwritebitdrive;
      Ctotal += .5 * wports * 2.0 * cols * (Clinew + Cwritebitdrive);
   }

   if (verbose)
   {
      fprintf(stderr, "Bitline -- Line                        == %g\n", 1e12*Clinew);
      fprintf(stderr, "Bitline -- Line -- metal               == %g\n", 1e12*bitlinelength*CM2metal);
      fprintf(stderr, "\n");
   }

   if (!cache)
      return (Ctotal*Powerfactor);
   else
      return (Ctotal*SensePowerfactor*.4);

}

/* estimate senseamp power dissipation in cache structures (Zyuban's method) */
double senseamp_power(unsigned int cols,
		      unsigned int rports)
{
   return ((double)cols * rports * tech_opt.vdd / 8 * .5e-3);
}

/* estimate comparator power consumption (this comparator is similar
   to the tag-match structure in a CAM */
double comparator_power(unsigned int compare_bits)
{
   double c1, c2;
   /* bottom part of comparator */
   c2 = (compare_bits) * (draincap(Wcompn, NCH, 1) + draincap(Wcompn, NCH, 2)) +
        draincap(Wevalinvp, PCH, 1) + draincap(Wevalinvn, NCH, 1);

   /* top part of comparator */
   c1 = (compare_bits) * (draincap(Wcompn, NCH, 1) + draincap(Wcompn, NCH, 2) +
                          draincap(Wcomppreequ, NCH, 1)) +
        gatecap(WdecNORn, 1.0) +
        gatecap(WdecNORp, 3.0);

   return (c1 + c2) * Powerfactor * AF;
}

double simple_array_power(unsigned int rows,
                          unsigned int cols,
                          unsigned int rports,
                          unsigned int wports,
                          bool_t cache,
			  double *r_power, /* per read power */
                          double *w_power) /* per write power */
{

   int ports = rports + wports;
   double wl_len = cols * (RegCellWidth + ((cache ? 2 : 1) * ports * BitlineSpacing));
   double bl_len = rows * (RegCellHeight + ports * WordlineSpacing);

   *r_power = array_decoder_power(rows, cols, rports);
   *r_power += array_rport_wordline_power(rows, cols, wl_len, rports);
   *r_power += array_rport_bitline_power(rows, cols, bl_len, rports, cache) * I_POPCOUNT_AF;
   if (cache) *r_power += senseamp_power(cols, rports);
   *r_power /= rports;

   *w_power = array_decoder_power(rows, cols, wports);
   *w_power += array_wport_bitline_power(rows, cols, bl_len, wports, cache);
   *w_power += array_wport_wordline_power(rows, cols, wl_len, rports) * I_POPCOUNT_AF;
   *w_power /= wports;
     
   return 0.0;
}

double array_power(unsigned int rows,
                   unsigned int cols,
                   unsigned int rports,
                   unsigned int wports,
                   bool_t cache,
		   double *r_power,
		   double *w_power)
{
   double sf = squarify(rows, cols);
   return simple_array_power(rows / sf, cols * sf, rports, wports, cache, r_power, w_power);
}

double cam_array_power(unsigned int rows,
		       unsigned int cols,
		       unsigned int rports,
		       unsigned int wports,
		       double *r_power,
		       double *w_power)
{
   double Ctotal, Ctlcap, Cblcap, Cwlcap, Cmlcap;
   double bitlinelength, wordlinelength;
   double nsize, psize;
   int ports;
   Ctotal = 0;

   ports = rports + wports;

   bitlinelength = rows * (CamCellHeight + ports * MatchlineSpacing);
   wordlinelength = cols * (CamCellWidth + ports * TaglineSpacing);

   /* Write ports */
   /* Compute bitline cap (for writing new tags) */
   Cblcap = Cmetal * bitlinelength + rows * draincap(Wmemcellr, NCH, 2);

   /* autosize wordline driver */
   psize = driver_size(Cmetal * wordlinelength + 2 * cols * gatecap(Wmemcellr, 2.0), Period / 8);
   nsize = psize * Wdecinvn / Wdecinvp;

   /* Compute wordline cap (for writing new tags) */
   Cwlcap = Cmetal * wordlinelength +
            draincap(nsize, NCH, 1) + draincap(psize, PCH, 1) +
            2 * cols * gatecap(Wmemcellr, 2.0);

   *w_power = (((cols * 2 * Cblcap) + (rows * Cwlcap)) * Powerfactor * AF) +
     array_decoder_power(rows, cols, wports) / wports;

   /* Read ports */
   Ctlcap = Cmetal * bitlinelength +
            rows * gatecappass(Wcomparen2, 2.0) +
            draincap(Wcompdrivern, NCH, 1) + draincap(Wcompdriverp, PCH, 1);

   Cmlcap = 2 * cols * draincap(Wcomparen1, NCH, 2) +
            Cmetal * wordlinelength + draincap(Wmatchpchg, NCH, 1) +
            gatecap(Wmatchinvn + Wmatchinvp, 10.0) +
            gatecap(Wmatchnandn + Wmatchnandp, 10.0);

   *r_power = ((cols * 2 * Ctlcap) + (rows * Cmlcap) + (2 * gatecap(Wmatchnorn + Wmatchnorp, 10.0))) * Powerfactor * AF;

   global_clockcap += rports * rows * gatecap(Wmatchpchg, 5.0);

   return 0.0;
}

/* very rough clock power estimates */
double clock_power(unsigned int npiperegs,
                   unsigned int nialu,
                   unsigned int nfpalu,
                   double die_length)
{

   double clocklinelength;
   double Cline, Cline2, Ctotal;
   double pipereg_clockcap = 0;
   double global_buffercap = 0;
   double Clockpower;

   /* Assume say 8 stages (kinda low now).
      FIXME: this could be a lot better; user could input
      number of pipestages, etc  */

   /* assume 8 pipe stages and try to estimate bits per pipe stage */
   /* pipe stage 0/1 */
   /* assume 50% extra in control signals (rule of thumb) */
   npiperegs = DIV_ROUND_UP(3 * npiperegs, 2);

   pipereg_clockcap = npiperegs * 4 * gatecap(10.0, 0);

   /* estimate based on 3% of die being in clock metal */
   Cline2 = Cmetal * (.03 * die_length * die_length / BitlineSpacing) * 1e6 * 1e6;

   /* another estimate */
   clocklinelength = die_length * (.5 + 4 * (.25 + 2 * (.25) + 4 * (.125)));
   Cline = 20 * Cmetal * (clocklinelength) * 1e6;
   global_buffercap = 12 * gatecap(1000.0, 10.0) + 16 * gatecap(200, 10.0) + 16 * 8 * 2 * gatecap(100.0, 10.00) + 2 * gatecap(.29 * 1e6, 10.0);
   /* global_clockcap is computed within each array structure for pre-charge tx's*/
   Ctotal = Cline + global_clockcap + pipereg_clockcap + global_buffercap;

   /* add I_ADD Clockcap and F_ADD Clockcap */
   Clockpower = Ctotal * Powerfactor + nialu * I_ADD_CLOCK + nfpalu * F_ADD_CLOCK;

   if (verbose)
   {
      fprintf(stderr, "Global Clock Power: %g\n", Clockpower);
      fprintf(stderr, " Global Metal Lines   (W): %g\n", Cline*Powerfactor);
      fprintf(stderr, " Global Metal Lines (3%%) (W): %g\n", Cline2*Powerfactor);
      fprintf(stderr, " Global Clock Buffers (W): %g\n", global_buffercap*Powerfactor);
      fprintf(stderr, " Global Clock Cap (Explicit) (W): %g\n", global_clockcap*Powerfactor + I_ADD_CLOCK + F_ADD_CLOCK);
      fprintf(stderr, " Global Clock Cap (Implicit) (W): %g\n", pipereg_clockcap*Powerfactor);
   }
   return (Clockpower);

}

double clock_power_factor(void)
{
   return 1 + (power_structs[ps_CLOCK].max_power / power_structs[ps_TOTALNOCLOCK].max_power);
}

void total_power(void)
{
   enum proc_struct_t ps;

   power_structs[ps_TOTALNOCLOCK].max_power = 0.0;
   for (ps = 0; ps < ps_TOTALNOCLOCK; ps++)
     power_structs[ps_TOTALNOCLOCK].max_power += power_structs[ps].max_power;

   power_structs[ps_TOTAL].max_power = power_structs[ps_TOTALNOCLOCK].max_power + power_structs[ps_CLOCK].max_power;
   power_structs[ps_TOTAL].rports = 1;
}

double resultbus_power(unsigned int npregs,
                       unsigned int nports,
                       unsigned int nalu,
                       unsigned int width)
{
   double Ctotal, Cline;

   double regfile_height = npregs * (RegCellHeight + WordlineSpacing * nports);

   /* assume num alu's == ialu  (FIXME: generate a more detailed result bus network model*/
   Cline = Cmetal * (regfile_height + .5 * nalu * 3200.0 * LSCALE);

   /* or use result bus length measured from 21264 die photo */
   /*  Cline = Cmetal * 3.3*1000;*/

   /* Assume ruu_issue_width result busses -- power can be scaled linearly
      for number of result busses (scale by writeback_access) */
   Ctotal = 2.0 * (width) * Cline;

#ifdef STATIC_AF
   return (Ctotal*Powerfactor*AF);
#else
   return (Ctotal*Powerfactor);
#endif

}

/* very rough global clock power estimates */
#ifdef GET_OUT
double global_clock_power(double die_length)
{

   double clocklinelength;
   double Cline, Cline2, Ctotal;
   double global_buffercap = 0;

   Cline2 = Cmetal * (.03 * die_length * die_length / BitlineSpacing) * 1e6 * 1e6;

   clocklinelength = die_length * (.5 + 4 * (.25 + 2 * (.25) + 4 * (.125)));
   Cline = 20 * Cmetal * (clocklinelength) * 1e6;
   global_buffercap = 12 * gatecap(1000.0, 10.0) + 16 * gatecap(200, 10.0) + 16 * 8 * 2 * gatecap(100.0, 10.00) + 2 * gatecap(.29 * 1e6, 10.0);
   Ctotal = Cline + global_buffercap;

   if (verbose)
   {
      fprintf(stderr, "Global Clock Power: %g\n", Ctotal*Powerfactor);
      fprintf(stderr, " Global Metal Lines   (W): %g\n", Cline*Powerfactor);
      fprintf(stderr, " Global Metal Lines (3%%) (W): %g\n", Cline2*Powerfactor);
      fprintf(stderr, " Global Clock Buffers (W): %g\n", global_buffercap*Powerfactor);
   }

   return (Ctotal*Powerfactor);
}
#endif /* GET_OUT */


double ialu_power(void)
{
   /* FIXME: ALU power is a simple constant, it would be better
      to include bit AFs and have different numbers for different
      types of operations */
   return I_ADD;
}

double falu_power(void)
{
   return F_ADD;
}

double
rat_array_power(unsigned int rows,
                unsigned int cols,
                unsigned int rports,
                unsigned int wports,
                double *r_power,
                double *w_power)
{
   /* RAT has shadow bits stored in each cell, this makes the
       cell size larger than normal array structures, so we must
       compute it here */
   int ports = wports + rports;
   double wl_len = cols * (RatCellWidth + ports * BitlineSpacing + RatShiftRegWidth * RatNumShift);
   double bl_len = rows * (RatCellHeight + ports * WordlineSpacing);

   *r_power = array_decoder_power(rows, cols, rports); 
   *r_power += array_rport_wordline_power(rows, cols, wl_len, rports);
   *r_power += array_rport_bitline_power(rows, cols, bl_len, rports, /* cache */FALSE) * I_POPCOUNT_AF;
   *r_power /= rports;

   *w_power = array_decoder_power(rows, cols, wports);
   *w_power += array_wport_wordline_power(rows, cols, wl_len, rports);
   *w_power += array_wport_bitline_power(rows, cols, bl_len, wports, /* cache? */FALSE) * I_POPCOUNT_AF;
   *w_power /= wports;
     
   return 0.0;
}

double arbiter_power(unsigned int lines)
{
   double Ctotal, Cor, Cpencode;
   int num_arbiter = 1;

   while (lines > 4)
   {
      lines = lines >> 2;
      num_arbiter += lines;
   }

   Cor = 4 * draincap(WSelORn, NCH, 1) + draincap(WSelORprequ, PCH, 1);

   Cpencode = draincap(WSelPn, NCH, 1) + draincap(WSelPp, PCH, 1) +
              2 * draincap(WSelPn, NCH, 1) + draincap(WSelPp, PCH, 2) +
              3 * draincap(WSelPn, NCH, 1) + draincap(WSelPp, PCH, 3) +
              4 * draincap(WSelPn, NCH, 1) + draincap(WSelPp, PCH, 4) +
              4 * gatecap(WSelEnn + WSelEnp, 20.0) +
              4 * draincap(WSelEnn, NCH, 1) + 4 * draincap(WSelEnp, PCH, 1);

   Ctotal = num_arbiter * (Cor + Cpencode);

   return (Ctotal*Powerfactor*AF);
}

void
cache_onebank_power(unsigned int nsets,
                    unsigned int assoc,
                    unsigned int dbits,
                    unsigned int tbits,
                    unsigned int nbanks,
                    unsigned int rwport,
                    unsigned int rport,
                    unsigned int wport,
                    unsigned int abits,
                    unsigned int obits,
		    unsigned int bbits,
                    struct objective_params_t *op,
		    double *power_tag_read,
		    double *power_tag_write,
		    double *power_data_read,
		    double *power_data_write)
{
   struct delay_power_result_t dpr;
   struct cache_params_t cp;
   struct subarray_params_t sap;
   struct area_result_t ar;

   cp.nsets = nsets;
   cp.assoc = assoc;
   cp.dbits = dbits;
   cp.tbits = tbits;
   cp.nbanks = nbanks;
   cp.rwport = rwport;
   cp.rport = cp.serport = rport;
   cp.wport = wport;
   cp.abits = obits;
   cp.obits = obits;
   cp.bbits = bbits;

   if (op == NULL)
   {
      cp.dweight = 0.4;
      cp.pweight = 0.4;
      cp.aweight = 0.2;
   }
   else
   {
      cp.dweight = op->delay_weight;
      cp.pweight = op->power_weight;
      cp.aweight = op->area_weight;
   }

   calc_delay_power_area(&cp, &tech_opt, &sap, &dpr, &ar);
   output_data(stderr, &cp, &tech_opt, &sap, &dpr, &ar);

   if (power_tag_read) *power_tag_read = dpr.total_power_read_tag * Powerfactor;
   if (power_tag_write) *power_tag_write = dpr.total_power_write_tag * Powerfactor;
   if (power_data_read) *power_data_read = dpr.total_power_read_data * Powerfactor;
   if (power_data_write) *power_data_write = dpr.total_power_write_data * Powerfactor;
}

