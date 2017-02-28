build: tema3.c pgm.c filters.c
	mpicc tema3.c pgm.c filters.c -o filtru
clean: filtru
	rm filtru
