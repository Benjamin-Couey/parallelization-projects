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

	while (numTwins < n) {
		num++;
		if(IsPrime(num)) {
			if( (num - 2) == lastPrime ) {
				numTwins++;
			}
			if(numTwins < n){
				lastPrime = num;
			}
		}
	}

	struct TwinPrime nthTwinPrime;
	nthTwinPrime.first = lastPrime;
	nthTwinPrime.second = num;
	return nthTwinPrime;
}
