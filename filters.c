#include "filters.h"

void smooth(int** matr, pgm* p, int n, int m){
	int filter[3][3] = {{1,1,1}, {1,1,1}, {1,1,1}};
	int factor = 9;
	int offset = 0;
	int** newMatr = p->filteredImg;
	for (int i = 1; i <= n-2; i++)
	{
		for (int j = 1; j <= m-2; j++)
		{
			newMatr[i-1][j-1] = (matr[i-1][j-1] * filter[0][0] + matr[i-1][j] * filter[0][1] +
						matr[i-1][j+1]*filter[0][2] + matr[i][j-1] * filter[1][0] + 
						matr[i][j] * filter[1][1] + matr[i][j+1]*filter[1][2] + 
						matr[i+1][j-1] * filter[2][0] + matr[i+1][j] * filter[2][1] + 
						matr[i+1][j+1] * filter[2][2]) / factor + offset;
			if (newMatr[i-1][j-1] > 255)
			{
				newMatr[i-1][j-1] = 255;
			}
			else if (newMatr[i-1][j-1] < 0){
				newMatr[i-1][j-1] = 0;
			}
		}
	}
}

void blur(int** matr, pgm* p, int n, int m){
	int filter[3][3] = {{1,2,1}, {2,4,2}, {1,2,1}};
	int factor = 16;
	int offset = 0;
	int** newMatr = p->filteredImg;
	for (int i = 1; i <= n-2; i++)
	{
		for (int j = 1; j <= m-2; j++)
		{
			newMatr[i-1][j-1] = (matr[i-1][j-1] * filter[0][0] + matr[i-1][j] * filter[0][1] +
						matr[i-1][j+1]*filter[0][2] + matr[i][j-1] * filter[1][0] + 
						matr[i][j] * filter[1][1] + matr[i][j+1]*filter[1][2] + 
						matr[i+1][j-1] * filter[2][0] + matr[i+1][j] * filter[2][1] + 
						matr[i+1][j+1] * filter[2][2]) / factor + offset;
			if (newMatr[i-1][j-1] > 255)
			{
				newMatr[i-1][j-1] = 255;
			}
			else if (newMatr[i-1][j-1] < 0){
				newMatr[i-1][j-1] = 0;
			}
		}

	}
}

void sharpen(int** matr, pgm *p, int n, int m){
	int filter[3][3] = {{0,-2,0}, {-2,11,-2}, {0,-2,0}};
	int factor = 3;
	int offset = 0;
	int** newMatr = p->filteredImg;
	for (int i = 1; i <= n-2; i++)
	{
		for (int j = 1; j <= m-2; j++)
		{
			newMatr[i-1][j-1] = (matr[i-1][j-1] * filter[0][0] + matr[i-1][j] * filter[0][1] +
						matr[i-1][j+1]*filter[0][2] + matr[i][j-1] * filter[1][0] + 
						matr[i][j] * filter[1][1] + matr[i][j+1]*filter[1][2] + 
						matr[i+1][j-1] * filter[2][0] + matr[i+1][j] * filter[2][1] + 
						matr[i+1][j+1] * filter[2][2]) / factor + offset;
			if (newMatr[i-1][j-1] > 255)
			{
				newMatr[i-1][j-1] = 255;
			}
			else if (newMatr[i-1][j-1] < 0){
				newMatr[i-1][j-1] = 0;
			}
		}
	}
}

void meanRemoval(int** matr, pgm *p, int n, int m){
	int filter[3][3] = {{-1,-1,-1}, {-1,9,-1}, {-1,-1,-1}};
	int factor = 1;
	int offset = 0;
	int** newMatr = p->filteredImg;
	for (int i = 1; i <= n-2; i++)
	{
		for (int j = 1; j <= m-2; j++)
		{
			newMatr[i-1][j-1] = (matr[i-1][j-1] * filter[0][0] + matr[i-1][j] * filter[0][1] +
						matr[i-1][j+1]*filter[0][2] + matr[i][j-1] * filter[1][0] + 
						matr[i][j] * filter[1][1] + matr[i][j+1]*filter[1][2] + 
						matr[i+1][j-1] * filter[2][0] + matr[i+1][j] * filter[2][1] + 
						matr[i+1][j+1] * filter[2][2]) / factor + offset;
			if (newMatr[i-1][j-1] > 255)
			{
				newMatr[i-1][j-1] = 255;
			}
			else if (newMatr[i-1][j-1] < 0){
				newMatr[i-1][j-1] = 0;
			}
		}
	}
}