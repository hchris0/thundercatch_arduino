/******************************************************************************
*                           as3935.c  v0.2                                    *
* ----------------------------------------------------------------------------*
* Code file for interfacing AS3935 ligntning sensor to a microcontroller      *
* connected in SPI.                                                           *
*                                                                             *
* Coded by C.H.                                                               *
*                                                                             *
* Last modified: 23 March 2016                                                *
*******************************************************************************/
#include "AS3935.h"
#include <SPI.h>
unsigned char AS3935::sendByteSPI(uint8_t data) {
    unsigned char c = SPI.transfer(data);
    return c;       // Store received data 
}

/******************************************************************************
* Function init(void)                                                         *
* ----------------------------------------------------------------------------*
* Overview: Initializes SPI and AS3935 Sensor                                 *
* Input:  Nothing                                                             *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::init(void) {
    
    SPI.begin();

    SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE1));
    // Init Chip Select pin
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    // Init Interrupt Pin

    writeReg(0x3C, 0x96);
    writeReg(0x3D, 0x96);
    setIndoors(true);
    setWDT(3);
    setNoise(3);
    maskDisturber(true);
}

/******************************************************************************
* Function writeReg(uint8_t address, uint8_t data)                            *
* ----------------------------------------------------------------------------*
* Overview: Writes a byte to sensor                                           *
* Input:  Address of sensor and data to write                                 *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::writeReg(uint8_t address, uint8_t data) {
    digitalWrite(CS_PIN, LOW);
    sendByteSPI(address);
    sendByteSPI(data);
    digitalWrite(CS_PIN, HIGH);
}

/******************************************************************************
* Function readReg(uint8_t address)                                           *
* ----------------------------------------------------------------------------*
* Overview: Reads a register from sensor                                      *
* Input:  Address of sensor to read                                           *
* Output: Data of register                                                    *
*******************************************************************************/
unsigned char AS3935::readReg(uint8_t address) {
    volatile unsigned char c;
    digitalWrite(CS_PIN, LOW);
    c = sendByteSPI(0x40 | address);
    c = sendByteSPI(0xff);
    digitalWrite(CS_PIN, HIGH);
    return c;
}

/******************************************************************************
* Func writeRegPart(uint8_t address, uint8_t value, int length, int start)    *
* ----------------------------------------------------------------------------*
* Overview: Writes a specific value to a register                             *
* Input:  Register address, value to write, length of data, start of data     *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::writeRegPart(uint8_t address, uint8_t value, int length, int start) {
    uint8_t val, bitmask, wrmask;

    val = readReg(address);
    bitmask = constructMask(length, start);
    wrmask = bitmask >> start;
    val &= ~bitmask;
    val |= (value & wrmask) << start;

    writeReg(address, val);
}

/******************************************************************************
* Function readRegPart(uint8_t address, int length, int start)                *
* ----------------------------------------------------------------------------*
* Overview: Reads a specific value from a register                            *
* Input:  Byte with register address, length of data, start of data           *
* Output: 8bit value                                                          *
*******************************************************************************/
char AS3935::readRegPart(uint8_t address, int length, int start) {
    uint8_t data;

    data = readReg(address);
    return getBitfield(data, length, start);
}

/******************************************************************************
* Function constructMask(int length, int start)                               *
* ----------------------------------------------------------------------------*
* Overview: Constructs a mask for a given lenght                              *
* Input:  Lenght of mask and bit mask starts                                  *
* Output: 8bit mask                                                           *
*******************************************************************************/
uint8_t AS3935::constructMask(int length, int start) {
    uint8_t mask = 0x00;

    if (length < 0 || length > 8 || start < 0 || start > 7)
    return 0;

    for (int i=start; i < (start+length); i++) {
    mask |= 1 << i;
    }
    return mask;
}

/******************************************************************************
* Function getBitfield(uint8_t data, int length, int start)                   *
* ----------------------------------------------------------------------------*
* Overview: Gets a specific bit value from a uint8_t                          *
* Input:  Byte with value, length of data, start of data                      *
* Output: 8bit extracted value                                                *
*******************************************************************************/
uint8_t AS3935::getBitfield(uint8_t data, int length, int start) {
    if (length < 0 || length > 8 || start < 0 || start > 7)
    return 0;

    return ((data & constructMask(length, start)) >> start);
}

/******************************************************************************
* Function setIndoors(bool indoor)                                            *
* ----------------------------------------------------------------------------*
* Overview: Sets sensor to indoors or outdoors                                *
* Input:  True for indoors, false for outdoors                                *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::setIndoors(bool indoor) {
    if (indoor)
        writeRegPart(0x00, 0x12, 5, 1);
    else
        writeRegPart(0x00, 0x0E, 5, 1);
}

/******************************************************************************
* Function getIndoors(void)                                                   *
* ----------------------------------------------------------------------------*
* Overview: Gets sensor value of indoors or outdoors                          *
* Input:  Nothing                                                             *
* Output: True for indoors, false for outdoors                                *
*******************************************************************************/
bool AS3935::getIndoors(void) {
    int i = readRegPart(0x00, 5, 1);
    if (i >= 0x12)
        return true;
    return false;
}

/******************************************************************************
* Function maskDisturber(bool disturber)                                      *
* ----------------------------------------------------------------------------*
* Overview: Masks or not the distrubers from interrupts                       *
* Input:  True for mask, false for no mask                                    *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::maskDisturber(bool disturber) {
    if (disturber)
        writeRegPart(0x03, 1, 1, 5);
    else
        writeRegPart(0x03, 0, 1, 5);
}

/******************************************************************************
* Function getMaskDistrurber(void)                                            *
* ----------------------------------------------------------------------------*
* Overview: Gets mask disturber value                                         *
* Input:  Nothing                                                             *
* Output: True for mask, false for no mask                                    *
*******************************************************************************/
bool AS3935::getMaskDisturber(void) {
    uint8_t r = readRegPart(0x03, 1, 5);
    if (r) {
        return true;
    }
    return false;
}

/******************************************************************************
* Function powerDown(bool powedown)                                           *
* ----------------------------------------------------------------------------*
* Overview: Powers down or enables the device                                 *
* Input:  True for powerdown, false for enable                                *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::powerDown(bool powerdown) {
    if (powerdown) {
        writeRegPart(0x00, 0, 1, 0); // PWD (powerdown) = 0, enabled
    } else {
        writeRegPart(0x00, 1, 1, 0); // PWD (powerdown) = 1, disabled
    }
}

/******************************************************************************
* Function setNoise(uint8_t noise)                                            *
* ----------------------------------------------------------------------------*
* Overview: Sets the noise value to sensor                                    *
* Input:  Value for noise                                                     *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::setNoise(uint8_t noise) {
    if (noise > 7) {
        noise = 7;
    }
    writeRegPart(0x01, noise, 3, 4);
}

/******************************************************************************
* Function getNoise(void)                                                     *
* ----------------------------------------------------------------------------*
* Overview: Gets the noise value of sensor                                    *
* Input:  Nothing                                                             *
* Output: noise value                                                         *
*******************************************************************************/
uint8_t AS3935::getNoise(void) {
    return readRegPart(0x01, 3, 4);
}

/******************************************************************************
* Function setMinimumStrikes(uint8_t strikes)                                 *
* ----------------------------------------------------------------------------*
* Overview: Sets the minimum strikes sensor will interrupt                    *
* Input:  Strikes                                                             *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::setMinimumStrikes(uint8_t strikes) {
    uint8_t value = 0x00;
    if (strikes == 1) {
        value = 0x00;
    } else if (strikes > 1) {
        value = 0x01;
    } else if (strikes > 5) {
        value = 0x02;
    } else if (strikes > 9) {
        value = 0x03;
    }
    writeRegPart(0x02, value, 2, 4);
}

/******************************************************************************
* Function getMinimumStrikes(void)                                            *
* ----------------------------------------------------------------------------*
* Overview: Gets the minimum strikes sensor will interrupt                    *
* Input:  Nothing                                                             *
* Output: Strikes                                                             *
*******************************************************************************/
uint8_t AS3935::getMinimumStrikes(void) {
    uint8_t reg = readRegPart(0x02, 2, 4);
    switch (reg) {
        case 0x00:
            return 1;
        case 0x01:
            return 5;
        case 0x02:
            return 9;
        case 0x03:
            return 16;
    }
    return -1;
}

/******************************************************************************
* Function getStromDistance(void)                                             *
* ----------------------------------------------------------------------------*
* Overview: Gets storm distance in case a lightning interrupt occurs          *
* Input:  Nothing                                                             *
* Output: Distance in km                                                      *
*******************************************************************************/
uint8_t AS3935::getStormDistance(void) {
    return readRegPart(0x07, 6, 0);
}

/******************************************************************************
* Function setSpikeRejection(uint8_t value)                                   *
* ----------------------------------------------------------------------------*
* Overview: Sets the value in which sensor detects spikes                     *
* Input:  Spike rejection value                                               *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::setSpikeRejection(uint8_t value) {
    if (value > 15) {
        value = 15;
    }
    writeRegPart(0x02, value, 4, 0);
}

/******************************************************************************
* Function getSpikeRejectionAS(void)                                          *
* ----------------------------------------------------------------------------*
* Overview: Sets the value in which sensor detects spikes                     *
* Input:  Nothing                                                             *
* Output: Spike rejection value                                               *
*******************************************************************************/
uint8_t AS3935::getSpikeRejection(void) {
    return readRegPart(0x02, 4, 0);
}

/******************************************************************************
* Function setWatchdogThreshold(uint8_t value)                                *
* ----------------------------------------------------------------------------*
* Overview: Sets the sensors watchdog threshold value                         *
* Input:  Watchdog threshold value                                            *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::setWDT(uint8_t value) {
    if (value > 15) {
        value = 15;
    }
    writeRegPart(0x01, value, 4, 0);
}

/******************************************************************************
* Function getWatchdogThreshold(void)                                         *
* ----------------------------------------------------------------------------*
* Overview: Gets the sensors watchdog threshold value                         *
* Input:  Nothing                                                             *
* Output: Watchdog threshold value                                            *
*******************************************************************************/
uint8_t AS3935::getWDT(void) {
    return readRegPart(0x01, 4, 0);
}

/******************************************************************************
* Function getInterruptCause(void)                                            *
* ----------------------------------------------------------------------------*
* Overview: Gets the cause of interrupt from sensor                           *
* Input:  Nothing                                                             *
* Output: Interrupt cause                                                     *
*******************************************************************************/
int AS3935::getInterruptCause(void) {
    uint8_t c = readRegPart(0x03, 4, 0);
    if (c == 1) {
        return 1;           // Noise level too high
    } else if (c == 4) {
        return 2;           // Disturber detected
    } else if (c == 8) {
        return 3;           // Lightning interrupt
    } else {
        return 0;           // Unknown interrupt
    }
}

/******************************************************************************
* Function resetASSensor(void)                                                *
* ----------------------------------------------------------------------------*
* Overview: Writes the default values to sensors registers                    *
* Input:  Nothing                                                             *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::reset(void)
{
  writeReg(0x3C, 0x96);
  writeReg(0x3D, 0x96);
}

void AS3935::LCOOnIntPin(void) {
    writeRegPart(0x08, 1, 1, 7);
}

void AS3935::LCOOffIntPin(void) {
    writeRegPart(0x08, 0, 1, 7);
}

/******************************************************************************
* Function setInternalCapacitors(uint8_t value)                               *
* ----------------------------------------------------------------------------*
* Overview: Sets a value to the internal tuning capacitors                    *
* Input:  Capacitor value                                                     *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::setInternalCapacitors(uint8_t value) {
    writeRegPart(0x08, value, 4, 0);
}

/******************************************************************************
* Function getInternalCapacitors(void)                                        *
* ----------------------------------------------------------------------------*
* Overview: Gets the internal tuning capacitors value                         *
* Input:  Nothing                                                             *
* Output: Capacitor value                                                     *
*******************************************************************************/
uint8_t AS3935::getInternalCapacitors(void) {
    return readRegPart(0x08, 4, 0);
}

/******************************************************************************
* Function clearStatistics(void)                                              *
* ----------------------------------------------------------------------------*
* Overview: Clears sensor statistics                                          *
* Input:  Nothing                                                             *
* Output: Nothing                                                             *
*******************************************************************************/
void AS3935::clearStatistics(void) {
    writeRegPart(0x02, 0, 1, 6);
    writeRegPart(0x02, 1, 1, 6);
}

