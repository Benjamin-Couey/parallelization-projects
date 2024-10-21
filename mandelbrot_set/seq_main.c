#include <argp.h>
#include <complex.h>
#include <stdio.h>

#include "mandelbrot_set.c"

static char doc[] = "mandelbrot_set -- A simple sequential C script that calculates the Mandelbrot set.";

static char args_doc[] = "Limit X resolution Y resolution";

struct arguments {
	char *args[3];
};

static error_t parse_opt( int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch(key) {
		case ARGP_KEY_ARG:
			if( state->arg_num >= 3 ){
				argp_usage( state );
			}
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if( state->arg_num < 3 ){
				argp_usage( state );
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { 0, parse_opt, args_doc, doc };

int main(int argc, char **argv){

	int max_iterations, x_resolution, y_resolution;

	struct arguments arguments;

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	sscanf(arguments.args[0],"%d",&max_iterations);
	sscanf(arguments.args[0],"%d",&x_resolution);
	sscanf(arguments.args[0],"%d",&y_resolution);

	double x_step = 3.0 / x_resolution;
	double y_step = 3.0 / y_resolution;

	FILE * file;
	file = fopen("mandelbrot_set.csv", "w+");
	for( double y=-1.5; y<=1.5; y+=y_step ){
		for( double x=-2.0; x<=1.0; x+=x_step ){
			double complex c = x + y * I;
			int iterations = in_mandelbrot_set( c, max_iterations );
			fprintf(file, "%d", iterations);
			if( x + x_step <= 1.0 ){
				fprintf(file, ",");
			}
		}
		fprintf(file, "\n");
	}
	fclose(file);

	return 0;
}
