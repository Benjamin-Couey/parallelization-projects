#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "seq_twin_prime.c"

#define ROOT_RANK 0

int main(int argc, char **argv){

	int my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

	int n;

	if (argc != 2) {
    fprintf(stderr,"Please provide the following on the command line: n.\n");
		MPI_Finalize();
    return 0;
  }

	sscanf(argv[1],"%d",&n);

	double begin = MPI_Wtime();

	// Variables all processes will use.
	int foundNthPrime = 0;
	int iteration = 0;

	// Variables only the root process will use.
	int numTwins;
	int lastPrime;
	int * gatheredResults;
	int * foundNthPrimeBuffer;
	if(my_rank == ROOT_RANK){
		numTwins = 0;
		gatheredResults = (int*)malloc(sizeof(int)*nprocs);
		foundNthPrimeBuffer = (int*)malloc(sizeof(int)*nprocs);
		for( int index=0; index<nprocs; index++ ){
			foundNthPrimeBuffer[index] = 0;
		}
	}

	while( !foundNthPrime ){
		// Each process calculates if a different number is prime.
		int num = 2 + my_rank + (nprocs * iteration);
		int result = IsPrime( num );

		// TODO: Avoid synchronizing every iteration, as this will massively hurt
		// performance.
		// Gather results of prime calculations.
		MPI_Gather( &result, 1, MPI_INT, gatheredResults, 1, MPI_INT, ROOT_RANK, MPI_COMM_WORLD );

		// Have the root process check for the presence of twin primes.
		if(my_rank == ROOT_RANK){
			for( int index=0; index<nprocs; index++ ){
				int num = 2 + index + (nprocs * iteration);
				if( gatheredResults[index] ){
					if( num - 2 == lastPrime ){
						numTwins++;
						if( numTwins == n ){
							printf("The %dth twin prime is the pair (%d, %d).\n", n, lastPrime, num);
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

	double end = MPI_Wtime();
	double seconds = end - begin;

	if(my_rank == ROOT_RANK){
	  printf("Took %f seconds.\n", seconds);
	}

	if(my_rank == ROOT_RANK) {
    free(gatheredResults);
		free(foundNthPrimeBuffer);
  }

	MPI_Finalize();
	return 0;
}
