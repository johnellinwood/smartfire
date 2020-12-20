#!/usr/bin/python
"""
fireplace.py: Python implementation of the Sit Group's Proflame 2 remote fireplace controller.
"""

__version__ = "0.0.1"

import logging
import sys

#from past.translation import autotranslate

#autotranslate('rflib')
from rflib import RfCat, MOD_ASK_OOK
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
            self._radio = RfCat()
            self._radio.setFreq(314973000)
            self._radio.setMdmModulation(MOD_ASK_OOK)
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
        print('Setting pilot:{}, light:{}, thermostat:{}, power:{}, front:{}, fan:{}, aux:{}, flame:{}'.format(
            pilot, light, thermostat, power, front, fan, aux, flame
        ))
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
