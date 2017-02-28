#include "pgm.h"

pgm* getpgmFile(char* fn){

	int i;
	pgm* pgmFile = calloc(1, sizeof(pgm));
	memset(pgmFile, 0, sizeof(pgm));
	FILE *img = fopen(fn, "rb");
	if (img == NULL)
	{
		puts("Error opening image !");
		return NULL;
	}
	char *line = calloc (LINE_SIZE, sizeof(char));
	char *word = calloc (LINE_SIZE, sizeof(char));

	pgmFile->special = calloc(3,sizeof(char));
	fgets(pgmFile->special, 3, img);
	fgets (line, 255, img);
	int howMuch = ftell(img);
	fgets (line, 255, img);

	while (line[0] == '#') {
  		//if a comment, get the rest of the line and a new word
  		howMuch = ftell(img);
  		fgets (line, 255, img);
	}
	fseek(img, howMuch, SEEK_SET);
	int width, height;
	fscanf (img, "%d", &width);
	fscanf (img, "%d", &height);
	fscanf (img, "%d", &pgmFile->maxval);
	
	fgets(line, 255, img);//reach end of line
	pgmFile->image = malloc ((height+2) * sizeof(int*));
	for (i = 0; i < height + 2; i++)
	{
		pgmFile->image[i] = malloc ((width+2) * sizeof(int));
	}
	
	width += 2;
	height += 2;
	pgmFile->h = height;
	pgmFile->w = width;

	howMuch = ftell(img);
	for (int i = 0; i <= height - 1; i++)
	{
		for (int j = 0; j <= width - 1; j++)
		{
			if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
			{
				pgmFile->image[i][j] = 0;
			}
			else{
				fscanf(img, "%d", &pgmFile->image[i][j]);
			}
		}
	}

	
	pgmFile->comments = calloc (howMuch, sizeof(char));

	fseek(img, 0, SEEK_SET);
	fread(pgmFile->comments, 1, howMuch, img);
	
	pgmFile->sizeComments = howMuch;
	free(line);
	free(word);
	fclose(img);
	return pgmFile;
}