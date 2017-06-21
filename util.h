#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

static const float pi = 3.14159265;
static const double Cn[8] = {0, 0.490393, 0.461940, 0.415735, 0.353553, 0.277785, 0.191342, 0.097545};
static const float cu[8] = {0.707107, 1, 1, 1, 1, 1, 1, 1};
static const float cv[8] = {0.707107, 1, 1, 1, 1, 1, 1, 1};
static const float cos_tb[8][8] = {
	  1.000000,   0.980785,   0.923880,   0.831470,   0.707107,   0.555570,   0.382683,   0.195090, 
	  1.000000,   0.831470,   0.382683,  -0.195090,  -0.707107,  -0.980785,  -0.923880,  -0.555570,
	  1.000000,   0.555570,  -0.382683,  -0.980785,  -0.707107,   0.195090,   0.923880,   0.831470,
	  1.000000,   0.195090,  -0.923880,  -0.555570,   0.707107,   0.831470,  -0.382683,  -0.980785, 
	  1.000000,  -0.195090,  -0.923880,   0.555570,   0.707107,  -0.831470,  -0.382683,   0.980785, 
	  1.000000,  -0.555570,  -0.382683,   0.980785,  -0.707107,  -0.195090,   0.923880,  -0.831470, 
	  1.000000,  -0.831470,   0.382683,   0.195090,  -0.707107,   0.980785,  -0.923880,   0.555570, 
	  1.000000,  -0.980785,   0.923880,  -0.831470,   0.707107,  -0.555570,   0.382683,  -0.195090 };
typedef struct PIC_BUF{
	int y[1024][1024];
	int cb[256][256];
	int cr[256][256];
}PIC_BUF;
void idct(int matrix[8][8]);
void BMP(int vertical_size, int horizontal_size, int frame_num);
void IDCT(int dest[8][8], int source[8][8]);
void fillY(int r, int c, int dct_recon[8][8], int frame_num);
void fillCb(int r, int c, int dct_recon[8][8], int frame_num);
void fillCr(int r, int c, int dct_recon[8][8], int frame_num); 
#endif