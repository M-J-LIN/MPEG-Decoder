#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
/*IDCT*/
void idct(int matrix[8][8]) {
    double tmp[8][8];
    for(int x = 0; x < 8; x++) {
        double t0 = (double)matrix[0][x] * Cn[4];
        double t1 = (double)matrix[2][x] * Cn[2];
        double t2 = (double)matrix[2][x] * Cn[6];
        double t3 = (double)matrix[4][x] * Cn[4];
        double t4 = (double)matrix[6][x] * Cn[6];
        double t5 = (double)matrix[6][x] * Cn[2];
        double a0 = t0 + t1 + t3 + t4;
        double a1 = t0 + t2 - t3 - t5;
        double a2 = t0 - t2 - t3 + t5;
        double a3 = t0 - t1 + t3 - t4;
        double x1 = (double)matrix[1][x];
        double x3 = (double)matrix[3][x];
        double x5 = (double)matrix[5][x];
        double x7 = (double)matrix[7][x];
        double b0 = x1*Cn[1] + x3*Cn[3] + x5*Cn[5] + x7*Cn[7];
        double b1 = x1*Cn[3] - x3*Cn[7] - x5*Cn[1] - x7*Cn[5];
        double b2 = x1*Cn[5] - x3*Cn[1] + x5*Cn[7] + x7*Cn[3];
        double b3 = x1*Cn[7] - x3*Cn[5] + x5*Cn[3] - x7*Cn[1];
        tmp[0][x] = a0 + b0;
        tmp[7][x] = a0 - b0;
        tmp[1][x] = a1 + b1;
        tmp[6][x] = a1 - b1;
        tmp[2][x] = a2 + b2;
        tmp[5][x] = a2 - b2;
        tmp[3][x] = a3 + b3;
        tmp[4][x] = a3 - b3;
    }
    for(int y = 0; y < 8; y++) {
        double t0 = tmp[y][0] * Cn[4];
        double t1 = tmp[y][2] * Cn[2];
        double t2 = tmp[y][2] * Cn[6];
        double t3 = tmp[y][4] * Cn[4];
        double t4 = tmp[y][6] * Cn[6];
        double t5 = tmp[y][6] * Cn[2];
        double a0 = t0 + t1 + t3 + t4;
        double a1 = t0 + t2 - t3 - t5;
        double a2 = t0 - t2 - t3 + t5;
        double a3 = t0 - t1 + t3 - t4;
        double x1 = (double)tmp[y][1];
        double x3 = (double)tmp[y][3];
        double x5 = (double)tmp[y][5];
        double x7 = (double)tmp[y][7];
        double b0 = x1*Cn[1] + x3*Cn[3] + x5*Cn[5] + x7*Cn[7];
        double b1 = x1*Cn[3] - x3*Cn[7] - x5*Cn[1] - x7*Cn[5];
        double b2 = x1*Cn[5] - x3*Cn[1] + x5*Cn[7] + x7*Cn[3];
        double b3 = x1*Cn[7] - x3*Cn[5] + x5*Cn[3] - x7*Cn[1];
        matrix[y][0] = 0.5 + a0 + b0;
        matrix[y][7] = 0.5 + a0 - b0;
        matrix[y][1] = 0.5 + a1 + b1;
        matrix[y][6] = 0.5 + a1 - b1;
        matrix[y][2] = 0.5 + a2 + b2;
        matrix[y][5] = 0.5 + a2 - b2;
        matrix[y][3] = 0.5 + a3 + b3;
        matrix[y][4] = 0.5 + a3 - b3;
    }
}
/*using y cb cr to combine R*/
int R(int y, int cb, int cr){
	float sum = y+1.402*(cr-128);
	if(sum >= 255)
		return 255;
	else if(sum <= 0)
		return 0;
	else
		return sum;
}
/*using y cb cr to combine G*/
int G(int y, int cb, int cr){
	float sum = y-0.34414*(cb-128)-0.71414*(cr-128);
	if(sum >= 255)
		return 255;
	else if(sum <= 0)
		return 0;
	else
		return sum;
}
/*using y cb cr to combine B*/
int B(int y, int cb, int cr){
	float sum = y+1.772*(cb-128);
	if(sum >= 255)
		return 255;
	else if(sum <= 0)
		return 0;
	else
		return sum;
}
/*output BMP file*/
static uint8_t bmp[1024][1024][3];
void genbmp(int height, int width, int frame_num){
	typedef struct { 
		/* type : Magic identifier,一般為BM(0x42,0x4d) */ 
		unsigned short int type; 
		unsigned int size;/* File size in bytes,全部的檔案大小 */ 
		unsigned short int reserved1, reserved2; /* 保留欄位 */ 
		unsigned int offset;/* Offset to image data, bytes */ 
	} FILEHEADER;
	typedef struct { 
		unsigned int size;/* Info Header size in bytes */ 
		int width,height;/* Width and height of image */ 
		unsigned short int planes;/* Number of colour planes */ 
		unsigned short int bits; /* Bits per pixel */ 
		unsigned int compression; /* Compression type */ 
		unsigned int imagesize; /* Image size in bytes */ 
		int xresolution,yresolution; /* Pixels per meter */ 
		unsigned int ncolours; /* Number of colours */ 
		unsigned int importantcolours; /* Important colours */ 
	} INFOHEADER;
	FILEHEADER fileheader;
	int Y = height, X = (width*3+3)/4*4;
	fileheader.type = 0x4d42, fileheader.size = 54+X*Y, fileheader.reserved1 = fileheader.reserved2 = 0, fileheader.offset = 54;
	INFOHEADER infoheader;
	infoheader.size = 40, infoheader.width = width, infoheader.height = height, infoheader.planes = 1, infoheader.bits = 24, infoheader.compression = 0;
	infoheader.imagesize = 0, infoheader.xresolution = infoheader.yresolution = 0x0b12, infoheader.ncolours = infoheader.importantcolours = 0;	
	FILE * wfp;
	char filename[100];
	sprintf(filename, "%03d.bmp", frame_num);
	wfp = fopen(filename, "wb");
	fwrite(&fileheader.type, 1, sizeof(fileheader.type), wfp);
	fwrite(&fileheader.size, 1, sizeof(fileheader.size), wfp);
	fwrite(&fileheader.reserved1, 1, sizeof(fileheader.reserved1), wfp);
	fwrite(&fileheader.reserved2, 1, sizeof(fileheader.reserved2), wfp);
	fwrite(&fileheader.offset, 1, sizeof(fileheader.offset), wfp);
	fwrite(&infoheader, 1, sizeof(INFOHEADER), wfp);
	uint8_t imgbuf[X];
	for(int i = Y-1; i >= 0; i--){
		memset(imgbuf, 0, sizeof(imgbuf));
		for(int j = 0; j < width; j++){
			imgbuf[j*3] = bmp[i][j][0];
			imgbuf[j*3+1] = bmp[i][j][1];
			imgbuf[j*3+2] = bmp[i][j][2];
		}
		fwrite(imgbuf, sizeof(uint8_t), X, wfp);
	}
	
	fclose(wfp);	
}
/*generate bmp buffer*/
void BMP(int height, int width, int frame_num, PIC_BUF pic_buf[]){
	int cb_h_idx, cr_h_idx, cb_w_idx, cr_w_idx;
	for (int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			bmp[i][j][2] = R(pic_buf[frame_num].ycbcr[0][i][j], pic_buf[frame_num].ycbcr[1][i>>1][j>>1], pic_buf[frame_num].ycbcr[2][i>>1][j>>1]);
			bmp[i][j][1] = G(pic_buf[frame_num].ycbcr[0][i][j], pic_buf[frame_num].ycbcr[1][i>>1][j>>1], pic_buf[frame_num].ycbcr[2][i>>1][j>>1]);
			bmp[i][j][0] = B(pic_buf[frame_num].ycbcr[0][i][j], pic_buf[frame_num].ycbcr[1][i>>1][j>>1], pic_buf[frame_num].ycbcr[2][i>>1][j>>1]);
		}
	}
	genbmp(height, width, frame_num);
}
/*fill y to picture buffer*/
void fillY(int r, int c, int dct_recon[8][8], int frame_num, PIC_BUF pic_buf[]){
	for(int m=0;m<8;m++) for(int n=0;n<8;n++) pic_buf[frame_num].ycbcr[0][r+m][c+n] = (dct_recon[m][n]<0)?0:(dct_recon[m][n]>255)?255:dct_recon[m][n];
}
/*fill cb to picture buffer*/
void fillCb(int r, int c, int dct_recon[8][8], int frame_num, PIC_BUF pic_buf[]){
	for(int m=0;m<8;m++) for(int n=0;n<8;n++) pic_buf[frame_num].ycbcr[1][r+m][c+n] = (dct_recon[m][n]<0)?0:(dct_recon[m][n]>255)?255:dct_recon[m][n];
}
/*fill cr to picture buffer*/
void fillCr(int r, int c, int dct_recon[8][8], int frame_num, PIC_BUF pic_buf[]){
	for(int m=0;m<8;m++) for(int n=0;n<8;n++) pic_buf[frame_num].ycbcr[2][r+m][c+n] = (dct_recon[m][n]<0)?0:(dct_recon[m][n]>255)?255:dct_recon[m][n];
}