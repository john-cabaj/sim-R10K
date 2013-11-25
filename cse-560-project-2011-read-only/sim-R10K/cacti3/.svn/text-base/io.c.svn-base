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
#include <math.h>
#include <stdlib.h>
#include "cacti_misc.h"
#include "cacti_params.h"
#include "def.h"

/*---------------------------------------------------------------*/

int input_data(int argc, 
	       char **argv, 
	       struct cache_params_t *cp,
	       struct tech_params_t *tp)
{
  int i;
  cp->abits = 32;
  cp->obits = 64;
  cp->nbanks = 1;
  cp->nsets = 1024;
  cp->dbits = 256;
  cp->tbits = 20;
  cp->assoc = 1;
  cp->rwport = 1;
  cp->rport = 0;
  cp->wport = 0;
  cp->serport = 0;

  cp->dweight = 0.33;
  cp->pweight = 0.33;
  cp->aweight = 0.33;
  
  cp->optimize_layout_f = TRUE;

  tp->tech_size = 0.09;
  tp->vdd = 0;

  for (i = 1; i < argc; i++)
    {
      switch (argv[i][0])
	{
	case 'c':
	  tp->tech_size = atof(&argv[i][1]);
	  break;
	case 'v':
	  tp->vdd = atof(&argv[i][1]);
	  break;
	case 'b':
	  cp->nbanks = atoi(&argv[i][1]);
	  break;
	case 'a':
	  cp->assoc = atoi(&argv[i][1]);
	  break;
	case 'd':
	  cp->dbits = atoi(&argv[i][1]);
	  break;
	case 't':
	  cp->tbits = atoi(&argv[i][1]);
	  break;
	case 's':
	  cp->nsets = atoi(&argv[i][1]);
	  break;
	case 'p':
	  cp->rwport = atoi(&argv[i][1]);
	  break;
	case 'r':
	  cp->rport = atoi(&argv[i][1]);
	  break;
	case 'w':
	  cp->wport = atoi(&argv[i][1]);
	  break;
	case 'i':
	  cp->abits = atoi(&argv[i][1]);
	  break;
	case 'o':
	  cp->obits = atoi(&argv[i][1]);
	  break;
	case 'l':
	  cp->dweight = atof(&argv[i][1]);
	  break;
	case 'e':
	  cp->pweight = atof(&argv[i][1]);
	  break;
	case 'n':
	  cp->optimize_layout_f = FALSE;
	  break;
	default:
	  fprintf(stderr, "Parameters: [id<parameter=default>]\n");
	  fprintf(stderr, "   [c<transistor-channel-length=0.09>]\n");
	  fprintf(stderr, "   [v<voltage=1.0>]\n");
	  fprintf(stderr, "   [b<banks=1>]\n");
	  fprintf(stderr, "   [s<sets-per-bank=1024>]\n");
	  fprintf(stderr, "   [d<data-bits-per-block=256>]\n");
	  fprintf(stderr, "   [t<tag-bits-per-block=20>]\n");
	  fprintf(stderr, "   [a<associativity=1>\n");
	  fprintf(stderr, "   [p<read/write-ports-per-bank=1>]\n");
	  fprintf(stderr, "   [r<read-ports-per-bank=0>]\n");
	  fprintf(stderr, "   [w<write-ports-per-bank=0>]\n");
	  fprintf(stderr, "   [i<input-address-bus-bits=32>]\n");
	  fprintf(stderr, "   [o<input-output-data-bus-bits=64]\n");
	  fprintf(stderr, "   [l<latency-weight=0.33>\n");
	  fprintf(stderr, "   [e<energy-weight=0.33>\n");
	  fprintf(stderr, "   [n<no-layout-optimizations>]\n");
	  exit(0);
	  break;
	}
    }

  if (tp->tech_size <= 0 || tp->tech_size > 0.81)
    {
      fprintf(stderr, "Feature size must be > 0 and <= 0.80 (um)\n");
      exit(0);
    }
  tp->fudgefactor = 0.80 / tp->tech_size;
  if (tp->vdd == 0.0)
    {
      tp->vdd = 4.5 / (pow(tp->fudgefactor, (2.0 / 3.0)));
      if (tp->vdd < 0.7) tp->vdd = 0.7;
      if (tp->vdd > 5.0) tp->vdd = 5.0;
    }
  VbitprePow = tp->vdd * 3.3 / 4.5;
  VddPow = tp->vdd;

  if (cp->nbanks < 1 || !IS_POWEROFTWO(cp->nbanks))
    {
      fprintf(stderr, "Number of banks must be >=1 and ^2!");
      exit(0);
    }
  
  if (cp->nsets < 1 || !IS_POWEROFTWO(cp->nsets))
    {
      fprintf(stderr, "Number of sets must be >= 1 and ^2\n");
      exit(0);
    }

  if (cp->dbits < 1)
    {
      fprintf(stderr, "Data bits per block must be >= 1\n");
      exit(0);
    }

  if (cp->tbits < 1)
    {
      fprintf(stderr, "Tag bits per block must be >= 1\n");
      exit(0);
    }

  if (cp->assoc < 1 || !IS_POWEROFTWO(cp->assoc))
    {
      fprintf(stderr, "Associativity must be >= 1 and ^2\n");
      exit(0);
    }
  
   if (cp->dbits < cp->obits)
   {
      fprintf(stderr, "Block size must be at least %d bits\n", cp->obits);
      exit(0);
   }

   cp->serport = cp->rport;
   if (cp->rwport + cp->wport + cp->rport < 1)
     {
       fprintf(stderr, "Must have at least one port!");
       exit(0);
     }
   
   if (cp->rwport > 2)
     {
       fprintf(stderr, "Maximum of 2 read/write ports!");
       exit(0);
     }

   return (1);
}

void output_delay_power(FILE *stream, 
			const struct cache_params_t *cp,
			const struct delay_power_result_t *dprp)
{
#ifdef ORIGINAL_FORMAT
   fprintf(stream, " address routing delay (ns): %g\n", dprp->subbank_address_routing_delay * 1e9);
   fprintf(stream, " address routing power (nJ): %g\n", dprp->subbank_address_routing_power*1e9);

   if (cp->nsets > 1)
   {
      fprintf(stream, " decode_data (ns): %g\n", dprp->data_decoder_delay * 1e9);
      fprintf(stream, "             (nJ): %g\n", dprp->data_decoder_power*1e9);
   }
   else
   {
      fprintf(stream, " tag_comparison (ns): %g\n", dprp->data_decoder_delay * 1e9);
      fprintf(stream, "                (nJ): %g\n", dprp->data_decoder_power*1e9);
   }
   fprintf(stream, " wordline and bitline data (ns): %g\n", (dprp->data_wordline_delay + dprp->data_bitline_delay) * 1e9);
   fprintf(stream, "            wordline power (nJ): %g\n", dprp->data_wordline_power*1e9);
   fprintf(stream, "             bitline power (nJ): %g\n", dprp->data_bitline_power*1e9);
   fprintf(stream, " sense_amp_data (ns): %g\n", dprp->data_senseamp_delay * 1e9);
   fprintf(stream, "                (nJ): %g\n", dprp->data_senseamp_power*1e9);
   if (cp->nsets > 1)
   {
      fprintf(stream, " decode_tag (ns): %g\n", dprp->tag_decoder_delay * 1e9);
      fprintf(stream, "            (nJ): %g\n", dprp->tag_decoder_power*1e9);
      fprintf(stream, " wordline and bitline tag (ns): %g\n", (dprp->tag_wordline_delay + dprp->tag_bitline_delay) * 1e9);

      fprintf(stream, "           wordline power (nJ): %g\n", dprp->tag_wordline_power*1e9);
      fprintf(stream, "            bitline power (nJ): %g\n", dprp->tag_bitline_power*1e9);
      fprintf(stream, " sense_amp_tag (ns): %g\n", dprp->tag_senseamp_delay * 1e9);
      fprintf(stream, "               (nJ): %g\n", dprp->tag_senseamp_power*1e9);
      fprintf(stream, " compare (ns): %g\n", dprp->tag_compare_delay * 1e9);
      fprintf(stream, "         (nJ): %g\n", dprp->tag_compare_power*1e9);
      if (cp->assoc == 1)
      {
         fprintf(stream, " valid signal driver (ns): %g\n", dprp->valid_driver_delay * 1e9);
         fprintf(stream, "                     (nJ): %g\n", dprp->valid_driver_power*1e9);
      }
      else
      {
         fprintf(stream, " mux driver (ns): %g\n", dprp->mux_driver_delay * 1e9);
         fprintf(stream, "            (nJ): %g\n", dprp->mux_driver_power*1e9);
         fprintf(stream, " sel inverter (ns): %g\n", dprp->selb_driver_delay * 1e9);
         fprintf(stream, "              (nJ): %g\n", dprp->selb_driver_power*1e9);
      }
   }
   fprintf(stream, " data output driver (ns): %g\n", dprp->data_output_delay * 1e9);
   fprintf(stream, "                    (nJ): %g\n", dprp->data_output_power*1e9);
   fprintf(stream, " total_out_driver (ns): %g\n", dprp->data_total_output_delay * 1e9);
   fprintf(stream, "                 (nJ): %g\n", dprp->data_total_output_power*1e9);

   fprintf(stream, " total data path (without output driver) (ns): %g\n", 
	  (dprp->subbank_address_routing_delay + 
	   dprp->data_decoder_delay + 
	   dprp->data_wordline_delay + 
	   dprp->data_bitline_delay + 
	   dprp->data_senseamp_delay) * 1e9);
   if (cp->nsets > 1)
   {
      if (cp->assoc == 1)
         fprintf(stream, " total tag path is dm (ns): %g\n", 
		(dprp->subbank_address_routing_delay + 
		 dprp->tag_decoder_delay + 
		 dprp->tag_wordline_delay + 
		 dprp->tag_bitline_delay + 
		 dprp->tag_senseamp_delay + 
		 dprp->tag_compare_delay +
		 dprp->valid_driver_delay) * 1e9);
      else
         fprintf(stream, " total tag path is set assoc (ns): %g\n", 
		(dprp->subbank_address_routing_delay + 
		 dprp->tag_decoder_delay + 
		 dprp->tag_wordline_delay + 
		 dprp->tag_bitline_delay + 
		 dprp->tag_senseamp_delay + 
		 dprp->tag_compare_delay + 
		 dprp->mux_driver_delay + 
		 dprp->selb_driver_delay) * 1e9);
   }
#else /* !ORIGINAL_FORMAT */
   double data_delay = 0.0, tag_delay = 0.0, data_power = 0.0, tag_power = 0.0;
   unsigned nport = (cp->rwport + cp->rport + cp->wport);
   fprintf(stream, "\nAccess latency: (ns)\n");
   fprintf(stream, " address_routing: %g (0 if 1 bank)\n", dprp->subbank_address_routing_delay * 1e9);
   fprintf(stream, "    data_decode: %g\n", dprp->data_decoder_delay * 1e9);
   fprintf(stream, "    data_wordline: %g\n", dprp->data_wordline_delay * 1e9);
   fprintf(stream, "    data_bitline: %g\n", dprp->data_bitline_delay * 1e9);
   fprintf(stream, "    data_senseamp: %g\n", dprp->data_senseamp_delay * 1e9);
   data_delay = (dprp->subbank_address_routing_delay + 
		 dprp->data_decoder_delay + 
		 dprp->data_wordline_delay + 
		 dprp->data_bitline_delay + 
		 dprp->data_senseamp_delay);
   fprintf(stream, " data_total: %g (decode + wordline + bitline + senseamp) \n", data_delay * 1e9);

  if (cp->nsets > 1)
    {
      fprintf(stream, "    tag_decode: %g\n", dprp->tag_decoder_delay * 1e9);
      fprintf(stream, "    tag_wordline: %g\n", dprp->tag_wordline_delay * 1e9);
      fprintf(stream, "    tag_bitline: %g\n", dprp->tag_bitline_delay * 1e9);
      fprintf(stream, "    tag_senseamp (ns): %g\n", dprp->tag_senseamp_delay * 1e9);
      fprintf(stream, "    tag_compare (ns): %g\n", dprp->tag_compare_delay * 1e9);
      fprintf(stream, "    tag_valid_signal_driver: %g (0 for set-associative caches)\n", dprp->valid_driver_delay * 1e9);
      fprintf(stream, "    tag_mux_driver: %g (0 for direct-mapped caches)\n", dprp->mux_driver_delay * 1e9);
      fprintf(stream, "    tag_selb_inverter: %g (0 for direct-mapped caches)\n", dprp->selb_driver_delay * 1e9);
      tag_delay = (dprp->subbank_address_routing_delay + 
		   dprp->tag_decoder_delay + 
		   dprp->tag_wordline_delay + 
		   dprp->tag_bitline_delay + 
		   dprp->tag_senseamp_delay + 
		   dprp->tag_compare_delay + 
		   MAX(dprp->valid_driver_delay, dprp->mux_driver_delay + dprp->selb_driver_delay));
      fprintf(stream, " tag_total: %g (decode + wordline + bitline + senseamp + compare + (DM ? (valid_signal_driver) : (mux_driver + selb_driver)))\n", tag_delay * 1e9);
    }
  fprintf(stream, " output_driver: %g\n", dprp->data_output_delay * 1e9);
  fprintf(stream, " bank_output_driver: %g (0 if 1 bank)\n", dprp->data_total_output_delay * 1e9);
  fprintf(stream, " total: %g (address_routing + MAX(tag_total, data_total) + output_driver + bank_output_driver)\n", 
	  (dprp->subbank_address_routing_delay + MAX(tag_delay, data_delay) + dprp->data_output_delay + dprp->data_total_output_delay) * 1e9);
  fprintf(stream, " cycle_time: %g\n", dprp->cycle_time*1e9);

  fprintf(stream, "\nAccess power (per bank per port) (nJ)\n");
  fprintf(stream, " address_routing: %g (0 if 1 bank)\n", (dprp->subbank_address_routing_power / nport) *1e9);
  fprintf(stream, "    data_decoder: %g\n", (dprp->data_decoder_power / nport) *1e9);
  fprintf(stream, "    data_wordline: %g\n", (dprp->data_wordline_power / nport) *1e9);
  fprintf(stream, "    data_bitline: %g\n", (dprp->data_bitline_power / nport) *1e9);
  fprintf(stream, "    data_senseamp: %g\n", (dprp->data_senseamp_power / nport) *1e9);
  data_power = (dprp->data_decoder_power + 
		dprp->data_wordline_power + 
		dprp->data_bitline_power + 
		dprp->data_senseamp_power);
  fprintf(stream, " data_total: %g (decoder + wordline + bitline + senseamp)\n", (data_power / nport) * 1e9);
  if (cp->nsets > 1)
    {
      fprintf(stream, "    tag_decoder: %g\n", (dprp->tag_decoder_power / nport) *1e9);
      fprintf(stream, "    tag_wordline: %g\n", (dprp->tag_wordline_power / nport) *1e9);
      fprintf(stream, "    tag_bitline: %g\n", (dprp->tag_bitline_power / nport) *1e9);
      fprintf(stream, "    tag_senseamp: %g\n", (dprp->tag_senseamp_power / nport) *1e9);
      fprintf(stream, "    tag_compare: %g\n", (dprp->tag_compare_power / nport) *1e9);
      fprintf(stream, "    tag_valid_signal_driver: %g (0 for set-associative caches)\n", (dprp->valid_driver_power / nport) *1e9);
      fprintf(stream, "    tag_mux_driver: %g (0 for direct-mapped caches)\n", (dprp->mux_driver_power / nport) *1e9);
      fprintf(stream, "    tag_selb_driver: %g (0 for direct-mapped caches)\n", (dprp->selb_driver_power / nport) *1e9);
      tag_power = (dprp->tag_decoder_power +
		   dprp->tag_wordline_power +
		   dprp->tag_bitline_power +
		   dprp->tag_senseamp_power +
		   dprp->tag_compare_power +
		   MAX(dprp->valid_driver_power, dprp->mux_driver_power + dprp->selb_driver_power));
      fprintf(stream, " tag_total: %g (decoder wordline + bitline + senseamp + compare + (DM ? (valid_signal_driver) : (mux_driver + selb_driver)))\n", (tag_power / nport) * 1e9);
   }
   fprintf(stream, " output_driver: %g\n", 
	   (dprp->data_output_power / nport) * 1e9);
   fprintf(stream, " bank_output_driver: %g (0 if 1 bank)\n", (dprp->data_total_output_power / nport) *1e9);
   fprintf(stream, " total: %g (address_routing + data_total + tag_total + output_driver + bank_output_driver)\n", 
	   ((dprp->subbank_address_routing_power + data_power + tag_power + dprp->data_output_power + dprp->data_total_output_power) / nport) * 1e9);
   fprintf(stream, " total_all_banks: %g (address_routing + nbanks * (data_total + tag_total + output_driver) + bank_output_driver)\n", 
	   ((dprp->subbank_address_routing_power + cp->nbanks * (data_power + tag_power + dprp->data_output_power) + dprp->data_total_output_power) / nport) * 1e9);
   fprintf(stream, " total_all_banks_all_ports: %g (address_routing + nbanks * (data_total + tag_total + output_driver) + bank_output_driver)\n", 
	   (dprp->subbank_address_routing_power + cp->nbanks * (data_power + tag_power + dprp->data_output_power) + dprp->data_total_output_power) * 1e9);

#endif /* ORIGINAL_FORMAT */
}

void output_area(FILE *stream, 
		 const struct cache_params_t *cp,
		 const struct tech_params_t *tp,
		 const struct area_result_t *arp)
{
#ifdef ORIGINAL_FORMAT
   fprintf(stream, "\nArea Components:\n\n");
   /*
       fprintf(stream, "Aspect Ratio Data height/width: %f\n", aspect_ratio_data);
       fprintf(stream, "Aspect Ratio Tag height/width: %f\n", aspect_ratio_tag);
       fprintf(stream, "Aspect Ratio Subbank height/width: %f\n", aspect_ratio_subbank);
       fprintf(stream, "Aspect Ratio Total height/width: %f\n\n", aspect_ratio_total);
   */
   fprintf(stream, "Aspect Ratio Total height/width: %f\n\n", arp->total_aspect_ratio);

   fprintf(stream, "Data array (cm^2): %g\n", hw2area(&arp->data_array_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Data predecode (cm^2): %g\n", hw2area(&arp->data_predecode_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Data colmux predecode (cm^2): %f\n", hw2area(&arp->data_colmux_predecode_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Data colmux post decode (cm^2): %f\n", hw2area(&arp->data_colmux_postdecode_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Data write signal (cm^2): %f\n", (cp->rwport + cp->rport + cp->wport)*hw2area(&arp->data_write_sig_hw, tp->fudgefactor) / 100000000.0);

   fprintf(stream, "\nTag array (cm^2): %f\n", hw2area(&arp->tag_array_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Tag predecode (cm^2): %f\n", hw2area(&arp->tag_predecode_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Tag colmux predecode (cm^2): %f\n", hw2area(&arp->tag_colmux_predecode_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Tag colmux post decode (cm^2): %f\n", hw2area(&arp->tag_colmux_postdecode_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Tag output driver decode (cm^2): %f\n", hw2area(&arp->tag_outdrv_decode_hw, tp->fudgefactor) / 100000000.0);
   fprintf(stream, "Tag output driver enable signals (cm^2): %f\n", (cp->rwport + cp->rport + cp->wport)*hw2area(&arp->tag_outdrv_sig_hw, tp->fudgefactor) / 100000000.0);

   fprintf(stream, "\nPercentage of data ramcells alone of total area: %f %%\n", 100*arp->data_mem_all_area / arp->bank_area);
   fprintf(stream, "Percentage of tag ramcells alone of total area: %f %%\n", 100*arp->tag_mem_all_area / arp->bank_area);
   fprintf(stream, "Percentage of total control/routing alone of total area: %f %%\n", 100*(arp->total_area - arp->data_mem_all_area - arp->tag_mem_all_area) / arp->total_area);
   fprintf(stream, "\nSubbank Efficiency : %f\n", arp->bank_efficiency);
   fprintf(stream, "Total Efficiency : %f\n", arp->total_efficiency);
   fprintf(stream, "\nTotal area One Subbank (cm^2): %f\n", arp->bank_area / 100000000.0);
   fprintf(stream, "Total area subbanked (cm^2): %f\n", arp->total_area / 100000000.0);
#endif /* ORIGINAL_FORMAT */
}

void output_data(FILE *stream,
		 const struct cache_params_t *cp,
		 const struct tech_params_t *tp,
		 const struct subarray_params_t *sap,
		 const struct delay_power_result_t *dprp,
		 const struct area_result_t *arp)
{
   double datapath, tagpath;

   //stream = fopen("cache_params.aux", "w");

   datapath = 
     dprp->subbank_address_routing_delay + 
     dprp->data_decoder_delay + 
     dprp->data_wordline_delay + 
     dprp->data_bitline_delay + 
     dprp->data_senseamp_delay + 
     dprp->data_total_output_delay + 
     dprp->data_output_delay;

   if (cp->assoc == 1)
   {
      tagpath = 
	dprp->subbank_address_routing_delay + 
	dprp->tag_decoder_delay + 
	dprp->tag_wordline_delay + 
	dprp->tag_bitline_delay + 
	dprp->tag_senseamp_delay + 
	dprp->tag_compare_delay + 
	dprp->valid_driver_delay;
   }
   else
   {
      tagpath = 
	dprp->subbank_address_routing_delay + 
	dprp->tag_decoder_delay + 
	dprp->tag_wordline_delay + 
	dprp->tag_bitline_delay + 
	dprp->tag_senseamp_delay + 
	dprp->tag_compare_delay + 
	dprp->mux_driver_delay + 
	dprp->selb_driver_delay + 
	dprp->data_output_delay + 
	dprp->data_total_output_delay;
   }

   /*
   if (stream)
   {
      ffprintf(stream, stream, "#define PARAMNDWL %d\n#define PARAMNDBL %d\n#define PARAMNSPD %d\n#define PARAMNTWL %d\n#define PARAMNTBL %d\n#define PARAMNTSPD %d\n#define PARAMSENSESCALE %f\n#define PARAMGS %d\n#define PARAMDNOR %d\n#define PARAMTNOR %d\n#define PARAMRPORTS %d\n#define PARAMWPORTS %d\n#define PARAMRWPORTS %d\n#define PARAMMUXOVER %d\n", dprp->best_Ndwl, dprp->best_Ndbl, dprp->best_Nspd, dprp->best_Ntwl, dprp->best_Ntbl, dprp->best_Ntspd, dprp->senseext_scale, (dprp->senseext_scale == 1.0), dprp->data_nor_inputs, dprp->tag_nor_inputs, parameters->num_read_ports, parameters->num_write_ports, parameters->num_readwrite_ports, dprp->best_muxover);
   }
   */
#  if OUTPUTTYPE == LONG
#ifdef ORIGINAL_FORMAT
   fprintf(stream, "\nCache Parameters:\n");
   fprintf(stream, "  Number of Subbanks: %d\n", cp->nbanks);
   fprintf(stream, "  Total Cache Size (bytes): %d\n", DIV_ROUND_UP(cp->nsets * cp->assoc * cp->dbits * cp->nbanks, 8));
   fprintf(stream, "  Size in bytes of Subbank: %d\n", DIV_ROUND_UP(cp->nsets * cp->assoc * cp->dbits, 8));
   fprintf(stream, "  Number of sets: %d\n", cp->nsets);
   fprintf(stream, "  Associativity: %d\n", cp->assoc);

   fprintf(stream, "  Block Size (bytes): %d\n", DIV_ROUND_UP(cp->dbits, 8));
   fprintf(stream, "  Read/Write Ports: %d\n", cp->rwport);
   fprintf(stream, "  Read Ports: %d\n", cp->rport);
   fprintf(stream, "  Write Ports: %d\n", cp->wport);
   fprintf(stream, "  Technology Size: %2.2fum\n", tp->tech_size);
   fprintf(stream, "  Vdd: %2.1fV\n", tp->vdd);

   fprintf(stream, "\nAccess Time (ns): %g\n", dprp->access_time*1e9);
   fprintf(stream, "Cycle Time (wave pipelined) (ns):  %g\n", dprp->cycle_time*1e9);
   if (cp->nsets == 1)
   {
      fprintf(stream, "Total Power all Banks (nJ): %g\n", dprp->total_power_allbanks*1e9);
      fprintf(stream, "Total Power Without Routing (nJ): %g\n", dprp->total_power_allbanks_norouting*1e9);
      fprintf(stream, "Total Routing Power (nJ): %g\n", dprp->total_address_routing_power*1e9);

      fprintf(stream, "Maximum Bank Power (nJ):  %g\n", 
	     (dprp->subbank_address_routing_power + 
	      dprp->data_decoder_power + 
	      dprp->data_wordline_power + 
	      dprp->data_bitline_power + 
	      dprp->data_senseamp_power + 
	      dprp->data_output_power + 
	      dprp->data_total_output_power)*1e9);
      //      fprintf(stream, "Power (W) - 500MHz:  %g\n",(dprp->decoder_power_data+dprp->wordline_power_data+dprp->bitline_power_data+dprp->sense_amp_power_data+dprp->data_output_power)*500*1e6);
   }
   else
   {
      fprintf(stream, "Total Power all Banks (nJ): %g\n", dprp->total_power_allbanks*1e9);
      fprintf(stream, "Total Power Without Routing (nJ): %g\n", dprp->total_power_allbanks_norouting*1e9);
      fprintf(stream, "Total Routing Power (nJ): %g\n", dprp->total_address_routing_power*1e9);
      fprintf(stream, "Maximum Bank Power (nJ):  %g\n", 
	     (dprp->subbank_address_routing_power + 
	      dprp->data_decoder_power + 
	      dprp->data_wordline_power + 
	      dprp->data_bitline_power + 
	      dprp->data_senseamp_power + 
	      dprp->data_total_output_power + 
	      dprp->tag_decoder_power + 
	      dprp->tag_wordline_power + 
	      dprp->tag_bitline_power + 
	      dprp->tag_senseamp_power + 
	      dprp->tag_compare_power + 
	      dprp->valid_driver_power + 
	      dprp->mux_driver_power + 
	      dprp->selb_driver_power + 
	      dprp->data_output_power)*1e9);
      //      fprintf(stream, "Power (W) - 500MHz:  %g\n",(dprp->decoder_power_data+dprp->wordline_power_data+dprp->bitline_power_data+dprp->sense_amp_power_data+dprp->total_out_driver_power_data+dprp->decoder_power_tag+dprp->wordline_power_tag+dprp->bitline_power_tag+dprp->sense_amp_power_tag+dprp->compare_part_power+dprp->drive_valid_power+dprp->drive_mux_power+dprp->selb_power+dprp->data_output_power)*500*1e6);
   }
   fprintf(stream, "\n");
#else /* !ORIGINAL_FORMAT */
   fprintf(stream, "Parameters:\n");
   fprintf(stream, "  banks: %d\n", cp->nbanks);
   fprintf(stream, "  sets per bank: %u\n", cp->nsets);
   fprintf(stream, "  associativity: %u\n", cp->assoc);
   fprintf(stream, "  data block size: %u bits => %u bytes\n", cp->dbits, DIV_ROUND_UP(cp->dbits, 8));
   fprintf(stream, "  tag size: %u bits\n", cp->tbits);
   fprintf(stream, "  data capacity per bank: %u bits => %u bytes => %u KB\n", 
	   (cp->nsets * cp->assoc * cp->dbits), 
	   DIV_ROUND_UP(cp->nsets * cp->assoc * cp->dbits, 8),
	   DIV_ROUND_UP(cp->nsets * cp->assoc * cp->dbits, 8192));
   fprintf(stream, "  data capacity: %u bits => %u bytes => %u KB\n", 
	   (cp->nsets * cp->assoc * cp->dbits * cp->nbanks), 
	   DIV_ROUND_UP(cp->nsets * cp->assoc * cp->dbits * cp->nbanks, 8),
	   DIV_ROUND_UP(cp->nsets * cp->assoc * cp->dbits * cp->nbanks, 8192));
   
   fprintf(stream, "  read/write ports: %d\n", cp->rwport);
   fprintf(stream, "  read ports: %d\n", cp->rport);
   fprintf(stream, "  write ports: %d\n", cp->wport);
   fprintf(stream, "  input address bits: %d\n", cp->abits);
   fprintf(stream, "  input/output data bits: %d\n", cp->obits);
   fprintf(stream, "  technology: %2.2fum\n", tp->tech_size);
   fprintf(stream, "  vdd: %2.1fV\n", tp->vdd);   
#endif /* ORIGINAL_FORMAT */
   output_subarray_params(stream, sap);
#ifdef ORIGINAL_FORMAT
   fprintf(stream, "Nor inputs (data): %d\n", dprp->data_decoder_nor_inputs);
   fprintf(stream, "Nor inputs (tag): %d\n", dprp->tag_decoder_nor_inputs);
#endif /* ORIGINAL_FORMAT */
   output_area(stream, cp, tp, arp);
#ifdef ORIGINAL_FOMAT 
   fprintf(stream, "\nTime Components:\n");

   fprintf(stream, " data side (with Output driver) (ns): %g\n", datapath / 1e-9);
   if (cp->nsets > 1)
      fprintf(stream, " tag side (with Output driver) (ns): %g\n", (tagpath) / 1e-9);
#endif /* ORIGINAL_FORMAT */
   output_delay_power(stream, cp, dprp);

#  else

   fprintf(stream, "%d %d %d  %d %d %d %d %d %d  %e %e %e %e  %e %e %e %e  %e %e %e %e  %e %e %e %e  %e %e\n",
          parameters->cache_size,
          parameters->block_size,
          parameters->associativity,
          dprp->best_Ndwl,
          dprp->best_Ndbl,
          dprp->best_Nspd,
          dprp->best_Ntwl,
          dprp->best_Ntbl,
          dprp->best_Ntspd,
          dprp->access_time,
          dprp->cycle_time,
          datapath,
          tagpath,
          dprp->decoder_delay_data,
          dprp->wordline_delay_data,
          dprp->bitline_delay_data,
          dprp->sense_amp_delay_data,
          dprp->decoder_delay_tag,
          dprp->wordline_delay_tag,
          dprp->bitline_delay_tag,
          dprp->sense_amp_delay_tag,
          dprp->compare_part_delay,
          dprp->drive_mux_delay,
          dprp->selb_delay,
          dprp->drive_valid_delay,
          dprp->data_output_delay,
          dprp->precharge_delay);



# endif
}


void
output_subarray_params(FILE *stream, 
		       const struct subarray_params_t *sap)
{
#ifdef ORIGINAL_FORMAT
  fprintf(stream, "Ndwl: %d\n", sap->Ndwl);
  fprintf(stream, "Ndbl: %d\n", sap->Ndbl);
  fprintf(stream, "Nspd: %d\n", sap->Nspd);
  fprintf(stream, "Ntwl: %d\n", sap->Ntwl);
  fprintf(stream, "Ntbl: %d\n", sap->Ntbl);
  fprintf(stream, "Ntspd: %d\n", sap->Ntspd);
#else /* !ORIGINAL_FORMAT */
  fprintf(stream, "\nLayout:\n");
  fprintf(stream, " data sets per wordline: %u\n", sap->Nspd);
  fprintf(stream, " data horizontal partitions: %u\n", sap->Ndwl);
  fprintf(stream, " data vertical partitions: %u\n", sap->Ndbl);
  fprintf(stream, " tag sets per wordline: %u\n", sap->Ntspd);
  fprintf(stream, " tag horizontal partitions: %u\n", sap->Ntwl);
  fprintf(stream, " tag vertical partitions: %u\n", sap->Ntbl);
#endif /* ORIGINAL_FORMAT */
}
