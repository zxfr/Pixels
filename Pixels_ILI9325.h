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
//#include <stdio.h>
//#include <Arduino.h>
//#include <avr/pgmspace.h>

#include "Pixels.h"

#ifndef PIXELS_ILI9325_H
#define PIXELS_ILI9325_H
#define PIXELS_MAIN

#if defined(PIXELS_ANTIALIASING_H)
#define PixelsBase PixelsAntialiased
#endif

class Pixels : public PixelsBase
#if defined(PIXELS_SPISW_H)
                            , public SPIsw
#elif defined(PIXELS_SPIHW_H)
                            , public SPIhw
#elif defined(PIXELS_PPI8_H)
                            , public PPI8
#elif defined(PIXELS_PPI16_H)
                            , public PPI16
#endif
{
protected:
    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

    void deviceWriteData(uint8_t high, uint8_t low) {
        writeData(high, low);
    }
public:
    Pixels() : PixelsBase(240, 320) { // TFT_PQ 2.4 + ITDB02 MEGA Shield v1.1 as defaults
        scrollSupported = true;
        setSpiPins(6, 7, 5, 3 ,4); // dummy code in PPI case // TODO
        setPpiPins(38, 39, 40, 41, 0); // dummy code in SPI case
    }

    Pixels(uint16_t width, uint16_t height) : PixelsBase(width, height) {
        scrollSupported = true;
        setSpiPins(6, 7, 5, 3 ,4); // dummy code in PPI case // TODO
        setPpiPins(38, 39, 40, 41, 0); // dummy code in SPI case
    }

    void init();
};

#if defined(PIXELS_ANTIALIASING_H)
#undef PixelsBase
#endif

void Pixels::init() {

    initInterface();

    chipSelect();

    writeCmdData(0xE5, 0x78F0);
    writeCmdData(0x01, 0x0100);
    writeCmdData(0x02, 0x0700); // line inversion
    writeCmdData(0x03, 0x1030); // write direction; alternatively 1038
    writeCmdData(0x04, 0x0000);
    writeCmdData(0x08, 0x0302);
    writeCmdData(0x09, 0x0000);
    writeCmdData(0x0A, 0x0000);

    writeCmdData(0x0C, 0x0000);
    writeCmdData(0x0D, 0x0000);
    writeCmdData(0x0F, 0x0000);

    // Power control
    writeCmdData(0x10, 0x0000);
    writeCmdData(0x11, 0x0007);
    writeCmdData(0x12, 0x0000);
    writeCmdData(0x13, 0x0000);
    writeCmdData(0x07, 0x0001);
    delay(220);
    writeCmdData(0x10, 0x1090);
    writeCmdData(0x11, 0x0227);
    delay(60);
    writeCmdData(0x12, 0x001F);
    delay(60);
    writeCmdData(0x13, 0x1500);

//  Power control alternative
//    writeCmdData(0x0010, 0x0000);
//    writeCmdData(0x0011, 0x0007);
//    writeCmdData(0x0012, 0x0000);
//    writeCmdData(0x0013, 0x0000);
//    delay(1000);
//    writeCmdData(0x0010, 0x14B0);
//    delay(500);
//    writeCmdData(0x0011, 0x0007);
//    delay(500);
//    writeCmdData(0x0012, 0x008E);
//    writeCmdData(0x0013, 0x0C00);

    writeCmdData(0x29, 0x0027); // 0x0015 ?
    writeCmdData(0x2B, 0x000D); // Frame rate
    delay(50);

    // Gamma tuning
    writeCmdData(0x0030, 0x0000);
    writeCmdData(0x0031, 0x0107);
    writeCmdData(0x0032, 0x0000);
    writeCmdData(0x0035, 0x0203);
    writeCmdData(0x0036, 0x0402);
    writeCmdData(0x0037, 0x0000);
    writeCmdData(0x0038, 0x0207);
    writeCmdData(0x0039, 0x0000);
    writeCmdData(0x003C, 0x0203);
    writeCmdData(0x003D, 0x0403);

    writeCmdData(0x20, 0x0000); // GRAM horizontal Address
    writeCmdData(0x21, 0x0000); // GRAM Vertical Address

    writeCmdData(0x50, 0x0000); // Window Horizontal RAM Address Start (R50h)
    writeCmdData(0x51, 0x00EF); // Window Horizontal RAM Address End (R51h)
    writeCmdData(0x52, 0x0000); // Window Vertical RAM Address Start (R52h)
    writeCmdData(0x53, 0x013F); // Window Vertical RAM Address End (R53h)
    writeCmdData(0x60, 0xA700); // Driver Output Control (R60h) - Gate Scan Line
    writeCmdData(0x61, 0x0003); // Driver Output Control (R61h) - enable VLE
//    writeCmdData(0x6A, 0x0000); // set initial scrolling

    writeCmdData(0x90, 0x0010); // Panel Interface Control 1 (R90h)
    writeCmdData(0x92, 0x0600);
    writeCmdData(0x07, 0x0133); // RGB565 color

    chipDeselect();
}

void Pixels::scrollCmd() {
    chipSelect();
    writeCmd(0x6A);
    deviceWriteData(highByte(currentScroll), lowByte(currentScroll));
    chipDeselect();
}

void Pixels::setFillDirection(uint8_t direction) {
    fillDirection = direction;
//    if ( order ) {
//        writeCmdData(0x03, 0x1030);
//    } else {
//        writeCmdData(0x03, 0x1030);
//    }
}

void Pixels::quickFill(int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    chipSelect();

    setRegion(x1, y1, x2, y2);
    int32_t counter = (int32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    registerSelect();

    uint8_t lo = lowByte(color);
    uint8_t hi = highByte(color);

    if ( lo == hi ) {
        for (int16_t i = 0; i < counter / 20; i++) {
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
            writeDataTwice(lo);
        }
        for (int32_t i = 0; i < counter % 20; i++) {
            writeDataTwice(lo);
        }
    } else {
        for (int16_t i = 0; i < counter / 20; i++) {
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
            writeData(hi, lo);
        }
        for (int32_t i = 0; i < counter % 20; i++) {
            writeData(hi, lo);
        }
    }
    chipDeselect();
}

void Pixels::setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

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

    writeCmdData(0x20,x1);
    writeCmdData(0x21,y1);
    writeCmdData(0x50,x1);
    writeCmdData(0x52,y1);
    writeCmdData(0x51,x2);
    writeCmdData(0x53,y2);
    writeCmd(0x22);
}
#endif
