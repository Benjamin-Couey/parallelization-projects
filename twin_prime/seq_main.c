#include <argp.h>
#include <stdio.h>
#include <time.h>

#include "seq_twin_prime.c"

static char doc[] = "seq_twin_prime -- A simple sequential C script that calculates the nth twin prime.";

static char args_doc[] = "Number of twin primes to calculate";

static struct argp_option options[] = {
	{ "verbose", 'v', 0, 0, "Provide verbose output." },
	{ 0 }
};

struct arguments {
	char *args[1];
	int verbose;
};

static error_t parse_opt( int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch(key) {
		case 'v':
			arguments->verbose = 1;
			break;
		case ARGP_KEY_ARG:
			if( state->arg_num >= 1 ){
				argp_usage( state );
			}
			arguments->args[state->arg_num] = arg;
			break;
		case ARGP_KEY_END:
			if( state->arg_num < 1 ){
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

	int n, verbose;

	struct arguments arguments;
	arguments.verbose = 0;

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	sscanf(arguments.args[0],"%d",&n);
	verbose = arguments.verbose;

	clock_t begin, end;
	if( verbose ){
		begin = clock();
	}

	struct twin_prime nth_twin_prime = get_nth_twin_prime(n, verbose);

	if( verbose ){
		end = clock();
	  double seconds = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Took %f seconds.\n", seconds);
	}

	printf("The %dth twin prime is the pair (%lld, %lld).\n", n, nth_twin_prime.first, nth_twin_prime.second);

	return 0;
}
