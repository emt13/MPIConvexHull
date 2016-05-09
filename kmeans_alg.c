/************************************************************************
 *   @project Final Project - Parallel Programming and Computing
 *   @file kmeans_alg.c
 *   @description This is an implementation of the k-means clustering
 *                algorithm. The program inputs an array of N points,
 *                each with M coordinates and performs clustering for
 *                the specified number of clusters (K). The results are
 *                provided in two output arrays:
 *                1.) An array of size [K][M] with the cluster centers
 *                2.) An array of size [N] called membership that stores
 *                    the membership of each point to the cluster center
 *                    array
 *
 *   @authors Alex Vargas
 *            Evan Thompson
 *            Alexander Soloviev
 *            Jim Olyha
 *
 ***********************************************************************/

/* Necessary includes */
#include "kmeans_alg.h"

/* Method: pointDistance2
 *
 * @param numCoords - the number of coordinates
 * @param point1 - the first point
 * @param point2 - the second point
 * @return ans - the distance between the two points
 *
 * @remarks Find the squared Euclid distance between two points.
 */
float pointDistance2( size_t numCoords
					, float* point1
					, float* point2 )
{
	size_t i;
	float ans=0.0;

	for (i = 0; i < numCoords; ++i)
	{
		ans += ( point1[i] - point2[i] ) * ( point1[i] - point2[i] );
	}

	return ans;
}

/* Method: findClosestCluster
 *
 * @param numClusters - the number of clusters to use
 * @param numCoords - the number of coordinates
 * @param point - the point for which to find the closest cluster
 * @param clusters - the array for all cluster center locations [numClusters][numCoords]
 * @return index - the array index of the closest cluster center
 *
 * @remarks Find the closest cluster to a particular point.
 */
size_t findClosestCluster( size_t numClusters
						 , size_t numCoords
						 , float* point
						 , float** clusters )
{
	size_t i, index;
	float dist, minDist;

	/* Initialize the minDist */
	index = 0;
	minDist = pointDistance2( numCoords, point, clusters[0] );

	/* Find a cluster with a smaller distance to the point */
	for (i = 1; i < numClusters; i++)
	{
		dist = pointDistance2( numCoords, point, clusters[i] );

		/* For comparisons, the square root is superfluous */
		if ( dist < minDist )
		{
			minDist = dist;
			index = i;
		}
	}
	return index;
}

/* Method: getLocalClusterSums
 *
 * @param points - a local array of input point values [numPoints][numCoords]
 * @param numPoints - the local number of points
 * @param numCoords - the number of coordinates
 * @param numClusters - the number of clusters to use
 * @param membership - the membership array of a point to a cluster [numPoints]
 * @param clusters - the array for all cluster center locations [numClusters][numCoords]
 * @param localClusterSize - the local size of each cluster [numClusters]
 * @param localClusters - the array for all local cluster centers [numClusters][numCoords]
 * @param delta - a reference to the delta value
 *
 * @remarks Find the local cluster sums, which are the sums of the points belonging
 *          to a particular cluster. Also update the membership[] and delta value.
 */
void getLocalClusterSums( float** points
						, size_t numPoints
						, size_t numCoords
						, size_t numClusters
						, size_t* membership
						, float** clusters
						, size_t* localClusterSize
						, float** localClusters
						, float* delta )
{
	size_t i, j, index;

	for (i = 0; i < numPoints; ++i)
	{
		/* Find the array index of the closest cluster center */
		index = findClosestCluster( numClusters, numCoords, points[i], clusters );

		/* If the membership of a point changed, delta must be incremented */
		if ( membership[i] != index )
		{
			*delta += 1.0;
		}

		/* Assign the membership to point i */
		membership[i] = index;

		/* Update the local cluster centers by getting the sum of all
		 * internal objects. The centroid is computed by averaging this sum. */
		++localClusterSize[index];
		for (j = 0; j < numCoords; ++j)
		{
			localClusters[index][j] += points[i][j];
		}
	}
}

/* Method: updateClusters
 *
 * @param numClusters - the number of clusters to use
 * @param numCoords - the number of coordinates
 * @param clusterSize - the global size of each cluster [numClusters]
 * @param clusters - the array for all cluster center locations [numClusters][numCoords]
 * @param localClusterSize - the local size of each cluster [numClusters]
 * @param localClusters - the array for all local cluster centers [numClusters][numCoords]
 *
 * @remarks Average the cluster sums to get the centroid values and reset the local
 *          variables.
 */
void updateClusters( size_t numClusters
				   , size_t numCoords
				   , size_t* clusterSize
				   , float** clusters
				   , size_t* localClusterSize
				   , float** localClusters )
{
	size_t i, j;
	for (i = 0; i < numClusters; ++i)
	{
		for (j = 0; j < numCoords; ++j)
		{
			if ( clusterSize[i] > 1 )
			{
				clusters[i][j] /= clusterSize[i];
			}
			localClusters[i][j] = 0.0; /* Reset local clusters */
		}
		localClusterSize[i] = 0; /* Reset local cluster sizes */
	}
}

/* Method: allocateVarsAlg
 *
 * @param numPoints - the local number of points
 * @param numCoords - the number of coordinates
 * @param numClusters - the number of clusters to use
 * @param membership - the membership array of a point to a cluster [numPoints]
 * @param clusterSize - the global size of each cluster [numClusters]
 * @param localClusterSize - the local size of each cluster [numClusters]
 * @param localClusters - the array for all local cluster centers [numClusters][numCoords]
 *
 * @remarks Allocate necessary variables that are local to the current MPI rank.
 */
void allocateVarsAlg( size_t numPoints
					, size_t numCoords
					, size_t numClusters
					, size_t* membership
					, size_t* clusterSize
					, size_t* localClusterSize
					, float** localClusters )
{
	size_t i;

	/* Initialize membership[] */
	for (i = 0; i < numPoints; ++i)
	{
		/* Note: numClusters is an intended invalid index */
		membership[i] = numClusters;
	}

	/* Initialize the clusterSize[] */
	clusterSize = (size_t*) calloc( numClusters, sizeof(size_t) );
	assert( clusterSize != NULL );

	/* Initialize the localClusterSize[] */
	localClusterSize = (size_t*) calloc( numClusters, sizeof(size_t) );
	assert( localClusterSize != NULL );

	/* Initialize the localClusters[][]. The array is not actually 2D,
	 * but is 1D so that MPI_Allreduce can be used on it. However,
	 * to index it as 2D, a layer of pointers is used for the rows.
	 * The full array is stored in localClusters[0].
	 */
	localClusters = (float**) malloc( numClusters * sizeof(float*) );
	assert( localClusters != NULL );

	localClusters[0] = (float*) calloc( numClusters * numCoords, sizeof(float) );
	assert( localClusters[0] != NULL );

	for (i = 1; i < numClusters; ++i)
	{
		/* Now connect the outer layer of pointers to the internal 1D array */
		localClusters[i] = localClusters[i-1] + numCoords;
	}
}

/* Method: deallocateVarsAlg
 *
 * @param clusterSize - the global size of each cluster [numClusters]
 * @param localClusterSize - the local size of each cluster [numClusters]
 * @param localClusters - the array for all local cluster centers [numClusters][numCoords]
 *
 * @remarks Deallocate necessary variables that are local to the current MPI rank.
 */
void deallocateVarsAlg( size_t* clusterSize
					  , size_t* localClusterSize
					  , float** localClusters )
{
	free( localClusters[0] ); /* localClusters[][] is actually 1D */
	free( localClusters );
	free( localClusterSize );
	free( clusterSize );
}

/* Method: kmeansClustering
 *
 * @param points - a local array of input point values [numPoints][numCoords]
 * @param numPoints - the local number of points
 * @param numCoords - the number of coordinates
 * @param numClusters - the number of clusters to use
 * @param threshold - the threshold value for computing the cluster centers
 * @param comm - the MPI communicator
 * @return clusters - the array for all cluster center locations [numClusters][numCoords]
 * @return membership - the membership array of a point to a cluster [numPoints]
 *
 * @remarks Perform the K-Means Clustering Algorithm with the specified input point
 *          values, number of clusters, and computational threshold.
 */
void kmeansClustering( float** points
					 , size_t numPoints
					 , size_t numCoords
					 , size_t numClusters
					 , float threshold
					 , MPI_Comm comm
					 , float** clusters
					 , size_t* membership )
{
	int rank;                   /* MPI rank */
	size_t loop=0;              /* The number of loop iterations */
	float delta;                /* The % of points that change their clusters */
	float globalDelta;          /* Global delta for all MPI ranks */
	size_t globalNumPoints;     /* The global number of data points */
	size_t* clusterSize;        /* The global size of each cluster [numClusters] */
	size_t* localClusterSize;   /* The local size of each cluster [numClusters] */
	float** localClusters;      /* Local cluster centers [numClusters][numCoords] */
	extern int _debug;          /* Debug information */

	if (_debug)
	{
		MPI_Comm_rank( comm, &rank );
	}

	/* Initialize membership[], cluster sizes, and localClusters[][] */
	allocateVarsAlg( numPoints, numCoords, numClusters, membership
				   , clusterSize, localClusterSize, localClusters );

	/* Get the total number of points */
	MPI_Allreduce( &numPoints, &globalNumPoints, 1, MPI_INT, MPI_SUM, comm );

	if (_debug)
	{
		printf( "%2d: numPoints=%d globalNumPoints=%d numClusters=%d numCoords=%d\n"
			  , rank, numPoints, globalNumPoints, numClusters, numCoords );
	}

	/* Iterate until below threshold or loop iterations exceeded */
	do
	{
		// Reset delta value
		delta = 0.0;

		/* Obtain the local cluster sums, which are stored in localClusters[][] */
		getLocalClusterSums( points, numPoints, numCoords, numClusters, membership
						   , clusters, localClusterSize, localClusters, &delta );

		/* Get the global cluster sums by adding the local cluster sums */
		MPI_Allreduce( localClusters[0], clusters[0], numClusters*numCoords
					 , MPI_FLOAT, MPI_SUM, comm);

		/* Get the global cluster sizes by adding the local cluster sizes */
		MPI_Allreduce( localClusterSize, clusterSize, numClusters
					 , MPI_INT, MPI_SUM, comm);

		/* Update the cluster centers and reset local variables */
		updateClusters( numClusters, numCoords, clusterSize, clusters
					  , localClusterSize, localClusters);

		/* Get the global delta value */
		MPI_Allreduce( &delta, &globalDelta, 1, MPI_FLOAT, MPI_SUM, comm );
		delta = globalDelta / globalNumPoints;

	} while ( delta > threshold && ++loop < 1000 );

	/* Deallocate all local variables */
	deallocateVarsAlg( clusterSize, localClusterSize, localClusters );
}