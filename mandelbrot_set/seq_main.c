#include <complex.h>
#include <stdio.h>

#include "mandelbrot_set.c"

int main(int argc, char **argv){

	int max_iterations = 100;
	int x_resolution = 144;
	int y_resolution = 144;

	double x_step = 4.0 / x_resolution;
	double y_step = 4.0 / x_resolution;

	FILE * file;
	file = fopen("mandelbrot_set.csv", "w+");
	for( double y=-2.0; y<4.0; y+=y_step ){
		for( double x=-2.0; x<4.0; x+=x_step ){
			double complex c = x + y * I;
			int iterations = in_mandelbrot_set( c, max_iterations );
			fprintf(file, "%d", iterations);
			if( x + x_step < 4.0 ){
				fprintf(file, ",");
			}
		}
		fprintf(file, "\n");
	}
	fclose(file);

	return 0;
}
