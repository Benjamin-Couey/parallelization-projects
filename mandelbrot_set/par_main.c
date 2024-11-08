#include <argp.h>
#include <complex.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mandelbrot_set.c"

#define ROOT_RANK 0
#define BUFSIZE 128
#define SIZE_PER_LINE 32

static char doc[] = "mandelbrot_set -- A simple C script, parallelized with MPI, that calculates the Mandelbrot set. Should be executed with mpirun.";

static char args_doc[] = "Limit X resolution Y resolution";

static struct argp_option options[] = {
	{ "verbose", 'v', 0, 0, "Provide verbose output." },
	{ "output", 'o', "FILE", 0, "Output to specified file instead of standard mandelbrot_set.csv." },
	{ "high density ratio", 'h', "RATIO", 0, "Override ratio of y_resolution in high density chunk."  },
	{ 0 }
};

struct arguments {
	char *args[3];
	int verbose;
	char *output_file;
	double high_density_ratio;
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
		case 'h':
			arguments->high_density_ratio = atof(arg);
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

int main(int argc, char **argv){

	int verbose, max_iterations, x_resolution, y_resolution;
	double high_density_ratio = 0.0;
	// Array to store the arguments needed by all processes. Space is allocated for
	// the string length of the output file, so that non-root ranks will know how
	// much space to allocate.
	int * arguments_buffer = (int*)malloc(sizeof(int)*5);
	// Char array to store the name of the output file.
	char * output_file;

	int my_rank, n_procs;

	MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&n_procs);

	// Have the root process parse the command line arguments so any output will
	// only be printed once.
	if(my_rank == ROOT_RANK) {
		struct arguments arguments;
		arguments.verbose = 0;
		arguments.high_density_ratio = 0.0;
		arguments.output_file = "mandelbrot_set.csv";

		argp_parse (&argp, argc, argv, 0, 0, &arguments);

		sscanf(arguments.args[0],"%d",&arguments_buffer[0]);
		sscanf(arguments.args[1],"%d",&arguments_buffer[1]);
		sscanf(arguments.args[2],"%d",&arguments_buffer[2]);
		arguments_buffer[3] = arguments.verbose;
		output_file = arguments.output_file;
		arguments_buffer[4] = strlen(arguments.output_file);
		high_density_ratio = arguments.high_density_ratio;
	}

	// Broadcast the command line arguments processed by root.
	MPI_Bcast( arguments_buffer, 5, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );
	// Only the non-root ranks need to explicitly allocate space.
	if(my_rank != ROOT_RANK) {
		output_file = (char*)malloc(sizeof(char)*arguments_buffer[4]);
	}
	MPI_Bcast( output_file, arguments_buffer[4], MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD );

	MPI_Bcast( &high_density_ratio, 1, MPI_DOUBLE, ROOT_RANK, MPI_COMM_WORLD );

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
	int low_density_size, high_density_size;
	// If the user provided the ratio of the y_resolution taken up by the high_density_size,
	// use that.
	if( high_density_ratio > 0.0 && high_density_ratio <= 1.0 ){
		double low_density_ratio = ( 1.0 - high_density_ratio ) / 2;
		low_density_size = (int)y_resolution * low_density_ratio;
		high_density_size = (int)y_resolution * high_density_ratio;
	} else {
		low_density_size = y_resolution * 5 / 18;
		high_density_size = y_resolution * 4 / 9;
	}

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
	// A 2D array to store the displacement of the text each rank will write to the
	// output file for each chunk of y values.
	int ** file_offsets = (int**)malloc(sizeof(int*)*num_chunks);
  for (int chunk_i=0; chunk_i<num_chunks; chunk_i++) {
    file_offsets[chunk_i]=(int*)malloc(sizeof(int)*n_procs);
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

	// The buffers to store the results of a rank's calculations in the form they
	// will be written to the output file.
	char ** result_buffer = (char**)malloc(sizeof(char*)*num_chunks);
  for (int chunk_i=0; chunk_i<num_chunks; chunk_i++) {
    result_buffer[chunk_i]=(char*)malloc(sizeof(char)*num_to_send[chunk_i][my_rank]*SIZE_PER_LINE);
  }

	if( verbose ){
		set_calc_begin = clock();
	}

	// For each chunk, calculate Mandelbrot set in range and store results in the
	// corresponding buffer.
	// A char array to store strings as ranks calculate how much space in the output
	// file their results will take up and when actually writing to the file.
	char * char_buffer = (char*)malloc(sizeof(char)*BUFSIZE);
	for( int chunk_i=0; chunk_i<num_chunks; chunk_i++ ){
		int y_i = start_y_i[chunk_i];
		char * moving_pointer = result_buffer[chunk_i];
		while( y_i<max_y_i[chunk_i] ){
			double y = -1.5 + (y_i * y_step);
			for( int x_i=0; x_i<x_resolution; x_i++ ){
				double x = -2.0 + x_i * x_step;
				double complex c = x + y * I;
				int iterations = in_mandelbrot_set( c, max_iterations );
				// Calculate how much space the result will take up in the file.
				snprintf(char_buffer, BUFSIZE, "%f,%f,%d\n", x, y, iterations );
				int line_size = strlen( char_buffer );
				strcpy( moving_pointer, char_buffer );
				moving_pointer = moving_pointer + line_size;
				file_offsets[chunk_i][my_rank] += line_size;
			}
			y_i++;
		}
	}

	if( verbose ){
		set_calc_end = clock();
	  double seconds = (double)(set_calc_end - set_calc_begin) / CLOCKS_PER_SEC;
		printf("Rank %d took %f seconds to calculate its share of the points.\n", my_rank, seconds);
	}

	// Gather the file_offsets calculated by reach rank.
	for( int chunk_i=0; chunk_i<num_chunks; chunk_i++ ){
		MPI_Allgather(
			MPI_IN_PLACE,
			1,
			MPI_INT,
			file_offsets[chunk_i],
			1,
			MPI_INT,
			MPI_COMM_WORLD
		);
	}

	// file_offsets currently stores the character length of each rank's chunk of
	// results to write. Need to recalculate it so it describes how far, from 0,
	// each rank should start writing it's results.
	for( int chunk_i=0; chunk_i<num_chunks; chunk_i++ ){
		for( int rank_i=0; rank_i<n_procs; rank_i++ ){
			if( rank_i>0 ){
				file_offsets[chunk_i][rank_i] += file_offsets[chunk_i][rank_i-1];
			} else if( chunk_i>0 ){
				file_offsets[chunk_i][rank_i] += file_offsets[chunk_i-1][n_procs-1];
			}
		}
	}

	for( int chunk_i=num_chunks-1; chunk_i>=0; chunk_i-- ){
		for( int rank_i=n_procs-1; rank_i>=0; rank_i-- ){
			if( rank_i>0 ){
				file_offsets[chunk_i][rank_i] = file_offsets[chunk_i][rank_i-1];
			} else {
				if( chunk_i>0 ){
					file_offsets[chunk_i][rank_i] = file_offsets[chunk_i-1][n_procs-1];
				} else {
					file_offsets[chunk_i][rank_i] = 0;
				}
			}
		}
	}

	// Have the processes write the results to a file.
	// Only need to write the header once, so only have the root write the header.
	int header_size = strlen("x,y,z\n");
	if( my_rank == ROOT_RANK ){
		FILE * file;
		file = fopen(output_file, "w+");
		fprintf(file, "x,y,z\n");
		fclose(file);
	}

	MPI_File file;
	MPI_File_open( MPI_COMM_WORLD, output_file, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file );

	for( int chunk_i=0; chunk_i<num_chunks; chunk_i++ ){
		MPI_File_set_view( file, header_size + file_offsets[chunk_i][my_rank], MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL );
		MPI_File_write_all( file, result_buffer[chunk_i], strlen(result_buffer[chunk_i]), MPI_CHAR, MPI_STATUS_IGNORE );
	}
	MPI_File_close(&file);

	if(my_rank != ROOT_RANK){
		free(output_file);
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
	free(char_buffer);

	if( my_rank == ROOT_RANK && verbose ){
		end = clock();
	  double seconds = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Took %f seconds.\n", seconds);
	}
	MPI_Finalize();
	return 0;
}
