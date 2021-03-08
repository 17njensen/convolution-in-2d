/*
Convolving an image with the provided impulse signals
*/


#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
	int ndim;	//number of dimensions
	int nchan;	//number of channels
	int d0;		//length of the first dimension
	int d1;		//length of second dimension or sample rate if audio
	int d2;		//length of third dimension
} dsp_file_header;

int main() {
	//impulses 
	int h_0[3][3] = { {-1, 0, 1},
					 {-2, 0, 2},
					 {-1, 0, 1} };
	int h_1[3][3] = { {-1, -2, -1},
					 { 0,  0,  0},
					 { 1,  2,  1} };

	FILE* fx, * fy;
	if (NULL == (fx = fopen("cameraman.bin", "rb")))
	{
		printf("ERROR: Can't open cameraman.bin for input.\n");
		return 0;
	}
	if (NULL == (fy = fopen("conv_cameraman.bin", "wb")))
	{
		printf("ERROR: cant open conv_cameraman.bin for output.\n");
		return 0;
	}

	//grab headers of each file
	dsp_file_header h0, ho;
	fread(&h0, sizeof(dsp_file_header), 1, fx);
	memcpy(&ho, &h0, sizeof(dsp_file_header));
	printf("Input is %d by %d\n", h0.d0, h0.d1);
	//adjust dimensions due to convolution
	ho.d0 = ho.d0 + 3 - 1;
	ho.d1 = ho.d1 + 3 - 1;
	//for x debug
	//ho.d0 = ho.d0 + 4; 
	//ho.d1 = ho.d1 + 4; 
	ho.nchan = 1;
	fwrite(&ho, sizeof(dsp_file_header), 1, fy);

	//determine dimensions
	int Rh = 3; //size of impulse
	int Ch = 3; 
	int Rx = h0.d0; //size of image rows - 256
	int Cx = h0.d1; //size of image cols - 256
	int Ry = Rx + Rh - 1; //len of conv - 1 256 1
	int Cy = Cx + Ch - 1; //len of conv - 1 256 1
	int Rz = Rx + 2 * (Rh - 1); //padded len - 2 256 2
	int Cz = Cx + 2 * (Ch - 1); //padded len - 2 256 2
	printf("Rx = %d, Cx = %d, Ry = %d, Cy = %d, Rz = %d, Cz = %d\n", Rx, Cx, Ry, Cy, Rz, Cz);
	//allocate dat for file storage
	float* x = (float*)calloc(sizeof(float), Rz*Cz);
	//float* h = (float*)calloc(sizeof(float), Rh*Ch);
	float* y0 = (float*)calloc(sizeof(float), Ry*Cy);
	float* y1 = (float*)calloc(sizeof(float), Ry*Cy);
	float* y = (float*)calloc(sizeof(float), Ry*Cy);

	//read input file
	int a = Ch-1; //start rows after top padding
	while (!feof(fx)) {
		//reads one row of the image after padding
		fread((x + (a*Cz + Ch-1)), sizeof(float), Cx, fx);
		printf("row: %d\n", a);
		a++; //next row
	}
	printf("X is now %d by %d\n", Rz, Cz);

	int i, ii;
	float tmp0;
	float tmp1;
	//Y0
	for (int k = 0; k < Ry; k++) { //m - rows
		for (int l = 0; l < Cy; l++) { //by n - by cols
			for (tmp0 = 0.0, i = 0; i < Rh; i++) {
				for (int j = 0; j < Ch; j++) {
					tmp0 += h_0[i][j] * x[(k+i)*Cz + (l+j)]; 
				}
			}
			y0[k*Cy + l] = tmp0;
		}
	}
	printf("Y0 has been processed\n");

	//Y1
	for (int kk = 0; kk < Ry; kk++) {
		for (int ll = 0; ll < Cy; ll++) {
			for (tmp1 = 0.0, ii = 0; ii < Rh; ii++) {
				for (int jj = 0; jj < Ch; jj++) {
					tmp1 += h_1[ii][jj] * x[(kk+ii)*Cz + (ll+jj)];
				}
			}
			y1[kk*Cy + ll] = tmp1;
		}
	}
	printf("Y1 has been processed\n");

	//combine two outputs
	for (int m = 0; m < Ry; m++) {
		for (int n = 0; n < Cy; n++) {
			y[m*Cy + n] = sqrt(pow(y0[m*Cy + n],2) + pow(y1[m*Cy + n],2)); 
		}
	}
	printf("y has been processed\n");
	//fwrite(x, sizeof(float), Rz*Cz, fy);
	//fwrite(y, sizeof(float), Ry*Cy, fy);
	//fwrite(y0, sizeof(float), Ry* Cy, fy);
	fwrite(y1, sizeof(float), Ry* Cy, fy);

	printf("y has been written\n");
	fclose(fx);
	fclose(fy);
	return 0;
}
