#ifndef __CACTI_PARAMS_H
#define __CACTI_PARAMS_H

#include <stdio.h>

/* Used to pass values around the program */

struct cache_params_t
{
  unsigned int nsets;
  unsigned int assoc;
  unsigned int dbits;
  unsigned int tbits;

  unsigned int nbanks;
  unsigned int rport;
  unsigned int wport;
  unsigned int rwport;
  unsigned int serport; /* single-ended bitline read ports */

  unsigned int obits;   /* data input/output bits */
  unsigned int abits;   /* decoder address bits (doesn't include tag) */
  unsigned int bbits;   /* bus bits */

  double dweight;
  double pweight;
  double aweight;

  unsigned int optimize_layout_f;
};

struct subarray_params_t
{
  unsigned int Ndwl;
  unsigned int Ndbl;
  unsigned int Nspd;
  unsigned int Ntwl;
  unsigned int Ntbl;
  unsigned int Ntspd;
  unsigned int muxover;
};

struct tech_params_t 
{
  double tech_size;
  double crossover;
  double standby;
  double vdd;
  double mhz;
  double fudgefactor;
};

struct delay_power_result_t 
{
  double access_time,cycle_time;
  double senseext_scale;
  double max_power, max_access_time;

  double total_power_read_tag;
  double total_power_read_data;
  double total_power_write_tag;
  double total_power_write_data;

  double total_power;

  double total_power_routing;
  double total_power_norouting;

  double total_power_data;
  double total_power_tag;

  double total_address_routing_power;
  double subbank_address_routing_delay;
  double subbank_address_routing_power;
  
  /* data-side decoder */
  double data_routing_power;

  double data_decoder_delay;
  double data_decoder_driver_delay;
  double data_decoder_3to8_delay;
  double data_decoder_inv_delay;
  double data_decoder_power;
  int data_decoder_nor_inputs;

  /* data-side wordline */
  double data_wordline_delay;
  double data_wordline_power;

  /* data-side bitlines */
  double data_bitline_delay;
  double data_bitline_power;
  
  /* data-side senseamps */
  double data_senseamp_delay;
  double data_senseamp_power;

  /* data-side output driver */
  double data_output_delay;
  double data_output_power;

  /* data-side total (all banks) output driver */
  double data_total_output_delay;
  double data_total_output_power;

  double tag_routing_power;
  /* tag-side decoder */
  double tag_decoder_delay;
  double tag_decoder_driver_delay;
  double tag_decoder_3to8_delay;
  double tag_decoder_inv_delay;
  double tag_decoder_power;
  int tag_decoder_nor_inputs;

  /* tag-side wordline */
  double tag_wordline_delay;
  double tag_wordline_power;

  /* tag-side bitlines */
  double tag_bitline_delay;
  double tag_bitline_power;
  
  /* tag-side senseamps */
  double tag_senseamp_delay;
  double tag_senseamp_power;

  double tag_compare_delay;
  double tag_compare_power;
  
  double mux_driver_delay;
  double mux_driver_power;
  
  double selb_driver_delay;
  double selb_driver_power;
  
  double valid_driver_delay;
  double valid_driver_power;
  
  double precharge_delay;
};


struct hw_t 
{
   double height;
   double width;
};

struct area_result_t 
{
  struct hw_t data_mem_hw;
  struct hw_t data_subarray_hw;
  struct hw_t data_subblock_hw;
  struct hw_t data_array_hw;

  struct hw_t data_predecode_hw;
  struct hw_t data_colmux_predecode_hw;
  struct hw_t data_colmux_postdecode_hw;
  struct hw_t data_write_sig_hw;
  double data_aspect_ratio;
  double data_area;
  double data_mem_all_area;
  double data_subarray_all_area;

  struct hw_t tag_mem_hw;
  struct hw_t tag_subarray_hw;
  struct hw_t tag_subblock_hw;
  struct hw_t tag_array_hw;

  struct hw_t tag_predecode_hw;
  struct hw_t tag_colmux_predecode_hw;
  struct hw_t tag_colmux_postdecode_hw;
  struct hw_t tag_outdrv_decode_hw;
  struct hw_t tag_outdrv_sig_hw;
  double tag_aspect_ratio;
  double tag_area;
  double tag_mem_all_area;
  double tag_subarray_all_area;

  double bank_area;
  double bank_aspect_ratio;
  double bank_efficiency;

  struct hw_t total_hw;
  double total_area;
  double total_efficiency;
  double total_aspect_ratio;
};

double hw2area(const struct hw_t *ap,
	       double fudgefactor);

void calc_delay_power_area(const struct cache_params_t *cp, 
			   const struct tech_params_t *tp,
			   struct subarray_params_t *sap,
			   struct delay_power_result_t *dprp, 
			   struct area_result_t *arp);

void calc_area(const struct cache_params_t *cp, 
	       const struct tech_params_t *tp,
	       const struct subarray_params_t *sap,
	       struct area_result_t *arp);

int organizational_parameters_valid(const struct cache_params_t *cp, 
				    const struct subarray_params_t *sap);

int input_data(int argc,
	       char **argv,
	       struct cache_params_t *cp,
	       struct tech_params_t *tp);

void output_data(FILE *stream,
		 const struct cache_params_t *cp,
		 const struct tech_params_t *tp,
		 const struct subarray_params_t *sap,
		 const struct delay_power_result_t *dprp, 
		 const struct area_result_t *arp);

void output_subarray_params(FILE *stream, 
			    const struct subarray_params_t *sap);
#endif /* __CACTI_PARAMS_H */
