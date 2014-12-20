/*
 * Pixels. Graphics library for TFT displays.
 *
 * Copyright (C) 2012-2013  Igor Repinetski
 *
 * The code is written in C/C++ for Arduino and can be easily ported to any microcontroller by rewritting the low level pin access functions.
 *
 * Text output methods of the library rely on Pixelmeister's font data format. See: http://pd4ml.com/pixelmeister
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License. To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/
 *
 * This library includes some code portions and algoritmic ideas derived from works of
 * - Andreas Schiffler -- aschiffler at ferzkopp dot net (SDL_gfx Project)
 * - K. Townsend http://microBuilder.eu (lpc1343codebase Project)
 */

/*
* Added support for Arduino Due including the 1.5.8 IDE
* Currently tested with Arduino Due + TFT320QVT (SSD1289) + TFT Mega Shield V1.1 by Lseeduino (you can also hook it directly to the Due)
*
* Collaborator: CMBSolutions
* Date: December, 20 2014
* Contact: git@cmbsolutions.nl
*/

/*
 * Parallel interface 16bit layer
 */

#include "Pixels.h"

#ifdef PIXELS_MAIN
	#error Pixels_PPI16.h must be included before Pixels_<CONTROLLER>.h
#endif

#ifndef PIXELS_PPI16_H
#define PIXELS_PPI16_H

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	#define DATAPORTH PORTA // 22-29
	#define DATAPORTL PORTC // 30-37
	#define DATADIRH DDRA
	#define DATADIRL DDRC
#elif defined(__arm__)
	// No defines needed
#else
	// shortage of pins for non-Mega boards
	#define DATAPORTH PORTD // 0-7
	#define DATAPORTL PORTB // 8-13
	#define DATADIRH DDRD
	#define DATADIRL DDRB
#endif

class PPI16 {
private:
    regtype *registerRD;
    regtype *registerWR;
    regtype *registerRST;
    regtype *registerRS;

    regsize bitmaskRD;
    regsize bitmaskWR;
    regsize bitmaskRST;
    regsize bitmaskRS;

    int16_t pinRS;
    int16_t pinWR;
    int16_t pinCS;
    int16_t pinRST;
    int16_t pinRD;

protected:
    void reset() {
        sbi(registerRST, bitmaskRST);
        delay(5);
        cbi(registerRST, bitmaskRST);
        delay(15);
        sbi(registerRST, bitmaskRST);
        delay(15);
    }

    void initInterface();

    void writeCmd(uint8_t b) {
        cbi(registerRS, bitmaskRS);
#if defined(__arm__)
 		LCD_Writ_Bus(0x00, b);
#else
		DATAPORTH = 0; DATAPORTL = b; pulse_low(registerWR, bitmaskWR);
#endif
    }

    void writeData(uint8_t data) {
        sbi(registerRS, bitmaskRS);
#if defined(__arm__)
		LCD_Writ_Bus(0x00, data);
#else
		DATAPORTH = 0; DATAPORTL = data; pulse_low(registerWR, bitmaskWR);
#endif
	}

    void writeData(uint8_t hi, uint8_t lo) {
        sbi(registerRS, bitmaskRS);
#if defined(__arm__)
		LCD_Writ_Bus(hi, lo);
#else
		DATAPORTH = hi; DATAPORTL = lo; pulse_low(registerWR, bitmaskWR);
#endif
	}

    void writeDataTwice(uint8_t b) {
        sbi(registerRS, bitmaskRS);
#if defined(__arm__)
		LCD_Writ_Bus(b, b);
#else
        DATAPORTH = b; DATAPORTL = b; pulse_low(registerWR, bitmaskWR);
#endif
	}

    void writeCmdData(uint8_t cmd, uint16_t data) {
        writeCmd(cmd);
        writeData(highByte(data), lowByte(data));
    }

	// Copy from UTFT Lib.
	void _set_direction_registers()
	{
		REG_PIOA_OER = 0x0000c000; //PA14,PA15 enable
		REG_PIOB_OER = 0x04000000; //PB26 enable
		REG_PIOD_OER = 0x0000064f; //PD0-3,PD6,PD9-10 enable
		REG_PIOA_OER = 0x00000080; //PA7 enable
		REG_PIOC_OER = 0x0000003e; //PC1 - PC5 enable
	}

public:
    /**
     * Overrides SPI pins
     * @param scl
     * @param sda
     * @param cs chip select
     * @param rst reset
     * @param wr write pin
     */
    inline void setSpiPins(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr) {
        // nop
    }

    /**
     * Overrides PPI pins
     * @param cs chip select
     */
    inline void setPpiPins(uint8_t rs, uint8_t wr, uint8_t cs, uint8_t rst, uint8_t rd) {
        pinRS = rs; // 38
        pinWR = wr; // 39
        pinCS = cs; // 40
        pinRST = rst; // 41
        pinRD = rd;
    }

    inline void registerSelect() {
        sbi(registerRS, bitmaskRS);
    }


	// Modified version from UTFT Lib
	inline void LCD_Writ_Bus(char VH, char VL)
	{
		REG_PIOA_CODR = 0x0000C080;
		REG_PIOC_CODR = 0x0000003E;
		REG_PIOD_CODR = 0x0000064F;
		REG_PIOA_SODR = ((VH & 0x06) << 13) | ((VL & 0x40) << 1);
		(VH & 0x01) ? REG_PIOB_SODR = 0x4000000 : REG_PIOB_CODR = 0x4000000;
		REG_PIOC_SODR = ((VL & 0x01) << 5) | ((VL & 0x02) << 3) | ((VL & 0x04) << 1) | ((VL & 0x08) >> 1) | ((VL & 0x10) >> 3);
		REG_PIOD_SODR = ((VH & 0x78) >> 3) | ((VH & 0x80) >> 1) | ((VL & 0x20) << 5) | ((VL & 0x80) << 2);
		pulse_low(registerWR, bitmaskWR);
	}
};

void PPI16::initInterface() {

	
#if defined(__arm__)
	_set_direction_registers();
#else
    DATADIRH = 0xFF;
    DATADIRL = 0xFF;

#endif

    registerRS	= portOutputRegister(digitalPinToPort(pinRS));
    registerWR	= portOutputRegister(digitalPinToPort(pinWR));
    registerCS	= portOutputRegister(digitalPinToPort(pinCS));
    registerRST	= portOutputRegister(digitalPinToPort(pinRST));
    if ( pinRD > 0 ) {
        registerRD	= portOutputRegister(digitalPinToPort(pinRD));
    }

    bitmaskRS	= digitalPinToBitMask(pinRS);
    bitmaskWR	= digitalPinToBitMask(pinWR);
    bitmaskCS	= digitalPinToBitMask(pinCS);
    bitmaskRST	= digitalPinToBitMask(pinRST);
    if ( pinRD > 0 ) {
        bitmaskRD	= digitalPinToBitMask(pinRD);
    }

    pinMode(pinRS,OUTPUT);
    pinMode(pinWR,OUTPUT);
    pinMode(pinCS,OUTPUT);
    pinMode(pinRST,OUTPUT);

#if defined(__arm__)
	_set_direction_registers();
#endif

    reset();
}
#endif
