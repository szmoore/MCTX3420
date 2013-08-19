#!/usr/bin/python

import sys
import os
import Gnuplot
import subprocess
import numpy

methods = ["sqlite", "csv", "binfile"]
data = {}
averages = 100
g = Gnuplot.Gnuplot()


for program in methods:
	data.update({program : []})
	
	numpoints = 1
	while (numpoints < 10000):
		bufsiz = 1
		while (bufsiz < 2):
			
			run = ["./"+program, str(bufsiz), str(numpoints)]
			times = []
			for i in xrange(averages):
				p = subprocess.Popen(run, stdout=subprocess.PIPE, stderr=subprocess.PIPE)	
				times.append(float(p.stdout.readline().strip(" \r\n")))

			times = numpy.array(times)
			data[program].append([numpoints,bufsiz,numpy.mean(times), numpy.std(times)])

			bufsiz *= 10
		numpoints += 100

	g("set xlabel \"Data Points\"")
	g("set ylabel \"Total Time (s)\"")
	g("set title \"Time to Store Data Points\"")
	g.replot(Gnuplot.Data(data[program], title=program, with_="lp",using="1:3"))

print "Done!"
sys.stdin.readline()
		

	
