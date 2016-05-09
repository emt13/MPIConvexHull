#***********************************************************************/
#   @project Final Project - Parallel Programming and Computing
#   @file Makefile
#   @description Makefile for programs running the k-means clustering
#                algorithm.
#
#   @authors Alex Vargas
#            Evan Thompson
#            Alexander Soloviev
#            Jim Olyha
#
#***********************************************************************/

INCFLAGS = -I.
WRNFLAGS = -Wall
DBGFLAGS = -_DEBUG
OPTFLAGS = -O3

BGQFLAGS = $(INCFLAGS)
MPIFLAGS = $(INCFLAGS) $(OPTFLAGS)

all: clcg4.h clcg4.c kmeans_alg.c kmeans_main.c
	module load xl; \
	bgxlc $(BGQFLAGS) -c clcg4.c -o clcg4.o; \
	mpixlc $(MPIFLAGS) kmeans_alg.c kmeans_main.c clcg4.o -o kmeans_run.xl