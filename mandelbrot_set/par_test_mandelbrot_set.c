#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 128

#define SAME 1
#define DIFF 0

void compare_seq_to_par( int seq_iter, int seq_x, int seq_y, int par_iter, int par_x, int par_y, int same ){
	char buffer[BUFSIZE];
	snprintf(
		buffer,
		sizeof(buffer),
		"./seq_mandelbrot_set %d %d %d -o temp_seq_mandelbrot_set.csv",
		seq_iter,
		seq_x,
		seq_y
	);
	system( buffer );
	snprintf(
		buffer,
		sizeof(buffer),
		"mpirun -np 4 ./par_mandelbrot_set %d %d %d -o temp_par_mandelbrot_set.csv",
		par_iter,
		par_x,
		par_y
	);
	system( buffer );

	FILE *fp;

	fp = popen("diff temp_seq_mandelbrot_set.csv temp_par_mandelbrot_set.csv", "r");
	CU_ASSERT(fp != NULL);
	// diff will print nothing and cause fgets to return NULL if the files are the
	// same.
	CU_ASSERT( (fgets(buffer, BUFSIZE, fp) == NULL) == same );
	pclose(fp);

	system("rm temp_seq_mandelbrot_set.csv");
	system("rm temp_par_mandelbrot_set.csv");
}

void test_par_against_seq(){

	compare_seq_to_par( 10, 10, 10, 10, 10, 10, SAME );
	compare_seq_to_par( 100, 10, 10, 100, 10, 10, SAME );
	compare_seq_to_par( 10, 100, 100, 10, 100, 100, SAME );
	compare_seq_to_par( 100, 100, 100, 100, 100, 100, SAME );

	compare_seq_to_par( 10, 10, 10, 100, 10, 10, DIFF );
	compare_seq_to_par( 10, 10, 10, 10, 100, 10, DIFF );
	compare_seq_to_par( 10, 10, 10, 10, 10, 100, DIFF );
	compare_seq_to_par( 10, 1, 10, 9, 1, 10, DIFF );
}

void compare_num_processes( char * seq_file_name, int np ){
	char buffer[BUFSIZE];
	snprintf(
		buffer,
		sizeof(buffer),
		"mpirun -np %d ./par_mandelbrot_set 100 100 100 -o temp_par_mandelbrot_set.csv",
		np
	);
	system( buffer );

	FILE *fp;
	snprintf( buffer, sizeof(buffer), "diff %s temp_par_mandelbrot_set.csv", seq_file_name );
	fp = popen(buffer, "r");
	CU_ASSERT(fp != NULL);
	// diff will print nothing and cause fgets to return NULL if the files are the
	// same.
	CU_ASSERT( (fgets(buffer, BUFSIZE, fp) == NULL) );
	pclose(fp);

	system("rm temp_par_mandelbrot_set.csv");
}

void test_num_processes_does_not_change(){
	system("./seq_mandelbrot_set 100 100 100 -o temp_seq_mandelbrot_set.csv");
	compare_num_processes("temp_seq_mandelbrot_set.csv", 3);
	compare_num_processes("temp_seq_mandelbrot_set.csv", 2);
	compare_num_processes("temp_seq_mandelbrot_set.csv", 1);
	system("rm temp_seq_mandelbrot_set.csv");
}

void compare_high_density_ratio( char * seq_file_name, double h ){
	char buffer[BUFSIZE];
	snprintf(
		buffer,
		sizeof(buffer),
		"mpirun -np 4 ./par_mandelbrot_set 100 100 100 -o temp_par_mandelbrot_set.csv -h %f",
		h
	);
	system( buffer );

	FILE *fp;
	snprintf( buffer, sizeof(buffer), "diff %s temp_par_mandelbrot_set.csv", seq_file_name );
	fp = popen(buffer, "r");
	CU_ASSERT(fp != NULL);
	// diff will print nothing and cause fgets to return NULL if the files are the
	// same.
	CU_ASSERT( (fgets(buffer, BUFSIZE, fp) == NULL) );
	pclose(fp);

	system("rm temp_par_mandelbrot_set.csv");
}

void test_high_density_ratio_does_not_change(){
	system("./seq_mandelbrot_set 100 100 100 -o temp_seq_mandelbrot_set.csv");
	compare_high_density_ratio("temp_seq_mandelbrot_set.csv", 1.0);
	compare_high_density_ratio("temp_seq_mandelbrot_set.csv", 0.4);
	compare_high_density_ratio("temp_seq_mandelbrot_set.csv", 0.66);
	compare_high_density_ratio("temp_seq_mandelbrot_set.csv", 0.0);
	system("rm temp_seq_mandelbrot_set.csv");
}

int main(){
	CU_initialize_registry();
	CU_pSuite suite = CU_add_suite("AddTestSuite", 0, 0);
	CU_add_test(suite, "test that par_main.c reports the same values as seq_main.c", test_par_against_seq);
	CU_add_test(suite, "test that par_main.c's output does not change for different numbers of processes", test_num_processes_does_not_change);
	CU_add_test(suite, "test that par_main.c's output does not change for different high density ratios", test_high_density_ratio_does_not_change);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return 0;
}
