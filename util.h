#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

static const float pi = 3.14159265;
static const double Cn[8] = {0, 0.490393, 0.461940, 0.415735, 0.353553, 0.277785, 0.191342, 0.097545};
typedef struct PIC_BUF{
	int can_not_write;
	unsigned char ycbcr[3][1024][1024];
}PIC_BUF;
void idct(int matrix[8][8]);
void BMP(int vertical_size, int horizontal_size, int frame_num, PIC_BUF pic_buf[]);
void fillY(int r, int c, int dct_recon[8][8], int frame_num, PIC_BUF pic_buf[]);
void fillCb(int r, int c, int dct_recon[8][8], int frame_num, PIC_BUF pic_buf[]);
void fillCr(int r, int c, int dct_recon[8][8], int frame_num, PIC_BUF pic_buf[]); 
#endif