mandelbrot_set : seq_main.c par_main.c test_mandelbrot_set.c
		gcc seq_main.c -lm -o seq_mandelbrot_set
		mpicc par_main.c -lm -o par_mandelbrot_set
		gcc test_mandelbrot_set.c -lm -lcunit -o test_mandelbrot_set
		./test_mandelbrot_set

clean:
	rm seq_mandelbrot_set par_mandelbrot_set test_mandelbrot_set
