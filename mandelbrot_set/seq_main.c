#include <complex.h>
#include <stdio.h>

#include "mandelbrot_set.c"

int main(int argc, char **argv){

	int max_iterations = 100;

	double complex c = -1 + 0 * I;

	int iterations = in_mandelbrot_set( c, max_iterations );

	printf("in_mandelbrot_set exited after %d iterations.\n", iterations);

	return 0;
}
