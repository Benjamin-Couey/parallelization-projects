datafile = 'mandelbrot_set.csv'
set datafile separator ","

set view map

set title "Mandelbrot set"
set xlabel "Real component"
set ylabel "Imaginary component"
set cblabel "Limit"

splot datafile using 1:2:3 with image

pause mouse close
