#include <stdio.h>
#include <time.h>

#include "twin_prime.h"

int is_prime(long long num){
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

struct twin_prime get_nth_twin_prime(int n, int verbose){
	long long last_prime = 2;
	int num_twins = 0;
	long long num = 2;
	double total_seconds = 0.0;
	clock_t begin, end;
	while (num_twins < n) {
		num++;
		if( verbose ){
			begin = clock();
		}
		int num_is_prime = is_prime(num);
		if( verbose ){
			end = clock();
			total_seconds += (double)(end - begin) / CLOCKS_PER_SEC;
		}
		if(num_is_prime) {
			if( (num - 2) == last_prime ) {
				num_twins++;
			}
			if(num_twins < n){
				last_prime = num;
			}
		}
	}

	if( verbose ){
		printf("Took %f seconds evaluating whether numbers were prime.\n", total_seconds);
	}

	struct twin_prime nth_twin_prime;
	nth_twin_prime.first = last_prime;
	nth_twin_prime.second = num;
	return nth_twin_prime;
}
