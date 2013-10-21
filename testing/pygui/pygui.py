#!/usr/bin/python

"""
	Python has a different style of documentation which will break doxygen...
"""

import sys
import os
import matplotlib.pyplot as plt
import numpy
import requests
import datetime
import time

#TODO: Replace with URL of testing server
api_url = "https://192.168.1.10/api"

def log(message):
	sys.stderr.write("%s: %s : %s\n" % (sys.argv[0], str(datetime.datetime.now()), message))

def update_plot(plot, axes, data_x, data_y):
	"""
		Update data to plot, allegedly this is faster than just replotting everything
	"""
	plot.set_xdata(numpy.append(plot.get_xdata(), data_x))
	plot.set_ydata(numpy.append(plot.get_ydata(), data_y))
	axes.relim()
	axes.autoscale_view()
	plt.draw()	

def main(argv):
	if (len(argv) < 2):
		sys.stderr.write("Usage: %s sensor_id\n" % argv[0])
		sys.stderr.write("Identifying sensors...\n\n")
		r = requests.get(api_url + "?sensors", verify=False)		
		print r.text
		return 1	
	
	plt.ion()

	fig = plt.figure()
	axes = fig.add_subplot(111)
	#NOTE: Comma here is *not* a typo and is extremely important and some kind of mysterious python magical tuple thing
	#		Do not remove the comma or things will break. Horribly.
	plot, = axes.plot([],[])

	start_time = 0
	
	while True:
		params = { "id" : argv[1], "start_time" : "-1", "format" : "tsv"}
		try:
			r = requests.get(api_url + "/sensors", params=params, verify=False)
		except:
			log("Failed to make request for data");
			return 1

		if r.status_code != 200:
			log("Bad status code %d" % r.status_code)
			print r.text
			return 1
		
		log("Got data")

		data_x = []
		data_y = []		

		count = 0
		for line in r.text.split("\n"):
			count += 1

			
			point = map(float, line.split("\t"))

			if point[0] > start_time:
				data_x.append(point[0])
				data_y.append(point[1])
				start_time = point[0]

		if count > 0:
			update_plot(plot, axes, data_x, data_y)
			time.sleep(0.5)

	return 0

# ... This is how you make main work in python. With string comparisons.
if __name__ == "__main__":
	exit(main(sys.argv))


