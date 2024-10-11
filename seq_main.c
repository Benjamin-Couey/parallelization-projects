#include <stdio.h>
#include <time.h>

#include "seq_twin_prime.c"

int main(int argc, char **argv){

	int n, verbose;

	if (argc != 3) {
    fprintf(stderr,"Please provide the following on the command line: n and verbose.\n");
    return 0;
  }

	sscanf(argv[1],"%d",&n);
	sscanf(argv[2],"%d",&verbose);

	clock_t begin, end;
	if( verbose ){
		begin = clock();
	}

	struct TwinPrime nthTwinPrime = GetNthTwinPrime(n, verbose);

	if( verbose ){
		end = clock();
	  double seconds = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Took %f seconds.\n", seconds);
	}

	printf("The %dth twin prime is the pair (%lld, %lld).\n", n, nthTwinPrime.first, nthTwinPrime.second);

	return 0;
}
