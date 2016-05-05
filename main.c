#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <clcg4.h>

int rank;
int comm_size;
int num_clusters;
int x_bound;
int y_bound;
long long data_points;

int** data; //has 2 rows, used to map coordinates
	    //index: 0  1  2  3  4  5  6 ...
	    //x:    [a][b][c][d][e][f][g]...
	    //y:    [a][b][c][d][e][f][g]...
long long data_size;

long long* centers;  // contains index in data of cluster center
			   // index: 0  1  2   3   ... num_clusters
			   //       [5][7][12][24] ...    [..]

void allocate_data(long long points_per_rank);

void print_data();

int main(int argc, char** argv){

	int dummy;	

	MPI_Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &dummy);	
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	InitDefault();

	num_clusters = atoi(argv[1]);
	data_points = atoll(argv[2]);
	x_bound = atoi(argv[3]);
	y_bound = atoi(argv[4]);

	printf("rank :%d, %d reporting for duty! (num_clusters: %d, data_points: %lld) \n", rank, argc, num_clusters, data_points);

	//sets up and generates the data points inside data
	allocate_data(data_points/(long long)comm_size);

	print_data();

	//start computing k cluster
	
	


	
	MPI_Finalize();

	return 0;
}

void print_data(){
	int i;
	for(i = 0; i < data_size; i++){
		if(i != 0 && i % 16 == 0) printf("\n");
		printf("(%d,%d) ", data[0][i], data[1][i]);
	}
	printf("\n");
}


void allocate_data(long long points_per_rank){

	data_size = points_per_rank;

	data = malloc(2 * sizeof(int*));
	
	data[0] = malloc(data_size * sizeof(int));
	data[1] = malloc(data_size * sizeof(int));

	//input the random numbers for x and y coordinates
	int i;
	for(i = 0; i < data_size; i++){
		data[0][i] = GenVal(rank) * x_bound;
		data[1][i] = GenVal(rank) * y_bound;
	}	
	
	centers = malloc(num_clusters * sizeof(long long));
	//start cluster centers at first num_clusters points
	for (i=0; i < num_clusters; i++){
		centers[i] = i; 
	}
	
}





//finds nearest cluster to given point, index in data**
int find_nearest_cluster(long long index){
	
	int   cluster_id, i;
    float dist, min_dist;

    /* find the cluster id that has min distance to object */
    cluster_id    = 0;
    min_dist = point_distance(index, centers[0]);

    for (i=1; i<num_clusters; i++) {
        dist = point_distance(index, clusters[i]);
        if (dist < min_dist) { /* find the min and its array index */
            min_dist = dist;
            cluster_id    = i;
        }
    }
    return(cluster_id);
	
}

//finds distance between two points, indices are places in data**
float point_distance(long long index1, long long index2){
	float result = 0;
	
	result += (data[0][index1] - data[0][index2]) * (data[0][index1] - data[0][index2]);
	result += (data[1][index1] - data[1][index2]) * (data[1][index1] - data[1][index2]);
	
	return result;
}