#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

#include "seq_twin_prime.c"

void TestIsPrimeOneToTen(){
	CU_ASSERT(0 == IsPrime(1));
	CU_ASSERT(1 == IsPrime(2));
	CU_ASSERT(1 == IsPrime(3));
	CU_ASSERT(0 == IsPrime(4));
	CU_ASSERT(1 == IsPrime(5));
	CU_ASSERT(0 == IsPrime(6));
	CU_ASSERT(1 == IsPrime(7));
	CU_ASSERT(0 == IsPrime(8));
	CU_ASSERT(0 == IsPrime(9));
	CU_ASSERT(0 == IsPrime(10));
}

void TestGetNthTwinPrimeFirstTen(){
	struct TwinPrime nthTwinPrime;
	nthTwinPrime = GetNthTwinPrime(1, 0);
	CU_ASSERT(nthTwinPrime.first == 3);
	CU_ASSERT(nthTwinPrime.second == 5);
	nthTwinPrime = GetNthTwinPrime(2, 0);
	CU_ASSERT(nthTwinPrime.first == 5);
	CU_ASSERT(nthTwinPrime.second == 7);
	nthTwinPrime = GetNthTwinPrime(3, 0);
	CU_ASSERT(nthTwinPrime.first == 11);
	CU_ASSERT(nthTwinPrime.second == 13);
	nthTwinPrime = GetNthTwinPrime(4, 0);
	CU_ASSERT(nthTwinPrime.first == 17);
	CU_ASSERT(nthTwinPrime.second == 19);
	nthTwinPrime = GetNthTwinPrime(5, 0);
	CU_ASSERT(nthTwinPrime.first == 29);
	CU_ASSERT(nthTwinPrime.second == 31);
	nthTwinPrime = GetNthTwinPrime(6, 0);
	CU_ASSERT(nthTwinPrime.first == 41);
	CU_ASSERT(nthTwinPrime.second == 43);
	nthTwinPrime = GetNthTwinPrime(7, 0);
	CU_ASSERT(nthTwinPrime.first == 59);
	CU_ASSERT(nthTwinPrime.second == 61);
	nthTwinPrime = GetNthTwinPrime(8, 0);
	CU_ASSERT(nthTwinPrime.first == 71);
	CU_ASSERT(nthTwinPrime.second == 73);
	nthTwinPrime = GetNthTwinPrime(9, 0);
	CU_ASSERT(nthTwinPrime.first == 101);
	CU_ASSERT(nthTwinPrime.second == 103);
	nthTwinPrime = GetNthTwinPrime(10, 0);
	CU_ASSERT(nthTwinPrime.first == 107);
	CU_ASSERT(nthTwinPrime.second == 109);
}

int main(){
	CU_initialize_registry();
	CU_pSuite suite = CU_add_suite("AddTestSuite", 0, 0);
	CU_add_test(suite, "test of IsPrime() on 1 through 10", TestIsPrimeOneToTen);
	CU_add_test(suite, "test of GetNthTwinPrime() for n 1 through 10", TestGetNthTwinPrimeFirstTen);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return 0;
}
