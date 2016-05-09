/************************************************************************
 *   @project Final Project - Parallel Programming and Computing
 *   @file kmeans_main.c
 *   @description This is the main file for the k-means clustering
 *                algorithm. It initializes a large array of random
 *                points and splits the workload among all MPI ranks.
 *                The initial clusters and the data points are generated
 *                randomly using the clcg4 library.
 *
 *   @authors Alex Vargas
 *            Evan Thompson
 *            Alexander Soloviev
 *            Jim Olyha
 *
 ***********************************************************************/

/* Necessary includes */
#include <unistd.h>

#include "clcg4.h"
#include "kmeans_alg.h"

/* Global debug variable */
int _debug;

/* Method: usage
 *
 * @param argv0 - the name of the executable
 * @param threshold - threshold value for computing the cluster centers
 *
 * @remarks Output the usage information.
 */
void usage(char* argv0, float threshold)
{
	char* help =
"Usage: %s [switches] -n num_clusters\n"
"       -p numPoints   : number of points per MPI process (N must be > 0)\n"
"       -n numClusters : number of clusters (K must be > 1)\n"
"       -c numCoords   : number of coordinates (M must be > 0)\n"
"       -t threshold   : threshold value (default %.4f)\n"
"       -o             : output timing results (default no)\n"
"       -d             : enable debug mode\n"
"       -h             : print this help information\n";
	fprintf( stderr, help, argv0, threshold );
}

/* Method: generatePoints
 *
 * @param numPoints - the local number of points
 * @param numCoords - the number of coordinates
 * @param globalNumPoints - the global number of points
 * @param points - a local array of input point values [numPoints][numCoords]
 * @param rank - the MPI rank of the current process
 *
 * @remarks Generate the point values on a hypercube bounded by globalNumPoints.
 */
void generatePoints( size_t numPoints
				   , size_t numCoords
				   , size_t globalNumPoints
				   , float** points
				   , int rank )
{
	int i, j;
	for (i = 0; i < numPoints; ++i)
	{
		for (j = 0; j < numCoords; ++j)
		{
			points[i][j] = GenVal(rank) * globalNumPoints;
		}
	}
}

/* Method: generateInitialClusters
 *
 * @param numClusters - the number of clusters to use
 * @param numCoords - the number of coordinates
 * @param globalNumPoints - the global number of points
 * @return clusters - the array for all cluster center locations [numClusters][numCoords]
 * @param rank - the MPI rank of the current process
 *
 * @remarks Generate the initial cluster locations on a hypercube bounded by
 *          globalNumPoints. Note that these clusters are the same across all
 *          processes!
 */
void generateInitialClusters( size_t numClusters
							, size_t numCoords
							, size_t globalNumPoints
							, float** clusters
							, int rank )
{
	int i, j;
	if ( rank == 0 )
	{
		for (i = 0; i < numClusters; ++i)
		{
			for (j = 0; j < numCoords; ++j)
			{
				clusters[i][j] = GenVal(0) * globalNumPoints;
			}
		}
	}

	MPI_Bcast( clusters[0], numClusters * numCoords, MPI_FLOAT, 0, MPI_COMM_WORLD);
}

/* Method: allocateVarsMain
 *
 * @param numPoints - the local number of points
 * @param numCoords - the number of coordinates
 * @param numClusters - the number of clusters to use
 * @param points - a local array of input point values [numPoints][numCoords]
 * @return clusters - the array for all cluster center locations [numClusters][numCoords]
 * @param membership - the membership array of a point to a cluster [numPoints]
 *
 * @remarks Allocate necessary variables that are local to the current MPI rank.
 */
void allocateVarsMain( size_t numPoints
					 , size_t numCoords
					 , size_t numClusters
					 , float** points
					 , float** clusters
					 , size_t* membership )
{
	int i;

	/* Initialize the points[][]. The array is not actually 2D,
	 * but is 1D. However, to index it as 2D, a layer of pointers
	 * is used for the rows. This array is the same across all
	 * processes.
	 */
	points = (float**) malloc( numPoints * sizeof(float*) );
	assert( points != NULL );

	points[0] = (float*)  malloc( numPoints * numCoords * sizeof(float) );
	assert( points[0] != NULL );

	for (i = 1; i < numPoints; ++i)
	{
		/* Now connect the outer layer of pointers to the internal 1D array */
		points[i] = points[i-1] + numCoords;
	}

	/* Do the same thing for the clusters as for the points. */
	clusters = (float**) malloc( numClusters * sizeof(float*) );
	assert( clusters != NULL );

	clusters[0] = (float*)  malloc( numClusters * numCoords * sizeof(float) );
	assert( clusters[0] != NULL );

	for (i = 1; i < numClusters; ++i)
	{
		/* Now connect the outer layer of pointers to the internal 1D array */
		clusters[i] = clusters[i-1] + numCoords;
	}

	/* Allocate the membership array */
	membership = (size_t*) malloc( numPoints * sizeof(size_t) );
	assert( membership != NULL );
}

/* Method: deallocateVarsMain
 *
 * @param points - a local array of input point values [numPoints][numCoords]
 * @param clusters - the array for all cluster center locations [numClusters][numCoords]
 * @param membership - the membership array of a point to a cluster [numPoints]
 *
 * @remarks Deallocate necessary variables that are local to the current MPI rank.
 */
void deallocateVarsMain( float** points
					   , float** clusters
					   , size_t* membership )
{
	free( points[0] );
	free( points );
	free( clusters[0] );
	free( clusters );
	free( membership );
}

/* Method: main
 */
int main(int argc, char** argv)
{
	int opt;                            /* For getopt() */
	extern char* optarg;                /* For getopt() */
	int isOutputTiming, isPrintUsage;   /* Important bools */
	size_t numPoints, numCoords,
		   numClusters, globalNumPoints;/* User-defined, data-related */
	float** points;                     /* Input point values [numPoints][numCoords] */
	float** clusters;                   /* Cluster center locations [numClusters][numCoords] */
	size_t* membership;                 /* Membership of a point to a cluster [numPoints] */
	float threshold;                    /* Threshold value for computing the cluster centers */
	double timing, clusteringTiming;    /* Timing information */
	int rank, nproc;                    /* MPI information */

	/* Default values */
	_debug          = 0;
	threshold       = 0.001;
	numPoints       = 0;
	numCoords       = 0;
	numClusters     = 0;
	isOutputTiming  = 0;
	isPrintUsage    = 0;

	/* Initialize MPI and get information */
	MPI_Init( &argc, &argv );

	MPI_Comm_size( MPI_COMM_WORLD, &nproc );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	/* Initialize the RNG streams */
	InitDefault();

	/* Set the RNG seed */
	/* SetInitialSeed( {MPI_Wtime(), MPI_Wtime(), MPI_Wtime(), MPI_Wtime()} ); */

	while ( ( opt=getopt(argc,argv,"p:n:c:t:doh") ) != EOF )
	{
		switch (opt)
		{
			case 'p': numPoints = atoi(optarg);
					  break;
			case 'n': numClusters = atoi(optarg);
					  break;
			case 'c': numCoords = atoi(optarg);
					  break;
			case 't': threshold = atof(optarg);
					  break;
			case 'o': isOutputTiming = 1;
					  break;
			case 'd': _debug = 1;
					  break;
			case 'h':
			default:  isPrintUsage = 1;
					  break;
		}
	}

	/* Bad input arguments */
	if ( numClusters <= 1 || isPrintUsage == 1 || numCoords < 1 ||
		 numPoints < 1 )
	{
		if ( rank == 0 )
		{
			usage( argv[0], threshold );
		}
		MPI_Finalize();
		exit( EXIT_FAILURE );
	}

	if (_debug)
	{
		printf( "Proc %d of %d currently running\n", rank, nproc );
	}

	MPI_Barrier( MPI_COMM_WORLD );

	/* Allocate necessary variables */
	allocateVarsMain( numPoints, numCoords, numClusters
					, points, clusters, membership);

	/* Get the global number of points */
	MPI_Allreduce( &numPoints, &globalNumPoints, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
	assert( globalNumPoints == numPoints * nproc );
	if ( globalNumPoints < numClusters )
	{
		if ( rank == 0 )
		{
			fprintf( stderr, "Error: number of clusters must be larger than "
							 "the number of data points to be clustered.\n" );
		}
		deallocateVarsMain( points, clusters, membership );
		MPI_Finalize();
		exit( EXIT_FAILURE );
	}

	/* Get a set of points for a particular MPI process */
	generatePoints( numPoints, numCoords, globalNumPoints, points, rank );

	/* Get the initial clusters */
	generateInitialClusters( numClusters, numCoords, globalNumPoints,
							 clusters, rank);

	/* Timing information */
	timing              = MPI_Wtime();
	clusteringTiming    = timing;

	/* Perform the kmeans algorithm */
	kmeansClustering( points, numPoints, numCoords, numClusters
					, threshold, MPI_COMM_WORLD, clusters, membership);

	/* Timing information */
	timing           = MPI_Wtime();
	clusteringTiming = timing - clusteringTiming;

	/* Deallocate used variables */
	deallocateVarsMain( points, clusters, membership );

	/* Output performance analysis */
	if ( isOutputTiming )
	{
		double maxClusteringTiming;

		/* Get the max clustering timing measured among all processes */
		MPI_Reduce( &clusteringTiming, &maxClusteringTiming, 1, MPI_DOUBLE,
					MPI_MAX, 0, MPI_COMM_WORLD);

		if (rank == 0)
		{
			printf("\nPerforming **** Simple Kmeans  (MPI) ****\n");
			printf("Num of processes   = %d\n", nproc);
			printf("globalNumPoints    = %d\n", globalNumPoints);
			printf("numCoords          = %d\n", numCoords);
			printf("numClusters        = %d\n", numClusters);
			printf("threshold          = %.4f\n", threshold);
			printf("Computation timing = %10.4f sec\n", maxClusteringTiming);
		}
	}

	MPI_Finalize();
	return( EXIT_SUCCESS );
}