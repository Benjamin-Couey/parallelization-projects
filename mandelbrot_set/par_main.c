#include <argp.h>
#include <complex.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mandelbrot_set.c"

#define ROOT_RANK 0

static char doc[] = "mandelbrot_set -- A simple C script, parallelized with MPI, that calculates the Mandelbrot set. Should be executed with mpirun.";

static char args_doc[] = "Limit X resolution Y resolution";

static struct argp_option options[] = {
	{ "verbose", 'v', 0, 0, "Provide verbose output." },
	{ "output", 'o', "FILE", 0, "Output to specified file instead of standard mandelbrot_set.csv." },
	{ 0 }
};

struct arguments {
	char *args[3];
	int verbose;
	char *output_file;
};

static error_t parse_opt( int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch(key) {
		case 'v':
			arguments->verbose = 1;
			break;
		case 'o':
      arguments->output_file = arg;
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
	// Variable only the root process will use.
	char * output_file;
	// Array to store the arguments needed by all processes. Space is not allocated
	// for the output_file argument since only root will use it.
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
		arguments.output_file = "mandelbrot_set.csv";

		argp_parse (&argp, argc, argv, 0, 0, &arguments);

		sscanf(arguments.args[0],"%d",&arguments_buffer[0]);
		sscanf(arguments.args[1],"%d",&arguments_buffer[1]);
		sscanf(arguments.args[2],"%d",&arguments_buffer[2]);
		arguments_buffer[3] = arguments.verbose;
		output_file = arguments.output_file;
	}

	// Broadcast the command line arguments processed by root.
	MPI_Bcast( arguments_buffer, 4, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

	max_iterations = arguments_buffer[0];
	x_resolution = arguments_buffer[1];
	y_resolution = arguments_buffer[2];
	verbose = arguments_buffer[3];

	free(arguments_buffer);

	double x_step = 3.0 / x_resolution;
	double y_step = 3.0 / y_resolution;

	clock_t begin, end, set_calc_begin, set_calc_end;
	if( my_rank == ROOT_RANK && verbose ){
		begin = clock();
	}

	// For high limits, the bulk of the computational time will be spent verifying
	// points that are inside the Mandelbrot set.
	// Since these points are densest for y values between about -0.66 and 0.66,
	// in an effort to balance the load that range will be divided seperate from the
	// other y values. This will create 3 chunks of y values that each need to be
	// divided amongst the ranks.

	// The range of -0.66 to 0.66 is about 1.33/3 or 4/9ths of the full -1.5 to 1.5
	// range of y values the script is calculating. This creates one high density
	// chunk with a low density chunk, each about 5/18ths, on either side.
	int low_density_size = y_resolution * 5 / 18;
	int high_density_size = y_resolution * 4 / 9;
	int num_chunks = 3;
	int * chunk_sizes = (int*)malloc(sizeof(int)*num_chunks);
	chunk_sizes[0] = low_density_size;
	chunk_sizes[1] = high_density_size;
	chunk_sizes[2] = low_density_size;

	// An array to store the starting and max y values for the chunks of y values
	// each ranks is responsible for.
	int * start_y_i = (int*)malloc(sizeof(int)*num_chunks);
	int * max_y_i = (int*)malloc(sizeof(int)*num_chunks);
	// A 2D array to store the size of each rank's result buffers for each chunk
	// of y values.
	int ** num_to_send = (int**)malloc(sizeof(int*)*num_chunks);
  for (int chunk_i=0; chunk_i<num_chunks; chunk_i++) {
    num_to_send[chunk_i]=(int*)malloc(sizeof(int)*n_procs);
  }

	// Have all ranks calculate the range of y values each rank will be responsible
	// for for each chunk of y values.
	// Technically, only the root process needs this information from other ranks
	// for MPI_Gatherv, but it is convienent to do all these calculations in one
	// place.
	int y_offset = 0;
	for( int chunk_i=0; chunk_i<num_chunks; chunk_i++ ){
		int y_per_rank = chunk_sizes[ chunk_i ] / n_procs;
		if( chunk_i > 0 ){
			y_offset += chunk_sizes[ chunk_i-1 ];
		}
		for( int rank_i=0; rank_i<n_procs; rank_i++ ){
			int temp_start_y_i = y_per_rank * rank_i + y_offset;
			int temp_max_y_i = y_per_rank * (rank_i + 1) + y_offset;
			if( rank_i == n_procs-1 ){
				if( chunk_i == num_chunks-1 ){
					temp_max_y_i = y_resolution;
				}else{
					temp_max_y_i = y_offset + chunk_sizes[ chunk_i ];
				}
			}
			int temp_points_per_rank = (temp_max_y_i-temp_start_y_i) * x_resolution;
			num_to_send[chunk_i][rank_i] = temp_points_per_rank;
			if( my_rank == rank_i ){
				start_y_i[chunk_i] = temp_start_y_i;
				max_y_i[chunk_i] = temp_max_y_i;
			}
		}
	}

	// The buffers to store the results of a rank's calculations.
	struct in_set_result ** result_buffer = (struct in_set_result**)malloc(sizeof(struct in_set_result*)*num_chunks);
  for (int chunk_i=0; chunk_i<num_chunks; chunk_i++) {
    result_buffer[chunk_i]=(struct in_set_result*)malloc(sizeof(struct in_set_result)*num_to_send[chunk_i][my_rank]);
  }

	// Variables only the root process will use.
	// A buffer to store the gathered results of all the ranks' calculations.
	struct in_set_result * gathered_results;
	// A 2D array to store the displacement of the results the root rank will be
  // receiving for each chunk of y values.
  int ** displacement = (int**)malloc(sizeof(int*)*num_chunks);

	if(my_rank == ROOT_RANK) {
		gathered_results = (struct in_set_result*)malloc(sizeof(struct in_set_result)*x_resolution*y_resolution);
	  for (int chunk_i=0; chunk_i<num_chunks; chunk_i++) {
	    displacement[chunk_i]=(int*)malloc(sizeof(int)*n_procs);
			for ( int rank_i=0; rank_i<n_procs; rank_i++) {
				if( rank_i > 0 ){
					displacement[chunk_i][rank_i] = displacement[chunk_i][rank_i-1] + num_to_send[chunk_i][rank_i-1];
				} else {
					if( chunk_i > 0 ) {
						displacement[chunk_i][rank_i] = displacement[chunk_i-1][n_procs-1] + num_to_send[chunk_i-1][n_procs-1];
					} else {
						displacement[chunk_i][rank_i] == 0;
					}
				}
		  }
	  }
	}

	if( verbose ){
		set_calc_begin = clock();
	}

	// For each chunk, calculate Mandelbrot set in range, store results in
	// corresponding buffer.
	for( int chunk_i=0; chunk_i<num_chunks; chunk_i++ ){
		int y_i = start_y_i[chunk_i];
		int buffer_i = 0;
		while( y_i<max_y_i[chunk_i] ){
			double y = -1.5 + (y_i * y_step);
			for( int x_i=0; x_i<x_resolution; x_i++ ){
				double x = -2.0 + x_i * x_step;
				double complex c = x + y * I;
				struct in_set_result result;
				result.x = x;
				result.y = y;
				result.iterations = in_mandelbrot_set( c, max_iterations );
				result_buffer[chunk_i][ buffer_i ] = result;
				buffer_i++;
			}
			y_i++;
		}
	}

	if( verbose ){
		set_calc_end = clock();
	  double seconds = (double)(set_calc_end - set_calc_begin) / CLOCKS_PER_SEC;
		printf("Rank %d took %f seconds to calculate its share of the points.\n", my_rank, seconds);
	}

	// Create a datatype for the in_set_result struct
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
	for( int chunk_i=0; chunk_i<num_chunks; chunk_i++){
		MPI_Gatherv(
			result_buffer[chunk_i],
			num_to_send[chunk_i][my_rank],
			mpi_in_set_result,
			gathered_results,
			num_to_send[chunk_i],
			displacement[chunk_i],
			mpi_in_set_result,
			ROOT_RANK,
			MPI_COMM_WORLD
		);
	}

	// Have the root process write the results to a file.
	// TODO: Look into trying to parallelize this process.
	if(my_rank == ROOT_RANK) {
		FILE * file;
		file = fopen(output_file, "w+");
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

	free(chunk_sizes);
	free(start_y_i);
	free(max_y_i);
	for(int chunk_i=0; chunk_i<num_chunks; chunk_i++){
    free(num_to_send[chunk_i]);
  }
  free(num_to_send);
	for(int chunk_i=0; chunk_i<num_chunks; chunk_i++){
    free(result_buffer[chunk_i]);
  }
	free(result_buffer);
	if(my_rank == ROOT_RANK) {
		for(int chunk_i=0; chunk_i<num_chunks; chunk_i++){
	    free(displacement[chunk_i]);
	  }
		free(gathered_results);
	}
	free(displacement);

	if( my_rank == ROOT_RANK && verbose ){
		end = clock();
	  double seconds = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Took %f seconds.\n", seconds);
	}
	MPI_Finalize();
	return 0;
}
