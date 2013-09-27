import sys, re, os
from parseit import printlut

def doit2(x):
    with open(x) as f:
        lut = {}
        rlut = {}
        i = 0
        for line in f:
            gpionum = int(line)
            lut[gpionum] = i
            rlut[i] = gpionum
            i += 1

        lutarr = []
        reverse = []
        for i in range(118): #Max safe gpio is 117
            lutarr.append(lut.get(i, 128))
        for i in range(len(rlut)):
            reverse.append(rlut[i])
        return (lutarr, reverse)
        
