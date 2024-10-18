twin_prime : seq_main.c par_main.c test_twin_prime.c par_test_twin_prime.c
		gcc seq_main.c -lm -o seq_twin_prime
		mpicc par_main.c -lm -o par_twin_prime
		gcc test_twin_prime.c -lm -lcunit -o test_twin_prime
		gcc par_test_twin_prime.c -lm -lcunit -o par_test_twin_prime
		./test_twin_prime
		./par_test_twin_prime

clean:
	rm seq_twin_prime par_twin_prime test_twin_prime par_test_twin_prime
