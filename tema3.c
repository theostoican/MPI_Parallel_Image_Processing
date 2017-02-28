#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include "filters.h"
#include "pgm.h"
#include <arpa/inet.h>

#ifndef LINE_SIZE
#define LINE_SIZE 1000
#endif

#define NEIGH 1000

#define max(X, Y) (((X) > (Y)) ? (X) : (Y))

/*
* Root of the minimum spanning tree
*/
void echoAlgorithmInitiator(int rank, int *neigh, size_t nrNeigh, int* children, int *len){
	int tok;
	int i;
	for (i = 0; i < nrNeigh; i++)
	{
		MPI_Send(&rank, 1, MPI_INT, neigh[i], 1, MPI_COMM_WORLD);
	}
	while (nrNeigh > 0)
	{
		MPI_Recv(&tok, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		if (tok != -1)/* this neighbour has, as parent, this node*/
		{
			children[(*len)++] = tok; 
			nrNeigh--;
		}
		
	}
}

void echoAlgorithmOther(int rank, int *neigh, int nrNeigh, int *parent, int* children, int *len){
	int id;
	int tok;
	int i;
	MPI_Status stat;
	MPI_Recv(&tok, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &stat);
	*parent = stat.MPI_SOURCE;

	if (nrNeigh > 0)
	{ 
		nrNeigh--;

		for (i = 0; i < nrNeigh+1; i++)
		{
			if (*parent != neigh[i])
			{
				tok = -1;
				MPI_Send(&tok, 1, MPI_INT, neigh[i], 1, MPI_COMM_WORLD);
			}
		}
		while (nrNeigh > 0)
		{
			MPI_Recv(&tok, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (tok != -1)/* the neighbour, who sent this message, has, as parent, this node*/
			{
				children[(*len)++] = tok; 
			}
			nrNeigh--;
		}
	}
	MPI_Send(&rank, 1, MPI_INT, *parent, 1, MPI_COMM_WORLD);
}

int main(int argc, char * argv[]) {



	if (argc < 4)
	{	
		printf("Invalid command\n");
	}



	MPI_Init(&argc, &argv);
	MPI_Status stat;
	
	FILE* top = fopen(argv[1], "r");

	int *neighb = calloc(NEIGH, sizeof(int));
	int *children = calloc(NEIGH, sizeof(int));
	int lenChildren = 0;
	int parent;

	int rank;
	int nProcesses;
	int i = -1;
	int j;
	int tok;
	size_t len = 0;
	int num_processed_lines = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	char* line = calloc(LINE_SIZE, sizeof (char));

	/*
	* Read the topology from file
	*/
	while(1)
	{
		getline(&line, &len, top);
		char* p = strtok(line, ": ");
		int id = atoi(p);
		if (id == rank)
		{
			p = strtok(NULL,": ");
			while (p != NULL)
			{
				neighb[++i] = atoi(p);
				p = strtok(NULL, ": ");
			}
			break;
		}
	}
	fclose(top);
	/*
	* Update the length, since "i" was previously used as an index
	*/
	len = i+1;
	if (rank == 0)
	{
		echoAlgorithmInitiator(0, neighb, len, children, &lenChildren);
	}

	else {
		echoAlgorithmOther(rank, neighb, len, &parent, children, &lenChildren);
	}
	if (rank == 0)
	{

		FILE *imagFile = fopen(argv[2], "r");
			if (imagFile == NULL || fgets(line, LINE_SIZE, imagFile)==NULL)
				return 0;
		int num_images = atoi(line);

		while (num_images > 0)
		{
			if (fgets(line, LINE_SIZE, imagFile) == NULL)
				return 0;

			/*
			* Get filter
			*/
			char *p = strtok(line, " \n");
			char *filterType = strdup(p);
			/*
			* Get image source file
			*/
			p = strtok(NULL, " \n");
			char *img_source = strdup(p);
			
			/*
			* Get image destination file
			*/
			p = strtok(NULL, " \n");
			char *img_dest = strdup(p);

			pgm* pgmFile = getpgmFile(img_source);

			free(img_source);
			pgmFile->filteredImg = calloc ((pgmFile->h - 2), sizeof(int*));
			for (i = 0; i < pgmFile->h - 2; i++)
			{
				pgmFile->filteredImg[i] = calloc ((pgmFile->w-2), sizeof(int));
			}

			int **hash = calloc (2, sizeof(int*));
			hash[0] = calloc (nProcesses, sizeof(int));
			hash[1] = calloc (nProcesses, sizeof(int));
			pgmFile->filter = filterType;
			int n = pgmFile->h-2;
			int chunk_dim = n/lenChildren;
			/*
			* Fewer lines than the number of children nodes
			*/
			if (chunk_dim == 0)
			{
				int index;
				
				for (i = 1, index = 0; i <= n; i++, index++)
				{
					int bufferSize = strlen(pgmFile->filter) + 1;
					MPI_Send(&bufferSize, 1, MPI_INT, children[index], 1, MPI_COMM_WORLD);
					MPI_Send(pgmFile->filter, bufferSize, MPI_CHAR, children[index], 1, MPI_COMM_WORLD);
					/*
					* Increase chunk dimension by 2 (since we include the 2 up and bottom margins)
					*/
					chunk_dim = 3;
					MPI_Send(&chunk_dim, 1, MPI_INT, children[index], 1, MPI_COMM_WORLD);
					chunk_dim = 0;
					MPI_Send(&pgmFile->w, 1, MPI_INT, children[index], 1, MPI_COMM_WORLD);

					/*
					* Store which part of the message we send to a particular child in order to reform it
					*/
					hash[0][children[index]] = i;
					hash[1][children[index]] = i;
					/*
					* Add +1 at "index + chunk_dim - 1" in order to take also the border
					*/
					for (j = i - 1; j <= i + 1; j++)
					{
						MPI_Send(pgmFile->image[j], pgmFile->w, MPI_INT, children[index], 1, MPI_COMM_WORLD);
					}
				}
				int temp;
				for (i = 0; i < n; i++)
				{
					for (j = hash[0][children[i]]; j <= hash[1][children[i]]; j++)
					{
						MPI_Recv(pgmFile->filteredImg[j-1], pgmFile->w-2, MPI_INT, children[i], 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					}
				}
			}
			else{
				int index = 1;
				for (i = 0; i < lenChildren; i++)
				{
					/*
					* The last child could potentially process more line
					*/
					if (n - index + 1 < 2 * chunk_dim && n - index + 1 > chunk_dim)
					{
						chunk_dim = n - index + 1;
					}
					int bufferSize = strlen(pgmFile->filter) + 1;
					MPI_Send(&bufferSize, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);
					MPI_Send(pgmFile->filter, bufferSize, MPI_CHAR, children[i], 1, MPI_COMM_WORLD);
					/*
					* Increase chunk dimension by 2 (since we include the 2 up and bottom margins)
					*/
					chunk_dim += 2;
					MPI_Send(&chunk_dim, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);
					chunk_dim -= 2;
					MPI_Send(&pgmFile->w, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);

					/*
					* Store which part of the message we send to a particular child in order to reform it
					*/
					hash[0][children[i]] = index;
					hash[1][children[i]] = index + chunk_dim - 1;
					/*
					* Add +1 at "index + chunk_dim - 1" in order to take also the border
					*/
					for (j = index - 1; j <= index + chunk_dim - 1 + 1; j++)
					{
						MPI_Send(pgmFile->image[j], pgmFile->w, MPI_INT, children[i], 1, MPI_COMM_WORLD);
					}
					index = index + chunk_dim;
				}
				/*
				* Receive the filtered blocks from children
				*/
				int temp;
				for (i = 0; i < lenChildren; i++)
				{
					for (j = hash[0][children[i]]; j <= hash[1][children[i]]; j++)
					{
						MPI_Recv(pgmFile->filteredImg[j-1], pgmFile->w-2, MPI_INT, children[i], 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					}
				}
			}
				
			FILE *out5 = fopen(img_dest, "w");
			fwrite(pgmFile->comments, 1, pgmFile->sizeComments, out5);

			for (int i = 0; i < pgmFile->h - 2; i++)
			{
				for (int j = 0; j < pgmFile->w - 2; j++)
				{

                	fprintf(out5,"%d\n", pgmFile->filteredImg[i][j]);
				}

			}
			fflush(out5);
			fclose(out5);

			free(img_dest);
			free(hash[0]);
			free(hash[1]);
			free(hash);
			
			for (i = 0; i < pgmFile->h - 2; i++)
			{
				free(pgmFile->filteredImg[i]);
			}
			for (i = 0; i < pgmFile->h; i++)
			{
				free(pgmFile->image[i]);
			}
			free(pgmFile->filteredImg);
			free(pgmFile->image);
			free(pgmFile->filter);
			free(pgmFile->special);
			free(pgmFile->comments);
			free(pgmFile);
			num_images--;

		}
		fclose(imagFile);
		/*
		* Send end tag to children 
		*/
		for (i = 0; i < lenChildren; i++)
		{
			MPI_Send(&tok, 1, MPI_INT, children[i], 2, MPI_COMM_WORLD);
		}
		int num_msgs = 0;
		int total_size = 0;
		int *block = calloc (1,sizeof(int));
		for (i = 0; i < lenChildren; i++)
		{
			MPI_Recv(&num_msgs, 1, MPI_INT, children[i], 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			total_size += num_msgs * 2;
			block = realloc(block, total_size * sizeof(int));
			if (block == NULL)
			{
				printf("Realloc failed\n");
				return 1;
			}

			MPI_Recv(block + total_size - num_msgs * 2 , 2*num_msgs, MPI_INT, children[i], 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		FILE* stats = fopen(argv[3], "w");
		int *leaves = calloc(nProcesses, sizeof(int));
		for (i = 0; i < total_size; i+=2)
		{
			leaves[block[i]] = block[i+1]; 
		}
		for (i = 0; i < nProcesses; i++)
		{
			fprintf(stats, "%d: %d\n", i, leaves[i]);
		}
		free(block);
		free(leaves);
		fclose(stats);
	}
	else
	{

		int **hash = calloc (2,sizeof(int*));
		hash[0] = calloc (nProcesses, sizeof(int));
		hash[1] = calloc (nProcesses, sizeof(int));

		/*
		* Receive the chunk from the parent
		*/
		pgm* pgmBlock = calloc(1,sizeof(pgm));
		pgmBlock->maxval = 0;
		int bufferSize = 0;

		while (1){
			/*
			* First two are size and type of filter
			*/
			MPI_Recv(&bufferSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
			int tag = stat.MPI_TAG;
			if (tag == 2){ //finish tag
				int childRank;
				int childNumberLines;
				int num_msgs;
				int total_msgs = 0;
				int total_size = 0;
				int *block = calloc(1, sizeof(int));
				if (lenChildren != 0)
				{

					for (i = 0; i < lenChildren; i++)
					{
						MPI_Send(&tok, 1, MPI_INT, children[i], 2, MPI_COMM_WORLD);
					}
					for (i = 0; i < lenChildren; i++)
					{
						MPI_Recv(&num_msgs, 1, MPI_INT, children[i], 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						total_msgs += num_msgs;
						total_size += num_msgs * 2;
						block = realloc(block, total_size * sizeof(int));

						MPI_Recv(block + total_size - num_msgs * 2, 2*num_msgs, MPI_INT, children[i], 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					}
					MPI_Send(&total_msgs, 1, MPI_INT, parent, 2, MPI_COMM_WORLD);
					MPI_Send(block, 2*total_msgs, MPI_INT, parent, 2, MPI_COMM_WORLD);
				}
				else {
					num_msgs = 1;
					free(block);
					block = calloc(2, sizeof(int));
					block[0] = rank;
					block[1] = num_processed_lines;
					MPI_Send(&num_msgs, 1, MPI_INT, parent, 2, MPI_COMM_WORLD);
					MPI_Send(block, 2, MPI_INT, parent, 2, MPI_COMM_WORLD);
				}
				free(block);
				break;
			}
			else
			{
				pgmBlock->filter = calloc (bufferSize, sizeof(char));
				MPI_Recv(pgmBlock->filter, bufferSize, MPI_CHAR, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				MPI_Recv(&pgmBlock->h, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(&pgmBlock->w, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				pgmBlock->image = calloc (pgmBlock->h, sizeof (int*));
				for (i = 0; i < pgmBlock->h; i++)
				{
					pgmBlock->image[i] = calloc (pgmBlock->w, sizeof(int));
					MPI_Recv(pgmBlock->image[i], pgmBlock->w, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}

				pgmBlock->filteredImg = calloc ((pgmBlock->h - 2), sizeof(int*));
				for (i = 0; i < pgmBlock->h - 2; i++)
				{
					pgmBlock->filteredImg[i] = calloc ((pgmBlock->w-2), sizeof(int));
				}
				if (lenChildren == 0)
				{
					/*
					This is a leaf.
					*/
					if (strcmp(pgmBlock->filter, "smooth") == 0)
					{
						smooth(pgmBlock->image, pgmBlock, pgmBlock->h, pgmBlock->w);
					}
					else if (strcmp(pgmBlock->filter, "blur") == 0)
					{
						blur(pgmBlock->image, pgmBlock, pgmBlock->h, pgmBlock->w);
					}
					else if (strcmp(pgmBlock->filter, "sharpen") == 0)
					{
						sharpen(pgmBlock->image, pgmBlock, pgmBlock->h, pgmBlock->w);	
					}
					else if (strcmp(pgmBlock->filter, "mean_removal") == 0)
					{
						meanRemoval(pgmBlock->image, pgmBlock, pgmBlock->h, pgmBlock->w);
					}
					/*
					* Send back to parent
					*/
					for (i = 0; i < pgmBlock->h - 2; i++)
					{
						MPI_Send(pgmBlock->filteredImg[i], pgmBlock->w - 2, MPI_INT, parent, 1, MPI_COMM_WORLD);
					}
					num_processed_lines += pgmBlock->h-2;
				}
				else
				{
					/*
					* This is an internal node.
					*/
					int n = pgmBlock->h - 2;
					int chunk_dim = n/lenChildren;
					int index = 1;
					for (i = 0; i < lenChildren; i++)
					{
						if (n - index + 1 < 2 * chunk_dim && n - index + 1 > chunk_dim)
						{
							chunk_dim = n - index + 1;
						}

						int bufferSize = strlen(pgmBlock->filter) + 1;
						MPI_Send(&bufferSize, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);
						MPI_Send(pgmBlock->filter, bufferSize, MPI_CHAR, children[i], 1, MPI_COMM_WORLD);

						chunk_dim += 2;
						MPI_Send(&chunk_dim, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);
						chunk_dim -= 2;
						MPI_Send(&pgmBlock->w, 1, MPI_INT, children[i], 1, MPI_COMM_WORLD);
						hash[0][children[i]] = index;
						hash[1][children[i]] = index + chunk_dim - 1;
						for (j = index - 1; j <= index + chunk_dim - 1 + 1; j++)
						{
							MPI_Send(pgmBlock->image[j], pgmBlock->w, MPI_INT, children[i], 1, MPI_COMM_WORLD);
						}
						index = index + chunk_dim;
					}


					/*
					* Receive the filtered blocks from children
					*/
					int temp;
					for (i = 0; i < lenChildren; i++)
					{
						for (j = hash[0][children[i]]; j <= hash[1][children[i]]; j++)
						{
							MPI_Recv(pgmBlock->filteredImg[j-1], pgmBlock->w - 2, MPI_INT, children[i], 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						}
					}



					/* 
					* Send back to the parent
					*/
					for (i = 0; i < pgmBlock->h - 2; i++)
					{
						MPI_Send(pgmBlock->filteredImg[i], pgmBlock->w - 2, MPI_INT, parent, 1, MPI_COMM_WORLD);
					}

				}
				for (i = 0; i < pgmBlock->h-2; i++)
				{
					free(pgmBlock->image[i]);
					free(pgmBlock->filteredImg[i]);// = malloc (pgmBlock->w * sizeof(int));
				}
				free(pgmBlock->image[pgmBlock->h-2]);
				free(pgmBlock->image[pgmBlock->h-1]);
				free(pgmBlock->image);
				free(pgmBlock->filteredImg);
				free(pgmBlock->filter);

			}
		}
		free(pgmBlock);
		free(hash[0]);
		free(hash[1]);
		free(hash);
	}
	free(neighb);
	free(children);
	free(line);
	MPI_Finalize();
	return 0;
}
