#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

#include "seq_twin_prime.c"

void test_is_prime_one_to_ten(){
	CU_ASSERT(0 == is_prime(1));
	CU_ASSERT(1 == is_prime(2));
	CU_ASSERT(1 == is_prime(3));
	CU_ASSERT(0 == is_prime(4));
	CU_ASSERT(1 == is_prime(5));
	CU_ASSERT(0 == is_prime(6));
	CU_ASSERT(1 == is_prime(7));
	CU_ASSERT(0 == is_prime(8));
	CU_ASSERT(0 == is_prime(9));
	CU_ASSERT(0 == is_prime(10));
}

void test_get_nth_twin_prime_first_ten(){
	struct twin_prime nth_twin_prime;
	nth_twin_prime = get_nth_twin_prime(1, 0);
	CU_ASSERT(nth_twin_prime.first == 3);
	CU_ASSERT(nth_twin_prime.second == 5);
	nth_twin_prime = get_nth_twin_prime(2, 0);
	CU_ASSERT(nth_twin_prime.first == 5);
	CU_ASSERT(nth_twin_prime.second == 7);
	nth_twin_prime = get_nth_twin_prime(3, 0);
	CU_ASSERT(nth_twin_prime.first == 11);
	CU_ASSERT(nth_twin_prime.second == 13);
	nth_twin_prime = get_nth_twin_prime(4, 0);
	CU_ASSERT(nth_twin_prime.first == 17);
	CU_ASSERT(nth_twin_prime.second == 19);
	nth_twin_prime = get_nth_twin_prime(5, 0);
	CU_ASSERT(nth_twin_prime.first == 29);
	CU_ASSERT(nth_twin_prime.second == 31);
	nth_twin_prime = get_nth_twin_prime(6, 0);
	CU_ASSERT(nth_twin_prime.first == 41);
	CU_ASSERT(nth_twin_prime.second == 43);
	nth_twin_prime = get_nth_twin_prime(7, 0);
	CU_ASSERT(nth_twin_prime.first == 59);
	CU_ASSERT(nth_twin_prime.second == 61);
	nth_twin_prime = get_nth_twin_prime(8, 0);
	CU_ASSERT(nth_twin_prime.first == 71);
	CU_ASSERT(nth_twin_prime.second == 73);
	nth_twin_prime = get_nth_twin_prime(9, 0);
	CU_ASSERT(nth_twin_prime.first == 101);
	CU_ASSERT(nth_twin_prime.second == 103);
	nth_twin_prime = get_nth_twin_prime(10, 0);
	CU_ASSERT(nth_twin_prime.first == 107);
	CU_ASSERT(nth_twin_prime.second == 109);
}

int main(){
	CU_initialize_registry();
	CU_pSuite suite = CU_add_suite("AddTestSuite", 0, 0);
	CU_add_test(suite, "test of is_prime() on 1 through 10", test_is_prime_one_to_ten);
	CU_add_test(suite, "test of get_nth_twin_prime() for n 1 through 10", test_get_nth_twin_prime_first_ten);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return 0;
}
