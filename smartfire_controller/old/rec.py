from rflib import *
d=RfCat()
d.setFreq(314958300)
d.setMdmModulation(MOD_ASK_OOK)
d.setMdmDRate(2400)
print "Starting"
d.RFxmit("\xe5\x96\x69\xb9\xaa\x65\xae\x55\x59\x6b\xa5\x95\x9a\xe5\x56\x95\xb9\x96\x55\x6e\xa5\x56\x68\x00\x00"*5)
print "TX Complete"

