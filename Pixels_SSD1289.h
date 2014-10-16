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
 * Pixels port to SSD1289 controller, PPI16 mode (TFT_320QVT)
 */

#include "Pixels.h"

#ifndef PIXELS_SSD1289_H
#define PIXELS_SSD1289_H
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
    void deviceWriteData(uint8_t high, uint8_t low) {
        writeData(high, low);
    }

    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

public:
    Pixels() : PixelsBase(240, 320) { // ElecFreaks TFT2.2SP shield as default
        scrollSupported = true;
        setSpiPins(4, 3, 7, 5, 6); // dummy code in PPI case
        setPpiPins(38, 39, 40, 41, 0); // dummy code in SPI case
    }

    Pixels(uint16_t width, uint16_t height) : PixelsBase( width, height) {
        scrollSupported = true;
        setSpiPins(4, 3, 7, 5, 6); // dummy code in PPI case
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

    writeCmdData(0x00,0x0001);
    writeCmdData(0x03,0xA8A4);
    writeCmdData(0x0C,0x0000);
    writeCmdData(0x0D,0x080C);
    writeCmdData(0x0E,0x2B00);
    writeCmdData(0x1E,0x00B7);
    writeCmdData(0x01,0x2B3F);
    writeCmdData(0x02,0x0600);
    writeCmdData(0x10,0x0000);
    writeCmdData(0x11,0x6070);
    writeCmdData(0x05,0x0000);
    writeCmdData(0x06,0x0000);
    writeCmdData(0x16,0xEF1C);
    writeCmdData(0x17,0x0003);
    writeCmdData(0x07,0x0233);
    writeCmdData(0x0B,0x0000);
    writeCmdData(0x0F,0x0000);
    writeCmdData(0x41,0x0000);
    writeCmdData(0x42,0x0000);
    writeCmdData(0x48,0x0000);
    writeCmdData(0x49,0x013F);
    writeCmdData(0x4A,0x0000);
    writeCmdData(0x4B,0x0000);
    writeCmdData(0x44,0xEF00);
    writeCmdData(0x45,0x0000);
    writeCmdData(0x46,0x013F);
    writeCmdData(0x30,0x0707);
    writeCmdData(0x31,0x0204);
    writeCmdData(0x32,0x0204);
    writeCmdData(0x33,0x0502);
    writeCmdData(0x34,0x0507);
    writeCmdData(0x35,0x0204);
    writeCmdData(0x36,0x0204);
    writeCmdData(0x37,0x0502);
    writeCmdData(0x3A,0x0302);
    writeCmdData(0x3B,0x0302);
    writeCmdData(0x23,0x0000);
    writeCmdData(0x24,0x0000);
    writeCmdData(0x25,0x8000);
    writeCmdData(0x4f,0x0000);
    writeCmdData(0x4e,0x0000);
    writeCmd(0x22);

    chipDeselect();
}

void Pixels::scrollCmd() {
    chipSelect();
    writeCmd(0x41);
    deviceWriteData(highByte(currentScroll), lowByte(currentScroll));
    chipDeselect();
}

void Pixels::setFillDirection(uint8_t direction) {
    fillDirection = direction;
}

void Pixels::quickFill (int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    chipSelect();

    setRegion(x1, y1, x2, y2);
    int32_t counter = (int32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    registerSelect();

    uint8_t lo = lowByte(color);
    uint8_t hi = highByte(color);

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

    writeCmdData(0x44,(x2<<8)+x1);
    writeCmdData(0x45,y1);
    writeCmdData(0x46,y2);
    writeCmdData(0x4e,x1);
    writeCmdData(0x4f,y1);
    writeCmd(0x22);
}
#endif
