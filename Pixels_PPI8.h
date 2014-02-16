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
 * Parallel interface 8bit layer
 */

#include "Pixels.h"

#ifdef PIXELS_MAIN
#error Pixels_PPI8.h must be included before Pixels_<CONTROLLER>.h
#endif

#ifndef PIXELS_PPI8_H
#define PIXELS_PPI8_H

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define DATAPORT PORTA
#define DATADIR DDRA
#else
#define DATAPORT PORTD
#define DATADIR DDRD
#endif

class PPI8 {
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
        DATAPORT = 0x00; pulse_low(registerWR, bitmaskWR); DATAPORT = b; pulse_low(registerWR, bitmaskWR);
    }

    void writeData(uint8_t data) {
        sbi(registerRS, bitmaskRS);
        DATAPORT = data; pulse_low(registerWR, bitmaskWR);
    }

    void writeData(uint8_t hi, uint8_t lo) {
        sbi(registerRS, bitmaskRS);
        DATAPORT = hi; pulse_low(registerWR, bitmaskWR); DATAPORT = lo; pulse_low(registerWR, bitmaskWR);
    }

    void writeDataTwice(uint8_t b) {
        DATAPORT = b; pulse_low(registerWR, bitmaskWR); pulse_low(registerWR, bitmaskWR);
    }

    void writeCmdData(uint8_t cmd, uint16_t data) {
        writeCmd(cmd);
        writeData(highByte(data), lowByte(data));
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
};

void PPI8::initInterface() {

    DATADIR = 0xFF;

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

    reset();
}
#endif
