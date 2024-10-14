#include <argp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "seq_twin_prime.c"

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

static error_t ParseOpt( int key, char *arg, struct argp_state *state) {
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

static struct argp argp = { options, ParseOpt, args_doc, doc };

int main(int argc, char **argv){

	int n, batchSize, verbose;
	int * argumentsBuffer = (int*)malloc(sizeof(int)*3);

	int my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

	// Have the root process parse the command line arguments so any output will
	// only be printed once.
	if(my_rank == ROOT_RANK) {
		struct arguments arguments;
		arguments.verbose = 0;

		argp_parse (&argp, argc, argv, 0, 0, &arguments);

		sscanf(arguments.args[0],"%d",&argumentsBuffer[0]);
		sscanf(arguments.args[1],"%d",&argumentsBuffer[1]);
		argumentsBuffer[2] = arguments.verbose;
	}

	// Broadcast the command line arguments processed by root.
	// TODO: Calculate number of values to send from structure of arguments
	MPI_Bcast( argumentsBuffer, 3, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

	n = argumentsBuffer[0];
	batchSize = argumentsBuffer[1];
	verbose = argumentsBuffer[2];

	free(argumentsBuffer);

	// Variables all processes will use.
	int foundNthPrime = 0;
	long long iteration = 0;
	int primesPerIter = nprocs * batchSize;
	int * resultBatch = (int*)malloc(sizeof(int)*batchSize);

	// Variables only the root process will use.
	int numTwins;
	long long lastPrime;
	int * gatheredResults;
	int * foundNthPrimeBuffer;
	double begin;
	if(my_rank == ROOT_RANK){
		numTwins = 0;
		lastPrime = 2;
		gatheredResults = (int*)malloc(sizeof(int)*primesPerIter);
		foundNthPrimeBuffer = (int*)malloc(sizeof(int)*nprocs);
		for( int index=0; index<nprocs; index++ ){
			foundNthPrimeBuffer[index] = 0;
		}
		if( verbose ){
			begin = MPI_Wtime();
		}
	}

	while( !foundNthPrime ){
		// Each process calculates if a different number is prime.
		for( int index=0; index<batchSize; index++ ){
			long long num = 2 + my_rank * batchSize + (primesPerIter * iteration) + index;
			resultBatch[ index ] = IsPrime( num );
		}

		// Gather results of prime calculations.
		MPI_Gather( resultBatch, batchSize, MPI_INT, gatheredResults, batchSize, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

		// Have the root process check for the presence of twin primes.
		if(my_rank == ROOT_RANK){
			for( int index=0; index<primesPerIter; index++ ){
				long long num = 2 + index + (primesPerIter * iteration);
				if( gatheredResults[index] ){
					if( num - 2 == lastPrime ){
						numTwins++;
						if( numTwins == n ){
							printf("The %dth twin prime is the pair (%lld, %lld).\n", n, lastPrime, num);
							for( int index=0; index<nprocs; index++ ){
								foundNthPrimeBuffer[index] = 1;
							}
						}
					}
					lastPrime = num;
				}
			}
		}

		// Scatter the root's foundNthPrime to report whether the processes can all
		// stop.
		MPI_Scatter( foundNthPrimeBuffer, 1, MPI_INT, &foundNthPrime, 1, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

		iteration++;
	}

	if(my_rank == ROOT_RANK && verbose){
		double end = MPI_Wtime();
		double seconds = end - begin;
	  printf("Took %f seconds.\n", seconds);
	}

	free(resultBatch);
	if(my_rank == ROOT_RANK) {
    free(gatheredResults);
		free(foundNthPrimeBuffer);
  }

	MPI_Finalize();
	return 0;
}
