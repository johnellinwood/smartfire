# SmartFire: The Smart Home Fireplace Controller

## Overview
The Sit Group's Proflame 2 control system is used in most modern natural gas and propane indoor residential fireplaces,
like Jotul, Mendota, Regency, Empire, Lennox, and others. It uses 
radio frequency transmission to remotely control the fireplace by way of a handheld transmitter and a fixed receiver 
inside the fireplace. This application provides a software implementation of the encoding necessary to transmit
compatible commands using generic radio interfaces. A python API, command line interface, and REST server
are included for local control over the fireplace. Additionally, an Amazon Lambda project is included for Alexa
SmartHome control of the fireplace. This controller is able to remotely control the device's main power, auxiliary 
power supply, pilot light, thermostat setting, 6-level fan, 6-level light, and 6-level main flame. The project requires
a linux computer and an RF transmitter. Compatible RF transmitters are ones with TI CC1101 chipsets, like the YardStick One. 
Supported computer devices are the Raspbery Pi, with future planned
support for Particle Photon. Thermostat setting is not yet supported, as it requires additional hardware. 

## Alexa Commands

All the fireplace components respond to on/off:

* "Alexa, turn <on|off> the <fireplace|front flame|fan|light|auxiliary|pilot>"

The 6-level components respond to level commands:

* "Alexa, set the <fireplace|fan|light> to <0-100> percent"
* "Alexa, set the <fireplace|fan|light> to <low|high>"

You may want to rename the fireplace Alexa devices on your network to avoid conflicts. Consider changing the "Light" to 
"Fireplace Light", for example.

## Getting Started

### Download
* Clone this git repository
* Dependencies: github atlas0fd00m/rfcat, pyside/pyside-setup

### Installation
* Install fireplace.service in systemd on local raspberry pi near fireplace
  * Start with 'systemctl start fireplace'
  * Fireplace server will start listening with REST interface on port 5000
  * Configure network to forward requests. e.g., port 1234 forwards to pi port 5000
* AWS Setup
  * Setup amazon lambda function with the 'alexa_smart_home' code
  * Set a lambda environment variable called 'BASE_URL' with value http://<host>:<nat-port>, e.g. 'http://173.67.100.100:1234'
  * Setup an 'alex smarthome trigger' in lambda to call the fireplace function
  * Now 'alexa, turn on the fireplace' should call the rest call on the pi and send the rf to turn on the fireplace

### Hardware Setup
* From amazon, "NooElec Aluminum Enclosure & EMI Shield, Black, for Great Scott Gadgets Ubertooth One & Yard Stick One"
* From amazon, "NooElec Yard Stick One USB Transceiver"
* Install and connect the YardStick to the pi. 
* Setup RFLib and RFCat and make sure it can send commands

### API Usage
```
from smartfire_controller.fireplace import Fireplace
fp = Fireplace()
# Turn the fireplace on
fp.power = True
```

### Compatibility
The reference implementation for this software works on a Raspberry Pi 3 B+ running Raspbian and a Yardstick radio dongle.
Future plans are to include support for the Particle Photon controlling a TI CC1101 radio chip. 

## Implementation

### Specification
There is no publically available specification for the Proflame 2 protocol. Its usage is described most thoroughly in [FCC ID T99058402300](https://fcc.report/FCC-ID/T99058402300)

### RF Modulation and Encoding Protocols
The Proflame 2 system uses a proprietary packet structure and encoding. Commands are sent to the receiver in a single
burst for each command. The fireplace responds to a successful command by echoing it back exactly.

#### Modulation
The command bursts are transmitted at 314,973 KHz using the On-Off Keying (OOK) variant of Amplitude Shift Keying (ASK)
which is a type of Amplitude Modulation (AM), and are sent at a transmission rate of 2400 baud. Inside the command burst
are the command packets. The same command packet is transmitted 5 times in each command burst, and the packet
repetitions are separated by 12 low amplitude bits (zeros).

#### Encoding
The command packets are encoded with a variant of Thomas Manchester encoding. In this variant, 0 is represented by 01,
a 1 by 10, zero padding (Z) by 00, and synchronization words (S) as 11. The encoded command packet is 182 bits, and the
decoded packet is 91 bits. If each data part of the packet were manchester encoded separately, and then separated with
the bit pattern 11, then the encoding of individual command packets could be considered standard Thomas Manchester
encoding.

#### Packet Structure
The decoded packet is made up of 7 words, each 13 bits. The first 3 words are a unique identifier for the transmitter,
possibly related to the serial number. The next two words are command words controlling the state of the fireplace.
The last 2 words are error detection words, possibly used for security. Each word starts with a synchronization symbol,
followed by a 1 as a guard bit, then 8 bits of data, a padding bit, a parity bit, and finally a 1 as an end guard bit.
The padding bit is 1 for the first word and 0 for all other words. The parity bit is calculated over the data bits and
the padding bit, and is 0 if there are an even number of ones and 1 if there are an odd number of ones.

Packet Words:
* Serial 1
* Serial 2
* Serial 3
* Command 1
* Command 2
* Error Detection 1
* Error Detection 2 

Word Parts:
* 1 synchronization Symbol with a value of 'S' decoded or '11' raw
* 1 start guard bit with a value of '1'
* 8 bits of data
* 1 padding bit
* 1 parity bit calculated over the data part
* 1 end guard bit with a value of '1'

#### Serial Number
The serial number will need to either be cloned from an existing remote, or randomly generated and paired directly with
the fireplace. One valid serial number is the the data + padding portions of the following three words: '0b001001011', 
'0b011110100', '0b000000100'.

#### Command Words
The data portion of the first command word is made up of 1 bit for the pilot light, 3 big-endian bits for the light 
level, 2 zeros, 1 bit for the thermostat setting and 1 bit for the unit's main power. The second command word is made 
up of 1 bit for the front flame / flame split, 3 big-endian bits for the fan blower level, 1 bit for the auxiliary power
outlet, and 3 big-endian bits for the main flame level. For each single bit variable in the command, a 0 represents off 
and a 1 represents on. In the case of the pilot, that means 1 is CPI and 0 is IPI. For the thermostat, that means 1 is 
either on or the smart thermostat. The 3 bit numbers are the level between 0 and 6, where 0 is off and 6 is high. 7 is 
not an allowed value.

#### Error Detection Words
The first error detection word is calculated from the first command word, and the second error detection word is 
calculated from the second command word. For the purposes of calculating the error detection words, both the command 
words and the error detection words can be viewed as two 4-bit nibbles. Each 4-bit nibble of an error detection word
is calculated using a function based on the two 4-bit nibbles of its corresponding command word.  

They are calculated as follows:

Let:
* A represent the high nibble of the command word
* B represent the low nibble of the command word
* C and D represent constants
* X represent the high nibble of the error detection word
* Y represent the low nibble of the error detection word

Then:
* X = ( C ^ ( A << 1 ) ^  A  ^ ( B << 1 ) ) & 0xF
* Y = ( D ^ A ^ B )

For the first error detection word, C=0b1101 and D=0. The second error detection word is calculated the same way, but
using the second command word as its input, and with values of C=0 and D=0b0111. The constants C and D are possibly
related to the serial number or could otherwise be unique for each device.

#### Packet Diagram
```
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
```

#### Packet Examples
Here is an example of a received packet and what command it represents.

AM Demodulated Bit String:
* 1110010110010110011010011011100110101010011001011010111001010101010110010110101110011001100101011001101011101010100101011001010110111001010101011001010110101110011010011001101001101

Decoded Command Packet:
* S100100101101S101111010011S100000010011S101010001011S111100010001S100000100011S101101011011

Parsed Values:
* Serial 1:  S1 0010 0101 1 0 1
* Serial 2:  S1 0111 1010 0 1 1
* Serial 3:  S1 0000 0010 0 1 1
* Command 1: S1 0101 0001 0 1 1
* Command 2: S1 1110 0010 0 0 1
* Error 1:   S1 0000 0100 0 1 1
* Error 2:   S1 0110 1011 0 1 1

Command Values:
* Pilot: Off / IPI
* Light: 5
* Thermostat: On
* Power: Off
* Front: On
* Fan: 6
* Aux: Off
* Flame: 2

## Examples

### Command Examples
* fire.set(power=False) # Turn off
* fire.set(power=True, flame=1, light=1, fan=0, front=False) # My favorite low mode
* fire.set(power=True, flame=6, light=6, fan=6, front=True) # Highest setting
