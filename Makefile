all: clcg4.h clcg4.c kcluster.c
	gcc -I. -Wall -c clcg4.c -o clcg4.o
	mpicc -I. -Wall -O3 kcluster.c clcg4.o -o a.out
