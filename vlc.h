#include <stdint.h>

/*Macroblock Addressing*/
static const int mbai_code[33] = {1, 3, 2, 3, 2, 3, 2, 7, 6, 11, 10, 9, 8, 7, 6, 23, 22, 21, 20, 19, 18, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24};
static const int mbai_code_length[33] = {1, 3, 3, 4, 4, 5, 5, 7, 7, 8, 8, 8, 8, 8, 8, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11};
static const int mbai_code_value[33] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
											 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33};
/*I-pictures*/
static const int mbtype_i[2] = {0b1, 0b01};
static const int mbtype_i_length[2] = {1, 2};
static const int mbtype_i_value[2] = {0b0001, 0b1001};
/*P-pictures*/
static const int mbtype_p[7] = {0b1, 0b01, 0b001, 0b00011, 0b00010, 0b00001, 0b000001};
static const int mbtype_p_length[7] = {1, 2, 3, 5, 5, 5, 6};
static const int mbtype_p_value[7] = {0b01010, 0b00010, 0b01000, 0b00001, 0b11010, 0b10010, 0b10001};
/*B-pictures*/
static const int mbtype_b[11] = {0b10, 0b11, 0b010, 0b011, 0b0010, 0b0011, 0b00011, 0b00010, 0b000011, 0b000010, 0b000001};
static const int mbtype_b_length[11] = {2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6};
static const int mbtype_b_value[11] = {0b01100, 0b01110, 0b00100, 0b00110, 0b01000, 0b01010, 0b00001, 0b11110, 0b11010, 0b10110, 0b10001};
/*D-pictures*/
static const int mbtype_d[1] = {1};
static const int mbtype_d_length[1] = {1};
static const int mbtype_d_value[1] = {0b00001};
/*Macroblock Pattern*/
static const int cbp_code[63] = { 		 	0b111,      0b1101,      0b1100,      0b1011,      0b1010,
									      0b10011,     0b10010,     0b10001,     0b10000,     0b01111,
									      0b01110,     0b01101,     0b01100,     0b01011,     0b01010, 
									      0b01001,     0b01000,    0b001111,    0b001110,    0b001101, 
									     0b001100,   0b0010111,   0b0010110,   0b0010101,   0b0010100, 
									    0b0010011,   0b0010010,   0b0010001,   0b0010000,  0b00011111, 
									   0b00011110,  0b00011101,  0b00011100,  0b00011011,  0b00011010, 
									   0b00011001,  0b00011000,  0b00010111,  0b00010110,  0b00010101, 
									   0b00010100,  0b00010011,  0b00010010,  0b00010001,  0b00010000,
									   0b00001111,  0b00001110,  0b00001101,  0b00001100,  0b00001011, 
									   0b00001010,  0b00001001,  0b00001000,  0b00000111,  0b00000110, 
									   0b00000101,  0b00000100, 0b000000111, 0b000000110, 0b000000101, 
									  0b000000100, 0b000000011, 0b000000010};
static const int cbp_code_length[63] = {3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 
											 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9};
static const int cbp_code_value[63] = {60,  4,  8, 16, 32, 12, 48, 20, 40, 28, 
								  44, 52, 56,  1, 61,  2, 62, 24, 36,  3, 
								  63,  5,  9, 17, 33,  6, 10, 18, 34,  7, 
								  11, 19, 35, 13, 49, 21, 41, 14, 50, 22, 
								  42, 15, 51, 23, 43, 25, 37, 26, 38, 29, 
								  45, 53, 57, 30, 46, 54, 58, 31, 47, 55, 59, 27, 39};
/*Motion Vectors*/
static const int motion_code[33] = {0b00000011001, 0b00000011011, 0b00000011101, 0b00000011111, 0b00000100001, 0b00000100011, 
									 0b0000010011,  0b0000010101,  0b0000010111,    0b00000111,    0b00001001,    0b00001011,
									    0b0000111,       0b00011,        0b0011,         0b011,           0b1,         0b010, 
									       0b0010,       0b00010,     0b0000110,    0b00001010,    0b00001000,    0b00000110, 
									 0b0000010110,  0b0000010100,  0b0000010010, 0b00000100010, 0b00000100000, 0b00000011110, 
									0b00000011100, 0b00000011010, 0b00000011000};
static const int motion_code_length[33] = {11, 11, 11, 11, 11, 11, 10, 10, 10, 8, 8, 8, 7, 5, 4, 3, 1, 
												3, 4, 5, 7, 8, 8, 8, 10, 10, 10, 11, 11, 11, 11, 11, 11};
static const int motion_code_value[33] = {-16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 
										  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
/*DCT Coefficients luminance*/
static const int dct_dc_size_luminance_code[9] = {0b100, 0b00, 0b01, 0b101, 0b110, 0b1110, 0b11110, 0b111110, 0b1111110};
static const int dct_dc_size_luminance_length[9] = {3, 2, 2, 3, 3, 4, 5, 6, 7};
static const int dct_dc_size_luminance_value[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
/*DCT Coefficients chominance*/
static const int dct_dc_size_chrominance_code[9] = {0b00, 0b01, 0b10, 0b110, 0b1110, 0b11110, 0b111110, 0b1111110, 0b11111110};
static const int dct_dc_size_chrominance_length[9] = {2, 2, 2, 3, 4, 5, 6, 7, 8};
static const int dct_dc_size_chrominance_value[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
/*dct_coeff_first*/
static const int dct_coeff_first_code[113]
static const int dct_coeff_first_length[113]
static const int dct_coeff_first_run[113]
static const int dct_coeff_first_level[113]
/*dct_coeff_second*/
static const int dct_coeff_second_code[113]
static const int dct_coeff_second_length[113]
static const int dct_coeff_second_run[113]
static const int dct_coeff_second_level[113]