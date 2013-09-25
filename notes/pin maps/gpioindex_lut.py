import sys, re, os
from parseit import printlut

def doit2(x):
    with open(x) as f:
        lut = {}
        i = 0
        for line in f:
            gpionum = int(line)
            lut[gpionum] = i
            i += 1

        lutarr = []
        for i in range(128):
            lutarr.append(lut.get(i, 128))
        return lutarr
        
