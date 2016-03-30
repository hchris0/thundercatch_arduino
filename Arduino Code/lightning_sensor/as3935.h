/******************************************************************************
*                           as3935.h  v0.2                                    *
* ----------------------------------------------------------------------------*
* Header file for interfacing AS3935 ligntning sensor to a microcontroller    *
* connected in SPI.                                                           *
*                                                                             *
* Coded by C.H.                                                               *
*                                                                             *
* Last modified: 23 March 2016                                                *
*******************************************************************************/
#ifndef AS3935_H_
#define AS3935_H_

#include <stdbool.h>
#include <stdint.h>

#define CS_PIN 7

class AS3935 {

// General functions to write to AS Sensor
private:
    unsigned char sendByteSPI(uint8_t data);
    void writeReg(uint8_t address, uint8_t data);
    void writeRegPart(uint8_t address, uint8_t value, int length, int start);
    char readRegPart(uint8_t address, int length, int start);
    // Utility functions helping on value extraction from registers
    uint8_t getBitfield(uint8_t value, int length, int start);
    uint8_t constructMask(int length, int start);
    
public:
    enum Event {UNKNOWN, NOISE, DISTURBER, LIGHTNING};
  	// Functions for sensor configuration
    void init(void);
    unsigned char readReg(uint8_t address);
  	void setIndoors(bool indoor);
  	bool getIndoors(void);
  	void maskDisturber(bool mask);
  	bool getMaskDisturber(void);
  	void powerDown(bool powerdown);
  	void setNoise(uint8_t noise);
  	uint8_t getNoise(void);
  	void setMinimumStrikes(uint8_t strikes);
  	uint8_t getMinimumStrikes(void);
  	uint8_t getStormDistance(void);
  	void setSpikeRejection(uint8_t value);
  	uint8_t getSpikeRejection(void);
  	void setWDT(uint8_t value);
  	uint8_t getWDT(void);
  	int getInterruptCause(void);
  	void reset(void);
  	void LCOOnIntPin(void);
  	void LCOOffIntPin(void);
  	void setInternalCapacitors(uint8_t value);
  	uint8_t getInternalCapacitors(void);
  	void clearStatistics(void);
};


#endif  /* AS3935_H_ */

