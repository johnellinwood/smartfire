#!/usr/bin/python
"""
fire.py: Python implementation of the Sit Group's Proflame 2 remote fireplace controller.

==========
Overview
==========
The Proflame 2 control system is used in most modern natural gas and propane indoor residential fireplaces, as defined
by FCC ID T99058402300. It uses radio frequency transmission to remotely control the fireplace by way of a receiver
inside the fireplace. This application provides a programmatic radio interface to these devices. An RfCat compatible
transmitter, like the Yardstick, is required for transmission. The reference implementation for this software works on
a Raspberry Pi 3 B+ running Raspbian and a Yardstick radio dongle. This controller is able to remotely control the
device's main power, auxiliary power supply, pilot light, thermostat setting, 6-level fan, 6-level light, and 6-level
main flame.

==========
RF Modulation and Encoding Protocols
==========
The Proflame 2 system uses a proprietary packet structure and encoding. Commands are sent to the receiver in a single
burst for each command. The fireplace responds to a successful command by echoing it back exactly.

----------
Modulation
----------
The command bursts are transmitted at 314,973 KHz using the On-Off Keying (OOK) variant of Amplitude Shift Keying (ASK)
which is a type of Amplitude Modulation (AM), and are sent at a transmission rate of 2400 baud. Inside the command burst
are the command packets. The same command packet is transmitted 5 times in each command burst, and the packet
repetitions are separated by 12 low amplitude bits (zeros).

----------
Encoding
----------
The command packets are encoded with a variant of Thomas Manchester encoding. In this variant, 0 is represented by 01,
a 1 by 10, zero padding (Z) by 00, and synchronization words (S) as 11. The encoded command packet is 182 bits, and the
decoded packet is 91 bits. If each data part of the packet were manchester encoded separately, and then separated with
the bit pattern 11, then the encoding of individual command packets could be considered standard Thomas Manchester
encoding.

----------
Packet Structure
----------
The decoded packet is made up of 7 words, each 13 bits. A command word starts with the synchronization symbol,
followed by a 1 as a guard bit, then 8 bits of data, a padding bit, a parity bit, and finally a 1 as an end guard bit.
The padding bit is 1 for the first word and 0 for all other words. The parity bit is calculated over the data bits and
the padding bit, and is 0 if there are an even number of ones and 1 if there are an odd number of ones. The first three
data words are a unique serial number. The next two data words are the actual fireplace command words. The last two
words are error detection and possibly obfuscation words.

^^^^^^^^^^
Serial Number
^^^^^^^^^^
The serial number will need to either be cloned from an existing remote, or randomly generated and paired directly with
the fireplace. One valid serial number is the following three data words + padding: '0b001001011', '0b011110100',
'0b000000100'.

^^^^^^^^^^
Command Words
^^^^^^^^^^
The first command word is made up of 1 bit for the pilot light, 3 bit-endian bits for the light level, 2 zeros, 1 bit
for the thermostat setting and 1 bit for the unit's main power. The second command word is made up of 1 bit for the
front flame / flame split, 3 big-endian bits for the fan blower level, 1 bit for the auxiliary power outlet, and 3
bit-endian bits for the main flame level. For each single bit variable in the command, a 0 represents off and a 1
represents on. In the case of the pilot, that means 1 is CPI and 0 is IPI. For the thermostat, that means 1 is either
on or the smart thermostat. The 3 bit numbers are the level between 0 and 6, where 0 is off and 6 is high. 7 is not
an allowed value.

^^^^^^^^^^
Error Detection Words
^^^^^^^^^^
The first error detection word is calculated from the first command word. It is made up of a big-endian high nibble,
which is the first nibble of the error detection's data transmitted, and a bit-endian low nibble which is the second
nibble transmitted. It is calculated from the nibbles of the first command word as follows:
  Let:
    A represent the high nibble of the command word
    B represent the low nibble of the command word
    C and D represent constants
    X represent the high nibble of the error detection word
    Z represent the low nibble of the error detection word
  Then:
    X = ( C ^ ( x << 1 ) ^  x  ^ ( y << 1 ) ) & 0xF
    Y = ( D ^ x ^ y )
For the first error detection word, C=0b1101 and D=0. The second error detection word is calculated the same way, but
using the second command word as its input, and with values of C=0 and D=0b0111. The constants C and D are possibly
related to the serial number or could otherwise be unique for each device.

^^^^^^^^^^
Packet Diagram
^^^^^^^^^^
        Bit
        1   2   3   4   5   6   7   8   9  10  11  12  13
Word |----------------------------------------------------|
   1 | S | 1 |      Serial Number             | 1 |Par| 1 | Serial Word 1
     |----------------------------------------------------|
   2 | S | 1 |      Serial Number             | 0 |Par| 1 | Serial Word 2
     |----------------------------------------------------|
   3 | S | 1 |      Serial Number             | 0 |Par| 1 | Serial Word 3
     |----------------------------------------------------|
   4 | S | 1 |CPI|    Light   | 0 | 0 |Th.|Pwr| 0 |Par| 1 | Command Word 1
     |----------------------------------------------------|
   5 | S | 1 |Fnt|     Fan    |Aux|   Flame   | 0 |Par| 1 | Command Word 2
     |----------------------------------------------------|
   6 | S | 1 |      Ecc       |      ECC      | 0 |Par| 1 | Error Detection Word 1
     |----------------------------------------------------|
   7 | S | 1 |      Ecc       |      ECC      | 0 |Par| 1 | Error Detection Word 2
     |----------------------------------------------------|

==========
Examples
==========

----------
Packet Examples
----------
AM Demodulated Bit String:
  * 1110010110010110011010011011100110101010011001011010111001010101010110010110101110011001100101011001101011101010100101011001010110111001010101011001010110101110011010011001101001101

Decoded Command Packet:
  * S100100101101S101111010011S100000010011S101010001011S111100010001S100000100011S101101011011

Parsed Values:
  * Serial 1:  S 1001001011 0 1
  * Serial 2:  S 1011110100 1 1
  * Serial 3:  S 1000000100 1 1
  * Command 1: S 1010100010 1 1
  * Command 2: S 1111000100 0 1
  * Error 1:   S 1000001000 1 1
  * Error 2:   S 1011010110 1 1

Command Values:
  * Pilot: Off / IPI
  * Light: 5
  * Thermostat: On
  * Power: Off
  * Front: On
  * Fan: 6
  * Aux: Off
  * Flame: 2

----------
Command Examples
----------
  * fire.set(power=False) # Turn off
  * fire.set(power=True, flame=1, light=1, fan=0, front=False) # My favorite low mode
  * fire.set(power=True, flame=6, light=6, fan=6, front=True) # Highest setting
"""

__version__ = "0.0.1"

import logging
import os
import sys
import time

import rflib
from bitstring import Bits, BitArray

# Serial number words, including padding bit at the end
DEFAULT_SERIAL = ['001001011', '011110100', '000000100']


class Fireplace(object):
    """Model for the fireplace state and controls"""

    def __init__(self, serial=None):
        self._radio = None
        self._serial = DEFAULT_SERIAL if serial is None else serial
        self._pilot = True
        self._light = 0
        self._thermostat = False
        self._power = False
        self._front = False
        self._fan = 0
        self._aux = False
        self._flame = 0

    @property
    def radio(self):
        if self._radio is None:
            # Single radio configuration. The USB interface gets confused if this used multiple times.
            self._radio = rflib.RfCat()
            self._radio.setFreq(314973000)
            self._radio.setMdmModulation(rflib.MOD_ASK_OOK)
            self._radio.setMdmDRate(2400)
        return self._radio

    @property
    def serial(self):
        return self._serial

    @property
    def pilot(self):
        return self._pilot

    @pilot.setter
    def pilot(self, value):
        self.set(pilot=value)

    @property
    def light(self):
        return self._light

    @light.setter
    def light(self, value):
        self.set(light=value)

    @property
    def thermostat(self):
        return self._thermostat

    @thermostat.setter
    def thermostat(self, value):
        self.set(thermostat=value)

    @property
    def power(self):
        return self._power

    @power.setter
    def power(self, value):
        self.set(power=value)

    @property
    def front(self):
        return self._front

    @front.setter
    def front(self, value):
        self.set(front=value)

    @property
    def fan(self):
        return self._fan

    @fan.setter
    def fan(self, value):
        self.set(fan=value)

    @property
    def aux(self):
        return self._aux

    @aux.setter
    def aux(self, value):
        self.set(aux=value)

    @property
    def flame(self):
        return self._flame

    @flame.setter
    def flame(self, value):
        self.set(flame=value)

    @property
    def state(self):
        return {'serial': self.serial, 'pilot': self.pilot, 'light': self.light, 'therostat': self.thermostat,
                'power': self.power,
                'front': self.front, 'fan': self.fan, 'aux': self.aux, 'flame': self.flame}

    @state.setter
    def state(self, values):
        set(**values)

    def set(self, serial=None, pilot=None, light=None, thermostat=None, power=None, front=None, fan=None, aux=None,
            flame=None):
        if pilot is not None:
            # set pilot
            self._pilot = pilot
        if light is not None:
            # set light
            if (light < 0) or (light > 6):
                raise ValueError("Light value must be between 0 and 6 inclusive")
            self._light = light
        if thermostat is not None:
            # set thermostat
            self._thermostat = thermostat
        if power is not None:
            # set power
            self._power = power
        if front is not None:
            # set front
            self._front = front
        if fan is not None:
            # set fan
            if (fan < 0) or (fan > 6):
                raise ValueError("Fan value must be between 0 and 6 inclusive")
            self._fan = fan
        if aux is not None:
            # set aux
            self._aux = aux
        if flame is not None:
            # set flame
            if (flame < 0) or (flame > 6):
                raise ValueError("Flame value must be between 0 and 6 inclusive")
            self._flame = flame

        packet = self.build_packet()
        self.send_packet(packet)

    def build_packet(self):
        """Build the complete encoded packet ready for transmission and return it as a BitArray"""
        packet_words = []

        # stat with the 3 word serial number
        packet_words.extend([Bits(bin=s) for s in self.serial])

        # build the first command word
        cmd1 = BitArray()
        cmd1.append('0b1' if self.pilot else '0b0')  # 1 pilot bit
        cmd1.append(Bits(uint=self.light, length=3))  # 3 light bits
        cmd1.append('0b00')  # 2 zero bits
        cmd1.append('0b1' if self.thermostat else '0b0')  # 1 thermostat bit
        cmd1.append('0b1' if self.power else '0b0')  # 1 power bit
        cmd1.append('0x0')  # 1 zero padding bit
        packet_words.append(cmd1)

        # build the second command word
        cmd2 = BitArray()
        cmd2.append('0b1' if self.front else '0b0')  # 1 pilot bit
        cmd2.append(Bits(uint=self.fan, length=3))  # 3 light bits
        cmd2.append('0b1' if self.aux else '0b0')  # 1 pilot bit
        cmd2.append(Bits(uint=self.flame, length=3))  # 3 light bits
        cmd2.append('0x0')  # 1 zero padding bit
        packet_words.append(cmd2)

        # calculate the first ecc word
        ecc1 = BitArray()
        ecc1_high = (0xD ^ cmd1[0:4].uint ^ (cmd1[0:4].uint << 1) ^ (cmd1[4:8].uint << 1)) & 0xF
        ecc1_low = cmd1[0:4].uint ^ cmd1[4:8].uint
        ecc1.append(Bits(uint=ecc1_high, length=4))  # 4 high ecc bits
        ecc1.append(Bits(uint=ecc1_low, length=4))  # 4 low ecc bits
        ecc1.append('0x0')  # 1 zero padding bit
        packet_words.append(ecc1)

        # calculate the second ecc word
        ecc2 = BitArray()
        ecc2_high = (cmd2[0:4].uint ^ (cmd2[0:4].uint << 1) ^ (cmd2[4:8].uint << 1)) & 0xF
        ecc2_low = cmd2[0:4].uint ^ cmd2[4:8].uint ^ 0x7
        ecc2.append(Bits(uint=ecc2_high, length=4))  # 4 high ecc bits
        ecc2.append(Bits(uint=ecc2_low, length=4))  # 4 low ecc bits
        ecc2.append('0x0')  # 1 zero padding bit
        packet_words.append(ecc2)

        # convert the packet array to a bit string for encoding
        packet_string = ''
        for word in packet_words:
            packet_string += 'S'  # sync symbol
            packet_string += '1'  # start guard bit
            packet_string += word[0:9].bin  # data
            parity = word.count('0x1') % 2  # calculate parity on all 9 bits
            packet_string += Bits(uint=parity, length=1).bin  # parity bit
            packet_string += '1'  # end guard bit 1
        packet_string += 'Z' * 9  # zero padding at the end, required for burst separation

        logging.debug('packet string:', packet_string)
        logging.debug('packet string length:', len(packet_string))
        for p in packet_string.split('S'):
            if len(p) == 0:
                continue
            logging.debug('S{} {} {} {}'.format(p[0:1], p[1:5], p[5:9], p[9:12]))

        # encode the packet

        # extended thomas manchester codes with sync and zero codes
        manchester_codes = {'S': '11', '0': '01', '1': '10', 'Z': '00'}
        packet_array = [manchester_codes[b] for b in packet_string]
        packet = BitArray()
        for b in packet_array:
            packet.append(Bits(bin=b))

        # return the result
        logging.debug('manchester encoded packet:', packet.bin)
        logging.debug('length:', len(packet.bin))
        return packet

    def send_packet(self, packet):
        """Transmit the packet over the radio 5 times"""
        logging.info('Transmitting {}'.format(packet.hex))
        self.radio.setModeIDLE()
        self.radio.RFxmit(data=packet.bytes, repeat=4)  # protocol requires 5 transmissions, which is what repeat=4 does


if __name__ == "__main__":
    logging.info('main')
    fire = Fireplace()
    # fire.set(power=True, flame=1)
    # fire.set(power=False)
    cmd = 'fire.set(' + ','.join(sys.argv[1:]) + ')'
    print(cmd)
    exec(cmd)
