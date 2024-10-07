#include <stdio.h>
#include <time.h>

#include "seq_twin_prime.c"

int main(int argc, char **argv){

	int n;

	if (argc != 2) {
    fprintf(stderr,"Please provide the following on the command line: n.\n");
    return 0;
  }

	sscanf(argv[1],"%d",&n);

  clock_t begin = clock();

	struct TwinPrime nthTwinPrime = GetNthTwinPrime(n);

  clock_t end = clock();
  double seconds = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("The %dth twin prime is the pair (%lld, %lld).\n", n, nthTwinPrime.first, nthTwinPrime.second);
  printf("Took %f seconds.\n", seconds);

	return 0;
}
