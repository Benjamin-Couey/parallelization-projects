#include <argp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "twin_prime.c"

#define ROOT_RANK 0

static char doc[] = "par_twin_prime -- A simple C script, parallelized with MPI, that calculates the nth twin prime. Should be executed with mpirun.";

static char args_doc[] = "Number of twin primes to calculate Size of batch";

static struct argp_option options[] = {
	{ "verbose", 'v', 0, 0, "Provide verbose output." },
	{ 0 }
};

struct arguments {
	char *args[2];
	int verbose;
};

static error_t parse_opt( int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch(key) {
		case 'v':
			arguments->verbose = 1;
			break;
		case ARGP_KEY_ARG:
			if( state->arg_num >= 2 ){
				argp_usage( state );
			}
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if( state->arg_num < 2 ){
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

	int n, batch_size, verbose;
	int * arguments_buffer = (int*)malloc(sizeof(int)*3);

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
		arguments_buffer[2] = arguments.verbose;
	}

	// Broadcast the command line arguments processed by root.
	// TODO: Calculate number of values to send from structure of arguments
	MPI_Bcast( arguments_buffer, 3, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

	n = arguments_buffer[0];
	batch_size = arguments_buffer[1];
	verbose = arguments_buffer[2];

	free(arguments_buffer);

	// Variables all processes will use.
	int found_nth_prime = 0;
	long long iteration = 0;
	int primes_per_iter = n_procs * batch_size;
	int * result_batch = (int*)malloc(sizeof(int)*batch_size);

	// Variables only the root process will use.
	int num_twins;
	long long last_prime;
	int * gathered_results;
	int * found_nth_prime_buffer;
	double begin;
	if(my_rank == ROOT_RANK){
		num_twins = 0;
		last_prime = 2;
		gathered_results = (int*)malloc(sizeof(int)*primes_per_iter);
		found_nth_prime_buffer = (int*)malloc(sizeof(int)*n_procs);
		for( int index=0; index<n_procs; index++ ){
			found_nth_prime_buffer[index] = 0;
		}
		if( verbose ){
			begin = MPI_Wtime();
		}
	}

	while( !found_nth_prime ){
		// Each process calculates if a different number is prime.
		for( int index=0; index<batch_size; index++ ){
			long long num = 2 + my_rank * batch_size + (primes_per_iter * iteration) + index;
			result_batch[ index ] = is_prime( num );
		}

		// Gather results of prime calculations.
		MPI_Gather( result_batch, batch_size, MPI_INT, gathered_results, batch_size, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

		// Have the root process check for the presence of twin primes.
		if(my_rank == ROOT_RANK){
			for( int index=0; index<primes_per_iter; index++ ){
				long long num = 2 + index + (primes_per_iter * iteration);
				if( gathered_results[index] ){
					if( num - 2 == last_prime ){
						num_twins++;
						if( num_twins == n ){
							printf("The %dth twin prime is the pair (%lld, %lld).\n", n, last_prime, num);
							for( int index=0; index<n_procs; index++ ){
								found_nth_prime_buffer[index] = 1;
							}
						}
					}
					last_prime = num;
				}
			}
		}

		// Scatter the root's found_nth_prime to report whether the processes can all
		// stop.
		MPI_Scatter( found_nth_prime_buffer, 1, MPI_INT, &found_nth_prime, 1, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

		iteration++;
	}

	if(my_rank == ROOT_RANK && verbose){
		double end = MPI_Wtime();
		double seconds = end - begin;
	  printf("Took %f seconds.\n", seconds);
	}

	free(result_batch);
	if(my_rank == ROOT_RANK) {
    free(gathered_results);
		free(found_nth_prime_buffer);
  }

	MPI_Finalize();
	return 0;
}
