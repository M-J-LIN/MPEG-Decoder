#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "bitstream.h"
#include "vlc.h"

static int DEBUG = 0;

InBitStream bitstream;
int decode_init(char *filename, int debug){
	DEBUG = debug;
	bitstream.open(filename);
	if(bitstream.isOpen()){
		printf("File %s is opened.\n", filename);
		return 1;
	}
	else{
		printf("Open file error.\n");
		return 0;
	}
}
int lookup(uint32_t bits, const int code[], const int length[], const int value[], int size){
	for(int i = 0; i < size; i++){		
		if(bits>>(32-length[i]) == code[i]){
			int redundant = bitstream.read(length[i]);
			return value[i];
		}
	}
	printf("lookup error!\n");
	exit(0);
}
int lookup_coeff(uint32_t bits, const int code[], const int length[], const int run[], const int level[], int size, int *_run, int *_level){
	for(int i = 0; i < size; i++){		
		if(bits>>(32-length[i]) == code[i]){
			int redundant = bitstream.read(length[i]);
			if(run[i] == -1){
				return -1;
			}
			else if(run[i] == -2){
				if(DEBUG) printf("dct_dc_first: ESCAPE\n");
	            *_run = bitstream.read(6);
	            int prefix = bitstream.read(8);
	            if(prefix == 0x80) // '1000 0000'
	                *_level = -256 + bitstream.read(8);
	            else if(prefix == 0x0) // '0000 0000'
	                *_level = bitstream.read(8);
	            else {
	                if(prefix>>7 == 1) *_level = -128 + (prefix&0x7f);
	                else *_level = prefix;
	            }
	            return 0;
			}
			else{
				*_run = run[i];
				if(bitstream.read(1) == 0) *_level = level[i];
				else *_level = -level[i];
				return 0;
			}
			
		}
	}
	printf("lookup_coeff error!\n");
	exit(0);
}
void block(int i){	
	memset(dct_zz, 0, sizeof(dct_zz[0])*64);
	int zz_pos = 0, run, level, ret;
	if(pattern_code[i]){
		if(DEBUG)
			printf("==BLOCK(%d)==\n", i);
		if(macroblock_intra){
			if(i<4){
				dct_dc_size_luminance = lookup((uint32_t)bitstream.nextbits(), dct_dc_size_luminance_code, dct_dc_size_luminance_length, 
												dct_dc_size_luminance_value, 9);
				if(dct_dc_size_luminance != 0){ 
					dct_dc_differential = bitstream.read(dct_dc_size_luminance);
					if ( dct_dc_differential & ( 1 << (dct_dc_size_luminance-1)) ) dct_zz[0] = dct_dc_differential ;
					else dct_zz[0] = ( -1 << (dct_dc_size_luminance) ) | (dct_dc_differential+1) ;
				}
				if(DEBUG){
					printf("\tdct_dc_size_luminance: %d\n", dct_dc_size_luminance);
					printf("\tdct_dc_differential_luminance: %d\n", dct_dc_differential);
				}
			}
			else{
				dct_dc_size_chrominance = lookup((uint32_t)bitstream.nextbits(), dct_dc_size_chrominance_code, dct_dc_size_chrominance_length, 
												dct_dc_size_chrominance_value, 9);
				if(dct_dc_size_chrominance != 0){ 
					dct_dc_differential = bitstream.read(dct_dc_size_chrominance);
					if ( dct_dc_differential & ( 1 << (dct_dc_size_chrominance-1)) ) dct_zz[zz_pos++] = dct_dc_differential ;
					else dct_zz[zz_pos++] = ( -1 << (dct_dc_size_chrominance) ) | (dct_dc_differential+1) ;
				}
				if(DEBUG){
					printf("\tdct_dc_size_chrominance: %d\n", dct_dc_size_chrominance);
					printf("\tdct_dc_differential_chrominance: %d\n", dct_dc_differential);
				}
			}
		}
		else{
			ret = lookup_coeff((uint32_t)bitstream.nextbits(), dct_coeff_first_code, dct_coeff_first_length, dct_coeff_first_run, dct_coeff_first_level, 113, &run, &level);
			if(ret == -1)
				printf("EOB-1\n");
			else{
				printf("dct_coeff_first: run = %d, level = %d\n", run, level);
				zz_pos = run;
				dct_zz[zz_pos] = level;
			}
		}
		if(picture_coding_type != 4){
			while(bitstream.nextbits()>>30 != 0b10) {
				ret = lookup_coeff((uint32_t)bitstream.nextbits(), dct_coeff_second_code, dct_coeff_second_length, dct_coeff_second_run, dct_coeff_second_level, 113, &run, &level);
				if(ret == -1){
					printf("EOB-2\n");
					break;
				}
				else{
					printf("dct_coeff_next: run = %d, level = %d\n", run, level);
					zz_pos += (run+1);
					dct_zz[zz_pos] = level;
				}
			}
		}
	}
}
void macroblock(){
	static int mcb_cnt = 1;
	if(DEBUG)
		printf("==macroblock(%d)==\n", mcb_cnt++);
	while(bitstream.nextbits()>>21 == 0b00000001111)
		macroblock_stuffing = bitstream.read(11);
	macroblock_address_increment = 0;
	while(bitstream.nextbits()>>21 == 0b00000001000){
		macroblock_escape = bitstream.read(11);
		macroblock_address_increment += 33;
	}
	macroblock_address_increment += lookup((uint32_t)bitstream.nextbits(), mbai_code, mbai_code_length, mbai_code_value, 33);
	if(picture_coding_type == 1) macroblock_type = lookup((uint32_t)bitstream.nextbits(), mbtype_i, mbtype_i_length, mbtype_i_value, 2); // I
	else if(picture_coding_type == 2) macroblock_type = lookup((uint32_t)bitstream.nextbits(), mbtype_p, mbtype_p_length, mbtype_p_value, 7); // P
	else if (picture_coding_type == 3) macroblock_type = lookup((uint32_t)bitstream.nextbits(), mbtype_b, mbtype_b_length, mbtype_b_value, 11); // B
	else macroblock_type = lookup((uint32_t)bitstream.nextbits(), mbtype_d, mbtype_d_length, mbtype_d_value, 1); // D
	macroblock_quant = macroblock_type>>4;
	macroblock_motion_forward = (macroblock_type>>3)&1;
	macroblock_motion_backward = (macroblock_type>>2)&1;
	macroblock_pattern = (macroblock_type>>1)&1;
	macroblock_intra = macroblock_type&1;

	if(macroblock_quant) quantizer_scale = bitstream.read(5);
	if(macroblock_motion_forward){
		motion_horizontal_forward_code = lookup((uint32_t)bitstream.nextbits(), motion_code, motion_code_length, motion_code_value, 33);
		if(forward_f != 1 && motion_horizontal_forward_code != 0) motion_horizontal_forward_r = bitstream.read(forward_r_size);
		motion_vertical_forward_code = lookup((uint32_t)bitstream.nextbits(), motion_code, motion_code_length, motion_code_value, 33);
		if(forward_f != 1 && motion_vertical_forward_code != 0) motion_vertical_forward_r = bitstream.read(forward_r_size);
	}
	if(macroblock_motion_backward){
		motion_horizontal_backward_code = lookup((uint32_t)bitstream.nextbits(), motion_code, motion_code_length, motion_code_value, 33);
		if(backward_f != 1 && motion_horizontal_backward_code != 0) motion_horizontal_backward_r = bitstream.read(backward_r_size);
		motion_vertical_backward_code = lookup((uint32_t)bitstream.nextbits(), motion_code, motion_code_length, motion_code_value, 33);
		if(backward_f != 1 && motion_vertical_backward_code != 0) motion_vertical_backward_r = bitstream.read(backward_r_size);
	}
	coded_block_pattern = 0;
	if(macroblock_pattern) coded_block_pattern = lookup((uint32_t)bitstream.nextbits(), cbp_code, cbp_code_length, cbp_code_value, 63);
	if(DEBUG)
		printf("macroblock_address_increment = %d, macroblock_type = %d, cbp = %d\n", macroblock_address_increment, macroblock_type, coded_block_pattern);
	memset(pattern_code, 0, sizeof(pattern_code[0])*6);
	for (int i = 0; i < 6; ++i)
	{
		if(coded_block_pattern&(1<<(5-i))) pattern_code[i] = 1;
		if(macroblock_intra) pattern_code[i] = 1;
	}
	for(int i = 0; i < 6; i++)
		block(i);
	if(picture_coding_type == 4) end_of_macroblock = bitstream.read(1);
	

}
void slice(){
	int redundant = bitstream.read(32);
	if(redundant < slice_start_code_start || redundant > slice_start_code_end)
		return;
	quantizer_scale = bitstream.read(5);
	while(bitstream.nextbits()>>31 == 1){
		extra_bit_slice = bitstream.read(1);
		extra_information_slice = bitstream.read(8);
	}
	extra_bit_slice = bitstream.read(1);
	if(DEBUG)
		printf("quantizer_scale = %d, extra_bit_slice = %d\n", quantizer_scale, extra_bit_slice);
	do{
		macroblock();
	}while(bitstream.nextbits()>>9 != 0);
	bitstream.next_start_code();
}
void picture(){
	static int pic_cnt = 1;
	printf("==picture(%d)==\n", pic_cnt++);
	if(bitstream.read(32) != picture_start_code)
		return;
	temporal_reference = bitstream.read(10);
	picture_coding_type = bitstream.read(3);
	vbv_delay = bitstream.read(16);
	if(picture_coding_type == 2 || picture_coding_type == 3){
		full_pel_forward_vector = bitstream.read(1);
		forward_f_code = bitstream.read(3);
		forward_r_size = forward_f_code-1;
		forward_f = 1<<forward_r_size;
	}
	if(picture_coding_type == 3){
		full_pel_backward_vector = bitstream.read(1);
		backward_f_code = bitstream.read(3);
		backward_r_size = backward_f_code-1;
		backward_f = 1<<backward_r_size;
	}
	while(bitstream.nextbits()>>31 == 1){
		extra_bit_picture = bitstream.read(1);
		extra_information_picture = bitstream.read(8);
	}
	extra_bit_picture = bitstream.read(1);
	if(DEBUG)
		printf("temporal_reference = %d, picture_coding_type = %d, vbv_delay = %d, extra_bit_picture = %d\n", temporal_reference, picture_coding_type, vbv_delay, extra_bit_picture);
	bitstream.next_start_code();
	if(bitstream.nextbits() == extension_start_code){
		int redundant = bitstream.read(32);
		while(bitstream.nextbits()>>8 != 0x000001) picture_extension_data = bitstream.read(8);
		bitstream.next_start_code();
	}
	if(bitstream.nextbits() == user_data_start_code){
		int redundant = bitstream.read(32);
		while(bitstream.nextbits()>>8 != 0x000001) user_data = bitstream.read(8);
		bitstream.next_start_code();
	}
	do{
		slice();
	}while(bitstream.nextbits() >= slice_start_code_start && bitstream.nextbits() <= slice_start_code_end);
}
void group_of_pictures(){
	if(bitstream.read(32) != group_start_code)
		return;
	time_code = bitstream.read(25);
	drop_frame_flag = time_code>>24;
	time_code_hours = time_code>>19&0x1f;
	time_code_minutes = time_code>>13&0x3f;
	time_code_seconds = time_code>>6&0x3f;
	time_code_pictures = time_code&0x3f;
	if(DEBUG)
		printf("time_code_hours = %d, time_code_minutes = %d, time_code_seconds = %d, time_code_pictures = %d\n", 
				time_code_hours, time_code_minutes, time_code_seconds, time_code_pictures);
	closed_gop = bitstream.read(1);
	broken_link = bitstream.read(1);
	bitstream.next_start_code();
	if(DEBUG)
		printf("time_code = %d\n", time_code);
	if(bitstream.nextbits() == extension_start_code){
		int redundant = bitstream.read(32);
		while(bitstream.nextbits()>>8 != 0x000001) group_extension_data = bitstream.read(8);
		bitstream.next_start_code();
	}
	if(bitstream.nextbits() == user_data_start_code){
		int redundant = bitstream.read(32);
		while(bitstream.nextbits()>>8 != 0x000001) user_data = bitstream.read(8);
		bitstream.next_start_code();
	}
	do{
		picture();
	}while(bitstream.nextbits() == picture_start_code);
}
void sequence_header(){
	if(bitstream.read(32) != sequence_header_code)
		return;
	horizontal_size = bitstream.read(12);
	vertical_size = bitstream.read(12);
	pel_aspect_ratio = bitstream.read(4);
	picture_rate = bitstream.read(4);
	bit_rate = bitstream.read(18);
	marker_bit = bitstream.read(1);
	vbv_buffer_size = bitstream.read(10);
	constrained_parameter_flag = bitstream.read(1);
	load_intra_quantizer_matrix = bitstream.read(1);
	if(load_intra_quantizer_matrix){
		for(int i = 0; i < 64; i++) intra_quantizer_matrix[i] = bitstream.read(8);
	}
	load_non_intra_quantizer_matrix = bitstream.read(1);
	if(load_non_intra_quantizer_matrix){
		for(int i = 0; i < 64; i++) non_intra_quantizer_matrix[i] = bitstream.read(8);
	}
	bitstream.next_start_code();
	if(bitstream.nextbits() == extension_start_code){
		int redundant = bitstream.read(32);
		while(bitstream.nextbits()>>8 != 0x000001) sequence_extension_data = bitstream.read(8);
		bitstream.next_start_code();
	}
	if(bitstream.nextbits() == user_data_start_code){
		int redundant = bitstream.read(32);
		while(bitstream.nextbits()>>8 != 0x000001) user_data = bitstream.read(8);
		bitstream.next_start_code();
	}
	if(DEBUG){
		printf("horizontal_size = %d, vertical_size = %d\n", horizontal_size, vertical_size);
		printf("pel_aspect_ratio = %d", pel_aspect_ratio);
		printf("picture_rate = %d\n", picture_rate);
		printf("bit_rate = %d\n", bit_rate);
		printf("marker_bit = %d\n", marker_bit);
		printf("vbv_buffer_size = %d\n", vbv_buffer_size);
		printf("constrained_parameter_flag = %d\n", constrained_parameter_flag);
		printf("load_intra_quantizer_matrix = %d\n", load_intra_quantizer_matrix);
	}
	
}
void decode_video_sequence(){
	bitstream.next_start_code();
	do{
		sequence_header();
		do{
			group_of_pictures();
		}while(bitstream.nextbits() == group_start_code);
	}while(bitstream.nextbits() == sequence_header_code);

	printf("end\n");
}