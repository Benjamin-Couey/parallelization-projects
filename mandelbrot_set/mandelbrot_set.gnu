datafile = 'mandelbrot_set.csv'
set datafile separator ","

set view map

set title "Mandelbrot set"
set xlabel "Real component"
set ylabel "Imaginary component"
set cblabel "Limit"

set xrange [-2.0:1.0]
set yrange [-1.5:1.5]

stats datafile nooutput
X = STATS_columns
Y = STATS_records

scale_x(x)=(3*x/X)-2.0
scale_y(y)=(3*y/Y)-1.5

splot datafile matrix using (scale_x($1)):(scale_y($2)):3 with image

pause mouse close
