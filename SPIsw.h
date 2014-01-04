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

#ifndef PIXELS_SPISW_H
#define PIXELS_SPISW_H

class SPIsw {
private:
    regtype *registerSCL;
    regtype *registerSDA;
    regtype *registerWR;
    regsize bitmaskSCL;
    regsize bitmaskSDA;
    regsize bitmaskWR;

    void busWrite(uint8_t data);

protected:
    void writeCmd(uint8_t b);
    void writeData(uint8_t data);

public:
    SPIsw();
    void initSPI(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t wr);
};

SPIsw::SPIsw() {
}

void SPIsw::initSPI(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t wr) {
    registerSCL	= portOutputRegister(digitalPinToPort(scl));
    bitmaskSCL	= digitalPinToBitMask(scl);
    registerSDA	= portOutputRegister(digitalPinToPort(sda));
    bitmaskSDA	= digitalPinToBitMask(sda);
    registerWR	= portOutputRegister(digitalPinToPort(wr));
    bitmaskWR	= digitalPinToBitMask(wr);
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
//  digitalWrite(WR, LOW);
  *registerWR &= ~bitmaskWR;
  busWrite(cmd);
}

void SPIsw::writeData(uint8_t data) {
//  digitalWrite(WR, HIGH);
  *registerWR |= bitmaskWR;
  busWrite(data);
}

#endif
