from rflib import *
import bitstring
import re

d = RfCat()
d.setModeIDLE()
d.setFreq(315000000)
d.setMdmModulation(MOD_ASK_OOK)
d.setMdmDRate(4800)
d.setMaxPower()
d.lowball()
d.setModeIDLE()

capture = ""
while (1):
	try:
		y, z = d.RFrecv()
		capture = y.encode('hex')
		print capture

		payloads = re.split('0000*', capture)
		print payloads

		for payload in payloads:

			binary = bin(int(payload,16))[2:]
			formatted = bitstring.BitArray(bin=(binary)).tobytes()
			print binary
			print formatted

	except ChipconUsbTimeoutException:
		print "except"
		pass

	