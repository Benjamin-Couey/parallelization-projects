twin_prime : seq_main.c seq_test_twin_prime.c
		gcc seq_main.c -lm -o seq_twin_prime
		gcc seq_test_twin_prime.c -lm -lcunit -o seq_test_twin_prime
		./seq_test_twin_prime

clean:
	rm seq_twin_prime seq_test_twin_prime
