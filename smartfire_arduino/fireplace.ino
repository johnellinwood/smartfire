#include <Arduino.h>
#include <SPI.h>
#include "pins_arduino.h" // variant pin definitions

#define RADIO_HEADER_RW_WRITE 0
#define RADIO_HEADER_RW_READ 1
#define RADIO_HEADER_ACCESS_STROBE 0
#define RADIO_HEADER_ACCESS_BURST 1

// Masks for reading chip status response (section 10.0, p31)
#define RADIO_STATUS_CHIP_RDYn 0x80 // mask and shift >> 7
#define RADIO_STATUS_STATE 0x70 // mask and shift >> 4
#define RADIO_STATUS_FIFO_BYTES_AVAILABLE 0x0F // mask only

#define BUTTON_PIN D2

//***************************************CC1101 define**************************************************//
// CC1101 CONFIG REGSITER
#define CC1101_IOCFG2       0x00        // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01        // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02        // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04        // Sync word, high INT8U
#define CC1101_SYNC0        0x05        // Sync word, low INT8U
#define CC1101_PKTLEN       0x06        // Packet length
#define CC1101_PKTCTRL1     0x07        // Packet automation control
#define CC1101_PKTCTRL0     0x08        // Packet automation control
#define CC1101_ADDR         0x09        // Device address
#define CC1101_CHANNR       0x0A        // Channel number
#define CC1101_FSCTRL1      0x0B        // Frequency synthesizer control
#define CC1101_FSCTRL0      0x0C        // Frequency synthesizer control
#define CC1101_FREQ2        0x0D        // Frequency control word, high INT8U
#define CC1101_FREQ1        0x0E        // Frequency control word, middle INT8U
#define CC1101_FREQ0        0x0F        // Frequency control word, low INT8U
#define CC1101_MDMCFG4      0x10        // Modem configuration
#define CC1101_MDMCFG3      0x11        // Modem configuration
#define CC1101_MDMCFG2      0x12        // Modem configuration
#define CC1101_MDMCFG1      0x13        // Modem configuration
#define CC1101_MDMCFG0      0x14        // Modem configuration
#define CC1101_DEVIATN      0x15        // Modem deviation setting
#define CC1101_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CC1101_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CC1101_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CC1101_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CC1101_BSCFG        0x1A        // Bit Synchronization configuration
#define CC1101_AGCCTRL2     0x1B        // AGC control
#define CC1101_AGCCTRL1     0x1C        // AGC control
#define CC1101_AGCCTRL0     0x1D        // AGC control
#define CC1101_WOREVT1      0x1E        // High INT8U Event 0 timeout
#define CC1101_WOREVT0      0x1F        // Low INT8U Event 0 timeout
#define CC1101_WORCTRL      0x20        // Wake On Radio control
#define CC1101_FREND1       0x21        // Front end RX configuration
#define CC1101_FREND0       0x22        // Front end TX configuration
#define CC1101_FSCAL3       0x23        // Frequency synthesizer calibration
#define CC1101_FSCAL2       0x24        // Frequency synthesizer calibration
#define CC1101_FSCAL1       0x25        // Frequency synthesizer calibration
#define CC1101_FSCAL0       0x26        // Frequency synthesizer calibration
#define CC1101_RCCTRL1      0x27        // RC oscillator configuration
#define CC1101_RCCTRL0      0x28        // RC oscillator configuration
#define CC1101_FSTEST       0x29        // Frequency synthesizer calibration control
#define CC1101_PTEST        0x2A        // Production test
#define CC1101_AGCTEST      0x2B        // AGC test
#define CC1101_TEST2        0x2C        // Various test settings
#define CC1101_TEST1        0x2D        // Various test settings
#define CC1101_TEST0        0x2E        // Various test settings
//CC1101 COMMAND STROBES
#define CC1101_SRES         0x30        // Reset chip.
#define CC1101_SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX/TX: Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC1101_SXOFF        0x32        // Turn off crystal oscillator.
#define CC1101_SCAL         0x33        // Calibrate frequency synthesizer and turn it off (enables quick start).
#define CC1101_SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1.
#define CC1101_STX          0x35        // In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear.
#define CC1101_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
#define CC1101_SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define CC1101_SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define CC1101_SPWD         0x39        // Enter power down mode when CSn goes high.
#define CC1101_SFRX         0x3A        // Flush the RX FIFO buffer.
#define CC1101_SFTX         0x3B        // Flush the TX FIFO buffer.
#define CC1101_SWORRST      0x3C        // Reset real time clock.
#define CC1101_SNOP         0x3D        // No operation. May be used to pad strobe commands to two INT8Us for simpler software.
//CC1101 STATUS REGSITER
#define CC1101_PARTNUM      0x30
#define CC1101_VERSION      0x31
#define CC1101_FREQEST      0x32
#define CC1101_LQI          0x33
#define CC1101_RSSI         0x34
#define CC1101_MARCSTATE    0x35
#define CC1101_WORTIME1     0x36
#define CC1101_WORTIME0     0x37
#define CC1101_PKTSTATUS    0x38
#define CC1101_VCO_VC_DAC   0x39
#define CC1101_TXBYTES      0x3A
#define CC1101_RXBYTES      0x3B
#define CC1101_RCCTRL1_STATUS 0x3C
#define CC1101_RCCTRL0_STATUS 0x3D
//CC1101 PATABLE,TXFIFO,RXFIFO
#define CC1101_PATABLE      0x3E
#define CC1101_TXFIFO       0x3F  // TX needs header RW bit set to 0
#define CC1101_RXFIFO       0x3F  // RX needs header RW bit set to 1

const char* CONFIG_REGISTER_NAMES[] = {
  "IOCFG2", //       0x00        // GDO2 output pin configuration
  "IOCFG1", //       0x01        // GDO1 output pin configuration
  "IOCFG0", //       0x02        // GDO0 output pin configuration
  "FIFOTHR", //      0x03        // RX FIFO and TX FIFO thresholds
  "SYNC1", //        0x04        // Sync word, high INT8U
  "SYNC0", //        0x05        // Sync word, low INT8U
  "PKTLEN", //       0x06        // Packet length
  "PKTCTRL1", //     0x07        // Packet automation control
  "PKTCTRL0", //     0x08        // Packet automation control
  "ADDR", //         0x09        // Device address
  "CHANNR", //       0x0A        // Channel number
  "FSCTRL1", //      0x0B        // Frequency synthesizer control
  "FSCTRL0", //      0x0C        // Frequency synthesizer control
  "FREQ2", //        0x0D        // Frequency control word, high INT8U
  "FREQ1", //        0x0E        // Frequency control word, middle INT8U
  "FREQ0", //        0x0F        // Frequency control word, low INT8U
  "MDMCFG4", //      0x10        // Modem configuration
  "MDMCFG3", //      0x11        // Modem configuration
  "MDMCFG2", //      0x12        // Modem configuration
  "MDMCFG1", //      0x13        // Modem configuration
  "MDMCFG0", //      0x14        // Modem configuration
  "DEVIATN", //      0x15        // Modem deviation setting
  "MCSM2", //        0x16        // Main Radio Control State Machine configuration
  "MCSM1", //        0x17        // Main Radio Control State Machine configuration
  "MCSM0", //        0x18        // Main Radio Control State Machine configuration
  "FOCCFG", //       0x19        // Frequency Offset Compensation configuration
  "BSCFG", //        0x1A        // Bit Synchronization configuration
  "AGCCTRL2", //     0x1B        // AGC control
  "AGCCTRL1", //     0x1C        // AGC control
  "AGCCTRL0", //     0x1D        // AGC control
  "WOREVT1", //      0x1E        // High INT8U Event 0 timeout
  "WOREVT0", //      0x1F        // Low INT8U Event 0 timeout
  "WORCTRL", //      0x20        // Wake On Radio control
  "FREND1", //       0x21        // Front end RX configuration
  "FREND0", //       0x22        // Front end TX configuration
  "FSCAL3", //       0x23        // Frequency synthesizer calibration
  "FSCAL2", //       0x24        // Frequency synthesizer calibration
  "FSCAL1", //       0x25        // Frequency synthesizer calibration
  "FSCAL0", //       0x26        // Frequency synthesizer calibration
  "RCCTRL1", //      0x27        // RC oscillator configuration
  "RCCTRL0", //      0x28        // RC oscillator configuration
  "FSTEST", //       0x29        // Frequency synthesizer calibration control
  "PTEST", //        0x2A        // Production test
  "AGCTEST", //      0x2B        // AGC test
  "TEST2", //        0x2C        // Various test settings
  "TEST1", //        0x2D        // Various test settings
  "TEST0"  //        0x2E        // Various test settings
};

const char* COMMAND_STROBE_NAMES[] = {
  "SRES", //         0x30        // Reset chip.
  "SFSTXON", //      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX/TX: Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
  "SXOFF", //        0x32        // Turn off crystal oscillator.
  "SCAL", //         0x33        // Calibrate frequency synthesizer and turn it off (enables quick start).
  "SRX", //          0x34        // Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1.
  "STX", //          0x35        // In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear.
  "SIDLE", //        0x36        // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
  "SAFC", //         0x37        // Perform AFC adjustment of the frequency synthesizer
  "SWOR", //         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
  "SPWD", //         0x39        // Enter power down mode when CSn goes high.
  "SFRX", //         0x3A        // Flush the RX FIFO buffer.
  "SFTX", //         0x3B        // Flush the TX FIFO buffer.
  "SWORRST", //      0x3C        // Reset real time clock.
  "SNOP"  //         0x3D        // No operation. May be used to pad strobe commands to two INT8Us for simpler software.
};

const char* STATUS_REGISTER_NAMES[] = {
  "PARTNUM", //      0x30
  "VERSION", //      0x31
  "FREQEST", //      0x32
  "LQI", //          0x33
  "RSSI", //         0x34
  "MARCSTATE", //    0x35
  "WORTIME1", //     0x36
  "WORTIME0", //     0x37
  "PKTSTATUS", //    0x38
  "VCO_VC_DAC", //   0x39
  "TXBYTES", //      0x3A
  "RXBYTES", //      0x3B
  "RCCTRL1_STATUS", // 0x3C
  "RCCTRL0_STATUS", // 0x3D
  "PATABLE", //      0x3E
  "FIFO" //         0x3F
};

const char* STATUS_STATE_NAMES[] = {
  "IDLE",             // 000  IDLE state (Also reported for some transitional states instead of SETTLING or CALIBRATE)
  "RX",               // 001  Receive mode
  "TX",               // 010  Transmit mode
  "FSTXON",           // 011  Fast TX ready
  "CALIBRATE",        // 100  Frequency synthesizer calibration is running
  "SETTLING",         // 101  PLL is settling
  "RXFIFO_OVERFLOW",  // 110  RX FIFO has overflowed. Read out any useful data, then flush the FIFO with SFRX
  "TXFIFO_UNDERFLOW"  // 111  TX FIFO has underflowed. Acknowledge with SFTX
};

// CC1101 Types
union RADIO_HEADER_T {
  struct { // bit packing goes from LSB to MSB as we go down the structure
    unsigned char address : 6; // Register address to manipulate
    unsigned char access : 1; // Burst access bit. 0 = strobe/single-byte, 1 = burst/multi-bute
    unsigned char rw : 1; // R/W bit. 0 = write, 1 = read
  };
  byte val;
};
typedef union RADIO_HEADER_T RADIO_HEADER_T;

union RADIO_STATUS_T {
  struct {
    unsigned char fifo_bytes_avail : 4; // The number of bytes available in the RX FIFO or free bytes in the TX FIFO
    unsigned char state : 3; // Indicates the current main state machine mode. See section 10.1, p. 31
    bool chip_rdyn : 1; // Stays high until power and crystal have stabilized. Should always be low when using the SPI interface.
  };
  byte val;
};
typedef union RADIO_STATUS_T RADIO_STATUS_T;

union PKTCTRL0_T {
  struct {
    byte gdo2_cfg : 6; // Default=0x29. R/W. Default is CHP_RDYn (See Table 41 on page 62).
    byte gdo2_inv : 1; // Default=0. R/W. Invert output, i.e. select active low (1) / high (0)
    byte none : 1;
  };
  byte val;
};
typedef union PKTCTRL0_T PKTCTRL0_T;

// TODO: create more structs for register bit packing

/** 
 * TI SmartRF Studio 7 outputs register settings for TrxEB with this structure
 */
struct registerSetting_t {
  uint8_t addr;
  uint8_t val;
};
typedef struct registerSetting_t registerSetting_t;

//const byte DEFAULT_SERIAL[3] = {0x25, 0x7A, 0x02}; // add 0x1 to end of first byte, then 0x0 to end of the other two
//class FireCommand {
//  public:
//    byte serial[3] = DEFAULT_SERIAL;
//    bool pilot = true;
//    byte light = 0;
//    bool thermostat = false;
//    bool power = false;
//    bool front = false;
//    byte fan = 0;
//    bool aux = false;
//    byte flame = 0;
//    void buildPacket(byte &arr) { // need to pass in a 25 byte array in which we write the packet
//      return result;
//    }
//}

/**
 * Automatically generated register settings from TI SmartRF Studio 7 for:
 *   F_symbol: 2398.7 baud (closest possible to 2400)
 *   F_center: 314.972687 MHz
 *   F_xosc:   26.0 MHz (local oscillator)
 *   Modulation: ASK/OOK
 *   Whitening: Off
 *   TX_power: 10dBm
 */ 
 /*
  * PASTE REGISTER SETTINGS HERE FROM THE TI SMARTRF STUDIO EXPORT 
  */
//static const registerSetting_t preferredSettings[]= 
//{
//  {CC1101_IOCFG2,      0x0B},
//  {CC1101_IOCFG0,      0x0C},
//  {CC1101_FIFOTHR,     0x47},
////  {CC1101_PKTCTRL0,    0x12}, // need last bit to be 0, i.e., even for fixed packet length
//  {CC1101_PKTCTRL0,    0x00},
//  {CC1101_PKTLEN,      0x19}, // 25 byte fixed length
//  {CC1101_FSCTRL1,     0x06},
//  {CC1101_FREQ2,       0x0C},
//  {CC1101_FREQ1,       0x1D},
//  {CC1101_FREQ0,       0x89},
//  {CC1101_MDMCFG4,     0xF6},
//  {CC1101_MDMCFG3,     0x83},
//  {CC1101_MDMCFG2,     0x30},
//  {CC1101_MDMCFG1,     0x00},
//  {CC1101_DEVIATN,     0x15},
//  {CC1101_MCSM0,       0x18},
//  {CC1101_FOCCFG,      0x16},
//  {CC1101_WORCTRL,     0xFB},
//  {CC1101_FREND0,      0x11},
//  {CC1101_FSCAL3,      0xE9},
//  {CC1101_FSCAL2,      0x2A},
//  {CC1101_FSCAL1,      0x00},
//  {CC1101_FSCAL0,      0x1F},
//  {CC1101_TEST2,       0x81},
//  {CC1101_TEST1,       0x35},
//};


static const registerSetting_t preferredSettings[]= 
{
  {CC1101_SYNC0,       0x00}, // set sync to 0
  {CC1101_SYNC1,       0x00}, // set sync to 0
  {CC1101_PKTLEN,      0x19}, // 25 byte fixed length
  {CC1101_PKTCTRL1,    0x00},
  {CC1101_PKTCTRL0,    0x00}, // LENGTH_CONFIG, etc
  {CC1101_FREQ2,       0x0C}, // ~315mhz from rf studio
  {CC1101_FREQ1,       0x1D},
  {CC1101_FREQ0,       0x45},
  {CC1101_MDMCFG4,     0xF6}, // bandwidth, symbol rate
  {CC1101_MDMCFG3,     0x83}, // symbol rate
  {CC1101_MDMCFG2,     0x30}, // 0 011 0 000 = 0x30
  {CC1101_MDMCFG1,     0x02}, // 0 000 00 10 = 0x02
  {CC1101_MCSM0,       0x18}, // 00 01 10 00 = 0x18
  {CC1101_FOCCFG,      0x14},  // 0001 0100 
  {CC1101_FREND0,      0x11}  // 0001 0001
};


// TODO: figure out how to make a simple serial println for this board!
void serial_println(const char *format) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format);
  Serial.println(str);
}
void serial_println(const char *format, auto a1) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1);
  Serial.println(str);
}
void serial_println(const char *format, auto a1, auto a2) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1, a2);
  Serial.println(str);
}
void serial_println(const char *format, auto a1, auto a2, auto a3) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1, a2, a3);
  Serial.println(str);
}
void serial_println(const char *format, auto a1, auto a2, auto a3, auto a4) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1, a2, a3, a4);
  Serial.println(str);
}
void serial_println(const char *format, auto a1, auto a2, auto a3, auto a4, auto a5) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1, a2, a3, a4, a5);
  Serial.println(str);
}
void serial_println(const char *format, auto a1, auto a2, auto a3, auto a4, auto a5, auto a6) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1, a2, a3, a4, a5, a6);
  Serial.println(str);
}
void serial_println(const char *format, auto a1, auto a2, auto a3, auto a4, auto a5, auto a6, auto a7) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1, a2, a3, a4, a5, a6, a7);
  Serial.println(str);
}
void serial_println(const char *format, auto a1, auto a2, auto a3, auto a4, auto a5, auto a6, auto a7, auto a8) {
  size_t strSize = 256;
  char str[256];
  snprintf(str, strSize, format, a1, a2, a3, a4, a5, a6, a7, a8);
  Serial.println(str);
}

// see pin definitions at https://github.com/arduino/ArduinoCore-avr/blob/master/variants/standard/pins_arduino.h#L38
void printPin(const char* name, byte pin) {
  serial_println("%s is %d", name, pin);
}

const char* getCommandStrobeName(const byte reg) {
  return COMMAND_STROBE_NAMES[reg - 0x30];
}

void printHeader(byte header) {
  RADIO_HEADER_T temp;
  temp.val = header;
  printHeader(temp);
}

void printHeader(RADIO_HEADER_T header) {
  byte val = header.val;
  byte rw = header.rw;
  byte access = header.access;
  byte reg = header.address;
  const char* regtext = "";
  if (reg <= 0x2F) {
    regtext = getConfigRegisterName(reg);
  } else if (reg <= 0x3D) {
    if (access) {
      regtext = rw ? getStatusRegisterName(reg) : "UNDEFINED";
    } else {
      regtext = getCommandStrobeName(reg);
    }
  } else if (reg == 0x3E) {
    regtext = "PATABLE";
  } else if (reg == 0x3F) {
    regtext = rw ? "RXFIFO" : "TXFIFO";
  } else {
    regtext = "UNDEFINED";
  }
  const char* rwtext = rw ? "READ" : "WRITE";
  const char* accesstext = access ? "BURST/MULTI-BYTE" : "STROBE/SINGLE-BYTE";
  serial_println("Header Byte [0x%02x]: { rw: %s[%d], access: %s[%d], reg: %s[0x%02x]}", val, rwtext, rw, accesstext, access, regtext, reg);
}

void printRadioStatus(const RADIO_STATUS_T status, byte rw) {
  const char* statetxt = STATUS_STATE_NAMES[status.state];
  const char* rdytxt = status.chip_rdyn ? "NOT READY" : "READY";
  const char* fifotxt = rw ? "RX" : "TX";
  const char* moreAvail = status.fifo_bytes_avail == 15 ? "+" : "";
  serial_println("Radio Status [0x%02x]: { ready: %s[%d], state: %s[%d], fifo: %s, avail: %d%s }", status.val, rdytxt, status.chip_rdyn, statetxt, status.state, fifotxt, status.fifo_bytes_avail, moreAvail);
}

void printRadioStatus(const byte radioStatus, byte rw) {
  RADIO_STATUS_T temp;
  temp.val = radioStatus;
  printRadioStatus(temp, rw);
}

const char* getStatusRegisterName(const byte reg) {
  return STATUS_REGISTER_NAMES[reg - 0x30];
}

void printStatusRegisterValue(const byte reg, const byte val) {
  serial_println("Status Register %s [0x%02x]: { value: 0x%02x }", getStatusRegisterName(reg), reg, val);
}

const char* getConfigRegisterName(const byte reg) {
  return CONFIG_REGISTER_NAMES[reg];
}

void printConfigRegisterValue(const byte reg, const byte val) {
  serial_println("Reading Config Register %s [0x%02x] : { value: 0x%02x }", getConfigRegisterName(reg), reg, val);
}

void printConfigRegisterSet(const byte reg, const byte val) {
  serial_println("Writing Config Register %s [0x%02x] : { value: 0x%02x }", getConfigRegisterName(reg), reg, val);
}

RADIO_HEADER_T buildHeader(byte rw, byte access, byte address) {
  RADIO_HEADER_T header;
  header.rw = rw;
  header.access = access;
  header.address = address;
  return header;
}



/**
 * Waits for the CC1101 to be ready to receive commands. Must be called AFTER the CS pin is set low, which happens in the SPI.beginTransaction();
 * See CC1101 spec section 10: 
 *   "When CSn is pulled low, the MCU must wait until CC1101 SO pin goes low before starting to transfer the header byte."
 */
void spiWait() {
    while(digitalRead(PIN_SPI_MISO)) ;
}

void spiSelect() { // activates the radio chip for reading commands
  digitalWrite(PIN_SPI_SS, LOW); 
}

void spiDeselect() { // stops the radio chip from reading commands, cancels any commands in progress of writing
  digitalWrite(PIN_SPI_SS, HIGH); 
}

void spiSelectAndWait() {
  spiSelect();
  spiWait();
}

RADIO_STATUS_T sendRadioHeader(RADIO_HEADER_T header) {
  printHeader(header);
  byte radioStatus; // status byte response from transmission of header byte
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0)); //500Kb, MSB first, MODE0 is CPOL=0, CPHA=0, see page 30 of CC1101 spec
  spiSelectAndWait();
  radioStatus = SPI.transfer(header.val);
  printRadioStatus(radioStatus, header.rw);
  spiDeselect();
  SPI.endTransaction();
  RADIO_STATUS_T res;
  res.val = radioStatus;
  return res;
}

/**
 * Send radio command strobe
 */
RADIO_STATUS_T sendRadioCmd(byte rw, byte address) { // command to send, i.e., what command register to write to
  // TODO add check to make sure address is in command address range
  RADIO_HEADER_T header = buildHeader(rw, RADIO_HEADER_ACCESS_STROBE, address);
  return sendRadioHeader(header);
}

byte getRadioStatusRegister(byte reg) { // return current value
  RADIO_HEADER_T header = buildHeader(RADIO_HEADER_RW_READ, RADIO_HEADER_ACCESS_BURST, reg);
  printHeader(header);
  byte radioStatus; // status byte response from command
  byte statusRegisterValue; // the value read from the status register
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0)); //500Kb, MSB first, MODE0 is CPOL=0, CPHA=0, see page 30 of CC1101 spec
  spiSelectAndWait();
  radioStatus = SPI.transfer(header.val); // send command and get radio status byte response
  statusRegisterValue = SPI.transfer(0); // SPI api is always two-way, so send 0's to just read a byte
  printRadioStatus(radioStatus, header.rw);
  spiDeselect();
  SPI.endTransaction();
  printStatusRegisterValue(header.address, statusRegisterValue);
  return statusRegisterValue;
}

// TODO this is almost exactly the same as status register
byte getRadioConfigRegister(byte reg) { // return current value
  RADIO_HEADER_T header = buildHeader(RADIO_HEADER_RW_READ, RADIO_HEADER_ACCESS_STROBE, reg);
  printHeader(header);
  byte radioStatus; // status byte response from command
  byte radioRegisterValue; // the value read from the config register
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0)); //500Kb, MSB first, MODE0 is CPOL=0, CPHA=0, see page 30 of CC1101 spec
  spiSelectAndWait();
  radioStatus = SPI.transfer(header.val); // send command and get radio status byte response
  radioRegisterValue = SPI.transfer(0); // SPI api is always two-way, so send 0's to just read a byte
  printRadioStatus(radioStatus, header.rw);
  spiDeselect();
  SPI.endTransaction();
  printConfigRegisterValue(header.address, radioRegisterValue);
  return radioRegisterValue;
}

void setRadioConfigRegister(byte reg, byte dataVal) { // return old valud
  RADIO_HEADER_T header = buildHeader(RADIO_HEADER_RW_WRITE, RADIO_HEADER_ACCESS_STROBE, reg);
  printHeader(header);
  byte radioStatus; // status byte response from command
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0)); //500Kb, MSB first, MODE0 is CPOL=0, CPHA=0, see page 30 of CC1101 spec
  spiSelectAndWait();
  // TODO: could use transfer with val16 to send both at the same time...
  radioStatus = SPI.transfer(header.val); // send command and get radio status byte response
  radioStatus = SPI.transfer(dataVal); // send the value to put in the register
  printRadioStatus(radioStatus, header.rw);
  spiDeselect();
  SPI.endTransaction();
  printConfigRegisterSet(header.address, dataVal);
}

// without a header file, this has to come after serial_println() and setRadioConfigRegister()
template <size_t N> 
void loadSettings(const registerSetting_t (&settings)[N]) {
  int settings_size = sizeof(settings) / sizeof(registerSetting_t);
  serial_println("Settings %d registers from array", settings_size);
  for(int i=0; i<settings_size; i++) {
    registerSetting_t setting = settings[i];
    byte addr = setting.addr;
    byte val = setting.val;
    setRadioConfigRegister(addr, val);
  }
}

template <size_t N>
void sendRadioBurst(byte (&burstData)[N], byte reg) {
  RADIO_HEADER_T header = buildHeader(RADIO_HEADER_RW_WRITE, RADIO_HEADER_ACCESS_BURST, reg);
  serial_println("Sending TX burst of %d bytes", N);
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0)); //500Kb, MSB first, MODE0 is CPOL=0, CPHA=0, see page 30 of CC1101 spec
  spiSelectAndWait();
  printHeader(header);
  RADIO_STATUS_T rs;
  rs.val = SPI.transfer(header.val);
  printRadioStatus(rs, header.rw);
  for (size_t i=0; i < N; i++) {
    serial_println("data: 0x%02x", burstData[i]);
    // wait to send data if tx fifo is too ful
//    while (rs.fifo_bytes_avail < 10) {
//      delay(1);
//      rs=sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP); 
//    }
    rs.val=SPI.transfer(burstData[i]);
    printRadioStatus(rs, header.rw);
  }
  spiDeselect();
  SPI.endTransaction();
}

void setOokPower(byte off, byte on) {
  //  set patable power for 1 to 0xC5, 10dBmW
  serial_println("Setting PA Table for OOK");
  byte ook_power[2];
  ook_power[0] = off;
  ook_power[1] = on;
  sendRadioBurst(ook_power, CC1101_PATABLE);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for serial monitor to connect
  Serial.println("setup()");

  RADIO_HEADER_T temp = buildHeader(RADIO_HEADER_RW_READ, RADIO_HEADER_ACCESS_STROBE, CC1101_TEST1);
  serial_println("Testing radio header type. Value is 0x%02x", temp.val);
  
  printPin("PIN_SPI_SS", PIN_SPI_SS);
  printPin("PIN_SPI_MOSI", PIN_SPI_MOSI);
  printPin("PIN_SPI_MISO", PIN_SPI_MISO);
  printPin("PIN_SPI_SCK", PIN_SPI_SCK);
  
  // LED pins
  // NOTE: THESE PINS ARE ACTIVE LOW!!!
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  // init light on at the beginning of setup
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);
  // Button pin
  pinMode(BUTTON_PIN, INPUT_PULLDOWN); // floating input button pull to low when open, high when pressed

  Serial.println("Initializing radio");
  setupCustom();
  Serial.println("Radio initialized");

  // turn light off at end of setup
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  Serial.println("end setup()");
}

void setupLib() { // use the CC1101 driver
//  // set CC1101 driver to use pins defined for our variant. make sure D10 is connected to SS, all others are shown on the nano 33 ble pinout diagram
//  ELECHOUSE_cc1101.setSpiPin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SPI_SS); 
//  // set CC1101 driver to use gdo pins. 
//  // connect D9 to GDO2, which is digital output from the radio. 
//  // connect A1/D15 to GDO0, which is digital output from the radio, but can also be used as analog temperature sensor.
//  ELECHOUSE_cc1101.setGDO(A1, D9);
//
//  ELECHOUSE_cc1101.Init();
//
//  if (ELECHOUSE_cc1101.getCC1101()){      // Check the CC1101 Spi connection.
//    Serial.println("Connection OK");
//  } else {
//    Serial.println("Connection Error");
//  }
//  
//  byte radioVersion = ELECHOUSE_cc1101.SpiReadStatus(CC1101_VERSION); // read version number
//  if (radioVersion == 0x14) {
//    Serial.println("Successfully read radio version as 0x14");
//  } else {
//    Serial.println("Failed to correctly read radio verison");
//  }
}

// TODO implement these methods:
// radio.setFreq(314973000)
// radio.setMdmModulation(MOD_ASK_OOK)
// radio.setMdmDRate(2400)

void setupCustom() { // use all our own SPI commands

  // Setup SPI pins
  pinMode(PIN_SPI_SS, OUTPUT);
  SPI.begin(); // should set SCK, MOSI, and MISO properly

  sendRadioCmd(RADIO_HEADER_RW_READ, CC1101_SNOP);
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
  getRadioStatusRegister(CC1101_PARTNUM);
  getRadioStatusRegister(CC1101_VERSION);
  // lib does the following
  // setCC ... communication mode
  // iocfg 0,2 ... config mode for GDO 0,2.. don't need yet, just settings for serial TX/RX, maybe later
  // pktctrl0
  // mdmcfg3,4
  // setModulation
  // PKTCTRL0 12 from studio?

  loadSettings(preferredSettings);  // load settings from RF Studio

  setOokPower(0, 197); // 197 is about 10dBmW, close to the maximum
  
  // overwrite some of the settings...
  
//  setRadioConfigRegister(CC1101_PKTCTRL0, 0); // 0 0 00 0 0 00, sets data whitening, fifo mode, no crc, fixed packet length
  
  // will send 958 total bits. 182 bits 5 times, separated by 12 bits each.  (5*182)+(4*12)=958 bits / 8 ~ 120 bytes.
  // we can send each fireplace command in 23 bytes. we still have to use the fifo to write this.
  // Max fifo length is 64 bytes. choices are to send the packet 5 separate times, but risk the 12 bit delay in between being inconsistent.
  // Best to just write the fifo for the fullly encoded and appended 5x sequence.
  // We can use fixezd packet length mode for up to 255 byte packets. 
  // for reading we might want to just read single packets as single transmissions though...
  // ok, lets set packet length to 23 bytes and do it 5 times
  
//  setRadioConfigRegister(CC1101_PKTLEN, 25); // set it to 25 bytes.. enough for the packet and the space afterwards
  
  // mdmcfg0 sets the channel spacing
  // mdmcfg1 sets the fec, preamble, channel spacing
  // mdmcfg2 sets the modulation type, encoding, and sync words
  // mdmcfg3 sets the symbol rate
  // mdmcfg4 sets the channel bandwidth
//  setRadioConfigRegister(CC1101_MDMCFG0, ); // F8
//  setRadioConfigRegister(CC1101_MDMCFG1, ); // 00
//  setRadioConfigRegister(CC1101_MDMCFG2, ); // 30
//  setRadioConfigRegister(CC1101_MDMCFG3, ); // 83
//  setRadioConfigRegister(CC1101_MDMCFG4, ); // F6
  // we want 314.973 MHz
  // we want 2400 symbol rate
  // we want 5khz bandwidth
  // FREQ0 45
  // FREQ1 1D
  // FREQ2 0C


}

void loopLib() { // loop with CC1101 driver commands
//  byte radioVersion = ELECHOUSE_cc1101.SpiReadStatus(CC1101_VERSION); // read version number
//  if (radioVersion == 0x14) {
//    Serial.println("Successfully read radio version as 0x14");
//  } else {
//    Serial.println("Failed to correctly read radio verison");
//  }
}

int loop_cnt = 0;
char s[64];
int buttonState = 0;
int prevButtonState = 0;
bool buttonStateChanged = false;
bool lastCommand = false;

void loopCustom() { // loop with our own SPI commands
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
}

// unencoded on commands, ready for direct modulation
// compare to \xe5\x96\x69\xb9\xaa\x65\xae\x55\x59\x6b\xa5\x95\x9a\xe5\x56\x95\xb9\x96\x55\x6e\xa5\x56\x68\x00\x00
byte FIREPLACE_ON_DATA[] = {
  0xE5, // 1110 0101
  0x96, // 1001 0110
  0x69, // 0110 1001
  0xB9, // 1011 1001
  0xAA, // 1010 1010
  0x65, // 0110 0101
  0xAE, // 1010 1110
  0x55, // 0101 0101
  0x59, // 0101 1001
  0x6B, // 0110 1011
  0xA5, // 1010 0101
  0x95, // 1001 0101
  0x9A, // 1001 1010
  0xE5, // 1110 0101
  0x56, // 0101 0110
  0x95, // 1001 0101
  0xB9, // 1011 1001
  0x96, // 1001 0110
  0x55, // 0101 0101
  0x6E, // 0110 1110
  0xA5, // 1010 0101
  0x56, // 0101 0110
  0x68, // 0110 1000
  0x00, // final padding
  0x00
};
byte FIREPLACE_OFF_DATA[] = {
  0xE5,
  0x96,
  0x69,
  0xB9,
  0xAA,
  0x65,
  0xAE,
  0x55,
  0x59,
  0x6B,
  0xA5,
  0x95,
  0x56,
  0xE5,
  0x56,
  0x95,
  0xB9,
  0xA6,
  0x59,
  0x6E,
  0xA5,
  0x56,
  0x68,
  0x00,
  0x00
};
byte FIREPLACE_TEST_SIGNAL[] = {
  0x00, 0x00, 0x55, 0x55, 0x00, 0xFF, 0x33, 0x33, 0xFF, 0x00
};

void fireplaceTestSignal() {
  serial_println("Fireplace test signal");
  setRadioConfigRegister(CC1101_PKTLEN, 10);
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SFTX); // flush tx fifo;
  sendRadioBurst(FIREPLACE_TEST_SIGNAL, CC1101_TXFIFO); // load data
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_STX); // enter TX mode
  delay(1000);
  getRadioStatusRegister(CC1101_MARCSTATE);
  getRadioStatusRegister(CC1101_TXBYTES);
}

void turnOnFireplace2() {
  serial_println("Turning fireplace on");
  // enter tx mode, send STX command strobe
  // config should set MCSM0 (main radio control) .FS_AUTOCAL to 3, auto calibrate after every 4th transmit
  // rfstudio set it to FSTXON, which is ok
  // send on commmand
  // fill FIFO for tx

  // set RW to 0 (for write) and use register 0x3F for TX fifo.  3F is single byte TX
  // use BURST to write the whole packet at once. 7F is multi byte / burst access to TX fifo
  // Write one header, then all the data bytes when doing burst
  // ok, register is just 3F... 

  // read TXBYTES.NUM_TXBYTES register regularly while filling

  for (int i=0; i < 5; i++) {

  getRadioStatusRegister(CC1101_MARCSTATE);
  getRadioStatusRegister(CC1101_TXBYTES);

  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SFTX); // flush tx fifo
  getRadioStatusRegister(CC1101_MARCSTATE);
  getRadioStatusRegister(CC1101_TXBYTES);

  sendRadioBurst(FIREPLACE_ON_DATA, CC1101_TXFIFO); // load data
  getRadioStatusRegister(CC1101_MARCSTATE);
  getRadioStatusRegister(CC1101_TXBYTES);

  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_STX); // enter TX mode
  getRadioStatusRegister(CC1101_MARCSTATE);
  
  byte txbytes = getRadioStatusRegister(CC1101_TXBYTES); // wait for data to drain
  while (txbytes > 30) { // leave room for 25 bytes plus some
    delay(1);
    txbytes = getRadioStatusRegister(CC1101_TXBYTES);
  }

  }
  
}

//
//void turnOnFireplace2() {
//  serial_println("Turning fireplace on");
//
//  for (int i=0; i < 5; i++) {
//  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SFTX); // flush tx fifo
//  sendRadioBurst(FIREPLACE_ON_DATA, CC1101_TXFIFO); // load data
//  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_STX); // enter TX mode
//  
//  byte txbytes = getRadioStatusRegister(CC1101_TXBYTES); // wait for data to drain
//  while (txbytes > 30) { // leave room for 25 bytes plus some
//    delay(1);
//    txbytes = getRadioStatusRegister(CC1101_TXBYTES);
//  }
//
//  }
//  
//}


void turnOnFireplace() {
  serial_println("Turning fireplace on");
  setRadioConfigRegister(CC1101_PKTLEN, 125); // make it a 5* the packet size
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SFTX); // flush tx fifo
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_STX); // enter TX mode
  for (int i=0; i < 5; i++) {
    sendRadioBurst(FIREPLACE_ON_DATA, CC1101_TXFIFO); // load data
    byte txbytes = getRadioStatusRegister(CC1101_TXBYTES) & 0x7F; // wait for data to drain
    while (txbytes > 37) { // leave room for 25 bytes plus some
      delay(1);
      txbytes = getRadioStatusRegister(CC1101_TXBYTES) & 0x7F;
    }
  }
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
}


void turnOffFireplace() {
  serial_println("Turning fireplace off");
  setRadioConfigRegister(CC1101_PKTLEN, 125); // make it a 5* the packet size
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SFTX); // flush tx fifo
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_STX); // enter TX mode
  for (int i=0; i < 5; i++) {
    sendRadioBurst(FIREPLACE_OFF_DATA, CC1101_TXFIFO); // load data
    byte txbytes = getRadioStatusRegister(CC1101_TXBYTES) & 0x7F; // wait for data to drain
    while (txbytes > 37) { // leave room for 25 bytes plus some
      delay(1);
      txbytes = getRadioStatusRegister(CC1101_TXBYTES) & 0x7F;
    }
  }
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
  sendRadioCmd(RADIO_HEADER_RW_WRITE, CC1101_SNOP);
}

// the loop function runs over and over again
// TODO: use interrupts for send/receive events instead of looping poll
void loop() {
  loop_cnt++;

  // Print loop count
  //  snprintf_P(s, sizeof(s), PSTR("loop(%i)"), loop_cnt);
  //  Serial.println(s);

  // Read button state
  prevButtonState = buttonState;
  buttonState = digitalRead(BUTTON_PIN);
  buttonStateChanged = prevButtonState != buttonState;

  if (buttonStateChanged) {
    // Set light based on button state
    if (buttonState == HIGH) {
      // turn LED on. the RGB LED is active low. Not well documented
      //Serial.println("button pressed");
      digitalWrite(LEDR, LOW);
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDB, LOW);
      // new button press, toggle fireplace
//      fireplaceTestSignal();
      bool newCommand = !lastCommand;
      if (newCommand) { // turn on fireplace
        turnOnFireplace();
      } else { // turn off fireplace
        turnOffFireplace();
      }
      lastCommand = newCommand;
    } else {
      // turn LED off:
      //Serial.println("button released");
      digitalWrite(LEDR, HIGH);
      digitalWrite(LEDG, HIGH);
      digitalWrite(LEDB, HIGH);
    }
  }

  // Send NOOP+WRITE to radio every 100th loop / 10 seconds
  if (loop_cnt % 100 == 0) {
    //Serial.println("Checking status");
    // loopCustom();
    //Serial.println("Status check complete");
  }

  // wait 1/10th of a second for next loop
  delay(100);  
 
}
