all: clcg4.h clcg4.c kcluster.c
	module load xl; \
	bgxlc -I. -Wall -c clcg4.c -o clcg4.o; \
	mpixlc -I. -Wall -O3 kcluster.c clcg4.o -o kcluster.xl
