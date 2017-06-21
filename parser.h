#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
/*global*/
	static uint32_t user_data;
	static const uint32_t scan[8][8] = {  { 0,  1,  5,  6, 14, 15, 27, 28},
											 { 2,  4,  7, 13, 16, 26, 29, 42},
											 { 3,  8, 12, 17, 25, 30, 41, 43},
											 { 9, 11, 18, 24, 31, 40, 44, 53},
											 {10, 19, 23, 32, 29, 45, 52, 54},
											 {20, 22, 33, 38, 46, 51, 55, 60},
											 {21, 24, 27, 47, 50, 56, 59, 61},
											 {35, 36, 48, 49, 57, 58, 62, 63}};
	static uint32_t mb_width;
	static uint32_t mb_height;
/*start codes*/
	static const uint32_t picture_start_code   = 0x00000100;
	static const uint32_t slice_start_code_start = 0x00000101;
	static const uint32_t slice_start_code_end = 0x000001af;
	static const uint32_t user_data_start_code = 0x000001b2;
	static const uint32_t sequence_header_code = 0x000001b3;
	static const uint32_t sequence_error_code  = 0x000001b4;
	static const uint32_t extension_start_code = 0x000001b5;
	static const uint32_t sequence_end_code    = 0x000001b7;
	static const uint32_t group_start_code     = 0x000001b8;

/*sequence header*/
	void sequence_header();	
	static uint32_t horizontal_size;
	static uint32_t vertical_size;
	static uint32_t pel_aspect_ratio;
	static uint32_t picture_rate;
	static uint32_t bit_rate;
	static uint32_t marker_bit;
	static uint32_t vbv_buffer_size;
	static uint32_t constrained_parameter_flag;
	static uint32_t load_intra_quantizer_matrix;
	static int intra_quantizer_matrix[8][8] = {  { 8, 16, 19, 22, 26, 27, 29, 34},
													  {16, 16, 22, 24, 27, 29, 34, 37},
													  {19, 22, 26, 27, 29, 34, 34, 38},
													  {22, 22, 26, 27, 29, 34, 37, 40},
													  {22, 26, 27, 29, 32, 35, 40, 48},
													  {26, 27, 29, 32, 35, 40, 48, 58},
													  {26, 27, 29, 34, 38, 46, 56, 69},
													  {27, 29, 35, 38, 46, 56, 69, 83}};
	static int load_non_intra_quantizer_matrix;
	static int non_intra_quantizer_matrix[8][8]= {  {16, 16, 16, 16, 16, 16, 16, 16},
														 {16, 16, 16, 16, 16, 16, 16, 16},
														 {16, 16, 16, 16, 16, 16, 16, 16},
														 {16, 16, 16, 16, 16, 16, 16, 16},
														 {16, 16, 16, 16, 16, 16, 16, 16},
														 {16, 16, 16, 16, 16, 16, 16, 16},
														 {16, 16, 16, 16, 16, 16, 16, 16},
														 {16, 16, 16, 16, 16, 16, 16, 16}};
	static int sequence_extension_data;
	static uint32_t intra_quantizer_matrix_zz[64] = {  8, 16, 19, 22, 26, 27, 29, 34,
													  16, 16, 22, 24, 27, 29, 34, 37,
													  19, 22, 26, 27, 29, 34, 34, 38,
													  22, 22, 26, 27, 29, 34, 37, 40,
													  22, 26, 27, 29, 32, 35, 40, 48,
													  26, 27, 29, 32, 35, 40, 48, 58,
													  26, 27, 29, 34, 38, 46, 56, 69,
													  27, 29, 35, 38, 46, 56, 69, 83};
/*Group of Pictures Layer*/
	void group_of_pictures();
	static uint32_t time_code;
	static uint32_t drop_frame_flag;
	static uint32_t time_code_hours;
	static uint32_t time_code_minutes;
	static uint32_t time_code_seconds;
	static uint32_t time_code_pictures;
	static uint32_t closed_gop;
	static uint32_t broken_link;
	static uint32_t group_extension_data;

/*Picture Layer*/
	void picture();
	static uint32_t temporal_reference;
	static uint32_t picture_coding_type;
	static uint32_t vbv_delay;
	static uint32_t full_pel_forward_vector;
	static uint32_t forward_f_code;
	static uint32_t forward_r_size;
	static uint32_t forward_f;
	static uint32_t full_pel_backward_vector;
	static uint32_t backward_f_code;
	static uint32_t backward_r_size;
	static uint32_t backward_f;
	static uint32_t extra_bit_picture;
	static uint32_t extra_information_picture;
	static uint32_t picture_extension_data;

/*Slice Layer*/
	void slice();
	static int quantizer_scale;
	static uint32_t extra_bit_slice;
	static uint32_t extra_information_slice;
	static uint32_t slice_vertical_position;

/*Macroblock Layer*/
	void macroblock();
	static uint32_t macroblock_stuffing;
	static uint32_t macroblock_escape;
	static int macroblock_address_increment;
	static uint32_t previous_macroblock_address;
	static uint32_t macroblock_address;
	static uint32_t mb_row;
	static uint32_t mb_column;
	static int macroblock_type;
	static uint32_t macroblock_quant;
	static uint32_t macroblock_motion_forward;
	static uint32_t macroblock_motion_backward;
	static uint32_t macroblock_pattern;
	static uint32_t macroblock_intra;
	static uint32_t motion_horizontal_forward_code;
	static uint32_t motion_horizontal_forward_r;
	static uint32_t motion_vertical_forward_code;
	static uint32_t motion_vertical_forward_r;
	static uint32_t motion_horizontal_backward_code;
	static uint32_t motion_horizontal_backward_r;
	static uint32_t motion_vertical_backward_code;
	static uint32_t motion_vertical_backward_r;
	static uint32_t coded_block_pattern;
	static uint32_t end_of_macroblock;
	static uint32_t pattern_code[6];
/*Block Layer*/	
	void block(int i);
	static int dct_dc_size_luminance;
	static uint32_t dct_dc_differential;
	static int dct_dc_size_chrominance;
	static int dct_coeff_first;
	static int dct_coeff_next;
	static uint32_t end_of_block;
	static int dct_zz[64] = {};
	static int dct_recon[8][8];
	static int dct_dc_y_past;
	static int dct_dc_cb_past;
	static int dct_dc_cr_past;
	static int past_intra_address;
int decode_init(char *filename, int debug);
void decode_video_sequence();
int lookup(uint32_t bits, const int code[], const int length[], const int value[], int size, int *_run, int *_level);
#endif