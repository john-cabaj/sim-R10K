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
#include "cacti_params.h"
#include "cacti_misc.h"

double logtwo_area(double x)
{
   if (x <= 0)
      printf("%e\n", x);
   return ( (double) (log(x) / log(2.0)) );
}

double hw2area(const struct hw_t *ap, 
	       double fudgefactor)
{
   return (ap->height * ap->width) / (fudgefactor * fudgefactor);
}

double hw2aspectratio(double h, 
		      double w)
{
  return (h > w) ? (h / w) : (w / h);
}


void calc_inverter(double Widthp, 
		   double Widthn,
		   struct hw_t *ap)
{
   double Width_n, Width_p;
   int foldp = 0, foldn = 0;
   if (Widthp > 10.0)
   {
      Widthp = Widthp / 2, foldp = 1;
   }
   if (Widthn > 10.0)
   {
      Widthn = Widthn / 2, foldn = 1;
   }
   ap->height = Widthp + Widthn + Widthptondiff + 2 * Widthtrack;
   Width_n = (foldn) ? (3 * Widthcontact + 2 * (Wpoly + 2 * ptocontact)) : (2 * Widthcontact + Wpoly + 2 * ptocontact) ;
   Width_p = (foldp) ? (3 * Widthcontact + 2 * (Wpoly + 2 * ptocontact)) : (2 * Widthcontact + Wpoly + 2 * ptocontact) ;
   ap->width = MAX(Width_n, Width_p);
}

void calc_data_mem(const struct cache_params_t *cp, 
		   const struct subarray_params_t *sap, 
		   double techscaling_factor,
		   struct area_result_t *arp) /* returns area of subarray */
{
   int noof_rows, noof_colns;
   
   noof_rows = DIV_ROUND_UP(cp->nsets, (sap->Ndbl * sap->Nspd));
   noof_colns = DIV_ROUND_UP((cp->dbits * cp->assoc * sap->Nspd), sap->Ndwl);
   arp->data_mem_hw.height = ceil((double)(noof_rows) / 16.0) * stitch_ramv + (BitHeight16x2 + 2 * Widthtrack * 2 * (cp->rwport + cp->rport + cp->wport - 1)) * ceil((double)(noof_rows) / 2.0);
   arp->data_mem_hw.width = ceil((double)(noof_colns) / 16.0) * (BitWidth16x2 + 16 * (Widthtrack * 2 * (cp->rwport + (cp->rport - cp->serport) + cp->wport - 1) + Widthtrack * cp->serport));
}

void calc_data_decode(const struct cache_params_t *cp,
		      const struct subarray_params_t *sap,
		      struct hw_t *ap) /* returns area of post decode */
{
   int noof_colns, numstack;
   double decodeNORwidth;
   double desiredrisetime, Cline, Rpdrive, psize, nsize;
   struct hw_t decinv, worddriveinv;

   noof_colns = DIV_ROUND_UP(cp->dbits * cp->assoc * sap->Nspd, sap->Ndwl);
   desiredrisetime = krise * log((double)(noof_colns)) / 2.0;
   Cline = (2 * Wmemcella * Leff * Cgatepass + Cwordmetal) * noof_colns;
   Rpdrive = desiredrisetime / (Cline * log(VSINV) * -1.0);
   psize = Rpchannelon / Rpdrive;
   if (psize > Wworddrivemax)
   {
      psize = Wworddrivemax;
   }
   numstack =
      ceil((1.0 / 3.0) * logtwo_area(DIV_ROUND_UP(cp->nsets, sap->Ndbl * sap->Nspd)));
   if (numstack == 0)
      numstack = 1;
   if (numstack > 5)
      numstack = 5;
   switch (numstack)
   {
   case 1:
      decodeNORwidth = WidthNOR1;
      break;
   case 2:
      decodeNORwidth = WidthNOR2;
      break;
   case 3:
      decodeNORwidth = WidthNOR3;
      break;
   case 4:
      decodeNORwidth = WidthNOR4;
      break;
   case 5:
      decodeNORwidth = WidthNOR4;
      break;
   default:
      printf("error:numstack=%d\n", numstack);
      printf("Cacti does not support a series stack of %d transistors !\n", numstack);
      exit(0);
      break;

   }
   nsize = psize * Wdecinvn / Wdecinvp;
   calc_inverter(Wdecinvp, Wdecinvn, &decinv);
   calc_inverter(psize, nsize, &worddriveinv);
   
   ap->height = (BitHeight16x2 + 2 * Widthtrack * 2 * (cp->rwport + cp->rport + cp->wport - 1));
   ap->width = (decodeNORwidth + decinv.height + worddriveinv.height) * (cp->rwport + cp->rport + cp->wport);
}

void calc_predecode(int noof_rows, 
		    const struct cache_params_t *cp,
		    struct hw_t *ap) /*returns the area of predecode */
/* this puts the different predecode blocks for the different ports side by side and does not put them as an array or something */
{
   int N3to8;
   N3to8 = ceil((1.0 / 3.0) * logtwo_area( (double) (noof_rows)));
   if (N3to8 == 0)
     N3to8 = 1;

   switch (N3to8)
   {
   case 1:
      ap->height = Predec_height1;
      ap->width = Predec_width1;
      break;
   case 2:
      ap->height = Predec_height2;
      ap->width = Predec_width2;
      break;
   case 3:
      ap->height = Predec_height3;
      ap->width = Predec_width3;
      break;
   case 4:
      ap->height = Predec_height4;
      ap->width = Predec_width4;
      break;
   case 5:
      ap->height = Predec_height5;
      ap->width = Predec_width5;
      break;
   case 6:
      ap->height = Predec_height6;
      ap->width = Predec_width6;
      break;
   default:
      printf("error:N3to8=%d\n", N3to8);
      exit(0);

   }

   ap->width *= (cp->rwport + cp->rport + cp->wport);
}

void calc_postdecode(int noof_rows, 
		     const struct cache_params_t *cp,
		     struct hw_t *ap)
{
   int numstack, decodeNORwidth;
   struct hw_t decinverter;
   calc_inverter(Wdecinvp, Wdecinvn, &decinverter);
   numstack =
      ceil((1.0 / 3.0) * logtwo_area( (double)(noof_rows)));
   if (numstack == 0)
      numstack = 1;
   if (numstack > 5)
      numstack = 5;
   switch (numstack)
   {
   case 1:
      decodeNORwidth = WidthNOR1;
      break;
   case 2:
      decodeNORwidth = WidthNOR2;
      break;
   case 3:
      decodeNORwidth = WidthNOR3;
      break;
   case 4:
      decodeNORwidth = WidthNOR4;
      break;
   case 5:
      decodeNORwidth = WidthNOR4;
      break;
   default:
      printf("error:numstack=%d\n", numstack);
      printf("Cacti does not support a series stack of %d transistors !\n", numstack);
      exit(0);
      break;

   }
   ap->height = (BitHeight + Widthtrack * 2 * (cp->rwport + cp->rport + cp->wport - 1)) * noof_rows;
   ap->width = (2 * decinverter.height + decodeNORwidth) * (cp->rwport + cp->rport + cp->wport);
}

void calc_colmux(const struct cache_params_t *cp,
		 struct hw_t *ap) /* gives the height of the colmux */
{
  ap->height = (2 * Wbitmuxn + 3 * (2 * Widthcontact + 1)) * (cp->rwport + cp->rport + cp->wport);
  ap->width = (BitWidth + Widthtrack * 2 * (cp->rwport + (cp->rport - cp->serport) + cp->wport - 1) + Widthtrack * cp->serport);
}

void calc_precharge(const struct cache_params_t *cp,
		    const struct subarray_params_t *sap,
		    struct hw_t *ap)
{
   if (sap->Ndbl * sap->Nspd > 1)
   {
      ap->height = (Wbitpreequ + 2 * Wbitdropv + Wwrite + 2 * (2 * Widthcontact +
                               1) + 3 * Widthptondiff) * 0.5 * (cp->rwport + cp->wport);
      ap->width = 2 * (BitWidth + Widthtrack * 2 * (cp->rwport + (cp->rport - cp->serport) + cp->wport - 1) + Widthtrack * cp->serport);
   }
   else
   {
      ap->height = (Wbitpreequ + 2 * Wbitdropv + Wwrite + 2 * (2 * Widthcontact + 1) + 3 * Widthptondiff) * (cp->rwport + cp->wport);
      ap->width = BitWidth + Widthtrack * 2 * (cp->rwport + (cp->rport - cp->serport) + cp->wport - 1) + Widthtrack * cp->serport;
   }
}

void calc_senseamp(const struct cache_params_t *cp, 
		   const struct subarray_params_t *sap,
		   struct hw_t *ap)
{
  if (sap->Ndbl * sap->Nspd > 1)
   {
      ap->height = 0.5 * SenseampHeight * (cp->rwport + cp->rport);
      ap->width = 2 * (BitWidth + Widthtrack * 2 * (cp->rwport + (cp->rport - cp->serport) + cp->wport - 1) + Widthtrack * cp->serport);
   }
   else
   {
      ap->height = SenseampHeight * (cp->rwport + cp->rport);
      ap->width = BitWidth + Widthtrack * 2 * (cp->rwport + (cp->rport - cp->serport) + cp->wport - 1) + Widthtrack * cp->serport;
   }
}

/* define OutdriveHeight OutdriveWidth DatainvHeight DatainvWidth */

void calc_tag_mem(const struct cache_params_t *cp,
		  const struct subarray_params_t *sap, 
		  double techscaling_factor,
		  struct area_result_t *arp) /* returns area of subarray */
{
   int noof_rows, noof_colns;
   int conservative_NSER = 0;

   noof_rows = DIV_ROUND_UP(cp->nsets, sap->Ntbl * sap->Ntspd);
   noof_colns = DIV_ROUND_UP(cp->tbits * cp->assoc * sap->Ntspd, sap->Ntwl);
   arp->tag_mem_hw.height = ceil((double)(noof_rows) / 16.0) * stitch_ramv + (BitHeight16x2 + 2 * Widthtrack * 2 * (cp->rwport + cp->rport + cp->wport - 1)) * DIV_ROUND_UP(noof_rows, 2);
   arp->tag_mem_hw.width = ceil((double)(noof_colns) / 16.0) * (BitWidth16x2 + 16 * (Widthtrack * 2 * (cp->rwport + (cp->rport - conservative_NSER) + cp->wport - 1) + Widthtrack * conservative_NSER));
}

void calc_tag_decode(const struct cache_params_t *cp, 
		     const struct subarray_params_t *sap,
		     struct hw_t *ap) /* returns area of post decode */
{
   int numstack;
   double decodeNORwidth;
   struct hw_t decinv, worddriveinv;

   numstack = ceil((1.0 / 3.0) * logtwo_area(DIV_ROUND_UP(cp->nsets, sap->Ntbl * sap->Ntspd)));
   if (numstack == 0)
      numstack = 1;
   if (numstack > 5)
      numstack = 5;
   switch (numstack)
   {
   case 1:
      decodeNORwidth = WidthNOR1;
      break;
   case 2:
      decodeNORwidth = WidthNOR2;
      break;
   case 3:
      decodeNORwidth = WidthNOR3;
      break;
   case 4:
      decodeNORwidth = WidthNOR4;
      break;
   case 5:
      decodeNORwidth = WidthNOR4;
      break;
   default:
      printf("error:numstack=%d\n", numstack);
      printf("Cacti does not support a series stack of %d transistors !\n", numstack);
      exit(0);
      break;

   }
   calc_inverter(Wdecinvp, Wdecinvn, &decinv);
   calc_inverter(Wdecinvp, Wdecinvn, &worddriveinv);
   ap->height = (BitHeight16x2 + 2 * Widthtrack * 2 * (cp->rwport + cp->rport + cp->wport - 1));
   ap->width = (decodeNORwidth + decinv.height + worddriveinv.height) * (cp->rwport + cp->rport + cp->wport);
}

void calc_comparatorbit(const struct cache_params_t *cp,
			struct hw_t *ap)
{
   ap->width = 3 * Widthcontact + 2 * (3 * Wpoly + 2 * ptocontact);
   ap->height = (Wcompn + 2 * (2 * Widthcontact + 1)) * (cp->rwport + cp->rport);
}

void calc_muxdrv_decode(const struct cache_params_t *cp,
			struct hw_t *ap)
{
   int noof_rows;
   struct hw_t predecode_area, postdecode_area;
   noof_rows = DIV_ROUND_UP(cp->dbits, cp->obits);
   calc_predecode(noof_rows, cp, &predecode_area);
   calc_postdecode(noof_rows, cp, &postdecode_area);
   ap->width = predecode_area.height + postdecode_area.width + noof_rows * Widthtrack * (cp->rwport + cp->rport + cp->wport);
   ap->height = MAX(predecode_area.width, postdecode_area.height);
}

void calc_muxdrv_sig(const struct cache_params_t *cp,
		     struct hw_t *ap) /* generates the 8B/b0*A signals */
{
   int noof_rows;
   struct hw_t outdrv_sig_area;
   noof_rows = DIV_ROUND_UP(cp->dbits, cp->obits);

   outdrv_sig_area.height = 0.5 * (WmuxdrvNORn + WmuxdrvNORp) + 9 * Widthcontact + 0.5 * (Wmuxdrv3n + Wmuxdrv3p) + Widthptondiff + 3 * Widthcontact;
   outdrv_sig_area.width = (3 * Widthcontact + 2 * (3 * Wpoly + 2 * ptocontact)) * noof_rows;
   switch (cp->assoc)
   {
   case 1:
      ap->height = outdrv_sig_area.height + noof_rows * Widthtrack * 2 + cp->assoc * Widthtrack;
      ap->width = outdrv_sig_area.width + noof_rows * Widthtrack + cp->assoc * Widthtrack;
      break;
   case 2:
      ap->height = outdrv_sig_area.height * 2 + noof_rows * Widthtrack * 3 + cp->assoc * Widthtrack;
      ap->width = outdrv_sig_area.width + noof_rows * Widthtrack + cp->assoc * Widthtrack;
      break;
   case 4:
      ap->height = outdrv_sig_area.height * 2 + noof_rows * Widthtrack * 5 + cp->assoc * Widthtrack;
      ap->width = outdrv_sig_area.width * 2 + noof_rows * Widthtrack + cp->assoc * Widthtrack;
      break;
   case 8:
      ap->height = outdrv_sig_area.height * 2 + noof_rows * Widthtrack * 9 + cp->assoc * Widthtrack;
      ap->width = outdrv_sig_area.width * 4 + noof_rows * Widthtrack + cp->assoc * Widthtrack;
      break;
   case 16:
      ap->height = outdrv_sig_area.height * 4 + noof_rows * Widthtrack * 18 + cp->assoc * Widthtrack;
      ap->width = outdrv_sig_area.width * 4 + noof_rows * Widthtrack + cp->assoc * Widthtrack;
      break;
   case 32:
      ap->height = outdrv_sig_area.height * 4 + noof_rows * Widthtrack * 35 + 2 * cp->assoc * Widthtrack;
      ap->width = 2 * (outdrv_sig_area.width * 4 + noof_rows * Widthtrack + cp->assoc * Widthtrack);
      break;
   default:
      printf("error:Associativity=%d\n", cp->assoc);
   }
}


void calc_data_subblock(const struct cache_params_t *cp,
			const struct subarray_params_t *sap,
			int SB, 
			int fudgefactor,
			struct area_result_t *arp)
{

   struct hw_t postdecode_hw, colmux_hw, precharge_hw, senseamp_hw;
   int N3to8, colmuxtracks_rem, outrdrvtracks_rem, writeseltracks_rem, SB_, bsize;
   double tracks_h, tracks_w;
   
   calc_data_mem(cp, sap, fudgefactor, arp);

   calc_data_decode(cp, sap, &postdecode_hw);
   calc_colmux(cp, &colmux_hw);
   calc_precharge(cp, sap, &precharge_hw);
   calc_senseamp(cp, sap, &senseamp_hw);

   arp->data_subarray_hw.height = arp->data_mem_hw.height + colmux_hw.height + precharge_hw.height + senseamp_hw.height + DatainvHeight * (cp->rwport + cp->wport) + OutdriveHeight * (cp->rwport + cp->wport);
   arp->data_subarray_hw.width = arp->data_mem_hw.width + postdecode_hw.width;

   bsize = DIV_ROUND_UP(cp->dbits, 8);
   SB_ = SB;
   if (SB_ == 0)
      SB_ = 1;

   colmuxtracks_rem = (sap->Ndbl * sap->Nspd > tracks_precharge_p) ? (sap->Ndbl * sap->Nspd - tracks_precharge_p) : 0;
   outrdrvtracks_rem = ((2 * bsize * cp->assoc) / (cp->obits) > tracks_outdrvselinv_p) ? ((2 * bsize * cp->assoc) / (cp->obits) - tracks_outdrvselinv_p) : 0;
   writeseltracks_rem = ((2 * bsize * cp->assoc) / (cp->obits) > tracks_precharge_nx2) ? ((2 * bsize * cp->assoc) / (cp->obits) - tracks_precharge_nx2) : 0;
   N3to8 = ceil((1.0 / 3.0) * logtwo_area(DIV_ROUND_UP(cp->nsets, sap->Ndbl * sap->Nspd)));
   if (N3to8 == 0)
      N3to8 = 1;

   tracks_h = Widthtrack * (N3to8 * 8 * (cp->rwport + cp->rport + cp->wport) + (cp->rwport + cp->wport) * colmuxtracks_rem + sap->Ndbl * sap->Nspd * cp->rport + 4 * outrdrvtracks_rem * (cp->rwport + cp->rport) + 4 * writeseltracks_rem * (cp->rwport + cp->wport) + (cp->rwport + cp->rport + cp->wport) * cp->obits / SB_);
   tracks_w = Widthtrack * (N3to8 * 8) * (cp->rwport + cp->rport + cp->wport);

   arp->data_subblock_hw.height = 2 * arp->data_subarray_hw.height + tracks_h;
   arp->data_subblock_hw.width = 2 * arp->data_subarray_hw.width + tracks_w;
}

void calc_data(const struct cache_params_t *cp, 
	       const struct subarray_params_t *sap, 
	       double fudgefactor,
	       struct area_result_t *arp)
{
   int SB, N3to8;
   double fixed_tracks_internal, fixed_tracks_external, variable_tracks;
   double data, driver_select, colmux, predecode, addresslines;
   int blocks, htree, htree_half, i, multiplier, iter_height;
   double inter_height, inter_width;

   SB = sap->Ndwl * sap->Ndbl / 4;
   N3to8 = ceil((1.0 / 3.0) * logtwo_area(DIV_ROUND_UP(cp->nsets, sap->Ndbl * sap->Nspd)));
   if (N3to8 == 0)
      N3to8 = 1;

   data = cp->obits * (cp->rwport + cp->rport + cp->wport) * Widthtrack;
   driver_select = (2 * cp->rwport + cp->rport + cp->wport) * cp->dbits * cp->assoc / cp->obits * Widthtrack;
   colmux = sap->Ndbl * sap->Nspd * (cp->rwport + cp->rport + cp->wport) * Widthtrack;
   predecode = (cp->rwport + cp->rport + cp->wport) * N3to8 * 8 * Widthtrack;
   addresslines = cp->abits * (cp->rwport + cp->rport + cp->wport) * Widthtrack;

   fixed_tracks_internal = colmux + predecode + driver_select;
   fixed_tracks_external = colmux + driver_select + addresslines;
   variable_tracks = data;

   calc_data_subblock(cp, sap, SB, fudgefactor, arp);
   
   arp->data_mem_all_area = sap->Ndwl * sap->Ndbl * hw2area(&arp->data_mem_hw, fudgefactor);
   arp->data_subarray_all_area = sap->Ndbl * sap->Ndwl * hw2area(&arp->data_subarray_hw, fudgefactor);


   if (SB == 0)
   {
      if (sap->Ndbl * sap->Ndwl == 1)
      {
         arp->data_array_hw.height = arp->data_subarray_hw.height + fixed_tracks_external + data;
         arp->data_array_hw.width = arp->data_subarray_hw.width + predecode;
      }
      else
      {
         arp->data_array_hw.height = 2 * arp->data_subarray_hw.height + fixed_tracks_external + data;
         arp->data_array_hw.width = arp->data_subarray_hw.width + predecode;
      }
   }
   if (SB == 1)
   {
      arp->data_array_hw.height = arp->data_subblock_hw.height;
      arp->data_array_hw.width = arp->data_subblock_hw.width;
   }
   if (SB == 2)
   {
      arp->data_array_hw.height = arp->data_subblock_hw.height;
      arp->data_array_hw.width = 2 * arp->data_subblock_hw.width + fixed_tracks_external + data ;
   }
   if (SB == 4)
   {
      arp->data_array_hw.height = 2 * arp->data_subblock_hw.height + fixed_tracks_external + data;
      arp->data_array_hw.width = 2 * arp->data_subblock_hw.width + fixed_tracks_internal + variable_tracks / 2;
   }
   if (SB == 8)
   {
      arp->data_array_hw.height = 2 * arp->data_subblock_hw.height + fixed_tracks_internal + variable_tracks / 2;
      arp->data_array_hw.width = 2 * (2 * arp->data_subblock_hw.width + variable_tracks / 4) + fixed_tracks_external + data;
   }

   if (SB > 8 )
   {
      blocks = SB / 4;
      htree = cacti_floor_log_base2(blocks);
      inter_height = arp->data_subblock_hw.height;
      inter_width = arp->data_subblock_hw.width;
      multiplier = 1;

      if ( htree % 2 == 0)
      {
         iter_height = htree / 2;
      }

      if (htree % 2 == 0)
      {
         for (i=0; i<=iter_height; i++)
         {
            if (i == iter_height)
            {
               arp->data_array_hw.height = 2 * inter_height + data / blocks * multiplier + fixed_tracks_external;
               arp->data_array_hw.width = 2 * inter_width + data / (2 * blocks) * multiplier + fixed_tracks_internal;
            }
            else
            {
               arp->data_array_hw.height = 2 * inter_height + data / blocks * multiplier + fixed_tracks_internal;
               arp->data_array_hw.width = 2 * inter_width + data / (2 * blocks) * multiplier + fixed_tracks_internal;
               inter_height = arp->data_array_hw.height ;
               inter_width = arp->data_array_hw.width;
               multiplier *= multiplier;
            }
         }
      }
      else
      {
         htree_half = htree - 1;
         iter_height = htree_half / 2;
         for (i=0; i<=iter_height; i++)
         {
            arp->data_array_hw.height = 2 * inter_height + data / blocks * multiplier + fixed_tracks_internal;
            arp->data_array_hw.width = 2 * inter_width + data / (2 * blocks) * multiplier + fixed_tracks_internal;
            inter_height = arp->data_array_hw.height;
            inter_width = arp->data_array_hw.width;
            multiplier = multiplier * 4;
         }
         arp->data_array_hw.width = 2 * inter_width + data / (2 * blocks) * multiplier + fixed_tracks_external;
      }
   }

   arp->data_aspect_ratio = hw2aspectratio(arp->data_array_hw.height, arp->data_array_hw.width);
}

void calc_tag_subblock(const struct cache_params_t *cp,
		       const struct subarray_params_t *sap,
		       int SB,
		       double fudgefactor,
		       struct area_result_t *arp)
{
   struct hw_t postdecode_hw, colmux_hw, precharge_hw, senseamp_hw, comp_hw;
   int N3to8, SB_, colmuxtracks_rem;
   double tracks_h, tracks_w;
   struct cache_params_t conservative_cp;
   bcopy((char*)cp, (char*)&conservative_cp, sizeof(struct cache_params_t));
   conservative_cp.serport = 0;

   calc_tag_mem(&conservative_cp, sap, fudgefactor, arp);
   calc_tag_decode(&conservative_cp, sap, &postdecode_hw);

   calc_colmux(&conservative_cp, &colmux_hw);
   calc_precharge(&conservative_cp, sap, &precharge_hw);
   calc_senseamp(&conservative_cp, sap, &senseamp_hw);
   calc_comparatorbit(&conservative_cp, &comp_hw);

   arp->tag_subarray_hw.height = arp->tag_mem_hw.height + colmux_hw.height + precharge_hw.height + senseamp_hw.height + comp_hw.height;
   arp->tag_subarray_hw.width = arp->tag_mem_hw.width + postdecode_hw.width;

   SB_ = SB;
   if (SB_ == 0)
      SB_ = 1;
   N3to8 = ceil((1.0 / 3.0) * logtwo_area(DIV_ROUND_UP(cp->nsets, sap->Ndbl * sap->Nspd)));
   if (N3to8 == 0)
      N3to8 = 1;

   colmuxtracks_rem = (sap->Ndbl * sap->Nspd > tracks_precharge_p) ? (sap->Ndbl * sap->Nspd - tracks_precharge_p) : 0;
   /*writeseltracks_rem = ((2*B*A)/(b0) > tracks_precharge_nx2) ? ((2*B*A)/(b0)-tracks_precharge_nx2) : 0; */

   tracks_h = Widthtrack * (N3to8 * 8 * (cp->rwport + cp->rport + cp->wport) + (cp->rwport + cp->wport) * colmuxtracks_rem + sap->Ndbl * sap->Nspd * cp->rport + (cp->rwport + cp->rport + cp->wport) * cp->tbits / SB_ + (cp->rport + cp->rwport) * cp->assoc);
   tracks_w = Widthtrack * (N3to8 * 8) * (cp->rwport + cp->rport + cp->wport);

   arp->tag_subblock_hw.height = 2 * arp->tag_subarray_hw.height + tracks_h;
   arp->tag_subblock_hw.width = 2 * arp->tag_subarray_hw.width + tracks_w;
}

void calc_tag(const struct cache_params_t *cp, 
	      const struct subarray_params_t *sap, 
	      double fudgefactor,
	      struct area_result_t *arp)
{
   int SB, N3to8;

   double fixed_tracks_internal, fixed_tracks_external, variable_tracks;
   double tag, assoc, colmux, predecode, addresslines;
   int blocks, htree, htree_half, i, multiplier, iter_height;
   double inter_height, inter_width;
   struct cache_params_t conservative_cp;
   bcopy((char*)cp, (char*)&conservative_cp, sizeof(struct cache_params_t));
   conservative_cp.serport = 0;

   SB = sap->Ndwl * sap->Ndbl / 4;
   N3to8 = ceil((1.0 / 3.0) * logtwo_area(DIV_ROUND_UP(cp->nsets, sap->Ndbl * sap->Nspd)));
   if (N3to8 == 0)
   {
      N3to8 = 1;
   }

   tag = cp->tbits * (cp->rwport + cp->rport + cp->wport) * Widthtrack;
   assoc = (cp->rwport + cp->rport) * cp->assoc * Widthtrack;
   colmux = sap->Ndbl * sap->Nspd * (cp->rwport + cp->rport + cp->wport) * Widthtrack;
   predecode = (cp->rwport + cp->rport + cp->wport) * N3to8 * 8 * Widthtrack;
   addresslines = cp->abits * (cp->rwport + cp->rport + cp->wport) * Widthtrack;

   calc_tag_subblock(&conservative_cp, sap, SB, fudgefactor, arp);

   arp->tag_mem_all_area = sap->Ntwl * sap->Ntbl * hw2area(&arp->tag_mem_hw, fudgefactor);
   arp->tag_subarray_all_area = sap->Ndbl * sap->Ndwl * hw2area(&arp->tag_subarray_hw, fudgefactor);

   fixed_tracks_internal = colmux + predecode + assoc;
   fixed_tracks_external = colmux + assoc + addresslines;
   variable_tracks = tag;

   if (SB == 0)
   {
      if (sap->Ndbl * sap->Ndwl == 1)
      {
         arp->tag_array_hw.height = arp->tag_subarray_hw.height + fixed_tracks_external + tag;
         arp->tag_array_hw.width = arp->tag_subarray_hw.width + predecode;
      }
      else
      {
         arp->tag_array_hw.height = 2 * arp->tag_subarray_hw.height + fixed_tracks_external + tag;
         arp->tag_array_hw.width = arp->tag_subarray_hw.width + predecode;
      }
   }
   else if (SB == 1)
   {
      arp->tag_array_hw.height = arp->tag_subblock_hw.height;
      arp->tag_array_hw.width = arp->tag_subblock_hw.width;
   }
   else if (SB == 2)
   {
      arp->tag_array_hw.height = arp->tag_subblock_hw.height;
      arp->tag_array_hw.width = 2 * arp->tag_subblock_hw.width + fixed_tracks_external + tag ;
   }
   else if (SB == 4)
   {
      arp->tag_array_hw.height = 2 * arp->tag_subblock_hw.height + fixed_tracks_external + tag;
      arp->tag_array_hw.width = 2 * arp->tag_subblock_hw.width + fixed_tracks_internal + variable_tracks / 2;
   }
   else if (SB == 8)
   {
      arp->tag_array_hw.height = 2 * arp->tag_subblock_hw.height + fixed_tracks_internal + variable_tracks / 2;
      arp->tag_array_hw.width = 2 * (2 * arp->tag_subblock_hw.width + variable_tracks / 4) + fixed_tracks_external + tag;
   }
   else if (SB > 8 )
   {
      blocks = SB / 4;
      htree = cacti_floor_log_base2(blocks);
      inter_height = arp->tag_subblock_hw.height;
      inter_width = arp->tag_subblock_hw.width;
      multiplier = 1;

      if ( htree % 2 == 0)
      {
         iter_height = htree / 2;

         for (i = 0;i <= iter_height;i++)
         {
            if (i == iter_height)
            {
               arp->tag_array_hw.height = 2 * inter_height + tag / blocks * multiplier + fixed_tracks_external;
               arp->tag_array_hw.width = 2 * inter_width + tag / (2 * blocks) * multiplier + fixed_tracks_internal;
            }
            else
            {
               arp->tag_array_hw.height = 2 * inter_height + tag / blocks * multiplier + fixed_tracks_internal;
               arp->tag_array_hw.width = 2 * inter_width + tag / (2 * blocks) * multiplier + fixed_tracks_internal;
               inter_height = arp->tag_array_hw.height ;
               inter_width = arp->tag_array_hw.width ;
               multiplier *= multiplier;
            }
         }
      }
      else
      {
	htree_half = htree - 1;
         iter_height = htree_half / 2;
         for (i = 0;i <= iter_height;i++)
         {
            arp->tag_array_hw.height = 2 * inter_height + tag / blocks * multiplier + fixed_tracks_internal;
            arp->tag_array_hw.width = 2 * inter_width + tag / (2 * blocks) * multiplier + fixed_tracks_internal;
            inter_height = arp->tag_array_hw.height ;
            inter_width = arp->tag_array_hw.width ;
            multiplier *= multiplier;
         }
         arp->tag_array_hw.width = 2 * inter_width + tag / (2 * blocks) * multiplier + fixed_tracks_external;
      }
   }

   arp->tag_aspect_ratio = hw2aspectratio(arp->tag_array_hw.height, arp->tag_array_hw.width);
}

void calc_nfa(const struct cache_params_t *cp, 
	      const struct subarray_params_t *sap, 
	      double fudgefactor,
	      struct area_result_t *arp)
{
   if (cp->dbits)
     {
       int rows_datasubarray = DIV_ROUND_UP(cp->nsets, sap->Ndbl * sap->Nspd);
       int colns_datasubarray = (sap->Ndbl * sap->Nspd);
       
       calc_data(cp, sap, fudgefactor, arp);
       calc_predecode(rows_datasubarray, cp, &arp->data_predecode_hw);
       calc_predecode(colns_datasubarray, cp, &arp->data_colmux_predecode_hw);
       calc_postdecode(colns_datasubarray, cp, &arp->data_colmux_postdecode_hw);
       calc_muxdrv_sig(cp, &arp->data_write_sig_hw);
       
       arp->data_area = 
	 hw2area(&arp->data_array_hw, fudgefactor) + 
	 hw2area(&arp->data_predecode_hw, fudgefactor) + 
	 hw2area(&arp->data_colmux_predecode_hw, fudgefactor) + 
	 hw2area(&arp->data_colmux_postdecode_hw, fudgefactor) + 
	 (cp->rwport + cp->wport) * hw2area(&arp->data_write_sig_hw, fudgefactor);
     }

   if (cp->tbits > 0)
     {
       int rows_tagsubarray = DIV_ROUND_UP(cp->nsets, sap->Ntbl * sap->Ntspd);
       int colns_tagsubarray = (sap->Ntbl * sap->Ntspd);
       
       calc_tag(cp, sap, fudgefactor, arp);
       calc_predecode(rows_tagsubarray, cp, &arp->tag_predecode_hw);
       calc_predecode(colns_tagsubarray, cp, &arp->tag_colmux_predecode_hw);
       calc_postdecode(colns_tagsubarray, cp, &arp->tag_colmux_postdecode_hw);
       if (cp->dbits > 0) 
	 calc_muxdrv_decode(cp, &arp->tag_outdrv_decode_hw);
       calc_muxdrv_sig(cp, &arp->tag_outdrv_sig_hw);
       
       arp->tag_area = 
	 hw2area(&arp->tag_array_hw, fudgefactor) + 
	 hw2area(&arp->tag_predecode_hw, fudgefactor) + 
	 hw2area(&arp->tag_colmux_predecode_hw, fudgefactor) + 
	 hw2area(&arp->tag_colmux_postdecode_hw, fudgefactor) + 
	 hw2area(&arp->tag_outdrv_decode_hw, fudgefactor) + 
	 (cp->rwport + cp->rport) * hw2area(&arp->tag_outdrv_sig_hw, fudgefactor);
     }

   arp->bank_area = arp->tag_area + arp->data_area;
   arp->bank_aspect_ratio = hw2aspectratio(arp->data_array_hw.height, arp->data_array_hw.width + arp->tag_array_hw.width);
   arp->bank_efficiency = 100 * (arp->data_mem_all_area + arp->tag_mem_all_area) / arp->bank_area;
}

void calc_fa_decode(const struct cache_params_t *cp,
		    struct hw_t *ap) /*returns area of post decode */
{
   int numstack;
   double decodeNORwidth, firstinv;
   struct hw_t decinv, worddriveinv;

   numstack = ceil((1.0 / 3.0) * logtwo_area(cp->nsets * cp->assoc));
   if (numstack == 0)
      numstack = 1;
   if (numstack > 6)
      numstack = 6;
   switch (numstack)
   {
   case 1:
      decodeNORwidth = WidthNOR1;
      break;
   case 2:
      decodeNORwidth = WidthNOR2;
      break;
   case 3:
      decodeNORwidth = WidthNOR3;
      break;
   case 4:
      decodeNORwidth = WidthNOR4;
      break;
   case 5:
      decodeNORwidth = WidthNOR5;
      break;
   case 6:
      decodeNORwidth = WidthNOR6;
      break;
   default:
      printf("error:numstack=%d\n", numstack);
      printf("Cacti does not support a series stack of %d transistors !\n", numstack);
      exit(0);
      break;

   }
   calc_inverter(Wdecinvp, Wdecinvn, &decinv);
   calc_inverter(Wdecinvp, Wdecinvn, &worddriveinv);
   switch (numstack)
   {
   case 1:
      firstinv = decinv.height;
      break;
   case 2:
      firstinv = decinv.height;
      break;
   case 3:
      firstinv = decinv.height;
      break;
   case 4:
      firstinv = decNandWidth;
      break;
   case 5:
      firstinv = decNandWidth;
      break;
   case 6:
      firstinv = decNandWidth;
      break;
   default:
      printf("error:numstack=%d\n", numstack);
      printf("Cacti does not support a series stack of %d transistors !\n", numstack);
      exit(0);
      break;

   }

   ap->height = BitHeight16x2;
   ap->width = (decodeNORwidth + firstinv + worddriveinv.height) * (cp->rwport + cp->wport);
}

void calc_fa_data_subblock(const struct cache_params_t *cp, 
			   const struct subarray_params_t *sap, 
			   double fudgefactor,
			   struct area_result_t *arp) /* returns area of subarray */
{
   struct hw_t fa_decode_hw;
   int noof_rowsdata, noof_colnsdata;
   int HTagbits;
   double precharge, widthoverhead, heightoverhead;

   noof_rowsdata = DIV_ROUND_UP(cp->assoc, sap->Ndbl);
   noof_colnsdata = cp->dbits;
   HTagbits = DIV_ROUND_UP(cp->tbits, 2);
   precharge = Wbitpreequ + 2 * Wbitdropv + Wwrite + 2 * (2 * Widthcontact + 1) + 3 * Widthptondiff;

   if ((cp->rwport == 1) && (cp->rport == 0) && (cp->wport == 0))
   {
      heightoverhead = 0;
      widthoverhead = 0;
   }
   else if ((cp->rwport == 1) && (cp->rport == 1) && (cp->wport == 0))
     {
       widthoverhead = FAWidthIncrPer_first_r_port ;
       heightoverhead = FAHeightIncrPer_first_r_port ;
     }
   else if ((cp->rwport == 1) && (cp->rport == 0) && (cp->wport == 1))
     {
       widthoverhead = FAWidthIncrPer_first_rw_or_w_port ;
       heightoverhead = FAHeightIncrPer_first_rw_or_w_port ;
     }
   else if (cp->rwport + cp->wport >= 2)
     {
       widthoverhead = FAWidthIncrPer_first_rw_or_w_port + (cp->rwport + cp->wport - 2) * FAWidthIncrPer_later_rw_or_w_port + cp->rport * FAWidthIncrPer_later_r_port;
       heightoverhead = FAHeightIncrPer_first_rw_or_w_port + (cp->rwport + cp->wport - 2) * FAHeightIncrPer_later_rw_or_w_port + cp->rport * FAHeightIncrPer_later_r_port;
     }
   else if ((cp->rwport == 0) && (cp->wport == 0))
     {
       widthoverhead = FAWidthIncrPer_first_r_port + (cp->rport - 1) * FAWidthIncrPer_later_r_port;
       heightoverhead = FAHeightIncrPer_first_r_port + (cp->rport - 1) * FAHeightIncrPer_later_r_port;
     }
   else if ((cp->rwport == 0) && (cp->wport == 1))
     {
       widthoverhead = cp->rport * FAWidthIncrPer_later_r_port ;
       heightoverhead = cp->rport * FAHeightIncrPer_later_r_port ;
     }
   else if ((cp->rwport == 1) && (cp->wport == 0))
     {
       widthoverhead = cp->rport * FAWidthIncrPer_later_r_port ;
       heightoverhead = cp->rport * FAHeightIncrPer_later_r_port ;
     }

   arp->data_mem_hw.height = ceil((double)(noof_rowsdata) / 16.0) * stitch_ramv + (CAM2x2Height_1p + 2 * heightoverhead) * ceil((double)(noof_rowsdata) / 2.0);

   arp->data_mem_hw.width = (ceil((double)(noof_colnsdata) / 16.0)) * (BitWidth16x2 + 16 * (Widthtrack * 2 * (cp->rwport + (cp->rport - cp->serport) + cp->wport - 1) + Widthtrack * cp->serport)) + 2 * (HTagbits * ((CAM2x2Width_1p + 2 * widthoverhead) - Widthcontact)) + (BitWidth + Widthtrack * 2 * (cp->rwport + cp->rport + cp->wport - 1)) + (FArowNANDWidth + FArowNOR_INVWidth) * (cp->rwport + cp->rport + cp->wport);

   calc_fa_decode(cp, &fa_decode_hw);

   arp->data_subarray_hw.height = arp->data_mem_hw.height + precharge * (cp->rwport + cp->wport) + SenseampHeight * (cp->rwport + cp->rport) + DatainvHeight * (cp->rwport + cp->wport) + FAOutdriveHeight * (cp->rwport + cp->rport);
   arp->data_subarray_hw.width = arp->data_mem_hw.width + fa_decode_hw.width;
}

void calc_fa_data(const struct cache_params_t *cp, 
		  const struct subarray_params_t *sap, 
		  double fudgefactor,
		  struct area_result_t *arp)
{

  int blocksel, N3to8;
  double fixed_tracks, predecode, base_height, base_width;
  
  int blocks, htree, htree_half, i, iter;
  double inter_height, inter_width;

   N3to8 = ceil((1.0 / 3.0) * logtwo_area(cp->assoc));
   if (N3to8 == 0)
      N3to8 = 1;

   calc_fa_data_subblock(cp, sap, fudgefactor, arp);
   blocksel = MAX(logtwo_area(DIV_ROUND_UP(cp->dbits, 8)), DIV_ROUND_UP(cp->dbits, cp->obits));
   blocksel = (blocksel > tracks_outdrvfanand_p) ? (blocksel - tracks_outdrvfanand_p) : 0;

   arp->data_mem_all_area = sap->Ndbl * hw2area(&arp->data_mem_hw, fudgefactor);
   arp->data_subarray_all_area = sap->Ndbl * hw2area(&arp->data_subarray_hw, fudgefactor);

   fixed_tracks = Widthtrack * (1 * (cp->rwport + cp->wport) + cp->obits * (cp->rwport + cp->rport + cp->wport) + cp->tbits * (cp->rwport + cp->rport + cp->wport) + blocksel * (cp->rwport + cp->rport + cp->wport));
   predecode = Widthtrack * (N3to8 * 8) * (cp->rwport + cp->wport);

   if (sap->Ndbl == 1)
   {
      arp->data_array_hw.height = arp->data_subarray_hw.height + fixed_tracks;
      arp->data_array_hw.width = arp->data_subarray_hw.width + predecode;
   }
   else if (sap->Ndbl == 2)
   {
      arp->data_array_hw.height = 2 * arp->data_subarray_hw.height + fixed_tracks;
      arp->data_array_hw.width = arp->data_subarray_hw.width + predecode;
   }
   else if (sap->Ndbl == 4)
   {
      arp->data_array_hw.height = 2 * arp->data_subarray_hw.height + fixed_tracks + predecode;
      arp->data_array_hw.width = 2 * arp->data_subarray_hw.width + predecode;
   }
   else if (sap->Ndbl > 4)
   {
      blocks = sap->Ndbl / 4;
      htree = cacti_floor_log_base2(blocks);
      base_height = 2 * arp->data_subarray_hw.height + fixed_tracks + predecode;
      base_width = 2 * arp->data_subarray_hw.width + predecode;

      inter_height = base_height;
      inter_width = base_width;

      if (htree % 2 == 0)
      {
         iter = htree / 2;
         for (i = 1;i <= iter;i++)
         {
            arp->data_array_hw.height = 2 * (inter_height) + fixed_tracks + predecode;
            inter_height = arp->data_array_hw.height ;
            arp->data_array_hw.width = 2 * (inter_width) + fixed_tracks + predecode;
            inter_width = arp->data_array_hw.width;
         }
      }
      else
      {
         htree_half = htree - 1;
         iter = htree_half / 2;
         if (iter == 0)
         {
            arp->data_array_hw.height = base_height;
            arp->data_array_hw.width = 2 * base_width + fixed_tracks + predecode;
         }
         else
         {
            for (i = 0;i <= iter;i++)
            {
               arp->data_array_hw.height = 2 * inter_height + fixed_tracks + predecode;
               arp->data_array_hw.width = 2 * inter_width + fixed_tracks + predecode;
               inter_height = arp->data_array_hw.height;
               inter_width = arp->data_array_hw.width;
            }
            arp->data_array_hw.width = 2 * inter_width + fixed_tracks + predecode;
         }
      }
   }

   arp->data_aspect_ratio = hw2aspectratio(arp->data_array_hw.height, arp->data_array_hw.width);
}

void calc_fa(const struct cache_params_t *cp,
	     const struct subarray_params_t *sap,
	     double fudgefactor,
	     struct area_result_t *arp)
{
   struct hw_t null_hw = {0.0, 0.0};

   calc_fa_data(cp, sap, fudgefactor, arp);
   calc_predecode(cp->assoc, cp, &arp->data_predecode_hw);
   arp->data_colmux_predecode_hw = null_hw;
   arp->data_colmux_postdecode_hw = null_hw;
   arp->data_write_sig_hw = null_hw;

   arp->tag_array_hw = null_hw;
   arp->tag_predecode_hw = null_hw;
   arp->tag_colmux_predecode_hw = null_hw;
   arp->tag_colmux_postdecode_hw = null_hw;
   calc_muxdrv_decode(cp, &arp->tag_outdrv_decode_hw);
   arp->tag_outdrv_sig_hw = null_hw;

   arp->data_area = 
     hw2area(&arp->data_array_hw, fudgefactor) + 
     hw2area(&arp->data_predecode_hw, fudgefactor);

   arp->tag_area = 
     hw2area(&arp->tag_outdrv_decode_hw, fudgefactor);

   arp->bank_area = arp->tag_area + arp->data_area;
   arp->bank_aspect_ratio = hw2aspectratio(arp->data_array_hw.height, arp->data_array_hw.width + arp->tag_array_hw.width);
   arp->bank_efficiency = 100 * (arp->data_mem_all_area + arp->tag_mem_all_area) / arp->bank_area;
}

void calc_area(const struct cache_params_t *cp,
	       const struct tech_params_t *tp,
	       const struct subarray_params_t *sap,
	       struct area_result_t *arp)
{
   int blocks, htree, htree_double;
   double base_height, base_width, inter_width, inter_height;
   int base_subbanks, inter_subbanks;
   int i, iter_height, iter_width, iter_width_double;

   if (cp->nsets > 1)
     calc_nfa(cp, sap, tp->fudgefactor, arp);
   else
     calc_fa(cp, sap, tp->fudgefactor, arp);

   if (cp->nbanks == 1)
   {
      arp->total_hw.height = arp->data_array_hw.height;
      arp->total_hw.width = arp->data_array_hw.width + arp->tag_array_hw.width;
   }
   else if (cp->nbanks == 2)
   {
      arp->total_hw.height = arp->data_array_hw.height + (cp->rwport + cp->rport + cp->wport) * cp->abits;
      arp->total_hw.width = (arp->data_array_hw.width + arp->tag_array_hw.width) * 2 + (cp->abits + cp->obits) * cp->nbanks * (cp->rwport + cp->rport + cp->wport);
   }
   else if (cp->nbanks == 4)
   {
      arp->total_hw.height = 2 * arp->data_array_hw.height + 2 * (cp->rwport + cp->rport + cp->wport) * cp->abits;
      arp->total_hw.width = (arp->data_array_hw.width + arp->tag_array_hw.width) * 2 + (cp->abits + cp->obits) * cp->nbanks * (cp->rwport + cp->rport + cp->wport);
   }
   else if (cp->nbanks == 8)
   {
      arp->total_hw.height = (arp->data_array_hw.width + arp->tag_array_hw.width) * 2 + (cp->abits + cp->obits) * cp->nbanks * (cp->rwport + cp->rport + cp->wport) * 0.5;
      arp->total_hw.width = 2 * (2 * arp->data_array_hw.height + 2 * (cp->rwport + cp->rport + cp->wport) * cp->abits) + (cp->abits + cp->obits) * cp->nbanks * (cp->rwport + cp->rport + cp->wport);
   }

   else if (cp->nbanks > 8 )
   {
      blocks = cp->nbanks / 16;
      htree = cacti_floor_log_base2(blocks);
      base_height = 2 * ((arp->data_array_hw.width + arp->tag_array_hw.width) * 2 + (cp->abits + cp->obits) * 16 * (cp->rwport + cp->rport + cp->wport) * 0.25) + (cp->abits + cp->obits) * 16 * (cp->rwport + cp->rport + cp->wport) * 0.5;
      base_width = 2 * (2 * arp->data_array_hw.height + 2 * (cp->rwport + cp->rport + cp->wport) * cp->abits) + (cp->abits + cp->obits) * 16 * (cp->rwport + cp->rport + cp->wport) * 0.25;
      base_subbanks = 16;
      if ( htree % 2 == 0)
      {
         iter_height = htree / 2;
      }
      else
      {
         iter_height = (htree - 1) / 2 ;
      }

      inter_height = base_height;
      inter_subbanks = base_subbanks;

      if (iter_height == 0)
      {
         arp->total_hw.height = base_height;
      }
      else
      {
         for (i = 1;i <= iter_height;i++)
         {
            arp->total_hw.height = 2 * (inter_height) + (cp->abits + cp->obits) * 4 * inter_subbanks * (cp->rwport + cp->rport + cp->wport) * 0.5;
            inter_height = arp->total_hw.height ;
            inter_subbanks = inter_subbanks * 4;
         }
      }

      inter_width = base_width ;
      inter_subbanks = base_subbanks;
      iter_width = 10;

      if ( htree % 2 == 0)
      {
         iter_width = htree / 2;
      }

      if (iter_width == 0)
      {
         arp->total_hw.width = base_width;
      }
      else
      {
         if ( htree % 2 == 0)
         {
            for (i = 1;i <= iter_width;i++)
            {
               arp->total_hw.width = 2 * (inter_width) + (cp->abits + cp->obits) * inter_subbanks * (cp->rwport + cp->rport + cp->wport);
               inter_width = arp->total_hw.height ;
               inter_subbanks = inter_subbanks * 4;
            }
         }
         else
         {
            htree_double = htree + 1;
            iter_width_double = htree_double / 2;
            for (i = 1;i <= iter_width_double;i++)
            {
               arp->total_hw.width = 2 * (inter_width) + (cp->abits + cp->obits) * inter_subbanks * (cp->rwport + cp->rport + cp->wport);
               inter_width = arp->total_hw.height ;
               inter_subbanks = inter_subbanks * 4;
            }
            arp->total_hw.width += (cp->abits + cp->obits) * (cp->rwport + cp->rport + cp->wport) * cp->nbanks / 2;
         }
      }
   }

   arp->total_area = hw2area(&arp->total_hw, tp->fudgefactor);
   arp->total_aspect_ratio = hw2aspectratio(arp->total_hw.height, arp->total_hw.width);
   arp->total_efficiency = 100 * cp->nbanks * (arp->data_mem_all_area + arp->tag_mem_all_area) / arp->total_area;
}

 
int organizational_parameters_valid(const struct cache_params_t *cp, 
				    const struct subarray_params_t *sap)
{
   /* don't want more than 8 subarrays for each of data/tag */
  if (!cp->optimize_layout_f &&
      (sap->Ndwl > 1 || sap->Ndbl > 1 || sap->Nspd > 1 || sap->Ntwl > 1 || sap->Ntbl > 1 || sap->Ntspd > 1))
    return (FALSE);

   if (cp->nsets > 1)
   {
      if (sap->Ndwl * sap->Ndbl > MAXSUBARRAYS)
         return (FALSE);
      if (sap->Ntwl * sap->Ntbl > MAXSUBARRAYS)
         return (FALSE);
      /* add more constraints here as necessary */
      if (cp->nsets / (8 * sap->Ndbl * sap->Nspd) <= 0)
         return (FALSE);
      if (cp->nsets / (8 * sap->Ntbl * sap->Ntspd) <= 0)
         return (FALSE);
      
      if (cp->dbits > 0)
	{
	  if ((cp->dbits * cp->assoc * sap->Nspd) / sap->Ndwl <= 0)
	    return (FALSE);
	  if ((cp->dbits * cp->assoc * sap->Ntspd / sap->Ntwl) <= 0)
	    return (FALSE);
	}
   }
   else
   {
      if ((cp->nsets * cp->assoc) / (2 * sap->Ndbl) <= 0)
         return (FALSE);
      if (sap->Ndbl > MAXN)
         return (FALSE);
      if (sap->Ntbl > MAXN)
         return (FALSE);
   }

   return (TRUE);
}

