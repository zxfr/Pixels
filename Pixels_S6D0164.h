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
 * Pixels port to Samsung S6D0164 controller (ElecFreaks 2.2 TFT 176*220)
 */

#include "Pixels.h"

#ifndef PIXELS_S6D0164_H
#define PIXELS_S6D0164_H
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
    Pixels() : PixelsBase(176, 220) { // TFT_PQ 2.4 + ITDB02 MEGA Shield v1.1 as defaults
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

    writeCmdData(0x11, 0x001A);
    writeCmdData(0x12, 0x3121);
    writeCmdData(0x13, 0x006C);
    writeCmdData(0x14, 0x4249);
    writeCmdData(0x10, 0x0800);
    delay(10);
    writeCmdData(0x11, 0x011A);
    delay(10);
    writeCmdData(0x11, 0x031A);
    delay(10);
    writeCmdData(0x11, 0x071A);
    delay(10);
    writeCmdData(0x11, 0x0F1A);
    delay(20);
    writeCmdData(0x11, 0x0F3A);
    delay(30);
    /*Initialization set sequence*/
    writeCmdData(0x01, 0x011C);
    writeCmdData(0x02, 0x0100);
    writeCmdData(0x03, 0x1030);
    writeCmdData(0x07, 0x0000);
    writeCmdData(0x08, 0x0808);
    writeCmdData(0x0B, 0x1100);
    writeCmdData(0x0C, 0x0000);
    writeCmdData(0x0F, 0x1401);
    writeCmdData(0x15, 0x0000);
    writeCmdData(0x20, 0x0000);//RAM Address Register AD7-AD0
    writeCmdData(0x21, 0x0000);//RAM Address Register AD15-AD8

//    writeCmdData(0x38, 0x00DB);
//    writeCmdData(0x39, 0x0000);
//    writeCmdData(0x50, 0x0001);//Gamma
//    writeCmdData(0x51, 0x020B);
//    writeCmdData(0x52, 0x0805);
//    writeCmdData(0x53, 0x0404);
//    writeCmdData(0x54, 0x0C0C);
//    writeCmdData(0x55, 0x000C);
//    writeCmdData(0x56, 0x0101);
//    writeCmdData(0x57, 0x0400);
//    writeCmdData(0x58, 0x1108);
//    writeCmdData(0x59, 0x050C);

    writeCmdData(0x31, 0x00DB);
    writeCmdData(0x32, 0x0000);

    writeCmdData(0x36, 0x00AF);//H end
    writeCmdData(0x37, 0x0000);//H start
    writeCmdData(0x38, 0x00DB);//V end
    writeCmdData(0x39, 0x0000);//V start

    writeCmdData(0x0F, 0x0B01);
    writeCmdData(0x07, 0x0016);
    delay(2);
    writeCmdData(0x07, 0x0017);
    writeCmd(0x22);
    delay(10);

    chipDeselect();
}

void Pixels::scrollCmd() {
    chipSelect();
    writeCmd(0x33);
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


    writeCmdData(0x37, x1); // window address
    writeCmdData(0x36, x2);
    writeCmdData(0x39, y1);
    writeCmdData(0x38, y2);
    writeCmdData(0x20, x1); // start address
    writeCmdData(0x21, y1);
    writeCmd(0x22);  // write ram
}
#endif
