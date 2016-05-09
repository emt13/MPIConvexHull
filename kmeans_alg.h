/************************************************************************
 *   @project Final Project - Parallel Programming and Computing
 *   @file kmeans_alg.h
 *   @description This is a header file that forward defines the k-means
 *                clustering function.
 *
 *   @authors Alex Vargas
 *            Evan Thompson
 *            Alexander Soloviev
 *            Jim Olyha
 *
 ***********************************************************************/
#ifndef _H_KMEANS_ALG
#define _H_KMEANS_ALG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

void kmeansClustering( float** points
					 , size_t numPoints
					 , size_t numCoords
					 , size_t numClusters
					 , float threshold
					 , MPI_Comm comm
					 , float** clusters
					 , size_t* membership );

extern int _debug;

#endif