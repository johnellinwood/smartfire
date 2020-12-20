from bitstring import *

fp = open('on.bit', 'r')
onbits = fp.readline().strip()
fp.close()
fp = open('off.bit', 'r')
offbits = fp.readline().strip()
fp.close()

print "On:  [{}] {}".format(len(onbits), onbits)
print "Off: [{}] {}".format(len(offbits), offbits)

on = BitArray(bin=onbits)

print ("On: bin[{}]={}, hex[{}]={}".format(on.len, on.bin, on.len/4, on.hex))
