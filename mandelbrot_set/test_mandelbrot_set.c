#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

#include "mandelbrot_set.c"

void test_in_mandelbrot_set(){
	double complex c = 0.2 + 0.4 * I;
	// Points we known to be in the set.
	CU_ASSERT(100 == in_mandelbrot_set(c, 100));
	c = 0.2 + -0.4 * I;
	CU_ASSERT(100 == in_mandelbrot_set(c, 100));
	c = -0.5 + 0.5 * I;
	CU_ASSERT(100 == in_mandelbrot_set(c, 100));
	c = -0.5 + 0.5 * I;
	CU_ASSERT(100 == in_mandelbrot_set(c, 100));
	c = -1.4 + 0.0 * I;
	CU_ASSERT(100 == in_mandelbrot_set(c, 100));
	// Not in the set: z1 = 1, z2 = 2, z3 = 3.
	c = 1.0 + 0.0 * I;
	CU_ASSERT(2 == in_mandelbrot_set(c, 100));
	// Not in the set: z1 = 1+1i, z2 = 1+3i.
	c = 1.0 + 1.0 * I;
	CU_ASSERT(1 == in_mandelbrot_set(c, 100));
	// Not in the set: z1 = 1-1i, z2 = 1-3i.
	c = 1.0 + -1.0 * I;
	CU_ASSERT(1 == in_mandelbrot_set(c, 100));
	// Not in the set: z1 = 0+2i, z2 = -4+2i.
	c = 0.0 + -2.0 * I;
	CU_ASSERT(1 == in_mandelbrot_set(c, 100));
	// Not in the set: z1 = 0+2i, z2 = -4+2i.
	c = 0.0 + 2.0 * I;
	CU_ASSERT(1 == in_mandelbrot_set(c, 100));
	// Not in the set: z1 = 0-2i, z2 = -4-2i.
	c = 0.0 + -2.0 * I;
	CU_ASSERT(1 == in_mandelbrot_set(c, 100));
}

void test_in_mandelbrot_set_greater_than_2(){
	// Should return 0 for any c with an absolute value greater than 2.
	double complex c = 2.1 + 0.0 * I;
	CU_ASSERT(0 == in_mandelbrot_set(c, 10));
	c = 0.0 + 2.1 * I;
	CU_ASSERT(0 == in_mandelbrot_set(c, 10));
	c = -2.1 + 0.0 * I;
	CU_ASSERT(0 == in_mandelbrot_set(c, 10));
	c = 0.0 + -2.1 * I;
	CU_ASSERT(0 == in_mandelbrot_set(c, 10));
}

void test_in_mandelbrot_set_varied_limit(){
	// Values in the Mandelbrot set should always hit the limit.
	double complex c = -1.0 + 0.0 * I;
	CU_ASSERT(10 == in_mandelbrot_set(c, 10));
	CU_ASSERT(100 == in_mandelbrot_set(c, 100));
	CU_ASSERT(1000 == in_mandelbrot_set(c, 1000));
	CU_ASSERT(10000 == in_mandelbrot_set(c, 10000));
	CU_ASSERT(100000 == in_mandelbrot_set(c, 100000));
}

int main(){
	CU_initialize_registry();
	CU_pSuite suite = CU_add_suite("AddTestSuite", 0, 0);
	CU_add_test(suite, "test in_mandelbrot_set() on known values", test_in_mandelbrot_set);
	CU_add_test(suite, "test in_mandelbrot_set() on |c| > 2", test_in_mandelbrot_set_greater_than_2);
	CU_add_test(suite, "test in_mandelbrot_set() for varied limits", test_in_mandelbrot_set_varied_limit);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return 0;
}
