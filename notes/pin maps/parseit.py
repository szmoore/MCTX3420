import sys, re, os
#lut size of 93 (46 pins/header; 2 headers; padding for 1-indexed)

def doit(x):
    '''generate the lut from the csv'''
    lut = {}
    reverselut = {}
    
    with open(x) as f:
        for line in f:
            m = re.search("P(\d)_(\d+),(\d+)", line)
            header = int(m.group(1))
            pin = int(m.group(2))
            gpionum = int(m.group(3))

            if header==8:
                header = 0
            else:
                header = 1

            lut[header*46+pin] = gpionum
            reverselut[gpionum] = header*46+pin
    lutarr = []
    reverselutarr =[]
    
    for i in range(0, 93):
        lutarr.append(lut.get(i, 0))

    for i in range(0, 128):
        reverselutarr.append(reverselut.get(i, 0))
        
    return (lutarr, reverselutarr)

def printlut(lut, name="g_gpio_lut"):
    '''print the lut for C'''
    rowsize = 14
    print("const unsigned char %s[%d] = {" % (name, len(lut)))
    low = 0
    high = rowsize
    for i in range(0, len(lut), rowsize):
        print("\t", end="")
        print(*("%3d" % g for g in lut[low:high]), sep=', ', end="")
        low = high
        high += rowsize
        if low < len(lut):
            print(",")
        else:
            print("")
    print("}")


        
