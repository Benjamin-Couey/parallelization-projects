mandelbrot_set : seq_main.c test_mandelbrot_set.c
		gcc seq_main.c -lm -o mandelbrot_set
		gcc test_mandelbrot_set.c -lm -lcunit -o test_mandelbrot_set
		./test_mandelbrot_set

clean:
	rm mandelbrot_set test_mandelbrot_set
