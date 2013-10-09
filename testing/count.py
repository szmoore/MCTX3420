#!/usr/bin/python -u

import sys
import os
import time
import datetime

start = datetime.datetime.now()
while True:
	timed = map(float, str(datetime.datetime.now() - start).split(":"))
	count = timed[0]*60.0*60.0+timed[1]*60.0+timed[2]
	print(str(count))
	

