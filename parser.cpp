#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "bitstream.h"
#include "vlc.h"
#include "util.h"

static int DEBUG = 0;
static int pic_cnt = 0;
#define DO_BMP
InBitStream bitstream;
int Sign(int a){ return (a > 0) ? 1 : (a < 0) ? -1 : 0;}
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
/*decode I-frame*/
void decode_intro_coded_macroblock(int block_num){
	/*recon dct*/
	for (int m=0; m<8; m++) {
		for (int n=0; n<8; n++) {
			int i = scan[m][n] ;
			dct_recon[m][n] = ( dct_zz[i] * quantizer_scale * intra_quantizer_matrix[m][n] ) /8 ;
			if ( ( dct_recon[m][n] & 1 ) == 0 )	dct_recon[m][n] -= Sign(dct_recon[m][n]) ;
			if (dct_recon[m][n] > 2047) dct_recon[m][n] = 2047 ;
			if (dct_recon[m][n] < -2048) dct_recon[m][n] = -2048 ;
		}
	}
	int dct_dc_add = 0;
    if(block_num < 4) {
        if(block_num == 0 && macroblock_address - past_intra_address > 1) dct_dc_add = 1024;
        else dct_dc_add = dct_dc_y_past;
    }
    else {
        if(macroblock_address - past_intra_address > 1) dct_dc_add = 1024;
        else {
            if(block_num == 4) dct_dc_add = dct_dc_cb_past;
            else dct_dc_add = dct_dc_cr_past;
        }
    }
    dct_recon[0][0] = dct_dc_add + dct_zz[0] * 8;
     
    if(block_num < 4) dct_dc_y_past = dct_recon[0][0];
    else if(block_num == 4) dct_dc_cb_past = dct_recon[0][0];
    else dct_dc_cr_past = dct_recon[0][0];
    past_intra_address = macroblock_address ;
    /*IDCT*/
    int dest[8][8];
    IDCT(dest, dct_recon);
    //IDCT(dct_recon);
    /*if(DEBUG){
    	for(int i = 0; i < 8; i++){
    		for(int j = 0; j < 8; j++){
    			printf("%3d ", dct_recon[i][j]);
    		}
    		printf("\n");
    	}
    }
    system("pause");*/
    if(block_num < 4) {
        int r = (mb_row<<4) + ((block_num>>1)<<3); // mb_row*16 + block/2*8
        int c = (mb_column<<4) + ((block_num&1)<<3); // mb_column*16 + block%2*8
        fillY(r, c, dest, pic_cnt);
    }
    else {
        int r = mb_row<<3; // mb_row*8
        int c = mb_column<<3; // mb_column*8
        if(block_num == 4)
           fillCb(r, c, dest, pic_cnt);
        else
           fillCr(r, c, dest, pic_cnt);
    }
}
/*decode P-frame*/
void decode_predictive_coded_macroblocks_p(int block_num){
	;
}
/*decode B-frame*/
void decode_predictive_coded_macroblocks_b(int block_num){
	;
}
void block(int block_num){	
	memset(dct_zz, 0, sizeof(int)*64);
	int zz_pos = 0, run, level, ret;
	if(pattern_code[block_num]){
		if(DEBUG)
			printf("==BLOCK(%d)==\n", block_num);
		if(macroblock_intra){
			if(block_num<4){
				//dct_dc_size_luminance = dct_dc_differential = 0;
				dct_dc_size_luminance = lookup((uint32_t)bitstream.nextbits(), dct_dc_size_luminance_code, dct_dc_size_luminance_length, 
												dct_dc_size_luminance_value, 9);
				if(dct_dc_size_luminance != 0){ 
					dct_dc_differential = bitstream.read(dct_dc_size_luminance);
					if ( dct_dc_differential & ( 1 << (dct_dc_size_luminance-1)) ) dct_zz[zz_pos] = dct_dc_differential ;
					else dct_zz[zz_pos] = ( -1 << (dct_dc_size_luminance) ) | (dct_dc_differential+1) ;
				}
				if(DEBUG){
					printf("\tdct_dc_size_luminance: %d\n", dct_dc_size_luminance);
					printf("\tdct_dc_differential_luminance: %d\n", dct_dc_differential);
				}
			}
			else{
				//dct_dc_size_chrominance = dct_dc_differential = 0;
				dct_dc_size_chrominance = lookup((uint32_t)bitstream.nextbits(), dct_dc_size_chrominance_code, dct_dc_size_chrominance_length, 
												dct_dc_size_chrominance_value, 9);
				if(dct_dc_size_chrominance != 0){ 
					dct_dc_differential = bitstream.read(dct_dc_size_chrominance);
					if ( dct_dc_differential & ( 1 << (dct_dc_size_chrominance-1)) ) dct_zz[zz_pos] = dct_dc_differential ;
					else dct_zz[zz_pos] = ( -1 << (dct_dc_size_chrominance) ) | (dct_dc_differential+1) ;
				}
				if(DEBUG){
					printf("\tdct_dc_size_chrominance: %d\n", dct_dc_size_chrominance);
					printf("\tdct_dc_differential_chrominance: %d\n", dct_dc_differential);
				}
			}
		}
		else{
			ret = lookup_coeff((uint32_t)bitstream.nextbits(), dct_coeff_first_code, dct_coeff_first_length, dct_coeff_first_run, dct_coeff_first_level, 113, &run, &level);
			if(ret == -1){
				if(DEBUG)
					printf("EOB-1\n");
			}
			else{
				if(DEBUG)
					printf("dct_coeff_first: run = %d, level = %d\n", run, level);
				zz_pos = run;
				dct_zz[zz_pos] = level;
			}
		}
		if(picture_coding_type != 4){
			while(bitstream.nextbits()>>30 != 0b10) {
				ret = lookup_coeff((uint32_t)bitstream.nextbits(), dct_coeff_second_code, dct_coeff_second_length, dct_coeff_second_run, dct_coeff_second_level, 113, &run, &level);
				if(ret == -1){
					if(DEBUG)
						printf("EOB-2\n");
					break;
				}
				else{
					if(DEBUG)
						printf("dct_coeff_next: run = %d, level = %d\n", run, level);
					zz_pos += run+1;
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
	macroblock_address = previous_macroblock_address + macroblock_address_increment;
    previous_macroblock_address = macroblock_address;
    mb_row = macroblock_address / mb_width;
    mb_column = macroblock_address % mb_width;
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
	for (int i = 0; i < 6; i++)
	{
		if(coded_block_pattern&(1<<(5-i))) pattern_code[i] = 1;
		if(macroblock_intra) pattern_code[i] = 1;
	}
	for(int i = 0; i < 6; i++){
		block(i);
		if(macroblock_intra) decode_intro_coded_macroblock(i);
		else if(picture_coding_type == 2) decode_predictive_coded_macroblocks_p(i);
		else decode_predictive_coded_macroblocks_b(i);
	}
	
	if(picture_coding_type == 4) end_of_macroblock = bitstream.read(1);

}
void slice(){
	int start_code = bitstream.read(32);
	if(start_code < slice_start_code_start || start_code > slice_start_code_end)
		return;
	slice_vertical_position = start_code&0xff;
	previous_macroblock_address=(slice_vertical_position-1)*mb_width-1;
	dct_dc_y_past = 1024;
    dct_dc_cb_past = 1024;
    dct_dc_cr_past = 1024;
    past_intra_address = -2;
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
	
	if(DEBUG)
		printf("==picture(%d)==\n", pic_cnt);
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
	
#ifdef DO_BMP
    BMP((int)vertical_size, (int)horizontal_size, pic_cnt);
#endif
   pic_cnt++; 
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
	mb_width = (horizontal_size+15)/16;
	mb_height = (vertical_size+15)/16;
	vertical_size = bitstream.read(12);
	pel_aspect_ratio = bitstream.read(4);
	picture_rate = bitstream.read(4);
	bit_rate = bitstream.read(18);
	marker_bit = bitstream.read(1);
	vbv_buffer_size = bitstream.read(10);
	constrained_parameter_flag = bitstream.read(1);
	load_intra_quantizer_matrix = bitstream.read(1);
	if(load_intra_quantizer_matrix){
		for(int i = 0; i < 8; i++)
			for(int j = 0; j < 8; j++)
		 		intra_quantizer_matrix[i][j] = bitstream.read(8);
	}
	load_non_intra_quantizer_matrix = bitstream.read(1);
	if(load_non_intra_quantizer_matrix){
		for(int i = 0; i < 8; i++) 
			for(int j = 0; j < 8; j++)
				non_intra_quantizer_matrix[i][j] = bitstream.read(8);
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