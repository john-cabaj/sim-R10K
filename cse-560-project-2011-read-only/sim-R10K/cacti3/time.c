/*------------------------------------------------------------
*                              CACTI 3.0
*               Copyright 2002 Compaq Computer Corporation
*                         All Rights Reserved
*
* Permission to use, copy, and modify this software and its documentation is
* hereby granted only under the following terms and conditions.  Both the
* above copyright notice and this permission notice must appear in all copies
* of the software, derivative works or modified versions, and any portions
* thereof, and both notices must appear in supporting documentation.
*
* Users of this software agree to the terms and conditions set forth herein,
* and hereby grant back to Compaq a non-exclusive, unrestricted, royalty-
* free right and license under any changes, enhancements or extensions
* made to the core functions of the software, including but not limited to
* those affording compatibility with other hardware or software
* environments, but excluding applications which incorporate this software.
* Users further agree to use their best efforts to return to Compaq any
* such changes, enhancements or extensions that they make and inform Compaq
* of noteworthy uses of this software.  Correspondence should be provided
* to Compaq at:
*
*                       Director of Licensing
*                       Western Research Laboratory
*                       Compaq Computer Corporation
*                       250 University Avenue
*                       Palo Alto, California  94301
*
* This software may be distributed (but not offered for sale or transferred
* for compensation) to third parties, provided such third parties agree to
* abide by the terms and conditions of this notice.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND COMPAQ COMPUTER CORP. DISCLAIMS ALL
* WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL COMPAQ COMPUTER
* CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
* DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
* PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
* ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
* SOFTWARE.
*------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "def.h"
#include "cacti_misc.h"
#include "cacti_params.h"

static double Cbitmetal;
static double Rwordmetal;
static double Rbitmetal;
static double FACwordmetal;
static double FACbitmetal;
static double FARwordmetal;
static double FARbitmetal;

double objective_function(const struct cache_params_t *cp,
                          const struct delay_power_result_t *dprp,
                          const struct area_result_t *arp)
{
   return
      pow(dprp->access_time, cp->dweight) *
      pow(dprp->total_power, cp->pweight) *
      pow(arp->total_area, cp->aweight);
}

int powers (int base, int n)
{
   int i, p;

   p = 1;
   for (i = 1; i <= n; ++i)
      p *= base;
   return p;
}

/*----------------------------------------------------------------------*/

double logtwo(double x)
{
   if (x <= 0)
      printf("%e\n", x);
   return ( (double) (log(x) / log(2.0)) );
}
/*----------------------------------------------------------------------*/
/* returns gate capacitance in Farads */
double gatecap(double width, /* gate width in um (length is Leff) */
               double wirelength) /* poly wire length going to gate in lambda */
{
   return (width*Leff*Cgate + wirelength*Cpolywire*Leff);
}

double gatecappass(width, wirelength) /* returns gate capacitance in Farads */
double width;           /* gate width in um (length is Leff) */
double wirelength;      /* poly wire length going to gate in lambda */
{
   return (width*Leff*Cgatepass + wirelength*Cpolywire*Leff);
}


/*----------------------------------------------------------------------*/

/* Routine for calculating drain capacitances.  The draincap routine
 * folds transistors larger than 10um */

/* returns drain cap in Farads */
double draincap(double width, /* um */
                bool_t nchannel, /* whether n or p-channel (boolean) */
                int stack)  /* number of transistors in series that are on */
{
   double Cdiffside, Cdiffarea, Coverlap, cap;

   Cdiffside = (nchannel) ? Cndiffside : Cpdiffside;
   Cdiffarea = (nchannel) ? Cndiffarea : Cpdiffarea;
   Coverlap = (nchannel) ? (Cndiffovlp + Cnoxideovlp) :
              (Cpdiffovlp + Cpoxideovlp);
   /* calculate directly-connected (non-stacked) capacitance */
   /* then add in capacitance due to stacking */
   if (width >= 10)
   {
      cap = 3.0 * Leff * width / 2.0 * Cdiffarea + 6.0 * Leff * Cdiffside +
            width * Coverlap;
      cap += (double)(stack - 1) * (Leff * width * Cdiffarea +
                                    4.0 * Leff * Cdiffside + 2.0 * width * Coverlap);
   }
   else
   {
      cap = 3.0 * Leff * width * Cdiffarea + (6.0 * Leff + width) * Cdiffside +
            width * Coverlap;
      cap += (double)(stack - 1) * (Leff * width * Cdiffarea +
                                    2.0 * Leff * Cdiffside + 2.0 * width * Coverlap);
   }
   return (cap);
}

/*----------------------------------------------------------------------*/

/* The following routines estimate the effective resistance of an
   on transistor as described in the tech report.  The first routine
   gives the "switching" resistance, and the second gives the 
   "full-on" resistance */

/* returns resistance in ohms */
double transresswitch(double width, /* um */
                      bool_t nchannel, /* whether n or p-channel (boolean) */
                      int stack)  /* number of transistors in series */
{
   double restrans;
   restrans = (nchannel) ? (Rnchannelstatic) :
              (Rpchannelstatic);
   /* calculate resistance of stack - assume all but switching trans
      have 0.8X the resistance since they are on throughout switching */
   return ((1.0 + ((stack - 1.0)*0.8))*restrans / width);
}

/*----------------------------------------------------------------------*/

double transreson(width, nchannel, stack)  /* returns resistance in ohms */
double width;           /* um */
int nchannel;           /* whether n or p-channel (boolean) */
int stack;              /* number of transistors in series */
{
   double restrans;
   restrans = (nchannel) ? Rnchannelon : Rpchannelon;

   /* calculate resistance of stack.  Unlike transres, we don't
      multiply the stacked transistors by 0.8 */
   return (stack*restrans / width);

}

/*----------------------------------------------------------------------*/

/* This routine operates in reverse: given a resistance, it finds
 * the transistor width that would have this R.  It is used in the
 * data wordline to estimate the wordline driver size. */

double restowidth(res, nchannel)  /* returns width in um */
double res;            /* resistance in ohms */
int nchannel;          /* whether N-channel or P-channel */
{
   double restrans;

   restrans = (nchannel) ? Rnchannelon : Rpchannelon;

   return (restrans / res);

}

/*----------------------------------------------------------------------*/

double horowitz(inputramptime, tf, vs1, vs2, rise)
double inputramptime,     /* input rise time */
tf,                /* time constant of gate */
vs1, vs2;          /* threshold voltages */
int rise;                /* whether INPUT rise or fall (boolean) */
{
   double a, b, td;

   a = inputramptime / tf;
   if (rise == RISE)
   {
      b = 0.5;
      td = tf * sqrt( log(vs1) * log(vs1) + 2 * a * b * (1.0 - vs1)) +
           tf * (log(vs1) - log(vs2));
   }
   else
   {
      b = 0.4;
      td = tf * sqrt( log(1.0 - vs1) * log(1.0 - vs1) + 2 * a * b * (vs1)) +
           tf * (log(1.0 - vs1) - log(1.0 - vs2));
   }
   return (td);
}



/*======================================================================*/



/*
 * This part of the code contains routines for each section as
 * described in the tech report.  See the tech report for more details
 * and explanations */

/*----------------------------------------------------------------------*/

void subbank_routing_length(const struct cache_params_t *cp,
                            const struct subarray_params_t *sap,
                            double *subbank_v,
                            double *subbank_h)
{
   unsigned int htree;
   unsigned int cols_data_subarray, rows_data_subarray, cols_tag_subarray, rows_tag_subarray;
   double inter_v, inter_h , sub_h = 0.0, sub_v = 0.0;
   double inter_subbanks;

   if (/* !fullyassoc */cp->nsets > 1)
   {
      cols_data_subarray = DIV_ROUND_UP((cp->dbits * cp->assoc * sap->Nspd), sap->Ndwl);
      rows_data_subarray = (cp->nsets * sap->Ndbl * sap->Nspd);

      if (sap->Ndwl * sap->Ndbl == 1)
      {
         sub_v = rows_data_subarray;
         sub_h = cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl == 2)
      {
         sub_v = rows_data_subarray;
         sub_h = 2 * cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl) * rows_data_subarray;
            sub_h = sqrt(sap->Ndwl * sap->Ndbl) * cols_data_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl / 2) * rows_data_subarray;
            sub_h = 2 * sqrt(sap->Ndwl * sap->Ndbl / 2) * cols_data_subarray;
         }
      }

      inter_v = sub_v;
      inter_h = sub_h;

      cols_tag_subarray = DIV_ROUND_UP((cp->tbits * cp->assoc * sap->Ntspd), sap->Ntwl);
      rows_tag_subarray = DIV_ROUND_UP(cp->nsets, sap->Ntbl * sap->Ntspd);

      if (sap->Ntwl * sap->Ntbl == 1)
      {
         sub_v = rows_tag_subarray;
         sub_h = cols_tag_subarray;
      }
      else if (sap->Ntwl * sap->Ntbl == 2)
      {
         sub_v = rows_tag_subarray;
         sub_h = 2 * cols_tag_subarray;
      }

      /* VLAD is this right? Shouldn't these be Ntbl instead of Ndbl? -AR */
      else if (sap->Ntwl * sap->Ntbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl) * rows_tag_subarray;
            sub_h = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl) * cols_tag_subarray;
         }
         else
         {
            sub_v = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl / 2) * rows_tag_subarray;
            sub_h = 2 * sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl / 2) * cols_tag_subarray;
         }
      }


      inter_v = MAX(sub_v, inter_v);
      inter_h += sub_h;

      sub_v = 0;
      sub_h = 0;

      if (cp->nbanks == 1 || cp->nbanks == 2)
      {
         sub_h = 0;
         sub_v = 0;
      }
      if (cp->nbanks == 4)
      {
         sub_h = 0;
         sub_v = inter_v;
      }

      inter_subbanks = cp->nbanks;

      while ((inter_subbanks > 2.0) && (cp->nbanks > 4.0))
      {

         sub_v += inter_v;
         sub_h += inter_h;

         inter_v = 2 * inter_v;
         inter_h = 2 * inter_h;
         inter_subbanks = inter_subbanks / 4.0;

         if (inter_subbanks == 4.0)
         {
            inter_h = 0;
         }

      }
      *subbank_v = sub_v;
      *subbank_h = sub_h;
   }
   else /* (nsets == 1) ? fully_assoc */
   {
#ifdef GET_OUT
      int rows_fa_subarray = DIV_ROUND_UP(cp->assoc, sap->Ndbl);
      int cols_fa_subarray = cp->dbits + cp->tbits;

      if (sap->Ndbl == 1)
      {
         sub_v = rows_fa_subarray;
         sub_h = cols_fa_subarray;
      }
      else if (sap->Ndbl == 2)
      {
         sub_v = rows_fa_subarray;
         sub_h = 2 * cols_fa_subarray;
      }
      else if (sap->Ndbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndbl) * rows_fa_subarray;
            sub_h = sqrt(sap->Ndbl) * cols_fa_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndbl / 2) * rows_fa_subarray;
            sub_h = 2 * sqrt(sap->Ndbl / 2) * cols_fa_subarray;
         }
      }
      inter_v = sub_v;
      inter_h = sub_h;

      sub_v = 0;
      sub_h = 0;

      if (cp->nbanks == 1.0 || cp->nbanks == 2.0)
      {
         sub_h = 0;
         sub_v = 0;
      }
      else if (cp->nbanks == 4.0)
      {
         sub_h = 0;
         sub_v = inter_v;
      }

      inter_subbanks = cp->nbanks;

      while ((inter_subbanks > 2.0) && (cp->nbanks > 4.0))
      {

         sub_v += inter_v;
         sub_h += inter_h;

         inter_v = 2 * inter_v;
         inter_h = 2 * inter_h;
         inter_subbanks = inter_subbanks / 4.0;

         if (inter_subbanks == 4.0)
         {
            inter_h = 0;
         }

      }
      *subbank_v = sub_v;
      *subbank_h = sub_h;
#endif /* GET_OUT */
   }
}

void data_subbank_dim(const struct cache_params_t *cp,
		      const struct subarray_params_t *sap,
		      double *subbank_h,
		      double *subbank_v)
{
   int htree;
   int cols_data_subarray, rows_data_subarray;
   double sub_h = 0.0, sub_v = 0.0;

   if (/* !fully_assoc */cp->nsets > 1 && cp->dbits > 0)
   {
      /* calculation of subbank dimensions */
      cols_data_subarray = DIV_ROUND_UP((cp->dbits * cp->assoc * sap->Nspd), sap->Ndwl);
      rows_data_subarray = DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd));

      if (sap->Ndwl * sap->Ndbl == 1)
      {
         sub_v = rows_data_subarray;
         sub_h = cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl == 2)
      {
         sub_v = rows_data_subarray;
         sub_h = 2 * cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl) * rows_data_subarray;
            sub_h = sqrt(sap->Ndwl * sap->Ndbl) * cols_data_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl / 2) * rows_data_subarray;
            sub_h = 2 * sqrt(sap->Ndwl * sap->Ndbl / 2) * cols_data_subarray;
         }
      }
      
      *subbank_h = sub_h;
      *subbank_h = sub_h;
   }
}

void tag_subbank_dim(const struct cache_params_t *cp,
		      const struct subarray_params_t *sap,
		      double *subbank_h,
		      double *subbank_v)
{

   int htree;
   int cols_tag_subarray, rows_tag_subarray;
   double sub_h = 0.0, sub_v = 0.0;

   if (cp->nsets > 1 && cp->tbits > 0)
     {
       rows_tag_subarray = DIV_ROUND_UP(cp->nsets, (sap->Ntbl * sap->Ntspd));
       cols_tag_subarray = DIV_ROUND_UP((cp->tbits * cp->assoc * sap->Ntspd), sap->Ntwl);
       
       if (sap->Ntwl * sap->Ntbl == 1)
	 {
	   sub_v = rows_tag_subarray;
	   sub_h = cols_tag_subarray;
	 }
       else if (sap->Ntwl * sap->Ntbl == 2)
	 {
	   sub_v = rows_tag_subarray;
	   sub_h = 2 * cols_tag_subarray;
	 }
       else if (sap->Ntwl * sap->Ntbl > 2)
	 {
	   htree = cacti_floor_log_base2(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl);
	   if (htree % 2 == 0)
	     {
	       sub_v = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl) * rows_tag_subarray;
	       sub_h = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl) * cols_tag_subarray;
	     }
	   else
	     {
	       sub_v = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl / 2) * rows_tag_subarray;
	       sub_h = 2 * sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl / 2) * cols_tag_subarray;
	     }
	 }

      *subbank_v = sub_v;
      *subbank_h = sub_h;
   }
}

void subbank_dim(const struct cache_params_t *cp,
                 const struct subarray_params_t *sap,
                 double *subbank_h,
                 double *subbank_v)
{
   int htree;
   int cols_data_subarray, rows_data_subarray, cols_tag_subarray, rows_tag_subarray;
   double sub_h = 0.0, sub_v = 0.0, inter_v, inter_h;

   if (/* !fully_assoc */cp->nsets > 1)
   {
      /* calculation of subbank dimensions */
      cols_data_subarray = DIV_ROUND_UP((cp->dbits * cp->assoc * sap->Nspd), sap->Ndwl);
      rows_data_subarray = DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd));

      if (sap->Ndwl * sap->Ndbl == 1)
      {
         sub_v = rows_data_subarray;
         sub_h = cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl == 2)
      {
         sub_v = rows_data_subarray;
         sub_h = 2 * cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl) * rows_data_subarray;
            sub_h = sqrt(sap->Ndwl * sap->Ndbl) * cols_data_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl / 2) * rows_data_subarray;
            sub_h = 2 * sqrt(sap->Ndwl * sap->Ndbl / 2) * cols_data_subarray;
         }
      }
      
      inter_v = sub_v;
      inter_h = sub_h;

      if (cp->tbits > 0)
	{

	  rows_tag_subarray = DIV_ROUND_UP(cp->nsets, (sap->Ntbl * sap->Ntspd));
	  cols_tag_subarray = DIV_ROUND_UP((cp->tbits * cp->assoc * sap->Ntspd), sap->Ntwl);
	  
	  if (sap->Ntwl * sap->Ntbl == 1)
	    {
	      sub_v = rows_tag_subarray;
	      sub_h = cols_tag_subarray;
	    }
	  else if (sap->Ntwl * sap->Ntbl == 2)
	    {
	      sub_v = rows_tag_subarray;
	      sub_h = 2 * cols_tag_subarray;
	    }
	  else if (sap->Ntwl * sap->Ntbl > 2)
	    {
	      htree = cacti_floor_log_base2(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl);
	      if (htree % 2 == 0)
		{
		  sub_v = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl) * rows_tag_subarray;
		  sub_h = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl) * cols_tag_subarray;
		}
	      else
		{
		  sub_v = sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl / 2) * rows_tag_subarray;
		  sub_h = 2 * sqrt(/* WAS: sap->Ndwl * sap->Ndbl */sap->Ntwl * sap->Ntbl / 2) * cols_tag_subarray;
		}
	    }

	  inter_v = MAX(sub_v, inter_v);
	  inter_h += sub_h;
	}

      *subbank_v = inter_v;
      *subbank_h = inter_h;
   }
#ifdef GET_OUT
   else
   {
      int rows_fa_subarray = DIV_ROUND_UP(cp->assoc, sap->Ndbl);
      int cols_fa_subarray = cp->dbits + cp->tbits;

      if (sap->Ndbl == 1)
      {
         sub_v = rows_fa_subarray;
         sub_h = cols_fa_subarray;
      }
      else if (sap->Ndbl == 2)
      {
         sub_v = rows_fa_subarray;
         sub_h = 2 * cols_fa_subarray;
      }

      if (sap->Ndbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndbl) * rows_fa_subarray;
            sub_h = sqrt(sap->Ndbl) * cols_fa_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndbl / 2) * rows_fa_subarray;
            sub_h = 2 * sqrt(sap->Ndbl / 2) * cols_fa_subarray;
         }
      }
      inter_v = sub_v;
      inter_h = sub_h;

      *subbank_v = inter_v;
      *subbank_h = inter_h;
   }
#endif /* GET_OUT */
}

void subbanks_routing_power(const struct cache_params_t *cp,
			    const struct subarray_params_t *sap,
                            double *subbank_h,
                            double *subbank_v,
                            double *power)
{
   double Ceq, Ceq_outdrv;
   int i, blocks, htree, subbank_mod;
   double wire_cap, wire_cap_outdrv, start_h, start_v, line_h, line_v;

   *power = 0.0;

   if (cp->nbanks == 1 || cp->nbanks == 2)
   {
      *power = 0.0;
   }
   else if (cp->nbanks == 4)
   {
      /* calculation of address routing power */
      wire_cap = Cbitmetal * (*subbank_v);
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

      /* calculation of out driver power */
      wire_cap_outdrv = Cbitmetal * (*subbank_v);
      Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
      *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
   }
   else if (cp->nbanks == 8)
   {
      wire_cap = Cbitmetal * (*subbank_v) + Cwordmetal * (*subbank_h);
      /* buffer stage */
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
      *power += 6 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
      *power += 4 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + (wire_cap - Cbitmetal * (*subbank_v));
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

      /* calculation of out driver power */
      wire_cap_outdrv = Cbitmetal * (*subbank_v) + Cwordmetal * (*subbank_h);
      Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
      *power += 6 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
      *power += 4 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + (wire_cap_outdrv - Cbitmetal * (*subbank_v));
      *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;

   }

   else if (cp->nbanks > 8)
   {
      blocks = DIV_ROUND_UP(cp->nbanks, 8);
      htree = cacti_floor_log_base2(blocks);
      if (htree % 2 == 0)
      {
         subbank_mod = htree;
         start_h = (*subbank_h * (powers(2, cacti_floor_log_base2(htree) + 1) - 1));
         start_v = *subbank_v;
      }
      else
      {
         subbank_mod = htree - 1;
         start_h = (*subbank_h * (powers(2, (htree + 1) / 2) - 1));
         start_v = *subbank_v;
      }

      if (subbank_mod == 0)
      {
         subbank_mod = 1;
      }

      line_v = start_v;
      line_h = start_h;

      for (i = 1;i <= blocks;i++)
      {
         wire_cap = line_v * Cbitmetal + line_h * Cwordmetal;

         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
         *power += 6 * cp->abits * Ceq * .5 * VddPow * VddPow;
         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
         *power += 4 * cp->abits * Ceq * .5 * VddPow * VddPow;
         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + (wire_cap - Cbitmetal * (*subbank_v));
         *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

         /* calculation of out driver power */
         wire_cap_outdrv = line_v * Cbitmetal + line_h * Cwordmetal;

         Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
         *power += 6 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
         Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
         *power += 4 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
         Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + (wire_cap_outdrv - Cbitmetal * (*subbank_v));
         *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;

         if (i % subbank_mod == 0)
	   line_v += 2 * (*subbank_v);
      }
   }
}

void data_subbank_routing_power(const struct cache_params_t *cp,
				const struct subarray_params_t *sap,
				double *subbank_h,
				double *subbank_v,
				double *power)
{
   double Ceq, Ceq_outdrv;
   int i, blocks, htree, subbank_mod;
   double wire_cap, wire_cap_outdrv, start_h, start_v, line_h, line_v;

   *power = 0.0;

   if (cp->nbanks == 1 || cp->nbanks == 2)
   {
      *power = 0.0;
   }
   else if (cp->nbanks == 4)
   {
      /* calculation of address routing power */
      wire_cap = Cbitmetal * (*subbank_v);
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

      /* calculation of out driver power */
      wire_cap_outdrv = Cbitmetal * (*subbank_v);
      Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
      *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
   }
   else if (cp->nbanks == 8)
   {
      wire_cap = Cbitmetal * (*subbank_v) + Cwordmetal * (*subbank_h);
      /* buffer stage */
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
      *power += 6 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
      *power += 4 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + (wire_cap - Cbitmetal * (*subbank_v));
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

      /* calculation of out driver power */
      wire_cap_outdrv = Cbitmetal * (*subbank_v) + Cwordmetal * (*subbank_h);
      Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
      *power += 6 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
      *power += 4 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + (wire_cap_outdrv - Cbitmetal * (*subbank_v));
      *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
   }

   else if (cp->nbanks > 8)
   {
      blocks = DIV_ROUND_UP(cp->nbanks, 8);
      htree = cacti_floor_log_base2(blocks);
      if (htree % 2 == 0)
      {
         subbank_mod = htree;
         start_h = (*subbank_h * (powers(2, cacti_floor_log_base2(htree) + 1) - 1));
         start_v = *subbank_v;
      }
      else
      {
         subbank_mod = htree - 1;
         start_h = (*subbank_h * (powers(2, (htree + 1) / 2) - 1));
         start_v = *subbank_v;
      }

      if (subbank_mod == 0)
      {
         subbank_mod = 1;
      }

      line_v = start_v;
      line_h = start_h;

      for (i = 1;i <= blocks;i++)
      {
         wire_cap = line_v * Cbitmetal + line_h * Cwordmetal;

         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
         *power += 6 * cp->abits * Ceq * .5 * VddPow * VddPow;
         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
         *power += 4 * cp->abits * Ceq * .5 * VddPow * VddPow;
         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + (wire_cap - Cbitmetal * (*subbank_v));
         *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

         /* calculation of out driver power */
         wire_cap_outdrv = line_v * Cbitmetal + line_h * Cwordmetal;

         Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
         *power += 6 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
         Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
         *power += 4 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;
         Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + (wire_cap_outdrv - Cbitmetal * (*subbank_v));
         *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;

         if (i % subbank_mod == 0)
	   line_v += 2 * (*subbank_v);
      }
   }
}

void tag_subbank_routing_power(const struct cache_params_t *cp,
			       const struct subarray_params_t *sap,
			       double *subbank_h,
			       double *subbank_v,
			       double *power)
{
   double Ceq, Ceq_outdrv;
   int i, blocks, htree, subbank_mod;
   double wire_cap, wire_cap_outdrv, start_h, start_v, line_h, line_v;

   *power = 0.0;

   if (cp->nbanks == 1 || cp->nbanks == 2)
   {
      *power = 0.0;
   }
   else if (cp->nbanks == 4)
   {
      /* calculation of address routing power */
      wire_cap = Cbitmetal * (*subbank_v);
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

      /* calculation of out driver power */
      wire_cap_outdrv = Cbitmetal * (*subbank_v);
      Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
      *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->tbits;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
   }
   else if (cp->nbanks == 8)
   {
      wire_cap = Cbitmetal * (*subbank_v) + Cwordmetal * (*subbank_h);
      /* buffer stage */
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
      *power += 6 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
      *power += 4 * cp->abits * Ceq * .5 * VddPow * VddPow;
      Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + (wire_cap - Cbitmetal * (*subbank_v));
      *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

      /* calculation of out driver power */
      wire_cap_outdrv = Cbitmetal * (*subbank_v) + Cwordmetal * (*subbank_h);
      Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
      *power += 6 * Ceq_outdrv * VddPow * VddPow * .5 * cp->tbits;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
      *power += 4 * Ceq_outdrv * VddPow * VddPow * .5 * cp->tbits;
      Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + (wire_cap_outdrv - Cbitmetal * (*subbank_v));
      *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->tbits;
   }

   else if (cp->nbanks > 8)
   {
      blocks = DIV_ROUND_UP(cp->nbanks, 8);
      htree = cacti_floor_log_base2(blocks);
      if (htree % 2 == 0)
      {
         subbank_mod = htree;
         start_h = (*subbank_h * (powers(2, cacti_floor_log_base2(htree) + 1) - 1));
         start_v = *subbank_v;
      }
      else
      {
         subbank_mod = htree - 1;
         start_h = (*subbank_h * (powers(2, (htree + 1) / 2) - 1));
         start_v = *subbank_v;
      }

      if (subbank_mod == 0)
      {
         subbank_mod = 1;
      }

      line_v = start_v;
      line_h = start_h;

      for (i = 1;i <= blocks;i++)
      {
         wire_cap = line_v * Cbitmetal + line_h * Cwordmetal;

         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + gatecap(Wdecdrivep + Wdecdriven, 0.0);
         *power += 6 * cp->abits * Ceq * .5 * VddPow * VddPow;
         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + wire_cap;
         *power += 4 * cp->abits * Ceq * .5 * VddPow * VddPow;
         Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) + (wire_cap - Cbitmetal * (*subbank_v));
         *power += 2 * cp->abits * Ceq * .5 * VddPow * VddPow;

         /* calculation of out driver power */
         wire_cap_outdrv = line_v * Cbitmetal + line_h * Cwordmetal;

         Ceq_outdrv = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) + gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
         *power += 6 * Ceq_outdrv * VddPow * VddPow * .5 * cp->tbits;
         Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + wire_cap_outdrv;
         *power += 4 * Ceq_outdrv * VddPow * VddPow * .5 * cp->tbits;
         Ceq_outdrv = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) + (wire_cap_outdrv - Cbitmetal * (*subbank_v));
         *power += 2 * Ceq_outdrv * VddPow * VddPow * .5 * cp->tbits;

         if (i % subbank_mod == 0)
	   line_v += 2 * (*subbank_v);
      }
   }
}

double address_routing_delay(const struct cache_params_t *cp,
                             const struct subarray_params_t *sap,
                             double *outrisetime,
                             double *power)
{
   double Ceq, tf, nextinputtime, delay_stage1, delay_stage2;
   double addr_h, addr_v;
   double wire_cap, wire_res;

   /* Calculate rise time.  Consider two inverters */

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   nextinputtime = horowitz(0.0, tf, VTHINV360x240, VTHINV360x240, FALL) /
                   (VTHINV360x240);

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   nextinputtime = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240,
                            RISE) /
                   (1.0 - VTHINV360x240);
   addr_h = 0;
   addr_v = 0;
   subbank_routing_length(cp, sap, &addr_v, &addr_h);
   wire_cap = Cbitmetal * addr_v + addr_h * Cwordmetal;
   wire_res = (Rwordmetal * addr_h + Rbitmetal * addr_v) / 2.0;

   /* buffer stage */

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   delay_stage1 = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240, FALL);
   nextinputtime = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240, FALL) / (VTHINV360x240);
   *power = cp->abits * Ceq * .5 * VddPow * VddPow;

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         wire_cap;
   tf = Ceq * (transreson(Wdecdriven, NCH, 1) + wire_res);
   delay_stage2 = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240, RISE);
   nextinputtime = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240, RISE) / (1.0 - VTHINV360x240);
   *power += cp->abits * Ceq * .5 * VddPow * VddPow;
   *outrisetime = nextinputtime;
   return (delay_stage1 + delay_stage2);
}



/* Decoder delay:  (see section 6.1 of tech report) */

double decoder_delay(const struct cache_params_t *cp,
                     const struct subarray_params_t *sap,
                     double *Tdecdrive,
                     double *Tdecoder1,
                     double *Tdecoder2,
                     double inrisetime,
                     double *outrisetime,
                     unsigned int *nor_inputs,
                     double *power)
{
   int numstack;
   double Ceq, Req, Rwire, rows, cols, tf, nextinputtime, vth;
   double l_inv_predecode, l_predec_nor_v = 0.0, l_predec_nor_h = 0.0;
   double wire_cap = 0.0, wire_res = 0.0, total_edge_length;
   int htree, exp;

   int addr_bits = cacti_ceil_log_base2(DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd)));
   /* Calculate rise time.  Consider two inverters */

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   nextinputtime = horowitz(0.0, tf, VTHINV360x240, VTHINV360x240, FALL) /
                   (VTHINV360x240);
   // power+=Ceq*.5*VddPow;

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   nextinputtime = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240,
                            RISE) /
                   (1.0 - VTHINV360x240);
   // power+=Ceq*.5*VddPow;

   /* First stage: driving the decoders */

   rows = DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd * /* AMIR: what is this 8 doing here? */8));
   cols = DIV_ROUND_UP((cp->dbits * cp->assoc * sap->Nspd), sap->Ndwl);
   total_edge_length = cp->dbits * cp->assoc * sap->Ndbl * sap->Nspd;

   if (sap->Ndwl * sap->Ndbl == 1)
   {
      l_inv_predecode = 1;
      wire_cap = Cwordmetal * l_inv_predecode * total_edge_length;
      wire_res = 0.5 * Rwordmetal * l_inv_predecode * total_edge_length;
   }
   else if (sap->Ndwl * sap->Ndbl == 2 )
   {
      l_inv_predecode = 0.5;
      wire_cap = Cwordmetal * l_inv_predecode * total_edge_length;
      wire_res = 0.5 * Rwordmetal * l_inv_predecode * total_edge_length;
   }
   else if (sap->Ndwl * sap->Ndbl > 2)
   {
      htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
      if (htree % 2 == 0)
      {
         exp = (htree / 2 - 1);
         l_inv_predecode = 0.25 / (powers(2, exp));
         wire_cap = Cwordmetal * l_inv_predecode * total_edge_length;
         wire_res = 0.5 * Rwordmetal * l_inv_predecode * total_edge_length;
      }
      else
      {
         exp = (htree - 3) / 2;
         l_inv_predecode = powers(2, exp);
         wire_cap = Cbitmetal * l_inv_predecode * rows * 8;
         wire_res = 0.5 * Rbitmetal * l_inv_predecode * rows * 8;
      }
   }

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         4 * gatecap(Wdec3to8n + Wdec3to8p, 10.0) * (sap->Ndwl * sap->Ndbl)
         + wire_cap;
   Rwire = wire_res;

   tf = (Rwire + transreson(Wdecdrivep, PCH, 1)) * Ceq;
   *Tdecdrive = horowitz(inrisetime, tf, VTHINV360x240, VTHNAND60x120,
                         FALL);

   nextinputtime = *Tdecdrive / VTHNAND60x120;
   *power += addr_bits * Ceq * .5 * VddPow * VddPow;

   /* second stage: driving a bunch of nor gates with a nand */

   numstack = ceil(0.33 * cacti_floor_log_base2(DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd))));
   if (numstack == 0)
      numstack = 1;
   if (numstack > 5)
      numstack = 5;

   if (sap->Ndwl * sap->Ndbl == 1 || sap->Ndwl * sap->Ndbl == 2 || sap->Ndwl * sap->Ndbl == 4)
   {
      l_predec_nor_v = 0;
      l_predec_nor_h = 0;
   }
   else if (sap->Ndwl * sap->Ndbl == 8)
   {
      l_predec_nor_v = 0;
      l_predec_nor_h = cols;
   }
   else if (sap->Ndwl * sap->Ndbl > 8)
   {
      htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
      if (htree % 2 == 0)
      {
         exp = (htree / 2 - 1);
         l_predec_nor_v = (powers(2, exp) - 1) * rows * 8;
         l_predec_nor_h = (powers(2, exp) - 1) * cols;
      }
      else
      {
         exp = (htree - 3) / 2;
         l_predec_nor_v = (powers(2, exp) - 1) * rows * 8;
         l_predec_nor_h = (powers(2, (exp + 1)) - 1) * cols;
      }
   }

   Ceq = 3 * draincap(Wdec3to8p, PCH, 1) + draincap(Wdec3to8n, NCH, 3) +
         gatecap(WdecNORn + WdecNORp, ((numstack * 40) + 20.0)) * rows +
         Cbitmetal * (rows * 8 + l_predec_nor_v) + Cwordmetal * (l_predec_nor_h);
   Rwire = Rbitmetal * (rows * 8 + l_predec_nor_v) / 2 + Rwordmetal * (l_predec_nor_h) / 2;

   tf = Ceq * (Rwire + transreson(Wdec3to8n, NCH, 3));
   *power += sap->Ndwl * sap->Ndbl * Ceq * VddPow * VddPow * 4 * ceil(0.33 * cacti_floor_log_base2(DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd))));

   /* we only want to charge the output to the threshold of the
      nor gate.  But the threshold depends on the number of inputs
      to the nor.  */

   switch (numstack)
   {
   case 1:
      vth = VTHNOR12x4x1;
      break;
   case 2:
      vth = VTHNOR12x4x2;
      break;
   case 3:
      vth = VTHNOR12x4x3;
      break;
   case 4:
      vth = VTHNOR12x4x4;
      break;
   case 5:
      vth = VTHNOR12x4x4;
      break;
   default:
      printf("error:numstack=%d\n", numstack);
      printf("Cacti does not support a series stack of %d transistors !\n", numstack);
      exit(0);
      break;
   }
   *Tdecoder1 = horowitz(nextinputtime, tf, VTHNAND60x120, vth, RISE);
   nextinputtime = *Tdecoder1 / (1.0 - vth);

   /* Final stage: driving an inverter with the nor */

   Req = transreson(WdecNORp, PCH, numstack);
   Ceq = (gatecap(Wdecinvn + Wdecinvp, 20.0) +
          numstack * draincap(WdecNORn, NCH, 1) +
          draincap(WdecNORp, PCH, numstack));
   tf = Req * Ceq;
   *Tdecoder2 = horowitz(nextinputtime, tf, vth, VSINV, FALL);

   *outrisetime = *Tdecoder2 / (VSINV);
   *nor_inputs = numstack;
   *power += Ceq * VddPow * VddPow;

   //printf("%g %g %g %d %d %d\n",*Tdecdrive,*Tdecoder1,*Tdecoder2,Ndwl, Ndbl,Nspd);

   //fprintf(stderr, "%f %f %f %f %d %d %d\n", (*Tdecdrive+*Tdecoder1+*Tdecoder2)*1e9, *Tdecdrive*1e9, *Tdecoder1*1e9, *Tdecoder2*1e9, Ndwl, Ndbl, Nspd);
   return (*Tdecdrive + *Tdecoder1 + *Tdecoder2);
}



/*----------------------------------------------------------------------*/

/* Decoder delay in the tag array (see section 6.1 of tech report) */


double decoder_tag_delay(const struct cache_params_t *cp,
                         const struct subarray_params_t *sap,
                         double *Tdecdrive,
                         double *Tdecoder1,
                         double *Tdecoder2,
                         double inrisetime,
                         double *outrisetime,
                         int *nor_inputs,
                         double *power)
{
   double Ceq, Req, Rwire, rows, cols, tf, nextinputtime, vth;
   int numstack, htree, exp;
   double l_inv_predecode, l_predec_nor_v = 0.0, l_predec_nor_h = 0.0;
   double wire_cap = 0.0, wire_res = 0.0, total_edge_length;

   int addr_bits = cacti_ceil_log_base2(DIV_ROUND_UP(cp->nsets, (sap->Ntbl * sap->Ntspd)));

   /* Calculate rise time.  Consider two inverters */

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   nextinputtime = horowitz(0.0, tf, VTHINV360x240, VTHINV360x240, FALL) /
                   (VTHINV360x240);

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   nextinputtime = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240,
                            RISE) /
                   (1.0 - VTHINV360x240);

   /* First stage: driving the decoders */

   rows = DIV_ROUND_UP(cp->nsets, 8 * sap->Ntbl * sap->Ntspd);
   cols = DIV_ROUND_UP(cp->tbits * cp->assoc * sap->Ntspd, sap->Ntwl);
   total_edge_length = cols * sap->Ntwl * sap->Ntbl;

   if (sap->Ntwl * sap->Ntbl == 1)
   {
      l_inv_predecode = 1;
      wire_cap = Cwordmetal * l_inv_predecode * total_edge_length;
      wire_res = 0.5 * Rwordmetal * l_inv_predecode * total_edge_length;

   }
   else if (sap->Ntwl * sap->Ntbl == 2)
   {
      l_inv_predecode = 0.5;
      wire_cap = Cwordmetal * l_inv_predecode * total_edge_length;
      wire_res = 0.5 * Rwordmetal * l_inv_predecode * total_edge_length;
   }
   else if (sap->Ntwl * sap->Ntbl > 2)
   {
      htree = cacti_floor_log_base2(sap->Ntwl * sap->Ntbl);
      if (htree % 2 == 0)
      {
         exp = (htree / 2 - 1);
         l_inv_predecode = 0.25 / (powers(2, exp));
         wire_cap = Cwordmetal * l_inv_predecode * total_edge_length;
         wire_res = 0.5 * Rwordmetal * l_inv_predecode * total_edge_length;
      }
      else
      {
         exp = (htree - 3) / 2;
         l_inv_predecode = powers(2, exp);
         wire_cap = Cbitmetal * l_inv_predecode * rows * 8;
         wire_res = 0.5 * Rbitmetal * l_inv_predecode * rows * 8;
      }
   }

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         4 * gatecap(Wdec3to8n + Wdec3to8p, 10.0) * (sap->Ntwl * sap->Ntbl) +
         wire_cap;
   Rwire = wire_res;

   tf = (Rwire + transreson(Wdecdrivep, PCH, 1)) * Ceq;
   *Tdecdrive = horowitz(inrisetime, tf, VTHINV360x240, VTHNAND60x120,
                         FALL);
   nextinputtime = *Tdecdrive / VTHNAND60x120;
   *power += addr_bits * Ceq * .5 * VddPow * VddPow;

   /* second stage: driving a bunch of nor gates with a nand */

   numstack = ceil(0.33 * cacti_ceil_log_base2(DIV_ROUND_UP(cp->nsets, sap->Ntbl * sap->Ntspd)));
   if (numstack == 0)
      numstack = 1;
   if (numstack > 5)
      numstack = 5;

   if (sap->Ntwl * sap->Ntbl == 1 || sap->Ntwl * sap->Ntbl == 2 || sap->Ntwl * sap->Ntbl == 4)
   {
      l_predec_nor_v = 0;
      l_predec_nor_h = 0;
   }
   else if (sap->Ntwl* sap->Ntbl == 8)
   {
      l_predec_nor_v = 0;
      l_predec_nor_h = cols;
   }

   else if (sap->Ntwl * sap->Ntbl > 8)
   {
      htree = cacti_floor_log_base2(sap->Ntwl * sap->Ntbl);
      if (htree % 2 == 0)
      {
         exp = (htree / 2 - 1);
         l_predec_nor_v = (powers(2, exp) - 1) * rows * 8;
         l_predec_nor_h = (powers(2, exp) - 1) * cols;
      }
      else
      {
         exp = (htree - 3) / 2;
         l_predec_nor_v = (powers(2, exp) - 1) * rows * 8;
         l_predec_nor_h = (powers(2, (exp + 1)) - 1) * cols;
      }
   }

   Ceq = 3 * draincap(Wdec3to8p, PCH, 1) + draincap(Wdec3to8n, NCH, 3) +
         gatecap(WdecNORn + WdecNORp, ((numstack * 40) + 20.0)) * rows +
         Cbitmetal * (rows * 8 + l_predec_nor_v) + Cwordmetal * (l_predec_nor_h);
   Rwire = Rbitmetal * (rows * 8 + l_predec_nor_v) / 2 + Rwordmetal * (l_predec_nor_h) / 2;

   tf = Ceq * (Rwire + transreson(Wdec3to8n, NCH, 3));
   *power += sap->Ntwl * sap->Ntbl * Ceq * VddPow * VddPow * 4 * ceil(0.33 * cacti_ceil_log_base2(DIV_ROUND_UP(cp->nsets, sap->Ntbl * sap->Ntspd)));

   /* we only want to charge the output to the threshold of the
      nor gate.  But the threshold depends on the number of inputs
      to the nor.  */

   switch (numstack)
   {
   case 1:
      vth = VTHNOR12x4x1;
      break;
   case 2:
      vth = VTHNOR12x4x2;
      break;
   case 3:
      vth = VTHNOR12x4x3;
      break;
   case 4:
      vth = VTHNOR12x4x4;
      break;
   case 5:
      vth = VTHNOR12x4x4;
      break;
   case 6:
      vth = VTHNOR12x4x4;
      break;
   default:
      printf("error:numstack=%d\n", numstack);
      printf("Cacti does not support a series stack of %d transistors !\n", numstack);
      exit(0);
      break;

   }

   *Tdecoder1 = horowitz(nextinputtime, tf, VTHNAND60x120, vth, RISE);
   nextinputtime = *Tdecoder1 / (1.0 - vth);

   /* Final stage: driving an inverter with the nor */

   Req = transreson(WdecNORp, PCH, numstack);
   Ceq = (gatecap(Wdecinvn + Wdecinvp, 20.0) +
          numstack * draincap(WdecNORn, NCH, 1) +
          draincap(WdecNORp, PCH, numstack));
   tf = Req * Ceq;
   *Tdecoder2 = horowitz(nextinputtime, tf, vth, VSINV, FALL);
   *outrisetime = *Tdecoder2 / (VSINV);


   *nor_inputs = numstack;
   *power += Ceq * VddPow * VddPow;

   return (*Tdecdrive + *Tdecoder1 + *Tdecoder2);
}

double fa_tag_delay(const struct cache_params_t *cp,
                    const struct subarray_params_t *sap,
                    double *Tagdrive,
                    double *Tag1,
                    double *Tag2,
                    double *Tag3,
                    double *Tag4,
                    double *Tag5,
                    double *outrisetime,
                    int *nor_inputs,
                    double *power)
{
   double Ceq, Req, Rwire, rows, tf, nextinputtime;
   double Tagdrive1 = 0, Tagdrive2 = 0;
   double Tagdrive0a = 0, Tagdrive0b = 0;
   double TagdriveA = 0, TagdriveB = 0;
   double TagdriveA1 = 0, TagdriveB1 = 0;
   double TagdriveA2 = 0, TagdriveB2 = 0;

   rows = DIV_ROUND_UP(cp->assoc, sap->Ntbl);

   /* Calculate rise time.  Consider two inverters */

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdriven, NCH, 1);
   nextinputtime = horowitz(0.0, tf, VTHINV360x240, VTHINV360x240, FALL) /
                   (VTHINV360x240);

   Ceq = draincap(Wdecdrivep, PCH, 1) + draincap(Wdecdriven, NCH, 1) +
         gatecap(Wdecdrivep + Wdecdriven, 0.0);
   tf = Ceq * transreson(Wdecdrivep, PCH, 1);
   nextinputtime = horowitz(nextinputtime, tf, VTHINV360x240, VTHINV360x240,
                            RISE) / (1.0 - VTHINV360x240);

   // If tag bitline divisions, add extra driver

   if (sap->Ntbl > 1)
   {
      Ceq = draincap(Wfadrivep, PCH, 1) + draincap(Wfadriven, NCH, 1) +
            gatecap(Wfadrive2p + Wfadrive2n, 0.0);
      tf = Ceq * transreson(Wfadriven, NCH, 1);
      TagdriveA = horowitz(nextinputtime, tf, VSINV, VSINV, FALL);
      nextinputtime = TagdriveA / (VSINV);
      *power += .5 * Ceq * VddPow * VddPow * cp->abits;

      if (sap->Ntbl <= 4)
      {
         Ceq = draincap(Wfadrive2p, PCH, 1) + draincap(Wfadrive2n, NCH, 1) +
               gatecap(Wfadecdrive1p + Wfadecdrive1n, 10.0) * 2 +
               + FACwordmetal * sqrt((rows + 1) * sap->Ntbl) / 2
               + FACbitmetal * sqrt((rows + 1) * sap->Ntbl) / 2;
         Rwire = FARwordmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 2 +
                 FARbitmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 2;
         tf = Ceq * (transreson(Wfadrive2p, PCH, 1) + Rwire);
         TagdriveB = horowitz(nextinputtime, tf, VSINV, VSINV, RISE);
         nextinputtime = TagdriveB / (1.0 - VSINV);
         *power += Ceq * VddPow * VddPow * cp->abits * .5 * 2;
      }
      else
      {
         Ceq = draincap(Wfadrive2p, PCH, 1) + draincap(Wfadrive2n, NCH, 1) +
               gatecap(Wfadrivep + Wfadriven, 10.0) * 2 +
               + FACwordmetal * sqrt((rows + 1) * sap->Ntbl) / 2
               + FACbitmetal * sqrt((rows + 1) * sap->Ntbl) / 2;
         Rwire = FARwordmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 2 +
                 FARbitmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 2;
         tf = Ceq * (transreson(Wfadrive2p, PCH, 1) + Rwire);
         TagdriveB = horowitz(nextinputtime, tf, VSINV, VSINV, RISE);
         nextinputtime = TagdriveB / (1.0 - VSINV);
         *power += Ceq * VddPow * VddPow * cp->abits * .5 * 4;

         Ceq = draincap(Wfadrivep, PCH, 1) + draincap(Wfadriven, NCH, 1) +
               gatecap(Wfadrive2p + Wfadrive2n, 10.0);
         tf = Ceq * transreson(Wfadriven, NCH, 1);
         TagdriveA1 = horowitz(nextinputtime, tf, VSINV, VSINV, FALL);
         nextinputtime = TagdriveA1 / (VSINV);
         *power += .5 * Ceq * VddPow * VddPow * cp->abits;

         if (sap->Ntbl <= 16)
         {
            Ceq = draincap(Wfadrive2p, PCH, 1) + draincap(Wfadrive2n, NCH, 1) +
                  gatecap(Wfadecdrive1p + Wfadecdrive1n, 10.0) * 2 +
                  + FACwordmetal * sqrt((rows + 1) * sap->Ntbl) / 4
                  + FACbitmetal * sqrt((rows + 1) * sap->Ntbl) / 4;
            Rwire = FARwordmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 4 +
                    FARbitmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 4;
            tf = Ceq * (transreson(Wfadrive2p, PCH, 1) + Rwire);
            TagdriveB1 = horowitz(nextinputtime, tf, VSINV, VSINV, RISE);
            nextinputtime = TagdriveB1 / (1.0 - VSINV);
            *power += Ceq * VddPow * VddPow * cp->abits * .5 * 8;
         }
         else
         {
            Ceq = draincap(Wfadrive2p, PCH, 1) + draincap(Wfadrive2n, NCH, 1) +
                  gatecap(Wfadrivep + Wfadriven, 10.0) * 2 +
                  + FACwordmetal * sqrt((rows + 1) * sap->Ntbl) / 4
                  + FACbitmetal * sqrt((rows + 1) * sap->Ntbl) / 4;
            Rwire = FARwordmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 4 +
                    FARbitmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 4;
            tf = Ceq * (transreson(Wfadrive2p, PCH, 1) + Rwire);
            TagdriveB1 = horowitz(nextinputtime, tf, VSINV, VSINV, RISE);
            nextinputtime = TagdriveB1 / (1.0 - VSINV);
            *power += Ceq * VddPow * VddPow * cp->abits * .5 * 8;

            Ceq = draincap(Wfadrivep, PCH, 1) + draincap(Wfadriven, NCH, 1) +
                  gatecap(Wfadrive2p + Wfadrive2n, 10.0);
            tf = Ceq * transreson(Wfadriven, NCH, 1);
            TagdriveA2 = horowitz(nextinputtime, tf, VSINV, VSINV, FALL);
            nextinputtime = TagdriveA2 / (VSINV);
            *power += .5 * Ceq * VddPow * VddPow * cp->abits;

            Ceq = draincap(Wfadrive2p, PCH, 1) + draincap(Wfadrive2n, NCH, 1) +
                  gatecap(Wfadecdrive1p + Wfadecdrive1n, 10.0) * 2 +
                  + FACwordmetal * sqrt((rows + 1) * sap->Ntbl) / 8
                  + FACbitmetal * sqrt((rows + 1) * sap->Ntbl) / 8;
            Rwire = FARwordmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 8 +
                    FARbitmetal * sqrt((rows + 1) * sap->Ntbl) * .5 / 8;
            tf = Ceq * (transreson(Wfadrive2p, PCH, 1) + Rwire);
            TagdriveB2 = horowitz(nextinputtime, tf, VSINV, VSINV, RISE);
            nextinputtime = TagdriveB2 / (1.0 - VSINV);
            *power += Ceq * VddPow * VddPow * cp->abits * .5 * 16;
         }
      }
   }

   /* Two more inverters for enable delay */

   Ceq = draincap(Wfadecdrive1p, PCH, 1) + draincap(Wfadecdrive1n, NCH, 1)
         + gatecap(Wfadecdrive2p + Wfadecdrive2n, 0.0);
   tf = Ceq * transreson(Wfadecdrive1n, NCH, 1);
   Tagdrive0a = horowitz(nextinputtime, tf, VSINV, VSINV,
                         FALL);
   nextinputtime = Tagdrive0a / (VSINV);
   *power += .5 * Ceq * VddPow * VddPow * cp->abits * sap->Ntbl;

   Ceq = draincap(Wfadecdrive2p, PCH, 1) + draincap(Wfadecdrive2n, NCH, 1) +
         + gatecap(Wfadecdrivep + Wfadecdriven, 10.0)
         + gatecap(Wfadecdrive2p + Wfadecdrive2n, 10.0);
   tf = Ceq * transreson(Wfadecdrive2p, PCH, 1);
   Tagdrive0b = horowitz(nextinputtime, tf, VSINV, VSINV,
                         RISE);
   nextinputtime = Tagdrive0b / (VSINV);
   *power += Ceq * VddPow * VddPow * cp->abits * .5 * sap->Ntbl;

   /* First stage */

   Ceq = draincap(Wfadecdrive2p, PCH, 1) + draincap(Wfadecdrive2n, NCH, 1) +
         gatecap(Wfadecdrivep + Wfadecdriven, 10.0);
   tf = Ceq * transresswitch(Wfadecdrive2n, NCH, 1);
   Tagdrive1 = horowitz(nextinputtime, tf, VSINV, VTHFA1, FALL);
   nextinputtime = Tagdrive1 / VTHFA1;
   *power += Ceq * VddPow * VddPow * cp->abits * .5 * sap->Ntbl;

   Ceq = draincap(Wfadecdrivep, PCH, 2) + draincap(Wfadecdriven, NCH, 2)
         + draincap(Wfaprechn, NCH, 1)
         + gatecap(Wdummyn, 10.0) * (rows + 1)
         + FACbitmetal * (rows + 1);

   Rwire = FARbitmetal * (rows + 1) * .5;
   tf = (Rwire + transreson(Wfadecdrivep, PCH, 1) +
         transresswitch(Wfadecdrivep, PCH, 1)) * Ceq;
   Tagdrive2 = horowitz(nextinputtime, tf, VTHFA1, VTHFA2,
                        RISE);
   nextinputtime = Tagdrive2 / (1 - VTHFA2);

   *Tagdrive = Tagdrive1 + Tagdrive2 + TagdriveA + TagdriveB + TagdriveA1 + TagdriveA2 + TagdriveB1 + TagdriveB2 + Tagdrive0a + Tagdrive0b;
   *power += Ceq * VddPow * VddPow * cp->abits * sap->Ntbl;

   /* second stage */

   Ceq = .5 * cp->abits * 2 * draincap(Wdummyn, NCH, 2)
         + draincap(Wfaprechp, PCH, 1)
         + gatecap(Waddrnandn + Waddrnandp, 10.0)
         + FACwordmetal * cp->abits;
   Rwire = FARwordmetal * cp->abits * .5;
   tf = Ceq * (Rwire + transreson(Wdummyn, NCH, 1) + transreson(Wdummyn, NCH, 1));
   *Tag1 = horowitz(nextinputtime, tf, VTHFA2, VTHFA3, FALL);
   nextinputtime = *Tag1 / VTHFA3;
   *power += Ceq * VddPow * VddPow * rows * sap->Ntbl;

   /* third stage */

   Ceq = draincap(Waddrnandn, NCH, 2)
         + 2 * draincap(Waddrnandp, PCH, 1)
         + gatecap(Wdummyinvn + Wdummyinvp, 10.0);
   tf = Ceq * (transresswitch(Waddrnandp, PCH, 1));
   *Tag2 = horowitz(nextinputtime, tf, VTHFA3, VTHFA4, RISE);
   nextinputtime = *Tag2 / (1 - VTHFA4);
   *power += Ceq * VddPow * VddPow * rows * sap->Ntbl;

   /* fourth stage */

   Ceq = (rows) * (gatecap(Wfanorn + Wfanorp, 10.0))
         + draincap(Wdummyinvn, NCH, 1)
         + draincap(Wdummyinvp, PCH, 1)
         + FACbitmetal * rows;
   Rwire = FARbitmetal * rows * .5;
   Req = Rwire + transreson(Wdummyinvn, NCH, 1);
   tf = Req * Ceq;
   *Tag3 = horowitz(nextinputtime, tf, VTHFA4, VTHFA5, FALL);
   *outrisetime = *Tag3 / VTHFA5;
   *power += Ceq * VddPow * VddPow * sap->Ntbl;

   /* fifth stage */

   Ceq = draincap(Wfanorp, PCH, 2)
         + 2 * draincap(Wfanorn, NCH, 1)
         + gatecap(Wfainvn + Wfainvp, 10.0);
   tf = Ceq * (transresswitch(Wfanorp, PCH, 1) + transreson(Wfanorp, PCH, 1));
   *Tag4 = horowitz(nextinputtime, tf, VTHFA5, VTHFA6, RISE);
   nextinputtime = *Tag4 / (1 - VTHFA6);
   *power += Ceq * VddPow * VddPow;

   /* final stage */

   Ceq = (gatecap(Wdecinvn + Wdecinvp, 20.0) +
          + draincap(Wfainvn, NCH, 1)
          + draincap(Wfainvp, PCH, 1));
   Req = transresswitch(Wfainvn, NCH, 1);
   tf = Req * Ceq;
   *Tag5 = horowitz(nextinputtime, tf, VTHFA6, VSINV, FALL);
   *outrisetime = *Tag5 / VSINV;
   *power += Ceq * VddPow * VddPow;

   // if (Ntbl==32)
   //   fprintf(stderr," 1st - %f %f\n 2nd - %f %f\n 3rd - %f %f\n 4th - %f %f\n 5th - %f %f\nPD : %f\nNAND : %f\n INV : %f\n NOR : %f\n INV : %f\n", TagdriveA*1e9, TagdriveB*1e9, TagdriveA1*1e9, TagdriveB1*1e9, TagdriveA2*1e9, TagdriveB2*1e9, Tagdrive0a*1e9,Tagdrive0b*1e9,Tagdrive1*1e9, Tagdrive2*1e9, *Tag1*1e9, *Tag2*1e9, *Tag3*1e9, *Tag4*1e9, *Tag5*1e9);
   return (*Tagdrive + *Tag1 + *Tag2 + *Tag3 + *Tag4 + *Tag5);
}


/*----------------------------------------------------------------------*/

/* Data array wordline delay (see section 6.2 of tech report) */


double wordline_delay(const struct cache_params_t *cp,
                      const struct subarray_params_t *sap,
                      double inrisetime,
                      double *outrisetime,
                      double *power)
{
   double Rpdrive;
   double desiredrisetime, psize, nsize;
   double tf, nextinputtime, Ceq, Rline, Cline;
   int cols;
   double Tworddrivedel, Twordchargedel;

   cols = DIV_ROUND_UP(cp->dbits * cp->assoc * sap->Nspd, sap->Ndwl);

   /* Choose a transistor size that makes sense */
   /* Use a first-order approx */

   desiredrisetime = krise * log((double)(cols)) / 2.0;
   Cline = (gatecappass(Wmemcella, 0.0) +
            gatecappass(Wmemcella, 0.0) +
            Cwordmetal) * cols;
   Rpdrive = desiredrisetime / (Cline * log(VSINV) * -1.0);
   psize = restowidth(Rpdrive, PCH);
   if (psize > Wworddrivemax)
      psize = Wworddrivemax;

   /* Now that we have a reasonable psize, do the rest as before */
   /* If we keep the ratio the same as the tag wordline driver,
      the threshold voltage will be close to VSINV */

   nsize = psize * Wdecinvn / Wdecinvp;

   Ceq = draincap(Wdecinvn, NCH, 1) + draincap(Wdecinvp, PCH, 1) +
         gatecap(nsize + psize, 20.0);
   tf = transreson(Wdecinvp, PCH, 1) * Ceq;
   *power += Ceq * VddPow * VddPow;

   Tworddrivedel = horowitz(inrisetime, tf, VSINV, VSINV, RISE);
   nextinputtime = Tworddrivedel / (1.0 - VSINV);

   Cline = (gatecappass(Wmemcella, (BitWidth - 2 * Wmemcella) / 2.0) +
            gatecappass(Wmemcella, (BitWidth - 2 * Wmemcella) / 2.0) +
            Cwordmetal) * cols +
           draincap(nsize, NCH, 1) + draincap(psize, PCH, 1);
   Rline = Rwordmetal * cols / 2;
   tf = (transreson(psize, PCH, 1) + Rline) * Cline;
   Twordchargedel = horowitz(nextinputtime, tf, VSINV, VSINV, FALL);
   *outrisetime = Twordchargedel / VSINV;
   *power += Cline * VddPow * VddPow;

   // fprintf(stderr, "%d %f %f\n", cols, Tworddrivedel*1e9, Twordchargedel*1e9);

   return (Tworddrivedel + Twordchargedel);
}


/*----------------------------------------------------------------------*/

/* Tag array wordline delay (see section 6.3 of tech report) */


double wordline_tag_delay(const struct cache_params_t *cp,
                          const struct subarray_params_t *sap,
                          double inrisetime,
                          double *outrisetime,
                          double *power)
{
   double tf;
   double Cline, Rline, Ceq, nextinputtime;
   double Tworddrivedel, Twordchargedel;

   /* number of tag bits */

   /* first stage */

   Ceq = draincap(Wdecinvn, NCH, 1) + draincap(Wdecinvp, PCH, 1) +
         gatecap(Wdecinvn + Wdecinvp, 20.0);
   tf = transreson(Wdecinvn, NCH, 1) * Ceq;

   Tworddrivedel = horowitz(inrisetime, tf, VSINV, VSINV, RISE);
   nextinputtime = Tworddrivedel / (1.0 - VSINV);
   *power += Ceq * VddPow * VddPow;

   /* second stage */
   Cline = (gatecappass(Wmemcella, (BitWidth - 2 * Wmemcella) / 2.0) +
            gatecappass(Wmemcella, (BitWidth - 2 * Wmemcella) / 2.0) +
            Cwordmetal) * DIV_ROUND_UP(cp->tbits * cp->assoc * sap->Ntspd, sap->Ntwl) +
           draincap(Wdecinvn, NCH, 1) + draincap(Wdecinvp, PCH, 1);
   Rline = Rwordmetal * DIV_ROUND_UP(cp->tbits * cp->assoc * sap->Ntspd, 2 * sap->Ntwl);
   tf = (transreson(Wdecinvp, PCH, 1) + Rline) * Cline;
   Twordchargedel = horowitz(nextinputtime, tf, VSINV, VSINV, FALL);
   *outrisetime = Twordchargedel / VSINV;
   *power += Cline * VddPow * VddPow;
   return (Tworddrivedel + Twordchargedel);

}

/*----------------------------------------------------------------------*/

/* Data array bitline: (see section 6.4 in tech report) */


double bitline_delay(const struct cache_params_t *cp,
                     const struct subarray_params_t *sap,
                     double inrisetime,
                     double *outrisetime,
                     double *power)
{
   double Tbit, Cline, Ccolmux, Rlineb, r1, r2, c1, c2, a, b, c;
   double m, tstep;
   double Cbitrow;    /* bitline capacitance due to access transistor */
   int rows, cols;
   int muxway;

   Cbitrow = draincap(Wmemcella, NCH, 1) / 2.0; /* due to shared contact */
   rows = DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd));
   cols = DIV_ROUND_UP((cp->dbits * cp->assoc * sap->Nspd), sap->Ndwl);
   if (DIV_ROUND_UP(cp->dbits, MAX(cp->obits, cp->bbits)) == 1 && sap->Ndbl * sap->Nspd == 1)
   {
      Cline = rows * (Cbitrow + Cbitmetal) + 2 * draincap(Wbitpreequ, PCH, 1);
      Ccolmux = 2 * gatecap(WsenseQ1to4, 10.0);
      Rlineb = Rbitmetal * rows / 2.0;
      r1 = Rlineb;
      //muxover=1;
   }
   else
   {
      if (sap->Ndbl * sap->Nspd > MAX_COL_MUX)
      {
         //muxover=8*B/cp->obits;
         muxway = MAX_COL_MUX;
      }
      else if (DIV_ROUND_UP(cp->dbits * sap->Ndbl * sap->Nspd, MAX(cp->obits, cp->bbits)) > MAX_COL_MUX)
      {
         muxway = MAX_COL_MUX;
         // muxover=(8*B/cp->obits)/(MAX_COL_MUX/(Ndbl*Nspd));
      }
      else
      {
         muxway = DIV_ROUND_UP(cp->dbits * sap->Nspd * sap->Ndbl, MAX(cp->obits, cp->bbits));
         // muxover=1;
      }

      Cline = rows * (Cbitrow + Cbitmetal) + 2 * draincap(Wbitpreequ, PCH, 1) +
              draincap(Wbitmuxn, NCH, 1);
      Ccolmux = muxway * (draincap(Wbitmuxn, NCH, 1)) + 2 * gatecap(WsenseQ1to4, 10.0);
      Rlineb = Rbitmetal * rows / 2.0;
      r1 = Rlineb +
           transreson(Wbitmuxn, NCH, 1);
   }
   r2 = transreson(Wmemcella, NCH, 1) +
        transreson(Wmemcella * Wmemcellbscale, NCH, 1);
   c1 = Ccolmux;
   c2 = Cline;
   *power += c1 * VddPow * VddPow * cp->obits * cp->assoc * sap->muxover;
   *power += c2 * VddPow * VbitprePow * cols;


   //fprintf(stderr, "Pow %f %f\n", c1*VddPow*VddPow*cp->obits*A*muxover*1e9, c2*VddPow*VbitprePow*cols*1e9);
   tstep = (r2 * c2 + (r1 + r2) * c1) * log((Vbitpre) / (Vbitpre - Vbitsense));

   /* take input rise time into account */

   m = Vdd / inrisetime;
   if (tstep <= (0.5*(Vdd - Vt) / m))
   {
      a = m;
      b = 2 * ((Vdd * 0.5) - Vt);
      c = -2 * tstep * (Vdd - Vt) + 1 / m * ((Vdd * 0.5) - Vt) *
          ((Vdd * 0.5) - Vt);
      Tbit = ( -b + sqrt(b * b - 4 * a * c)) / (2 * a);
   }
   else
   {
      Tbit = tstep + (Vdd + Vt) / (2 * m) - (Vdd * 0.5) / m;
   }

   *outrisetime = Tbit / (log((Vbitpre - Vbitsense) / Vdd));
   return (Tbit);
}

/*----------------------------------------------------------------------*/

/* Tag array bitline: (see section 6.4 in tech report) */


double bitline_tag_delay(const struct cache_params_t *cp,
                         const struct subarray_params_t *sap,
                         double inrisetime,
                         double *outrisetime,
                         double *power)
{
   double Tbit, Cline, Ccolmux, Rlineb, r1, r2, c1, c2, a, b, c;
   double m, tstep;
   double Cbitrow;    /* bitline capacitance due to access transistor */
   int rows, cols;

   Cbitrow = draincap(Wmemcella, NCH, 1) / 2.0; /* due to shared contact */
   rows = DIV_ROUND_UP(cp->nsets, (sap->Ntbl * sap->Ntspd));
   cols = DIV_ROUND_UP((cp->tbits * cp->assoc * sap->Ntspd), sap->Ntwl);

   if (sap->Ntbl * sap->Ntspd == 1)
   {
      Cline = rows * (Cbitrow + Cbitmetal) + 2 * draincap(Wbitpreequ, PCH, 1);
      Ccolmux = 2 * gatecap(WsenseQ1to4, 10.0);
      Rlineb = Rbitmetal * rows / 2.0;
      r1 = Rlineb;
   }
   else
   {
      Cline = rows * (Cbitrow + Cbitmetal) + 2 * draincap(Wbitpreequ, PCH, 1) +
              draincap(Wbitmuxn, NCH, 1);
      Ccolmux = sap->Ntspd * sap->Ntbl * (draincap(Wbitmuxn, NCH, 1)) + 2 * gatecap(WsenseQ1to4, 10.0);
      Rlineb = Rbitmetal * rows / 2.0;
      r1 = Rlineb +
           transreson(Wbitmuxn, NCH, 1);
   }
   r2 = transreson(Wmemcella, NCH, 1) +
        transreson(Wmemcella * Wmemcellbscale, NCH, 1);

   c1 = Ccolmux;
   c2 = Cline;
   *power += c1 * VddPow * VddPow;
   *power += c2 * VddPow * VbitprePow * cols;

   //fprintf(stderr, "Pow %f %f\n", c1*VddPow*VddPow*1e9, c2*VddPow*VbitprePow*cols*1e9);

   tstep = (r2 * c2 + (r1 + r2) * c1) * log((Vbitpre) / (Vbitpre - Vbitsense));

   /* take into account input rise time */

   m = Vdd / inrisetime;
   if (tstep <= (0.5*(Vdd - Vt) / m))
   {
      a = m;
      b = 2 * ((Vdd * 0.5) - Vt);
      c = -2 * tstep * (Vdd - Vt) + 1 / m * ((Vdd * 0.5) - Vt) *
          ((Vdd * 0.5) - Vt);
      Tbit = ( -b + sqrt(b * b - 4 * a * c)) / (2 * a);
   }
   else
   {
      Tbit = tstep + (Vdd + Vt) / (2 * m) - (Vdd * 0.5) / m;
   }

   *outrisetime = Tbit / (log((Vbitpre - Vbitsense) / Vdd));
   return (Tbit);
}



/*----------------------------------------------------------------------*/

/* It is assumed the sense amps have a constant delay
   (see section 6.5) */

double sense_amp_delay(double inrisetime,
                       double *outrisetime,
                       unsigned int rows,
                       double *power)
{
   *outrisetime = tfalldata;
   *power *= psensedata;
   return (tsensedata + 2.5e-10);
}

/*--------------------------------------------------------------*/

double sense_amp_tag_delay(double inrisetime,
                           double *outrisetime,
                           double *power)
{
   *outrisetime = tfalltag;
   *power *= psensetag;
   return (tsensetag + 2.5e-10);
}

/*----------------------------------------------------------------------*/

/* Comparator Delay (see section 6.6) */


double compare_time(const struct cache_params_t *cp,
                    const struct subarray_params_t *sap,
                    double inputtime,
                    double *outputtime)
{
   double Req, Ceq, tf, st1del, st2del, st3del, nextinputtime, m;
   double c1, c2, r1, r2, tstep, a, b, c;
   double Tcomparatorni;
   int cols;

   /* First Inverter */

   Ceq = gatecap(Wcompinvn2 + Wcompinvp2, 10.0) +
         draincap(Wcompinvp1, PCH, 1) + draincap(Wcompinvn1, NCH, 1);
   Req = transreson(Wcompinvp1, PCH, 1);
   tf = Req * Ceq;
   st1del = horowitz(inputtime, tf, VTHCOMPINV, VTHCOMPINV, FALL);
   nextinputtime = st1del / VTHCOMPINV;

   /* Second Inverter */

   Ceq = gatecap(Wcompinvn3 + Wcompinvp3, 10.0) +
         draincap(Wcompinvp2, PCH, 1) + draincap(Wcompinvn2, NCH, 1);
   Req = transreson(Wcompinvn2, NCH, 1);
   tf = Req * Ceq;
   st2del = horowitz(inputtime, tf, VTHCOMPINV, VTHCOMPINV, RISE);
   nextinputtime = st1del / (1.0 - VTHCOMPINV);

   /* Third Inverter */

   Ceq = gatecap(Wevalinvn + Wevalinvp, 10.0) +
         draincap(Wcompinvp3, PCH, 1) + draincap(Wcompinvn3, NCH, 1);
   Req = transreson(Wcompinvp3, PCH, 1);
   tf = Req * Ceq;
   st3del = horowitz(nextinputtime, tf, VTHCOMPINV, VTHEVALINV, FALL);
   nextinputtime = st1del / (VTHEVALINV);

   /* Final Inverter (virtual ground driver) discharging compare part */

   cols = cp->tbits * sap->Ntbl * sap->Ntspd;

   r1 = transreson(Wcompn, NCH, 2);
   r2 = transreson(Wevalinvn, NCH, 1);  /* was switch */
   c2 = (cp->tbits) * (draincap(Wcompn, NCH, 1) + draincap(Wcompn, NCH, 2)) +
        draincap(Wevalinvp, PCH, 1) + draincap(Wevalinvn, NCH, 1);
   c1 = (cp->tbits) * (draincap(Wcompn, NCH, 1) + draincap(Wcompn, NCH, 2))
        + draincap(Wcompp, PCH, 1) + gatecap(Wmuxdrv12n + Wmuxdrv12p, 20.0) +
        cols * Cwordmetal;

   /* time to go to threshold of mux driver */

   tstep = (r2 * c2 + (r1 + r2) * c1) * log(1.0 / VTHMUXDRV1);

   /* take into account non-zero input rise time */

   m = Vdd / nextinputtime;

   if ((tstep) <= (0.5*(Vdd - Vt) / m))
   {
      a = m;
      b = 2 * ((Vdd * VTHEVALINV) - Vt);
      c = -2 * (tstep) * (Vdd - Vt) + 1 / m * ((Vdd * VTHEVALINV) - Vt) * ((Vdd * VTHEVALINV) - Vt);
      Tcomparatorni = ( -b + sqrt(b * b - 4 * a * c)) / (2 * a);
   }
   else
   {
      Tcomparatorni = (tstep) + (Vdd + Vt) / (2 * m) - (Vdd * VTHEVALINV) / m;
   }
   *outputtime = Tcomparatorni / (1.0 - VTHMUXDRV1);

   return (Tcomparatorni + st1del + st2del + st3del);
}

double half_compare_time(const struct cache_params_t *cp,
                         const struct subarray_params_t *sap,
                         double inputtime,
                         double *outputtime,
                         double *power)
{
   double Req, Ceq, tf, st1del, st2del, st3del, nextinputtime, m;
   double c1, c2, r1, r2, tstep, a, b, c;
   double Tcomparatorni;
   int cols;

   /* First Inverter */

   Ceq = gatecap(Wcompinvn2 + Wcompinvp2, 10.0) +
         draincap(Wcompinvp1, PCH, 1) + draincap(Wcompinvn1, NCH, 1);
   Req = transreson(Wcompinvp1, PCH, 1);
   tf = Req * Ceq;
   st1del = horowitz(inputtime, tf, VTHCOMPINV, VTHCOMPINV, FALL);
   nextinputtime = st1del / VTHCOMPINV;
   *power += Ceq * VddPow * VddPow * 2 * cp->assoc;

   /* Second Inverter */

   Ceq = gatecap(Wcompinvn3 + Wcompinvp3, 10.0) +
         draincap(Wcompinvp2, PCH, 1) + draincap(Wcompinvn2, NCH, 1);
   Req = transreson(Wcompinvn2, NCH, 1);
   tf = Req * Ceq;
   st2del = horowitz(nextinputtime, tf, VTHCOMPINV, VTHCOMPINV, RISE);
   nextinputtime = st2del / (1.0 - VTHCOMPINV);
   *power += Ceq * VddPow * VddPow * 2 * cp->assoc;

   /* Third Inverter */

   Ceq = gatecap(Wevalinvn + Wevalinvp, 10.0) +
         draincap(Wcompinvp3, PCH, 1) + draincap(Wcompinvn3, NCH, 1);
   Req = transreson(Wcompinvp3, PCH, 1);
   tf = Req * Ceq;
   st3del = horowitz(nextinputtime, tf, VTHCOMPINV, VTHEVALINV, FALL);
   nextinputtime = st3del / (VTHEVALINV);
   *power += Ceq * VddPow * VddPow * 2 * cp->assoc;

   /* Final Inverter (virtual ground driver) discharging compare part */

   cols = DIV_ROUND_UP(cp->tbits * sap->Ntbl * sap->Ntspd, 2);

   r1 = transreson(Wcompn, NCH, 2);
   r2 = transreson(Wevalinvn, NCH, 1); /* was switch */
   c2 = DIV_ROUND_UP(cp->tbits, 2) * (draincap(Wcompn, NCH, 1) + draincap(Wcompn, NCH, 2)) +
        draincap(Wevalinvp, PCH, 1) + draincap(Wevalinvn, NCH, 1);
   c1 = DIV_ROUND_UP(cp->tbits, 2) * (draincap(Wcompn, NCH, 1) + draincap(Wcompn, NCH, 2))
        + draincap(Wcompp, PCH, 1) + gatecap(WmuxdrvNANDn + WmuxdrvNANDp, 20.0) +
        cols * Cwordmetal;
   *power += c2 * VddPow * VddPow * 2 * cp->assoc;
   *power += c1 * VddPow * VddPow * (cp->assoc - 1);


   /* time to go to threshold of mux driver */

   tstep = (r2 * c2 + (r1 + r2) * c1) * log(1.0 / VTHMUXNAND);

   /* take into account non-zero input rise time */

   m = Vdd / nextinputtime;

   if ((tstep) <= (0.5*(Vdd - Vt) / m))
   {
      a = m;
      b = 2 * ((Vdd * VTHEVALINV) - Vt);
      c = -2 * (tstep) * (Vdd - Vt) + 1 / m * ((Vdd * VTHEVALINV) - Vt) * ((Vdd * VTHEVALINV) - Vt);
      Tcomparatorni = ( -b + sqrt(b * b - 4 * a * c)) / (2 * a);
   }
   else
   {
      Tcomparatorni = (tstep) + (Vdd + Vt) / (2 * m) - (Vdd * VTHEVALINV) / m;
   }
   *outputtime = Tcomparatorni / (1.0 - VTHMUXNAND);


   return (Tcomparatorni + st1del + st2del + st3del);
}




/*----------------------------------------------------------------------*/

/* Delay of the multiplexor Driver (see section 6.7) */


double mux_driver_delay(const struct cache_params_t *cp,
                        const struct subarray_params_t *sap,
                        double inputtime,
                        double *outputtime,
                        double wirelength)
{
   double Ceq, Req, tf, nextinputtime;
   double Tst1, Tst2, Tst3;

   /* first driver stage - Inverte "match" to produce "matchb" */
   /* the critical path is the DESELECTED case, so consider what
      happens when the address bit is true, but match goes low */

   Ceq = gatecap(WmuxdrvNORn + WmuxdrvNORp, 15.0) * sap->muxover +
         draincap(Wmuxdrv12n, NCH, 1) + draincap(Wmuxdrv12p, PCH, 1);
   Req = transreson(Wmuxdrv12p, PCH, 1);
   tf = Ceq * Req;
   Tst1 = horowitz(inputtime, tf, VTHMUXDRV1, VTHMUXDRV2, FALL);
   nextinputtime = Tst1 / VTHMUXDRV2;

   /* second driver stage - NOR "matchb" with address bits to produce sel */

   Ceq = gatecap(Wmuxdrv3n + Wmuxdrv3p, 15.0) + 2 * draincap(WmuxdrvNORn, NCH, 1) +
         draincap(WmuxdrvNORp, PCH, 2);
   Req = transreson(WmuxdrvNORn, NCH, 1);
   tf = Ceq * Req;
   Tst2 = horowitz(nextinputtime, tf, VTHMUXDRV2, VTHMUXDRV3, RISE);
   nextinputtime = Tst2 / (1 - VTHMUXDRV3);

   /* third driver stage - invert "select" to produce "select bar" */


   Ceq = cp->obits * gatecap(Woutdrvseln + Woutdrvselp + Woutdrvnorn + Woutdrvnorp, 20.0) +
         draincap(Wmuxdrv3p, PCH, 1) + draincap(Wmuxdrv3n, NCH, 1) +
         Cwordmetal * wirelength;
   Req = (Rwordmetal * wirelength) / 2.0 + transreson(Wmuxdrv3p, PCH, 1);
   tf = Ceq * Req;
   Tst3 = horowitz(nextinputtime, tf, VTHMUXDRV3, VTHOUTDRINV, FALL);
   *outputtime = Tst3 / (VTHOUTDRINV);

   return (Tst1 + Tst2 + Tst3);

}

double mux_driver_delay_dualin(const struct cache_params_t *cp,
                               const struct subarray_params_t *sap,
                               double inputtime1,
                               double *outputtime,
                               double wirelength_v,
                               double wirelength_h,
                               double *power)
{
   double Ceq, Req, tf, nextinputtime;
   double Tst1, Tst2, Tst3;

   /* first driver stage - Inverte "match" to produce "matchb" */
   /* the critical path is the DESELECTED case, so consider what
      happens when the address bit is true, but match goes low */

   Ceq = gatecap(WmuxdrvNORn + WmuxdrvNORp, 15.0) * sap->muxover
         + draincap(WmuxdrvNANDn, NCH, 2) + 2 * draincap(WmuxdrvNANDp, PCH, 1);
   Req = transreson(WmuxdrvNANDp, PCH, 1);
   tf = Ceq * Req;
   Tst1 = horowitz(inputtime1, tf, VTHMUXNAND, VTHMUXDRV2, FALL);
   nextinputtime = Tst1 / VTHMUXDRV2;
   *power += Ceq * VddPow * VddPow * (cp->assoc - 1);

   /* second driver stage - NOR "matchb" with address bits to produce sel */

   Ceq = gatecap(Wmuxdrv3n + Wmuxdrv3p, 15.0) + 2 * draincap(WmuxdrvNORn, NCH, 1) +
         draincap(WmuxdrvNORp, PCH, 2);
   Req = transreson(WmuxdrvNORn, NCH, 1);
   tf = Ceq * Req;
   Tst2 = horowitz(nextinputtime, tf, VTHMUXDRV2, VTHMUXDRV3, RISE);
   nextinputtime = Tst2 / (1 - VTHMUXDRV3);
   *power += Ceq * VddPow * VddPow;

   /* third driver stage - invert "select" to produce "select bar" */

   /*   fprintf(stderr, "%f %f %f %f\n",
        ((Rwordmetal*wirelength)/2.0 + transreson(Wmuxdrv3p,PCH,1)), 
        ((Rwordmetal*0)/2.0 + transreson(Wmuxdrv3p,PCH,1)), 
        (cp->obits*gatecap(Woutdrvseln+Woutdrvselp+Woutdrvnorn+Woutdrvnorp,20.0)+
        draincap(Wmuxdrv3p,PCH,1) + draincap(Wmuxdrv3n,NCH,1) +
        Cwordmetal*wirelength),
        (cp->obits*gatecap(Woutdrvseln+Woutdrvselp+Woutdrvnorn+Woutdrvnorp,20.0)+
        draincap(Wmuxdrv3p,PCH,1) + draincap(Wmuxdrv3n,NCH,1) +
        Cwordmetal*0));
   */

   Ceq = cp->obits * gatecap(Woutdrvseln + Woutdrvselp + Woutdrvnorn + Woutdrvnorp, 20.0) +
         draincap(Wmuxdrv3p, PCH, 1) + draincap(Wmuxdrv3n, NCH, 1) +
         Cwordmetal * wirelength_h + Cbitmetal * wirelength_v;
   Req = (Rwordmetal * wirelength_h + Rbitmetal * wirelength_v) / 2.0 + transreson(Wmuxdrv3p, PCH, 1);
   tf = Ceq * Req;
   Tst3 = horowitz(nextinputtime, tf, VTHMUXDRV3, VTHOUTDRINV, FALL);
   *outputtime = Tst3 / (VTHOUTDRINV);
   *power += Ceq * VddPow * VddPow;


   return (Tst1 + Tst2 + Tst3);

}

double senseext_driver_delay(const struct cache_params_t *cp,
                             const struct subarray_params_t *sap,
                             double inputtime,
                             double *outputtime,
                             double wirelength_sense_v,
                             double wirelength_sense_h,
                             double *power)
{
   double Ceq, Req, tf, nextinputtime;
   double Tst1, Tst2;

   /* first driver stage */

   Ceq = draincap(Wsenseextdrv1p, PCH, 1) + draincap(Wsenseextdrv1n, NCH, 1) +
         gatecap(Wsenseextdrv2n + Wsenseextdrv2p, 10.0);
   Req = transreson(Wsenseextdrv1n, NCH, 1);
   tf = Ceq * Req;
   Tst1 = horowitz(inputtime, tf, VTHSENSEEXTDRV, VTHSENSEEXTDRV, FALL);
   nextinputtime = Tst1 / VTHSENSEEXTDRV;
   *power += Ceq * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;

   /* second driver stage */

   Ceq = draincap(Wsenseextdrv2p, PCH, 1) + draincap(Wsenseextdrv2n, NCH, 1) +
         Cwordmetal * wirelength_sense_h + Cbitmetal * wirelength_sense_v;

   Req = (Rwordmetal * wirelength_sense_h + Rbitmetal * wirelength_sense_v) / 2.0 + transreson(Wsenseextdrv2p, PCH, 1);

   tf = Ceq * Req;
   Tst2 = horowitz(nextinputtime, tf, VTHSENSEEXTDRV, VTHOUTDRNAND, RISE);

   *outputtime = Tst2 / (1 - VTHOUTDRNAND);
   *power += Ceq * VddPow * VddPow * .5 * cp->obits * ((cp->nsets == 1) ? 1 : cp->assoc) * sap->muxover;

   //   fprintf(stderr, "Pow %f %f\n", Ceq*VddPow*VddPow*.5*cp->obits*A*muxover*1e9, Ceq*VddPow*VddPow*.5*cp->obits*A*muxover*1e9);

   return (Tst1 + Tst2);

}

double total_out_driver_delay(const struct cache_params_t *cp,
                              const struct subarray_params_t *sap,
                              double inputtime,
                              double *outputtime,
                              double *power)
{
   double single_power, total_senseext_driver, single_senseext_driver;
   int cols_data_subarray, rows_data_subarray, cols_tag_subarray, rows_tag_subarray, htree;
   double subbank_v, subbank_h, sub_v = 0.0, sub_h = 0.0, inter_v, inter_h;
#ifdef GET_OUT
   int cols_fa_subarray, rows_fa_subarray;
#endif /* GET_OUT */
   int nbanks;

   single_power = 0.0;
   total_senseext_driver = 0.0;
   single_senseext_driver = 0.0;

   if (cp->nsets > 1)
   {
      cols_data_subarray = DIV_ROUND_UP(cp->dbits * cp->assoc * sap->Nspd, sap->Ndwl);
      rows_data_subarray = DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd));

      if (sap->Ndwl * sap->Ndbl == 1)
      {
         sub_v = rows_data_subarray;
         sub_h = cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl == 2)
      {
         sub_v = rows_data_subarray;
         sub_h = 2 * cols_data_subarray;
      }
      else if (sap->Ndwl * sap->Ndbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl) * rows_data_subarray;
            sub_h = sqrt(sap->Ndwl * sap->Ndbl) * cols_data_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl / 2) * rows_data_subarray;
            sub_h = 2 * sqrt(sap->Ndwl * sap->Ndbl / 2) * cols_data_subarray;
         }
      }
      
      inter_v = sub_v;
      inter_h = sub_h;

      rows_tag_subarray = DIV_ROUND_UP(cp->nsets, sap->Ntbl * sap->Ntspd);
      cols_tag_subarray = DIV_ROUND_UP(cp->tbits * cp->assoc * sap->Ntspd, sap->Ntwl);

      if (sap->Ntwl * sap->Ntbl == 1)
      {
         sub_v = rows_tag_subarray;
         sub_h = cols_tag_subarray;
      }
      if (sap->Ntwl * sap->Ntbl == 2)
      {
         sub_v = rows_tag_subarray;
         sub_h = 2 * cols_tag_subarray;
      }

      if (sap->Ntwl * sap->Ntbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl) * rows_tag_subarray;
            sub_h = sqrt(sap->Ndwl * sap->Ndbl) * cols_tag_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndwl * sap->Ndbl / 2) * rows_tag_subarray;
            sub_h = 2 * sqrt(sap->Ndwl * sap->Ndbl / 2) * cols_tag_subarray;
         }
      }

      inter_v = MAX(sub_v, inter_v);
      inter_h += sub_h;
      subbank_h = inter_h;
      subbank_v = inter_v;
   }
#ifdef GET_OUT
   else
   {
      rows_fa_subarray = DIV_ROUND_UP(cp->assoc, sap->Ndbl);
      cols_tag_subarray = cp->tbits;
      cols_fa_subarray = cp->dbits + cols_tag_subarray;

      if (sap->Ndbl == 1)
      {
         sub_v = rows_fa_subarray;
         sub_h = cols_fa_subarray;
      }
      else if (sap->Ndbl == 2)
      {
         sub_v = rows_fa_subarray;
         sub_h = 2 * cols_fa_subarray;
      }
      else if (sap->Ndbl > 2)
      {
         htree = cacti_floor_log_base2(sap->Ndbl);
         if (htree % 2 == 0)
         {
            sub_v = sqrt(sap->Ndbl) * rows_fa_subarray;
            sub_h = sqrt(sap->Ndbl) * cols_fa_subarray;
         }
         else
         {
            sub_v = sqrt(sap->Ndbl / 2) * rows_fa_subarray;
            sub_h = 2 * sqrt(sap->Ndbl / 2) * cols_fa_subarray;
         }
      }
      inter_v = sub_v;
      inter_h = sub_h;

      subbank_v = inter_v;
      subbank_h = inter_h;
   }
#endif /* GET_OUT */

   if (cp->nbanks <= 2)
   {
      subbank_h = 0;
      subbank_v = 0;
      single_senseext_driver = senseext_driver_delay(cp, sap, inputtime, outputtime, subbank_v, subbank_h, &single_power);
      total_senseext_driver += single_senseext_driver;
      *power += single_power;
   }

   nbanks = cp->nbanks;

   while (nbanks > 2)
   {
      if (nbanks == 4)
         subbank_h = 0;

      single_senseext_driver = senseext_driver_delay(cp, sap, inputtime, outputtime, subbank_v, subbank_h, &single_power);

      nbanks = DIV_ROUND_UP(nbanks, 4);
      subbank_v *= 2;
      subbank_h *= 2;
      inputtime = *outputtime;
      total_senseext_driver += single_senseext_driver;
      *power += single_power;
   }
   return (total_senseext_driver);
}

/* Valid driver (see section 6.9 of tech report)
   Note that this will only be called for a direct mapped cache */

double valid_driver_delay(const struct cache_params_t *cp,
                          const struct subarray_params_t *sap,
                          double inputtime,
                          double *power)
{
   double Ceq, Tst1, tf;
   int rows, cols, htree, l_valdrv_v = 0, l_valdrv_h = 0, exp;
   double wire_cap, wire_res;
   double subbank_v, subbank_h;

   rows = DIV_ROUND_UP(cp->nsets, (8 * sap->Ntbl * sap->Ntspd));
   cols = DIV_ROUND_UP((cp->tbits * cp->assoc * sap->Ntspd), sap->Ntwl);

   /* calculate some layout info */

   if (sap->Ntwl * sap->Ntbl == 1)
   {
      l_valdrv_v = 0;
      l_valdrv_h = cols;
   }
   else if (sap->Ntwl * sap->Ntbl == 2 || sap->Ntwl * sap->Ntbl == 4)
   {
      l_valdrv_v = 0;
      l_valdrv_h = 2 * cols;
   }
   else if (sap->Ntwl * sap->Ntbl > 4)
   {
      htree = cacti_floor_log_base2(sap->Ntwl * sap->Ntbl);
      if (htree % 2 == 0)
      {
         exp = (htree / 2 - 1);
         l_valdrv_v = (powers(2, exp) - 1) * rows;
         l_valdrv_h = sqrt(sap->Ntwl * sap->Ntbl) * cols;
      }
      else
      {
         exp = (htree + 1) / 2 - 1;
         l_valdrv_v = (powers(2, exp) - 1) * rows;
         l_valdrv_h = sqrt(sap->Ntwl * sap->Ntbl / 2) * cols;
      }
   }

   subbank_routing_length(cp, sap, &subbank_v, &subbank_h);

   wire_cap = Cbitmetal * (l_valdrv_v + subbank_v) + Cwordmetal * (l_valdrv_h + subbank_h);
   wire_res = Rwordmetal * (l_valdrv_h + subbank_h) * 0.5 + Rbitmetal * (l_valdrv_v + subbank_v) * 0.5;

   Ceq = draincap(Wmuxdrv12n, NCH, 1) + draincap(Wmuxdrv12p, PCH, 1) + wire_cap + Cout;
   tf = Ceq * (transreson(Wmuxdrv12p, PCH, 1) + wire_res);
   Tst1 = horowitz(inputtime, tf, VTHMUXDRV1, 0.5, FALL);
   *power += Ceq * VddPow * VddPow;

   return (Tst1);
}


/*----------------------------------------------------------------------*/

/* Data output delay (data side) -- see section 6.8
   This is the time through the NAND/NOR gate and the final inverter 
   assuming sel is already present */

double dataoutput_delay(const struct cache_params_t *cp,
                        const struct subarray_params_t *sap,
                        double inrisetime,
                        double *outrisetime,
                        double *power)
{
   double Ceq, Rwire;
   double tf;
   double nordel, outdel, nextinputtime;
   double l_outdrv_v = 0.0, l_outdrv_h = 0.0;
   double rows, cols, rows_fa_subarray, cols_fa_subarray;
   int htree, exp;

   if (cp->nsets > 1)
   {
      rows = DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd));
      cols = DIV_ROUND_UP(cp->dbits * cp->assoc * sap->Nspd, sap->Ndwl);

      /* calculate some layout info */

      if (sap->Ndwl * sap->Ndbl == 1)
      {
         l_outdrv_v = 0;
         l_outdrv_h = cols;
      }
      else if (sap->Ndwl * sap->Ndbl == 2 || sap->Ndwl * sap->Ndbl == 4)
      {
         l_outdrv_v = 0;
         l_outdrv_h = 2 * cols;
      }
      else if (sap->Ndwl * sap->Ndbl > 4)
      {
         htree = cacti_floor_log_base2(sap->Ndwl * sap->Ndbl);
         if (htree % 2 == 0)
         {
            exp = (htree / 2 - 1);
            l_outdrv_v = (powers(2, exp) - 1) * rows;
            l_outdrv_h = sqrt(sap->Ndwl * sap->Ndbl) * cols;
         }
         else
         {
            exp = (htree + 1) / 2 - 1;
            l_outdrv_v = (powers(2, exp) - 1) * rows;
            l_outdrv_h = sqrt(sap->Ndwl * sap->Ndbl / 2) * cols;
         }
      }
   }
   else
   {
      rows_fa_subarray = DIV_ROUND_UP(cp->assoc, sap->Ndbl);
      cols_fa_subarray = cp->dbits + cp->tbits;
      if (sap->Ndbl == 1)
      {
         l_outdrv_v = 0;
         l_outdrv_h = cols_fa_subarray;
      }
      else if (sap->Ndbl == 2 || sap->Ndbl == 4)
      {
         l_outdrv_v = 0;
         l_outdrv_h = 2 * cols_fa_subarray;
      }

      else if (sap->Ndbl > 4)
      {
         htree = cacti_floor_log_base2(sap->Ndbl);
         if (htree % 2 == 0)
         {
            exp = (htree / 2 - 1);
            l_outdrv_v = (powers(2, exp) - 1) * rows_fa_subarray;
            l_outdrv_h = sqrt(sap->Ndbl) * cols_fa_subarray;
         }
         else
         {
            exp = (htree + 1) / 2 - 1;
            l_outdrv_v = (powers(2, exp) - 1) * rows_fa_subarray;
            l_outdrv_h = sqrt(sap->Ndbl / 2) * cols_fa_subarray;
         }
      }
   }

   /* Delay of NOR gate */

   Ceq = 2 * draincap(Woutdrvnorn, NCH, 1) + draincap(Woutdrvnorp, PCH, 2) +
         gatecap(Woutdrivern, 10.0);
   tf = Ceq * transreson(Woutdrvnorp, PCH, 2);
   nordel = horowitz(inrisetime, tf, VTHOUTDRNOR, VTHOUTDRIVE, FALL);
   nextinputtime = nordel / (VTHOUTDRIVE);
   *power += Ceq * VddPow * VddPow * .5 * cp->obits;

   /* Delay of final output driver */

   Ceq = (draincap(Woutdrivern, NCH, 1) + draincap(Woutdriverp, PCH, 1)) * cp->assoc * sap->muxover
         + Cwordmetal * l_outdrv_h + Cbitmetal * l_outdrv_v + gatecap(Wsenseextdrv1n + Wsenseextdrv1p, 10.0);
   Rwire = (Rwordmetal * l_outdrv_h + Rbitmetal * l_outdrv_v) / 2;

   *power += Ceq * VddPow * VddPow * .5 * cp->obits;

   tf = Ceq * (transreson(Woutdrivern, NCH, 1) + Rwire);
   outdel = horowitz(nextinputtime, tf, VTHOUTDRIVE, 0.5, RISE);
   *outrisetime = outdel / 0.5;
   return (outdel + nordel);
}

/*----------------------------------------------------------------------*/

/* Sel inverter delay (part of the output driver)  see section 6.8 */

double selb_delay_tag_path(double inrisetime,
                           double *outrisetime,
                           double *power)
{
   double Ceq, Tst1, tf;

   Ceq = draincap(Woutdrvseln, NCH, 1) + draincap(Woutdrvselp, PCH, 1) +
         gatecap(Woutdrvnandn + Woutdrvnandp, 10.0);
   tf = Ceq * transreson(Woutdrvseln, NCH, 1);
   Tst1 = horowitz(inrisetime, tf, VTHOUTDRINV, VTHOUTDRNAND, RISE);
   *outrisetime = Tst1 / (1.0 - VTHOUTDRNAND);
   *power += Ceq * VddPow * VddPow;

   return (Tst1);
}


/*----------------------------------------------------------------------*/

/* This routine calculates the extra time required after an access before
 * the next access can occur [ie.  it returns (cycle time-access time)].
 */

double precharge_delay(double worddata)
{
   double Ceq, tf, pretime;

   /* as discussed in the tech report, the delay is the delay of
      4 inverter delays (each with fanout of 4) plus the delay of
      the wordline */

   Ceq = draincap(Wdecinvn, NCH, 1) + draincap(Wdecinvp, PCH, 1) +
         4 * gatecap(Wdecinvn + Wdecinvp, 0.0);
   tf = Ceq * transreson(Wdecinvn, NCH, 1);
   pretime = 4 * horowitz(0.0, tf, 0.5, 0.5, RISE) + worddata;

   return (pretime);
}



/*======================================================================*/

void calc_delay_power_area(const struct cache_params_t *cp,
                           const struct tech_params_t *tp,
                           struct subarray_params_t *sap,
                           struct delay_power_result_t *dprp,
                           struct area_result_t *arp)
{
   struct area_result_t artemp;
   struct delay_power_result_t dprtemp;
   struct subarray_params_t satemp;

   int rows, columns, cols_subarray, rows_subarray;
   int l_muxdrv_v = 0.0, l_muxdrv_h = 0.0, exp, htree;
   double bank_h, bank_v, subbank_h, subbank_v;
   double wirelength_v, wirelength_h;
   double before_mux_delay = 0.0, after_mux_delay = 0.0;
   double time_till_compare = 0.0, time_till_select = 0.0;
   double outrisetime = 0.0, inrisetime = 0.0, addr_inrisetime = 0.0;
   float scale_init;
#ifdef GET_OUT
   double tag_delay_part1 = 0.0, tag_delay_part2 = 0.0, tag_delay_part3 = 0.0, tag_delay_part4 = 0.0, tag_delay_part5 = 0.0, tag_delay_part6 = 0.0;
#endif /* GET_OUT */
   double max_delay = 0;

   double min_access_time = 0.0;
   struct subarray_params_t min_access_time_sap;
   double min_total_power = 0.0;
   struct subarray_params_t min_total_power_sap;
   double min_total_area = 0.0;
   struct subarray_params_t min_total_area_sap;
   double max_total_efficiency = 0.0;
   struct subarray_params_t max_total_efficiency_sap;
   double objective, objective_temp;

   memset(&artemp, 0, sizeof(artemp));
   memset(&dprtemp, 0, sizeof(dprtemp));
   memset(&satemp, 0, sizeof(satemp));

   VbitprePow = tp->vdd * 3.3 / 4.5;
   VddPow = tp->vdd;

   rows = cp->nsets;
   columns = cp->dbits * cp->assoc;

   /* go through possible Ndbl,Ndwl and find the smallest */
   /* Because of area considerations, I don't think it makes sense
      to break either dimension up larger than MAXN */

   /* only try moving output drivers for set associative cache */
   if (cp->assoc != 1)
      scale_init = 0.1;
   else
      scale_init = 1.0;

   if (cp->nsets == 1)
      /* If model is a fully associative cache - use larger cell size */
   {
      FACbitmetal = ((32 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1) + cp->rport)) * Cmetal);
      FACwordmetal = ((8 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1)) + WIREPITCH * (cp->rport)) * Cmetal);
      FARbitmetal = (((32 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1) + cp->rport)) / WIREWIDTH) * Rmetal);
      FARwordmetal = (((8 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1)) + WIREPITCH * (cp->rport)) / WIREWIDTH) * Rmetal);
   }
   Cbitmetal = ((16 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1) + cp->rport)) * Cmetal);
   Cwordmetal = ((8 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1)) + WIREPITCH * (cp->rport)) * Cmetal);
   Rbitmetal = (((16 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1) + cp->rport)) / WIREWIDTH) * Rmetal);
   Rwordmetal = (((8 + 2 * WIREPITCH * ((cp->wport + cp->rwport - 1)) + WIREPITCH * (cp->rport)) / WIREWIDTH) * Rmetal);

   dprp->cycle_time = BIGNUM;
   dprp->access_time = BIGNUM;
   dprp->total_power_read_tag = BIGNUM;
   dprp->total_power_write_tag = BIGNUM;
   dprp->total_power_read_data = BIGNUM;
   dprp->total_power_write_data = BIGNUM;
   dprp->total_power = BIGNUM;
#ifdef GET_OUT
   sap->muxover = DIV_ROUND_UP(cp->dbits, cp->obits);
#endif /* GET_OUT */
   arp->total_efficiency = 0;
   arp->total_area = BIGNUM;

   dprtemp.senseext_scale = 1.0;

   if (cp->nsets > 1)
   {
      /* Set associative or direct map cache model */
      for (satemp.Nspd = 1; satemp.Nspd <= MAXSPD; satemp.Nspd *= 2)
         for (satemp.Ndwl = 1; satemp.Ndwl <= MAXN; satemp.Ndwl *= 2)
            for (satemp.Ndbl = 1; satemp.Ndbl <= MAXN; satemp.Ndbl *= 2)
               for (satemp.Ntspd = 1; satemp.Ntspd <= MAXSPD; satemp.Ntspd *= 2)
		 /* AR: Why is Ntwl not iterated over from 1 to MAXN?  It's a mystery and so is CACTI */
                  for (satemp.Ntwl = 1; satemp.Ntwl <= 1; satemp.Ntwl *= 2)
                     for (satemp.Ntbl = 1; satemp.Ntbl <= MAXN; satemp.Ntbl *= 2)
                     {
                        if (!organizational_parameters_valid (cp, &satemp))
                           continue;

                        bank_h = 0;
                        bank_v = 0;

                        if (DIV_ROUND_UP(cp->dbits, MAX(cp->obits, cp->bbits)) == 1 && satemp.Ndbl * satemp.Nspd == 1)
                           satemp.muxover = 1;
                        else if (satemp.Ndbl * satemp.Nspd > MAX_COL_MUX)
                           satemp.muxover = DIV_ROUND_UP(cp->dbits, MAX(cp->obits, cp->bbits));
                        else if (DIV_ROUND_UP(cp->dbits * satemp.Ndbl * satemp.Nspd, MAX(cp->obits, cp->bbits)) > MAX_COL_MUX)
                           satemp.muxover = DIV_ROUND_UP(cp->dbits, MAX(cp->obits, cp->bbits)) / (MAX_COL_MUX / (satemp.Ndbl * satemp.Nspd));
                        else
                           satemp.muxover = 1;

                        calc_area(cp, tp, &satemp, &artemp);

#ifdef GET_OUT
                        subbank_dim(cp, &satemp, &bank_h, &bank_v);
                        subbanks_routing_power(cp, &satemp, &bank_h, &bank_v, &dprtemp.total_address_routing_power);
                        dprtemp.total_address_routing_power *= (cp->rwport + cp->rport + cp->wport);
                        dprtemp.total_address_routing_power /= tp->fudgefactor;
#endif /* GET_OUT */

                        dprtemp.subbank_address_routing_delay = 0;
                        dprtemp.subbank_address_routing_power = 0;
                        if (cp->nbanks > 2)
                        {
                           dprtemp.subbank_address_routing_delay = address_routing_delay(cp, &satemp, &outrisetime,
                                                                   &dprtemp.subbank_address_routing_power);
#ifdef GET_OUT
                           dprtemp.subbank_address_routing_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
                           dprtemp.subbank_address_routing_delay /= tp->fudgefactor;
                           dprtemp.subbank_address_routing_power /= tp->fudgefactor;
                        }

                        /* Calculate data side of cache */
                        inrisetime = outrisetime;
                        addr_inrisetime = outrisetime;

                        max_delay = 0;

			if (cp->dbits > 0)
			  {
			    data_subbank_dim(cp, &satemp, &bank_h, &bank_v);
			    data_subbank_routing_power(cp, &satemp, &bank_h, &bank_v, &dprtemp.data_routing_power);
			    dprtemp.data_routing_power /= tp->fudgefactor;

			    /* data-side decoder */
			    dprtemp.data_decoder_power = 0;
			    dprtemp.data_decoder_delay = decoder_delay(cp, &satemp,
								       &dprtemp.data_decoder_driver_delay,
								       &dprtemp.data_decoder_3to8_delay,
								       &dprtemp.data_decoder_inv_delay,
								       inrisetime,
								       &outrisetime,
								       &dprtemp.data_decoder_nor_inputs,
								       &dprtemp.data_decoder_power);
#ifdef GET_OUT
			    dprtemp.data_decoder_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
			    dprtemp.data_decoder_delay /= tp->fudgefactor;
			    dprtemp.data_decoder_driver_delay /= tp->fudgefactor;
			    dprtemp.data_decoder_3to8_delay /= tp->fudgefactor;
			    dprtemp.data_decoder_inv_delay /= tp->fudgefactor;
			    dprtemp.data_decoder_power /= tp->fudgefactor;
			    
			    max_delay = MAX(max_delay, dprtemp.data_decoder_delay);
			    inrisetime = outrisetime;
			    
			    /* data-side wordlines */
			    dprtemp.data_wordline_power = 0;
			    dprtemp.data_wordline_delay = wordline_delay(cp, &satemp, inrisetime, &outrisetime,
									 &dprtemp.data_wordline_power);
#ifdef GET_OUT
			    dprtemp.data_wordline_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
			    dprtemp.data_wordline_delay /= tp->fudgefactor;
			    dprtemp.data_wordline_power /= tp->fudgefactor;
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.data_wordline_delay);
			    
			    /* data-side bitlines */
			    dprtemp.data_bitline_power = 0;
			    dprtemp.data_bitline_delay = bitline_delay(cp, &satemp, inrisetime, &outrisetime,
								       &dprtemp.data_bitline_power);
#ifdef GET_OUT
			    dprtemp.data_bitline_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
			    dprtemp.data_bitline_delay /= tp->fudgefactor;
			    dprtemp.data_bitline_power /= tp->fudgefactor;
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.data_bitline_delay);
			    
			    /* data-side sense-amps */
			    dprtemp.data_senseamp_power = cp->obits * cp->assoc * satemp.muxover / 2;
			    dprtemp.data_senseamp_delay = sense_amp_delay(inrisetime, &outrisetime, DIV_ROUND_UP(cp->nsets, satemp.Ndbl * satemp.Nspd),
									  &dprtemp.data_senseamp_power);
#ifdef GET_OUT
			    dprtemp.data_senseamp_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
			    dprtemp.data_senseamp_delay /= tp->fudgefactor;
			    dprtemp.data_senseamp_power /= tp->fudgefactor;
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.data_senseamp_delay);
			    
			    /* data-side output driver */
			    dprtemp.data_output_power = 0;
			    dprtemp.data_output_delay = dataoutput_delay(cp, &satemp, inrisetime, &outrisetime,
									 &dprtemp.data_output_power);
#ifdef GET_OUT
			    dprtemp.data_output_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
			    dprtemp.data_output_delay /= tp->fudgefactor;
			    dprtemp.data_output_power /= tp->fudgefactor;
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.data_output_delay);
			    
			    /* data-side total output driver (includes banks) */
			    dprtemp.data_total_output_power = 0;
			    dprtemp.data_total_output_delay = 0;
			    
			    if (cp->nbanks > 2 )
			      {
				subbank_v = 0;
				subbank_h = 0;
				subbank_routing_length(cp, &satemp, &subbank_v, &subbank_h);
				
				dprtemp.data_total_output_delay = senseext_driver_delay(cp, &satemp, inrisetime, &outrisetime, subbank_v, subbank_h,
											&dprtemp.data_total_output_power);
				
#ifdef GET_OUT
				dprtemp.data_total_output_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
				dprtemp.data_total_output_delay /= tp->fudgefactor;
				dprtemp.data_total_output_power /= tp->fudgefactor;
				inrisetime = outrisetime;
				max_delay = MAX(max_delay, dprtemp.data_total_output_delay);
			      }
			    
			    /* if the associativity is 1, the data output can come right
			       after the sense amp.   Otherwise, it has to wait until 
			       the data access has been done. */
			    
			    before_mux_delay =
			      dprtemp.subbank_address_routing_delay +
			      dprtemp.data_decoder_delay +
			      dprtemp.data_wordline_delay + dprtemp.data_bitline_delay +
			      dprtemp.data_senseamp_delay;
			    after_mux_delay = 0;
			    
			    if (cp->assoc == 1)
			      {
				before_mux_delay +=
				  dprtemp.data_total_output_delay + dprtemp.data_output_delay;
			      }
			    else
			      {
				after_mux_delay +=
				  dprtemp.data_total_output_delay + dprtemp.data_output_delay;
			      }
			  }

                        /*
                         * Now worry about the tag side.
                         */

                        /* tag-side decoder */
			if (cp->tbits > 0)
			  {
			    tag_subbank_dim(cp, &satemp, &bank_h, &bank_v);
			    tag_subbank_routing_power(cp, &satemp, &bank_h, &bank_v, &dprtemp.tag_routing_power);
			    dprtemp.tag_routing_power /= tp->fudgefactor;

			    dprtemp.tag_decoder_power = 0;
			    dprtemp.tag_decoder_delay = decoder_tag_delay(cp, &satemp,
									  &dprtemp.tag_decoder_driver_delay,
									  &dprtemp.tag_decoder_3to8_delay,
									  &dprtemp.tag_decoder_inv_delay, addr_inrisetime, &outrisetime,
									  &dprtemp.tag_decoder_nor_inputs,
									  &dprtemp.tag_decoder_power);
#ifdef GET_OUT
			    dprtemp.tag_decoder_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
			    dprtemp.tag_decoder_delay /= tp->fudgefactor;
			    dprtemp.tag_decoder_driver_delay /= tp->fudgefactor;
			    dprtemp.tag_decoder_3to8_delay /= tp->fudgefactor;
			    dprtemp.tag_decoder_inv_delay /= tp->fudgefactor;
			    dprtemp.tag_decoder_power /= tp->fudgefactor;
			    max_delay = MAX(max_delay, dprtemp.tag_decoder_delay);
			    inrisetime = outrisetime;

			    /* tag-side wordline */
			    dprtemp.tag_wordline_power = 0;
			    dprtemp.tag_wordline_delay = wordline_tag_delay(cp, &satemp, inrisetime, &outrisetime,
									    &dprtemp.tag_wordline_power);
#ifdef GET_OUT
			    dprtemp.tag_wordline_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
			    dprtemp.tag_wordline_delay /= tp->fudgefactor;
			    dprtemp.tag_wordline_power /= tp->fudgefactor;
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.tag_wordline_delay);
			    
			    /* tag-side bitline */
			    dprtemp.tag_bitline_power = 0;
			    dprtemp.tag_bitline_delay = bitline_tag_delay(cp, &satemp, inrisetime, &outrisetime,
									  &dprtemp.tag_bitline_power);
#ifdef GET_OUT
			    dprtemp.tag_bitline_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
			    dprtemp.tag_bitline_delay /= tp->fudgefactor;
			    dprtemp.tag_bitline_power /= tp->fudgefactor;
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.tag_bitline_delay);
			    
			    
			    dprtemp.tag_senseamp_power = cp->tbits;
			    dprtemp.tag_senseamp_delay = sense_amp_tag_delay(inrisetime, &outrisetime,
									     &dprtemp.tag_senseamp_power);
#ifdef GET_OUT
			    dprtemp.tag_senseamp_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
			    dprtemp.tag_senseamp_power /= tp->fudgefactor;
			    
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.tag_senseamp_delay);


			    /* split comparator - look at half the address bits only */
			    dprtemp.tag_compare_power = 0;
			    dprtemp.tag_compare_delay = half_compare_time(cp, &satemp, inrisetime, &outrisetime,
									  &dprtemp.tag_compare_power);
#ifdef GET_OUT
			    dprtemp.tag_compare_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
			    dprtemp.tag_compare_delay /= tp->fudgefactor;
			    dprtemp.tag_compare_power /= tp->fudgefactor;
			    inrisetime = outrisetime;
			    max_delay = MAX(max_delay, dprtemp.tag_compare_delay);
			    
			    dprtemp.valid_driver_power = 0;
			    dprtemp.selb_driver_power = 0;
			    dprtemp.mux_driver_power = 0;

			    if (cp->assoc == 1)
			      {
				dprtemp.mux_driver_delay = 0;
				
				dprtemp.selb_driver_delay = 0;
				dprtemp.selb_driver_power = 0;
				
				dprtemp.valid_driver_delay = valid_driver_delay(cp, &satemp, inrisetime,
										&dprtemp.valid_driver_power);
#ifdef GET_OUT
				dprtemp.valid_driver_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
				dprtemp.valid_driver_delay /= tp->fudgefactor;
				dprtemp.valid_driver_power /= tp->fudgefactor;
				
				max_delay = MAX(max_delay, dprtemp.valid_driver_delay);

				time_till_compare =
				  dprtemp.subbank_address_routing_delay +
				  dprtemp.tag_decoder_delay +
				  dprtemp.tag_wordline_delay +
				  dprtemp.tag_bitline_delay +
				  dprtemp.tag_senseamp_delay;
				
				time_till_select = time_till_compare +
				  dprtemp.tag_compare_delay +
				  dprtemp.valid_driver_delay;
				/*
				 * From the above info, calculate the total access time
				 */
				dprtemp.access_time = MAX(before_mux_delay + after_mux_delay, time_till_select);
			      }
			    else
			      {
				/* default scale is full wirelength */
				
				cols_subarray = DIV_ROUND_UP(cp->dbits * cp->assoc * satemp.Nspd, satemp.Ndwl);
				rows_subarray = DIV_ROUND_UP(cp->nsets, satemp.Ndbl * satemp.Nspd);
				
				if (satemp.Ndwl * satemp.Ndbl == 1)
				  {
				    l_muxdrv_v = 0;
				    l_muxdrv_h = cols_subarray;
				  }
				else if (satemp.Ndwl * satemp.Ndbl == 2 || satemp.Ndwl * satemp.Ndbl == 4)
				  {
				    l_muxdrv_v = 0;
				    l_muxdrv_h = 2 * cols_subarray;
				  }
				else if (satemp.Ndwl * satemp.Ndbl > 4)
				  {
				    htree = cacti_floor_log_base2(satemp.Ndwl * satemp.Ndbl);
				    if (htree % 2 == 0)
				      {
					exp = (htree / 2 - 1);
					l_muxdrv_v = (powers(2, exp) - 1) * rows_subarray;
					l_muxdrv_h = sqrt(satemp.Ndwl * satemp.Ndbl) * cols_subarray;
				      }
				    else
				      {
					exp = (htree + 1) / 2 - 1;
					l_muxdrv_v = (powers(2, exp) - 1) * rows_subarray;
					l_muxdrv_h = sqrt(satemp.Ndwl * satemp.Ndbl / 2) * cols_subarray;
				      }
				  }
				
				wirelength_v = (l_muxdrv_v);
				wirelength_h = (l_muxdrv_h);
				
				/* dualin mux driver - added for split comparator
				   - inverter replaced by nand gate */
				dprtemp.mux_driver_delay = mux_driver_delay_dualin(cp, &satemp,
										   inrisetime, &outrisetime,
										   wirelength_v, wirelength_h,
										   &dprtemp.mux_driver_power);
#ifdef GET_OUT
				dprtemp.mux_driver_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
				dprtemp.mux_driver_delay /= tp->fudgefactor;
				dprtemp.mux_driver_power /= tp->fudgefactor;
				max_delay = MAX(max_delay, dprtemp.mux_driver_delay);
				
				dprtemp.selb_driver_delay = selb_delay_tag_path(inrisetime, &outrisetime,
										&dprtemp.selb_driver_power);
#ifdef GET_OUT
				dprtemp.selb_driver_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
				dprtemp.selb_driver_delay /= tp->fudgefactor;
				dprtemp.selb_driver_power /= tp->fudgefactor;
				max_delay = MAX(max_delay, dprtemp.selb_driver_delay);
				
				dprtemp.valid_driver_delay = 0;
				dprtemp.valid_driver_power = 0;
				
				time_till_compare =
				  dprtemp.subbank_address_routing_delay +
				  dprtemp.tag_decoder_delay +
				  dprtemp.tag_wordline_delay +
				  dprtemp.tag_bitline_delay +
				  dprtemp.tag_senseamp_delay;
				
				time_till_select =
				  time_till_compare +
				  dprtemp.tag_compare_delay +
				  dprtemp.mux_driver_delay +
				  dprtemp.selb_driver_delay;
				
				dprtemp.access_time = MAX(before_mux_delay, time_till_select) + after_mux_delay;
			      }
			  }
			else /* tbits == 0 */ 
			  {
			    dprtemp.access_time = before_mux_delay + after_mux_delay;
			  }
			/*
			 * Calcuate the cycle time
			 */
			
			// precharge_del = precharge_delay(wordline_data);
			
			//     cycle_time = access_time + precharge_del;
			
			dprtemp.cycle_time = dprtemp.access_time / WAVE_PIPE;
			if (max_delay > dprtemp.cycle_time)
			  dprtemp.cycle_time = max_delay;
			
                        /*
                         * The parameters are for a 0.8um process.  A quick way to
                         * scale the results to another process is to divide all
                         * the results by tp->fudgefactor.  Normally, tp->fudgefactor is 1.
                         */

			if (cp->dbits > 0)
			  {
			    /* Adding time to power may look strange, but that's the way it is */
			    dprtemp.data_senseamp_power += (dprtemp.data_output_delay + dprtemp.data_total_output_delay) * 500e-6 * 5 / tp->fudgefactor;
			    if (cp->tbits > 0 && cp->nsets > 1)
			      dprtemp.tag_senseamp_power +=
				(time_till_compare +
				 dprtemp.tag_compare_delay +
				 dprtemp.mux_driver_delay +
				 dprtemp.selb_driver_delay +
				 dprtemp.data_output_delay +
				 dprtemp.data_total_output_delay) * 500e-6 * 5 / tp->fudgefactor;
			  }
			    
                        dprtemp.total_power_read_tag =
			  dprtemp.tag_routing_power +
			  dprtemp.tag_decoder_power +
			  dprtemp.tag_wordline_power +
			  dprtemp.tag_bitline_power +
			  dprtemp.tag_senseamp_power +
			  dprtemp.tag_compare_power +
			  dprtemp.valid_driver_power +
			  dprtemp.selb_driver_power;
			
                        dprtemp.total_power_read_data =
			  dprtemp.data_routing_power + 
			  dprtemp.data_decoder_power +
			  dprtemp.data_wordline_power +
			  dprtemp.data_bitline_power +
			  dprtemp.data_senseamp_power +
			  dprtemp.data_output_power +
			  dprtemp.data_total_output_power +
			  dprtemp.mux_driver_power;
			
                        dprtemp.total_power_write_tag =
			  dprtemp.tag_routing_power +
			  dprtemp.tag_decoder_power +
			  dprtemp.tag_wordline_power +
			  dprtemp.tag_bitline_power;
			
                        dprtemp.total_power_write_data =
			  dprtemp.data_routing_power + 
			  dprtemp.data_decoder_power +
			  dprtemp.data_wordline_power +
			  dprtemp.data_bitline_power +
			  dprtemp.data_output_power +
			  dprtemp.data_total_output_power;
			
                        dprtemp.total_power_tag =
			  (cp->rport + cp->rwport + cp->wport) * (cp->nbanks * (dprtemp.tag_decoder_power +
										dprtemp.tag_wordline_power +
										dprtemp.tag_bitline_power +
										dprtemp.tag_senseamp_power +
										dprtemp.tag_compare_power) + 
								  dprtemp.tag_routing_power);

                        dprtemp.total_power_data =
			  (cp->rport + cp->rwport + cp->wport) * (cp->nbanks * (dprtemp.data_decoder_power +
										dprtemp.data_wordline_power +
										dprtemp.data_bitline_power) +
								  dprtemp.data_total_output_power +
								  dprtemp.data_routing_power) + 
			  (cp->rport + cp->rwport) * (cp->nbanks * (dprtemp.data_senseamp_power + 
								    dprtemp.data_output_power + 
								    dprtemp.mux_driver_power));

                        dprtemp.total_power_norouting =
			  (cp->nbanks * ((cp->rport + cp->rwport + cp->wport) * (dprtemp.subbank_address_routing_power + 
										 dprtemp.tag_decoder_power +
										 dprtemp.tag_wordline_power +
										 dprtemp.tag_bitline_power +
										 dprtemp.tag_senseamp_power +
										 dprtemp.tag_compare_power +
										 dprtemp.valid_driver_power +
										 dprtemp.selb_driver_power +
										 dprtemp.data_decoder_power +
										 dprtemp.data_wordline_power +
										 dprtemp.data_bitline_power) + 
					 (cp->rport + cp->rwport) * (dprtemp.data_senseamp_power + 
								     dprtemp.data_output_power + 
								     dprtemp.mux_driver_power)));


			dprtemp.total_power_routing = 
			  (cp->rport + cp->wport + cp->rwport) * (dprtemp.total_address_routing_power +
								  dprtemp.data_total_output_power);

			dprtemp.total_power = 
			  dprtemp.total_power_norouting + dprtemp.total_power_routing;

#ifdef GET_OUT

                        printf("try %d\n", ++counter);
                        output_subarray_params(&satemp);
                        printf("access_time (nS): %f\n", dprtemp.access_time * 1e9);
                        printf("total_power (nJ): %f\n", dprtemp.total_power_onebank * 1e9);
                        printf("total_area (cm^2): %f\n", artemp.total_area / 100000000.0);
                        printf("total_efficiency: %f\n", artemp.total_efficiency);
                        printf("\n\n");
#endif /* GET_OUT */

                        if (dprtemp.access_time < min_access_time)
                        {
                           min_access_time = dprtemp.access_time;
                           bcopy((char *)&satemp, (char *)&min_access_time_sap, sizeof(struct subarray_params_t));
                        }

                        if (dprtemp.total_power < min_total_power)
                        {
                           min_total_power = dprtemp.total_power;
                           bcopy((char *)&satemp, (char *)&min_total_power_sap, sizeof(struct subarray_params_t));
                        }

                        if (artemp.total_area < min_total_area)
                        {
                           min_total_area = artemp.total_area;
                           bcopy((char *)&satemp, (char *)&min_total_area_sap, sizeof(struct subarray_params_t));
                        }

                        if (artemp.total_efficiency > max_total_efficiency)
                        {
                           max_total_efficiency = artemp.total_efficiency;
                           bcopy((char *)&satemp, (char *)&max_total_efficiency_sap, sizeof(struct subarray_params_t));
                        }

                        /* Minimize quantities */
                        objective_temp = objective_function(cp, &dprtemp, &artemp);
                        objective = objective_function(cp, dprp, arp);
                        if (objective_temp < objective)
                        {
                           bcopy((char *)&dprtemp, (char*)dprp, sizeof(struct delay_power_result_t));
                           bcopy((char *)&artemp, (char*)arp, sizeof(struct area_result_t));
                           bcopy((char *)&satemp, (char*)sap, sizeof(struct subarray_params_t));
                        }
                     }
   }
   else         /* Fully associative model - only vary Ndbl|Ntbl */
   {
#ifdef GET_OUT

      for (satemp.Ndbl = 1; satemp.Ndbl <= MAXN; satemp.Ndbl *= 2)
      {
         satemp.Ntbl = satemp.Ndbl;
         satemp.Ndwl = satemp.Nspd = satemp.Ntwl = satemp.Ntspd = 1;

         if (!organizational_parameters_valid(cp, &satemp))
            continue;

         if (DIV_ROUND_UP(cp->dbits, cp->obits) == 1 && satemp.Ndbl * satemp.Nspd == 1)
            satemp.muxover = 1;
         else if (satemp.Ndbl * satemp.Nspd > MAX_COL_MUX)
            satemp.muxover = DIV_ROUND_UP(cp->dbits, cp->obits);
         else if (DIV_ROUND_UP(cp->dbits * satemp.Ndbl * satemp.Nspd, cp->obits) > MAX_COL_MUX)
            satemp.muxover = DIV_ROUND_UP(cp->dbits * satemp.Ndbl * satemp.Nspd, cp->obits * MAX_COL_MUX);
         else
            satemp.muxover = 1;

         calc_area(cp, tp, &satemp, &artemp);

         bank_h = 0;
         bank_v = 0;

         subbank_dim(cp, &satemp, &bank_h, &bank_v);

         subbanks_routing_power(cp, &satemp, &bank_h, &bank_v, &dprtemp.total_address_routing_power);
#ifdef GET_OUT
         dprtemp.total_address_routing_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
         dprtemp.total_address_routing_power /= tp->fudgefactor;

         dprtemp.subbank_address_routing_delay = 0;
         dprtemp.subbank_address_routing_power = 0;

         if (cp->nbanks > 2 )
         {
            dprtemp.subbank_address_routing_delay = address_routing_delay(cp, &satemp, &outrisetime, &dprtemp.subbank_address_routing_power);
#ifdef GET_OUT 
            dprtemp.subbank_address_routing_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
            dprtemp.subbank_address_routing_delay /= tp->fudgefactor;
            dprtemp.subbank_address_routing_power /= tp->fudgefactor;
         }

         /* Calculate data side of cache */
         inrisetime = outrisetime;
         addr_inrisetime = outrisetime;

         max_delay = 0;
         /* tag path contained here */
         dprtemp.data_decoder_power = 0;
         dprtemp.data_decoder_delay = fa_tag_delay(cp, &satemp,
                                      &tag_delay_part1, &tag_delay_part2,
                                      &tag_delay_part3, &tag_delay_part4,
                                      &tag_delay_part5, &tag_delay_part6,
                                      &outrisetime,
                                      &dprtemp.tag_decoder_nor_inputs,
                                      &dprtemp.data_decoder_power);
#ifdef GET_OUT
         dprtemp.data_decoder_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
         dprtemp.data_decoder_delay /= tp->fudgefactor;
         dprtemp.data_decoder_power /= tp->fudgefactor;
         inrisetime = outrisetime;
         max_delay = MAX(max_delay, dprtemp.data_decoder_delay);

         dprtemp.data_wordline_power = 0;
         dprtemp.data_wordline_delay = wordline_delay(cp, &satemp, inrisetime, &outrisetime,
                                       &dprtemp.data_wordline_power);
#ifdef GET_OUT
         dprtemp.data_wordline_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
         dprtemp.data_wordline_delay /= tp->fudgefactor;
         dprtemp.data_wordline_power /= tp->fudgefactor;
         inrisetime = outrisetime;
         max_delay = MAX(max_delay, dprtemp.data_wordline_delay);

         dprtemp.data_bitline_power = 0;
         dprtemp.data_bitline_delay = bitline_delay(cp, &satemp, inrisetime, &outrisetime,
                                      &dprtemp.data_bitline_power);
#ifdef GET_OUT
         dprtemp.data_bitline_power *= (cp->rwport + cp->rport + cp->wport);
#endif /* GET_OUT */
         dprtemp.data_bitline_delay /= tp->fudgefactor;
         dprtemp.data_bitline_power /= tp->fudgefactor;
         inrisetime = outrisetime;
         max_delay = MAX(max_delay, dprtemp.data_bitline_delay);

         dprtemp.data_senseamp_power = cp->obits * satemp.muxover / 2;
         dprtemp.data_senseamp_delay = sense_amp_delay(inrisetime, &outrisetime, DIV_ROUND_UP(cp->assoc, satemp.Ndbl),
                                       &dprtemp.data_senseamp_power);
#ifdef GET_OUT
         dprtemp.data_senseamp_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
         dprtemp.data_senseamp_delay /= tp->fudgefactor;
         dprtemp.data_senseamp_power /= tp->fudgefactor;
         inrisetime = outrisetime;
         max_delay = MAX(max_delay, dprtemp.data_senseamp_delay);

         dprtemp.data_output_power = 0;
         dprtemp.data_output_delay = dataoutput_delay(cp, &satemp, inrisetime, &outrisetime,
                                     &dprtemp.data_output_power);
#ifdef GET_OUT
         dprtemp.data_output_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
         dprtemp.data_output_delay /= tp->fudgefactor;
         dprtemp.data_output_power /= tp->fudgefactor;
         inrisetime = outrisetime;
         max_delay = MAX(max_delay, dprtemp.data_output_delay);

         dprtemp.data_total_output_power = 0;
         if (cp->nbanks > 2 )
         {
            subbank_v = 0;
            subbank_h = 0;
            subbank_routing_length(cp, &satemp, &subbank_v, &subbank_h);

            dprtemp.data_total_output_delay = senseext_driver_delay(cp, &satemp, inrisetime, &outrisetime, subbank_v, subbank_h,
                                              &dprtemp.data_total_output_power);
#ifdef GET_OUT 
            dprtemp.data_total_output_power *= (cp->rwport + cp->rport);
#endif /* GET_OUT */
            dprtemp.data_total_output_delay /= tp->fudgefactor;
            dprtemp.data_total_output_power /= tp->fudgefactor;
            inrisetime = outrisetime;
            max_delay = MAX(max_delay, dprtemp.data_total_output_delay);
         }

         dprtemp.access_time =
            dprtemp.subbank_address_routing_delay +
            dprtemp.data_decoder_delay +
            dprtemp.data_wordline_delay +
            dprtemp.data_bitline_delay +
            dprtemp.data_senseamp_delay +
            dprtemp.data_output_delay +
            dprtemp.data_total_output_delay;

         /*
          * Calcuate the cycle time
          */

         //  precharge_del = precharge_delay(wordline_data);

         //  cycle_time = access_time + precharge_del;

         dprtemp.cycle_time = dprtemp.access_time / WAVE_PIPE;
         if (max_delay > dprtemp.cycle_time)
            dprtemp.cycle_time = max_delay;

         /*
          * The parameters are for a 0.8um process.  A quick way to
          * scale the results to another process is to divide all
          * the results by tp->fudgefactor.  Normally, tp->fudgefactor is 1.
          */

         dprtemp.data_senseamp_power += (dprtemp.data_output_power + dprtemp.data_total_output_power) * 500e-6 * 5 / tp->fudgefactor;

         dprtemp.total_power_read_onebank =
            dprtemp.subbank_address_routing_power +
            dprtemp.data_decoder_power +
            dprtemp.data_wordline_power +
            dprtemp.data_bitline_power +
            dprtemp.data_senseamp_power +
            dprtemp.data_output_power +
            dprtemp.data_total_output_power;

         dprtemp.total_power_write_onebank =
            dprtemp.subbank_address_routing_power +
            dprtemp.data_decoder_power +
            dprtemp.data_wordline_power +
            dprtemp.data_bitline_power;

         dprtemp.total_power_allbanks_norouting =
            (cp->nbanks) * (dprtemp.data_decoder_power +
                            dprtemp.data_wordline_power +
                            dprtemp.data_bitline_power +
                            dprtemp.data_senseamp_power +
                            dprtemp.data_output_power);

         dprtemp.total_power_allbanks = dprtemp.total_power_allbanks_norouting + dprtemp.total_address_routing_power;

         if (dprtemp.access_time < min_access_time)
         {
            min_access_time = dprtemp.access_time;
            bcopy((char *)&satemp, (char *)&min_access_time_sap, sizeof(struct subarray_params_t));
         }

         if (dprtemp.total_power_read_onebank < min_total_power_read_onebank)
         {
            min_total_power_read_onebank = dprtemp.total_power_read_onebank;
            bcopy((char *)&satemp, (char *)&min_total_power_read_onebank_sap, sizeof(struct subarray_params_t));
         }

         if (dprtemp.total_power_allbanks < min_total_power_allbanks)
         {
            min_total_power_allbanks = dprtemp.total_power_allbanks;
            bcopy((char *)&satemp, (char *)&min_total_power_allbanks_sap, sizeof(struct subarray_params_t));
         }

         if (artemp.total_area < min_total_area)
         {
            min_total_area = artemp.total_area;
            bcopy((char *)&satemp, (char *)&min_total_area_sap, sizeof(struct subarray_params_t));
         }

         if (artemp.total_efficiency > max_total_efficiency)
         {
            max_total_efficiency = artemp.total_efficiency;
            bcopy((char *)&satemp, (char *)&max_total_efficiency_sap, sizeof(struct subarray_params_t));
         }

         /* Minimize quantities */
         if (objective_function(cp, &dprtemp, &artemp) < objective_function(cp, dprp, arp))
         {
            bcopy((char *)&dprtemp, (char*)dprp, sizeof(struct delay_power_result_t));
            bcopy((char *)&artemp, (char*)arp, sizeof(struct area_result_t));
            bcopy((char *)&satemp, (char*)sap, sizeof(struct subarray_params_t));
         }
      }
#endif /* GET_OUT */
   }
}

