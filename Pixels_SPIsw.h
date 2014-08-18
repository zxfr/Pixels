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
 * Software (bit bang) SPI layer
 */

#include "Pixels.h"

#ifdef PIXELS_MAIN
#error Pixels_SPIsw.h must be included before Pixels_<CONTROLLER>.h
#endif

#ifndef PIXELS_SPISW_H
#define PIXELS_SPISW_H

bool eightBit = true;

class SPIsw {
private:
    uint8_t pinSCL;
    uint8_t pinSDA;
    uint8_t pinWR;
    uint8_t pinCS;
    uint8_t pinRST;

    regtype *registerSCL;
    regtype *registerSDA;
    regtype *registerWR;
    regsize bitmaskSCL;
    regsize bitmaskSDA;
    regsize bitmaskWR;

    void busWrite(uint8_t data);

protected:
    void reset() {
        digitalWrite(pinRST,LOW);
        delay(100);
        digitalWrite(pinRST,HIGH);
        delay(100);
    }

    void writeCmd(uint8_t b);
    void writeData(uint8_t data);

    void writeData(uint8_t hi, uint8_t lo) {
        writeData(hi);
        writeData(lo);
    }

    void writeDataTwice(uint8_t b) {
        writeData(b);
        writeData(b);
    }

    void writeCmdData(uint8_t cmd, uint16_t data) {
        writeCmd(cmd);
        writeData(highByte(data));
        writeData(lowByte(data));
    }

public:
    void initInterface();

//    void setSPIEightBit(bool bits) {
//        eightBit = bits;
//    }

    /**
     * Overrides SPI pins
     * @param scl
     * @param sda
     * @param cs chip select
     * @param rst reset
     * @param wr write pin; if not omitted (and not equals to 255) - switches to eight bit mode transfer
     */
    inline void setSpiPins(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr = 255) {
        pinSCL = scl;
        pinSDA = sda;
        pinCS = cs;
        pinRST = rst;
        pinWR = wr;
        eightBit = wr != 255;
    }

    /**
     * Overrides PPI pins
     * @param cs chip select
     */
    inline void setPpiPins(uint8_t rs, uint8_t wr, uint8_t cs, uint8_t rst, uint8_t rd) {
    }

    inline void registerSelect() {
    }
};

void SPIsw::initInterface() {
    registerSCL	= portOutputRegister(digitalPinToPort(pinSCL));
    bitmaskSCL	= digitalPinToBitMask(pinSCL);
    registerSDA	= portOutputRegister(digitalPinToPort(pinSDA));
    bitmaskSDA	= digitalPinToBitMask(pinSDA);
    registerWR	= portOutputRegister(digitalPinToPort(pinWR));
    bitmaskWR	= digitalPinToBitMask(pinWR);
    registerCS	= portOutputRegister(digitalPinToPort(pinCS));
    bitmaskCS	= digitalPinToBitMask(pinCS);

    pinMode(pinSCL,OUTPUT);
    pinMode(pinSDA,OUTPUT);
    pinMode(pinWR,OUTPUT);
    pinMode(pinRST,OUTPUT);
    pinMode(pinCS,OUTPUT);

    reset();
}

void SPIsw::busWrite(uint8_t data) {
    uint8_t c = 8;
    do {
        if (data & 0x80) {
            *registerSDA |= bitmaskSDA;
        } else {
           *registerSDA &= ~bitmaskSDA;
        }
        data = data<<1;
        *registerSCL &= ~bitmaskSCL;
        asm ("nop");
        *registerSCL |= bitmaskSCL;
    } while (--c > 0);
}

void SPIsw::writeCmd(uint8_t cmd) {
    if ( eightBit ) {
        *registerWR &= ~bitmaskWR;
    } else {
        cbi(registerSDA, bitmaskSDA);
        cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
        sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
    }
    busWrite(cmd);
}

void SPIsw::writeData(uint8_t data) {
    if ( eightBit ) {
        *registerWR |= bitmaskWR;
    } else {
        sbi(registerSDA, bitmaskSDA);
        cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
        sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
    }
    busWrite(data);
}
#endif
