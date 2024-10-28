#include <argp.h>
#include <complex.h>
#include <stdio.h>
#include <time.h>

#include "mandelbrot_set.c"

static char doc[] = "mandelbrot_set -- A simple sequential C script that calculates the Mandelbrot set.";

static char args_doc[] = "Limit X resolution Y resolution";

static struct argp_option options[] = {
	{ "verbose", 'v', 0, 0, "Provide verbose output." },
	{ "output", 'o', "FILE", 0, "Output to specified file instead of standard mandelbrot_set.csv." },
	{ 0 }
};

struct arguments {
	char *args[3];
	int verbose;
	char *output_file;
};

static error_t parse_opt( int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch(key) {
		case 'v':
			arguments->verbose = 1;
			break;
		case 'o':
      arguments->output_file = arg;
      break;
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

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv){

	int verbose, max_iterations, x_resolution, y_resolution;
	char * output_file;

	struct arguments arguments;
	arguments.verbose = 0;
	arguments.output_file = "mandelbrot_set.csv";

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	sscanf(arguments.args[0],"%d",&max_iterations);
	sscanf(arguments.args[1],"%d",&x_resolution);
	sscanf(arguments.args[2],"%d",&y_resolution);
	verbose = arguments.verbose;
	output_file = arguments.output_file;

	double x_step = 3.0 / x_resolution;
	double y_step = 3.0 / y_resolution;

	clock_t begin, end;
	if( verbose ){
		begin = clock();
	}

	FILE * file;
	file = fopen(output_file, "w+");
	fprintf(file, "x,y,z\n");
	double x, y;
	for( int y_i=0; y_i<y_resolution; y_i++ ){
		y = -1.5 + (y_i * y_step);
		for( int x_i=0; x_i<x_resolution; x_i++ ){
			x = -2.0 + x_i * x_step;
			double complex c = x + y * I;
			int iterations = in_mandelbrot_set( c, max_iterations );
			fprintf(file, "%f,%f,%d\n", x, y, iterations);
		}
	}
	fclose(file);

	if( verbose ){
		end = clock();
	  double seconds = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Took %f seconds.\n", seconds);
	}

	return 0;
}
