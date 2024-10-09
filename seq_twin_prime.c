#include <stdio.h>
#include <time.h>

#include "seq_twin_prime.h"

int IsPrime(long long num){
	if (num > 1) {
		for (long long divisor=2; divisor < num; divisor++) {
			if (num % divisor == 0) {
				return 0;
			}
		}
		return 1;
	}
	return 0;
}

struct TwinPrime GetNthTwinPrime(int n){
	long long lastPrime = 2;
	int numTwins = 0;
	long long num = 2;
	double totalSeconds = 0.0;
	while (numTwins < n) {
		num++;
		clock_t begin = clock();
		int isPrime = IsPrime(num);
		clock_t end = clock();
		totalSeconds += (double)(end - begin) / CLOCKS_PER_SEC;
		if(isPrime) {
			if( (num - 2) == lastPrime ) {
				numTwins++;
			}
			if(numTwins < n){
				lastPrime = num;
			}
		}
	}

	printf("Took %f seconds evaluating whether numbers were prime.\n", totalSeconds);

	struct TwinPrime nthTwinPrime;
	nthTwinPrime.first = lastPrime;
	nthTwinPrime.second = num;
	return nthTwinPrime;
}
