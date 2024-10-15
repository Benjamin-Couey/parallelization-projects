#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

#include <stdio.h>

#define BUFSIZE 128

void run_command( char * cmd, char * result ) {
	char buffer[BUFSIZE] = {0};
	FILE *fp;

	fp = popen(cmd, "r");
	CU_ASSERT(fp != NULL);
	while( fgets(buffer, BUFSIZE, fp) != NULL ){
		strcpy(result, buffer);
	}
	CU_ASSERT(!pclose(fp));
}

void test_par_against_seq(){
	char seq_result[BUFSIZE] = {0};
	char par_result[BUFSIZE] = {0};

	run_command( "./seq_twin_prime 1", seq_result );
	run_command( "mpirun -np 4 ./par_twin_prime 1 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	run_command( "./seq_twin_prime 10", seq_result );
	run_command( "mpirun -np 4 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	run_command( "./seq_twin_prime 100", seq_result );
	run_command( "mpirun -np 4 ./par_twin_prime 100 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
}

void test_num_processes_does_not_change(){
	char seq_result[BUFSIZE] = {0};
	char par_result[BUFSIZE] = {0};

	run_command( "./seq_twin_prime 10", seq_result );
	run_command( "mpirun -np 1 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	run_command( "./seq_twin_prime 10", seq_result );
	run_command( "mpirun -np 2 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	run_command( "./seq_twin_prime 10", seq_result );
	run_command( "mpirun -np 3 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
}

void test_batch_size_does_not_change(){
	char seq_result[BUFSIZE] = {0};
	char par_result[BUFSIZE] = {0};

	run_command( "./seq_twin_prime 10", seq_result );
	run_command( "mpirun -np 4 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	run_command( "./seq_twin_prime 10", seq_result );
	run_command( "mpirun -np 4 ./par_twin_prime 10 10", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	run_command( "./seq_twin_prime 10", seq_result );
	run_command( "mpirun -np 4 ./par_twin_prime 10 100", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
}

int main(){
	CU_initialize_registry();
	CU_pSuite suite = CU_add_suite("AddTestSuite", 0, 0);
	CU_add_test(suite, "test that par_main.c reports the same values as seq_main.c", test_par_against_seq);
	CU_add_test(suite, "test that par_main.c's output does not change for different numbers of processes", test_num_processes_does_not_change);
	CU_add_test(suite, "test that par_main.c's output does not change for different batch sizes", test_batch_size_does_not_change);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return 0;
}
