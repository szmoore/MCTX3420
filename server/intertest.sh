rm intertest
make intertest
./intertest > test.dat

cmd="set title \"Interferometer Test\""
cmd="$cmd; set xlabel \"Time (s)\""
cmd="$cmd; set ylabel \"Phase (rad)\""
cmd="$cmd; plot \"test.dat\" u 1:3 t \"Specified\" w l, \"test.dat\" u 1:5 t \"Calculated\" w p"
gnuplot --persist -e "$cmd"

cmd="set title \"Interferometer Test\""
cmd="$cmd; set xlabel \"Time (s)\""
cmd="$cmd; set ylabel \"Delta Phase (rad)\""
cmd="$cmd; plot \"test.dat\" u 1:2 t \"Specified\" w l, \"test.dat\" u 1:4 t \"Measured\" w p"
gnuplot --persist -e "$cmd"
