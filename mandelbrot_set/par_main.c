#include <argp.h>
#include <complex.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mandelbrot_set.c"

#define ROOT_RANK 0

static char doc[] = "mandelbrot_set -- A simple sequential C script that calculates the Mandelbrot set.";

static char args_doc[] = "Limit X resolution Y resolution";

static struct argp_option options[] = {
	{ "verbose", 'v', 0, 0, "Provide verbose output." },
	{ 0 }
};

struct arguments {
	char *args[3];
	int verbose;
};

static error_t parse_opt( int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch(key) {
		case 'v':
			arguments->verbose = 1;
			break;
		case ARGP_KEY_ARG:
			if( state->arg_num >= 3 ){
				argp_usage( state );
			}
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if( state->arg_num < 3 ){
				argp_usage( state );
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

struct in_set_result {
	double x, y;
	int iterations;
};

int main(int argc, char **argv){

	int verbose, max_iterations, x_resolution, y_resolution;
	int * arguments_buffer = (int*)malloc(sizeof(int)*4);

	int my_rank, n_procs;

	MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&n_procs);

	// Have the root process parse the command line arguments so any output will
	// only be printed once.
	if(my_rank == ROOT_RANK) {
		struct arguments arguments;
		arguments.verbose = 0;

		argp_parse (&argp, argc, argv, 0, 0, &arguments);

		sscanf(arguments.args[0],"%d",&arguments_buffer[0]);
		sscanf(arguments.args[1],"%d",&arguments_buffer[1]);
		sscanf(arguments.args[2],"%d",&arguments_buffer[2]);
		arguments_buffer[3] = arguments.verbose;
	}

	// Broadcast the command line arguments processed by root.
	// TODO: Calculate number of values to send from structure of arguments
	MPI_Bcast( arguments_buffer, 4, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

	max_iterations = arguments_buffer[0];
	x_resolution = arguments_buffer[1];
	y_resolution = arguments_buffer[2];
	verbose = arguments_buffer[3];

	free(arguments_buffer);

	// printf("My rank is %d, max_iterations: %d, x_resolution: %d, y_resolution: %d, verbose: %d\n", my_rank, max_iterations, x_resolution, y_resolution, verbose);
	double x_step = 3.0 / x_resolution;
	double y_step = 3.0 / y_resolution;

	clock_t begin, end, set_calc_begin, set_calc_end;
	if( my_rank == ROOT_RANK && verbose ){
		begin = clock();
	}

	// For high limits, the bulk of the computational time will be spent verifying
	// points that are inside the Mandelbrot set.
	// Since these points are densest for y values between about -0.66 and 0.66,
	// in an effort to balance the load that range will be divided evenly between
	// all the ranks. The remaining y values will be divided seperately.

	// The range of -0.66 to 0.66 is about 0.44 or 4/9ths of the full -1.5 to 1.5
	// range of y values the script is calculating. This leaves two low density areas,
	// each about 5/18ths on either side.
	int low_density_size = y_resolution * 5 / 18;
	int high_density_size = y_resolution * 4 / 9;

	printf(
		"low_density_size is %d and high_density_size is %d\n",
		low_density_size,
		high_density_size
	);

	// Variables all processes will use.
	double x, y;
	int buffer_i = 0;
	int y_per_rank;
	int points_per_rank = 0;
	// An array to store the starting and max y indices for the low and high density
	// y values each rank is responsible for.
	int * y_i = (int*)malloc(sizeof(int)*3);
	int * max_y_i = (int*)malloc(sizeof(int)*3);
	// An array to store the size of each rank's result buffer.
  int * num_to_send = (int*)malloc(sizeof(int)*n_procs);
	for( int index=0; index<n_procs; index++ ){
		num_to_send[index] = 0;
	}
	// Have each rank calculate the range of y values each rank will be responsible
	// for and size of each rank's result buffer.
	// Technically, only the root process needs this information for MPI_Gatherv,
	// but it is convienent to do all these calculations in one place.

 	// TODO: This is currently bad from a readability/maintainability standpoint.
	// Consider a simpler way to divy up the y values?

	// Divide the first low density y indices.
	y_per_rank = low_density_size / n_procs;
	for( int index=0; index<n_procs; index++ ){
		int temp_y_i = y_per_rank * index;
		int temp_max_y_i = y_per_rank * (index + 1);
		// Assign any leftover y points to the last rank.
		if( index == n_procs-1 ){
			temp_max_y_i = low_density_size;
		}
		int temp_points_per_rank = (temp_max_y_i-temp_y_i) * x_resolution;
		num_to_send[index] += temp_points_per_rank;
		// Save the results of this calculation to the approprite rank.
		if( my_rank == index ){
			y_i[0] = temp_y_i;
			max_y_i[0] = temp_max_y_i;
			points_per_rank += temp_points_per_rank;
		}
	}

	// Divide the high density y indices.
	y_per_rank = high_density_size / n_procs;
	for( int index=0; index<n_procs; index++ ){
		int temp_y_i = y_per_rank * index + low_density_size;
		int temp_max_y_i = y_per_rank * (index + 1) + low_density_size;
		// Assign any leftover y points to the last rank.
		if( index == n_procs-1 ){
			temp_max_y_i = low_density_size + high_density_size;
		}
		int temp_points_per_rank = (temp_max_y_i-temp_y_i) * x_resolution;
		num_to_send[index] += temp_points_per_rank;
		// Save the results of this calculation to the approprite rank.
		if( my_rank == index ){
			y_i[1] = temp_y_i;
			max_y_i[1] = temp_max_y_i;
			points_per_rank += temp_points_per_rank;
		}
	}

	// Divide the second low density y indices.
	y_per_rank = low_density_size / n_procs;
	for( int index=0; index<n_procs; index++ ){
		int temp_y_i = y_per_rank * index + low_density_size + high_density_size;
		int temp_max_y_i = y_per_rank * (index + 1) + low_density_size + high_density_size;
		// Assign any leftover y points to the last rank.
		if( index == n_procs-1 ){
			temp_max_y_i = y_resolution;
		}
		int temp_points_per_rank = (temp_max_y_i-temp_y_i) * x_resolution;
		num_to_send[index] += temp_points_per_rank;
		// Save the results of this calculation to the approprite rank.
		if( my_rank == index ){
			y_i[2] = temp_y_i;
			max_y_i[2] = temp_max_y_i;
			points_per_rank += temp_points_per_rank;
		}
	}

	// A buffer to store the results of a rank's calculations.
	struct in_set_result * result_buffer = (struct in_set_result*)malloc(sizeof(struct in_set_result)*points_per_rank);

	// Variables only the root process will use.
	// A buffer to store the gathered results of all the ranks' calculations.
	struct in_set_result * gathered_results;
	// An array to store the displacement of the results the root rank will be
  // receiving.
  int * displacement;

	if(my_rank == ROOT_RANK) {
		gathered_results = (struct in_set_result*)malloc(sizeof(struct in_set_result)*x_resolution*y_resolution);
		displacement = (int*)malloc(sizeof(int)*n_procs);
		for ( int index=0; index<n_procs; index++) {
			displacement[ index ] = (index > 0) ? ( displacement[ index-1 ] + num_to_send[ index-1 ] ) : 0;
	  }
	}

	if( verbose ){
		set_calc_begin = clock();
	}

	// printf("Rank %d calculating points for y from %d to %d for a total of %d points\n", my_rank, y_i, max_y_i, points_per_rank);
	// Calculate Mandelbrot set in range, store results in buffer.
	for( int index=0; index<3; index++ ){
		int y_index = y_i[index];
		while( y_index<max_y_i[index] ){
			y = -1.5 + (y_index * y_step);
			for( int x_i=0; x_i<x_resolution; x_i++ ){
				x = -2.0 + x_i * x_step;
				// printf("Rank %d calculating point %f,%f\n", my_rank, x, y);
				double complex c = x + y * I;
				struct in_set_result result;
				result.x = x;
				result.y = y;
				result.iterations = in_mandelbrot_set( c, max_iterations );
				result_buffer[ buffer_i ] = result;
				buffer_i++;
			}
			y_index++;
		}
	}

	if( verbose ){
		set_calc_end = clock();
	  double seconds = (double)(set_calc_end - set_calc_begin) / CLOCKS_PER_SEC;
		printf("Rank %d took %f seconds to calculate its share of the points.\n", my_rank, seconds);
	}


	//Create a datatype for the nth worker_results[n] struct
	MPI_Datatype mpi_in_set_result;
	int lengths[3] = { 1, 1, 1 };
	// Calculate the displacement of the struct's fields.
	MPI_Aint displacements[3];
	struct in_set_result dummy_result;
	MPI_Aint base_address;
	MPI_Get_address(&dummy_result, &base_address);
  MPI_Get_address(&dummy_result.x, &displacements[0]);
  MPI_Get_address(&dummy_result.y, &displacements[1]);
  MPI_Get_address(&dummy_result.iterations, &displacements[2]);
	displacements[0] = MPI_Aint_diff(displacements[0], base_address);
  displacements[1] = MPI_Aint_diff(displacements[1], base_address);
  displacements[2] = MPI_Aint_diff(displacements[2], base_address);
	MPI_Datatype types[3] = { MPI_DOUBLE, MPI_DOUBLE, MPI_INT };
	MPI_Type_create_struct( 3, lengths, displacements, types, &mpi_in_set_result );
	MPI_Type_commit(&mpi_in_set_result);

	// Gather results of Mandelbrot set calculations
	MPI_Gatherv(
		result_buffer,
		points_per_rank,
		mpi_in_set_result,
		gathered_results,
		num_to_send,
		displacement,
		mpi_in_set_result,
		ROOT_RANK,
		MPI_COMM_WORLD
	);

	// if( my_rank == ROOT_RANK ){
	// 	for( int b_i=0; b_i<(x_resolution*y_resolution); b_i++ ){
	// 		printf(
	// 			"Some rank calculated point %f,%f; stopped after %d iterations\n",
	// 			gathered_results[b_i].x,
	// 			gathered_results[b_i].y,
	// 			gathered_results[b_i].iterations
	// 		);
	// 	}
	// }

	// Have the root process write the results to a file.
	// TODO: Look into trying to parallelize this process.
	if(my_rank == ROOT_RANK) {
		FILE * file;
		file = fopen("mandelbrot_set.csv", "w+");
		fprintf(file, "x,y,z\n");
		for( int b_i=0; b_i<(x_resolution*y_resolution); b_i++ ){
			fprintf(
				file,
				"%f,%f,%d\n",
				gathered_results[b_i].x,
				gathered_results[b_i].y,
				gathered_results[b_i].iterations
			);
		}
		fclose(file);
	}

	free(y_i);
	free(max_y_i);
	free(num_to_send);
	free(result_buffer);
	if(my_rank == ROOT_RANK) {
		free(displacement);
		free(gathered_results);
	}

	if( my_rank == ROOT_RANK && verbose ){
		end = clock();
	  double seconds = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Took %f seconds.\n", seconds);
	}
	MPI_Finalize();
	return 0;
}
