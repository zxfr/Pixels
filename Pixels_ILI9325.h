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
 * Pixels port to ILI9325 controller (TFT_PQ 2.4, ITDB02 MEGA Shield v1.1, Arduino Mega)
 */
#include <stdio.h>
#include <Arduino.h>
#include <avr/pgmspace.h>

#include "Pixels.h"

#ifndef PIXELS_ILI9325_H
#define PIXELS_ILI9325_H

class PixelsILI9325 : public Pixels {   
private:
    void deviceWriteCmd(uint8_t b);
    void deviceWriteData(uint8_t hi, uint8_t lo);
    void deviceWriteCmdData(uint8_t cmd, uint16_t data);

protected:

    regtype *registerRS; // register select
    regtype *registerWR; // write strobe
    regtype *registerRD; // read strobe
    regtype *registerRST; // reset

    regsize bitmaskRS;
    regsize bitmaskWR;
    regsize bitmaskRD;
    regsize bitmaskRST;

    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

public:
    PixelsILI9325();
    PixelsILI9325(uint16_t width, uint16_t height);

    void init();
};


PixelsILI9325::PixelsILI9325() : Pixels(240, 320, 40) { // CS is hardcoded in the lib version
    scrollSupported = true;
}

PixelsILI9325::PixelsILI9325(uint16_t width, uint16_t height) : Pixels( width, height, 40) {
    scrollSupported = true;
}

void PixelsILI9325::deviceWriteCmd(uint8_t b) {
    cbi(registerRS, bitmaskRS);
    deviceWrite(0x00, b);
}

void PixelsILI9325::deviceWriteData(uint8_t hi, uint8_t lo) {
    sbi(registerRS, bitmaskRS);
    deviceWrite(hi, lo);
}

void PixelsILI9325::deviceWriteCmdData(uint8_t cmd, uint16_t data) {
    deviceWriteCmd(cmd);
    deviceWriteData(highByte(data), lowByte(data));
}

void PixelsILI9325::init() {

    int16_t pinRS = 38;
    int16_t pinWR = 39;
    int16_t pinCS = 40;
    int16_t pinRST = 41;
    int16_t pinRD = -1;

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    DDRA = 0xFF;
#else
    DDRD = 0xFF;
#endif

    registerRS	= portOutputRegister(digitalPinToPort(pinRS));
    registerWR	= portOutputRegister(digitalPinToPort(pinWR));
    registerRST	= portOutputRegister(digitalPinToPort(pinRST));
    if ( pinRD > 0 ) {
        registerRD	= portOutputRegister(digitalPinToPort(pinRD));
    }

    bitmaskRS	= digitalPinToBitMask(pinRS);
    bitmaskWR	= digitalPinToBitMask(pinWR);
    bitmaskRST	= digitalPinToBitMask(pinRST);
    if ( pinRD > 0 ) {
        bitmaskRD	= digitalPinToBitMask(pinRD);
    }

    pinMode(pinRS,OUTPUT);
    pinMode(pinWR,OUTPUT);
    pinMode(pinCS,OUTPUT);
    pinMode(pinRST,OUTPUT);

    sbi(registerRST, bitmaskRST);
    delay(5);
    cbi(registerRST, bitmaskRST);
    delay(15);
    sbi(registerRST, bitmaskRST);
    delay(15);

    CSELECT;

    deviceWriteCmdData(0xE5, 0x78F0);
    deviceWriteCmdData(0x01, 0x0100);
    deviceWriteCmdData(0x02, 0x0700); // line inversion
    deviceWriteCmdData(0x03, 0x1030); // write direction; alternatively 1038
    deviceWriteCmdData(0x04, 0x0000);
    deviceWriteCmdData(0x08, 0x0302);
    deviceWriteCmdData(0x09, 0x0000);
    deviceWriteCmdData(0x0A, 0x0000);

    deviceWriteCmdData(0x0C, 0x0000);
    deviceWriteCmdData(0x0D, 0x0000);
    deviceWriteCmdData(0x0F, 0x0000);

    // Power control
    deviceWriteCmdData(0x10, 0x0000);
    deviceWriteCmdData(0x11, 0x0007);
    deviceWriteCmdData(0x12, 0x0000);
    deviceWriteCmdData(0x13, 0x0000);
    deviceWriteCmdData(0x07, 0x0001);
    delay(220);
    deviceWriteCmdData(0x10, 0x1090);
    deviceWriteCmdData(0x11, 0x0227);
    delay(60);
    deviceWriteCmdData(0x12, 0x001F);
    delay(60);
    deviceWriteCmdData(0x13, 0x1500);

//  Power control alternative
//    deviceWriteCmdData(0x0010, 0x0000);
//    deviceWriteCmdData(0x0011, 0x0007);
//    deviceWriteCmdData(0x0012, 0x0000);
//    deviceWriteCmdData(0x0013, 0x0000);
//    delay(1000);
//    deviceWriteCmdData(0x0010, 0x14B0);
//    delay(500);
//    deviceWriteCmdData(0x0011, 0x0007);
//    delay(500);
//    deviceWriteCmdData(0x0012, 0x008E);
//    deviceWriteCmdData(0x0013, 0x0C00);

    deviceWriteCmdData(0x29, 0x0027); // 0x0015 ?
    deviceWriteCmdData(0x2B, 0x000D); // Frame rate
    delay(50);

    // Gamma tuning
    deviceWriteCmdData(0x0030, 0x0000);
    deviceWriteCmdData(0x0031, 0x0107);
    deviceWriteCmdData(0x0032, 0x0000);
    deviceWriteCmdData(0x0035, 0x0203);
    deviceWriteCmdData(0x0036, 0x0402);
    deviceWriteCmdData(0x0037, 0x0000);
    deviceWriteCmdData(0x0038, 0x0207);
    deviceWriteCmdData(0x0039, 0x0000);
    deviceWriteCmdData(0x003C, 0x0203);
    deviceWriteCmdData(0x003D, 0x0403);

    deviceWriteCmdData(0x20, 0x0000); // GRAM horizontal Address
    deviceWriteCmdData(0x21, 0x0000); // GRAM Vertical Address

    deviceWriteCmdData(0x50, 0x0000); // Window Horizontal RAM Address Start (R50h)
    deviceWriteCmdData(0x51, 0x00EF); // Window Horizontal RAM Address End (R51h)
    deviceWriteCmdData(0x52, 0x0000); // Window Vertical RAM Address Start (R52h)
    deviceWriteCmdData(0x53, 0x013F); // Window Vertical RAM Address End (R53h)
    deviceWriteCmdData(0x60, 0xA700); // Driver Output Control (R60h) - Gate Scan Line
    deviceWriteCmdData(0x61, 0x0003); // Driver Output Control (R61h) - enable VLE
//    deviceWriteCmdData(0x6A, 0x0000); // set initial scrolling

    deviceWriteCmdData(0x90, 0x0010); // Panel Interface Control 1 (R90h)
    deviceWriteCmdData(0x92, 0x0600);
    deviceWriteCmdData(0x07, 0x0133); // RGB565 color

    CDESELECT;
}

void PixelsILI9325::scrollCmd() {
    CSELECT;
    deviceWriteCmd(0x6A);
    deviceWriteData(highByte(currentScroll), lowByte(currentScroll));
    CDESELECT;
}

void PixelsILI9325::setFillDirection(uint8_t direction) {
    fillDirection = direction;
//    if ( order ) {
//        deviceWriteCmdData(0x03, 0x1030);
//    } else {
//        deviceWriteCmdData(0x03, 0x1030);
//    }
}

void PixelsILI9325::quickFill(int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    CSELECT;

    setRegion(x1, y1, x2, y2);
    int32_t counter = (int32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    sbi(registerRS, bitmaskRS);

    uint8_t lo = lowByte(color);
    uint8_t hi = highByte(color);

    if ( lo == hi ) {

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        PORTA = color;
#else
        PORTD = color;
#endif

        for (int16_t i = 0; i < counter / 20; i++) {
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
        }
        for (int32_t i = 0; i < counter % 20; i++) {
            deviceWriteTwice(lo);
        }
    } else {
        for (int16_t i = 0; i < counter / 20; i++) {
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
        }
        for (int32_t i = 0; i < counter % 20; i++) {
            deviceWrite(hi, lo);
        }
    }
    CDESELECT;
}

void PixelsILI9325::setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    if ( orientation != PORTRAIT ) {
        int16_t buf;
        switch( orientation ) {
        case LANDSCAPE:
            buf = x1;
            x1 = deviceWidth - y1 - 1;
            y1 = buf;
            buf = x2;
            x2 = deviceWidth - y2 - 1;
            y2 = buf;
            break;
        case PORTRAIT_FLIP:
            y1 = deviceHeight - y1 - 1;
            y2 = deviceHeight - y2 - 1;
            x1 = deviceWidth - x1 - 1;
            x2 = deviceWidth - x2 - 1;
            break;
        case LANDSCAPE_FLIP:
            buf = y1;
            y1 = deviceHeight - x1 - 1;
            x1 = buf;
            buf = y2;
            y2 = deviceHeight - x2 - 1;
            x2 = buf;
            break;
        }

        if (x2 < x1) {
            swap(x1, x2);
        }
        if (y2 < y1) {
            swap(y1, y2);
        }
    }

    deviceWriteCmdData(0x20,x1);
    deviceWriteCmdData(0x21,y1);
    deviceWriteCmdData(0x50,x1);
    deviceWriteCmdData(0x52,y1);
    deviceWriteCmdData(0x51,x2);
    deviceWriteCmdData(0x53,y2);
    deviceWriteCmd(0x22);
}


#endif
