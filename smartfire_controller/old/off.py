from rflib import *
from bitstring import *

fp = open('on.bit', 'r')
onbits = fp.readline().strip()
fp.close()
fp = open('off.bit', 'r')
offbits = fp.readline().strip()
fp.close()
on = BitArray(bin=onbits)
off = BitArray(bin=offbits)

print "On:  [{}] {}".format(len(onbits), onbits)
print "Off: [{}] {}".format(len(offbits), offbits)
print ("On: bin[{}]={}, hex[{}]={}".format(on.len, on.bin, on.len/4, on.hex))

d=RfCat()
d.setFreq(314973000)
d.setMdmModulation(MOD_ASK_OOK)
d.setMdmDRate(2400)

print "Starting"

d.setModeIDLE()
d.RFxmit(data=off.bytes, repeat=4)

print "TX Complete"
