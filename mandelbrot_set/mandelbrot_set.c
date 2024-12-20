#include <complex.h>

#include "mandelbrot_set.h"

int in_mandelbrot_set( double complex c, int limit ){
	// The absolute value of z must remain <= 2 for c to be in the set.
	double complex z = 0.0 + 0.0 * I;

	// Since c = z_1, cabs(c) > 2 are not in the set.
	if( cabs(c) > 2.0 ){
		return 0;
	}

	// Perform naive escape time algorithm.
	int i=0;
	while( i<limit ){
		z = z * z + c;
		if( cabs(z) > 2.0 ){
			return i;
		}
		i++;
	}
	return i;
}
