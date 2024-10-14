#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

#include <stdio.h>

#define BUFSIZE 128

void RunCommand( char * cmd, char * result ) {
	char buffer[BUFSIZE] = {0};
	FILE *fp;

	fp = popen(cmd, "r");
	CU_ASSERT(fp != NULL);
	while( fgets(buffer, BUFSIZE, fp) != NULL ){
		strcpy(result, buffer);
	}
	CU_ASSERT(!pclose(fp));
}

void TestParAgainstSeq(){
	char seq_result[BUFSIZE] = {0};
	char par_result[BUFSIZE] = {0};

	RunCommand( "./seq_twin_prime 1", seq_result );
	RunCommand( "mpirun -np 4 ./par_twin_prime 1 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	RunCommand( "./seq_twin_prime 10", seq_result );
	RunCommand( "mpirun -np 4 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	RunCommand( "./seq_twin_prime 100", seq_result );
	RunCommand( "mpirun -np 4 ./par_twin_prime 100 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
}

void TestNumProcessesDoesNotChange(){
	char seq_result[BUFSIZE] = {0};
	char par_result[BUFSIZE] = {0};

	RunCommand( "./seq_twin_prime 10", seq_result );
	RunCommand( "mpirun -np 1 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	RunCommand( "./seq_twin_prime 10", seq_result );
	RunCommand( "mpirun -np 2 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	RunCommand( "./seq_twin_prime 10", seq_result );
	RunCommand( "mpirun -np 3 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
}

void TestBatchSizeDoesNotChange(){
	char seq_result[BUFSIZE] = {0};
	char par_result[BUFSIZE] = {0};

	RunCommand( "./seq_twin_prime 10", seq_result );
	RunCommand( "mpirun -np 4 ./par_twin_prime 10 1", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	RunCommand( "./seq_twin_prime 10", seq_result );
	RunCommand( "mpirun -np 4 ./par_twin_prime 10 10", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
	RunCommand( "./seq_twin_prime 10", seq_result );
	RunCommand( "mpirun -np 4 ./par_twin_prime 10 100", par_result );
	CU_ASSERT(strcmp(seq_result, par_result) == 0);
}

int main(){
	CU_initialize_registry();
	CU_pSuite suite = CU_add_suite("AddTestSuite", 0, 0);
	CU_add_test(suite, "test that par_main.c reports the same values as seq_main.c", TestParAgainstSeq);
	CU_add_test(suite, "test that par_main.c's output does not change for different numbers of processes", TestNumProcessesDoesNotChange);
	CU_add_test(suite, "test that par_main.c's output does not change for different batch sizes", TestBatchSizeDoesNotChange);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return 0;
}
