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
 * Pixels port to HX8340-B controller, hardware SPI mode, ITDB02-2.2SP
 */

#include "Pixels.h"

#ifndef PIXELS_HX8352_H
#define PIXELS_HX8352_H
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
    void deviceWriteData(uint8_t high, uint8_t low);

    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

public:
    Pixels() : PixelsBase(240, 400) {
        scrollSupported = true;
        setSpiPins(13, 11, 10, 7, 9); // dummy code in PPI case
        setPpiPins(38, 39, 40, 41, 0); // dummy code in SPI case
    }

    Pixels(uint16_t width, uint16_t height) : PixelsBase( width, height) {
        scrollSupported = true;
        setSpiPins(13, 11, 10, 7 ,9); // dummy code in PPI case
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

    writeCmd(0x83);
    writeData(0x00,0x02);  //TESTM=1

    writeCmd(0x85);
    writeData(0x00,0x03);  //VDC_SEL=011
    writeCmd(0x8B);
    writeData(0x00,0x01);
    writeCmd(0x8C);
    writeData(0x00,0x93); //STBA[7]=1,STBA[5:4]=01,STBA[1:0]=11

    writeCmd(0x91);
    writeData(0x00,0x01); //DCDC_SYNC=1

    writeCmd(0x83);
    writeData(0x00,0x00); //TESTM=0
     //Gamma Setting

    writeCmd(0x3E);
    writeData(0x00,0xB0);
    writeCmd(0x3F);
    writeData(0x00,0x03);
    writeCmd(0x40);
    writeData(0x00,0x10);
    writeCmd(0x41);
    writeData(0x00,0x56);
    writeCmd(0x42);
    writeData(0x00,0x13);
    writeCmd(0x43);
    writeData(0x00,0x46);
    writeCmd(0x44);
    writeData(0x00,0x23);
    writeCmd(0x45);
    writeData(0x00,0x76);
    writeCmd(0x46);
    writeData(0x00,0x00);
    writeCmd(0x47);
    writeData(0x00,0x5E);
    writeCmd(0x48);
    writeData(0x00,0x4F);
    writeCmd(0x49);
    writeData(0x00,0x40);
//**********Power On sequence************

    writeCmd(0x17);
    writeData(0x00,0x91);

    writeCmd(0x2B);
    writeData(0x00,0xF9);

    delay(10);

    writeCmd(0x1B);
    writeData(0x00,0x14);

    writeCmd(0x1A);
    writeData(0x00,0x11);

    writeCmd(0x1C);
    writeData(0x00,0x06);	  //0d

    writeCmd(0x1F);
    writeData(0x00,0x42);

    delay(20);

    writeCmd(0x19);
    writeData(0x00,0x0A);

    writeCmd(0x19);
    writeData(0x00,0x1A);

    delay(40);

    writeCmd(0x19);
    writeData(0x00,0x12);

    delay(40);

    writeCmd(0x1E);
    writeData(0x00,0x27);

    delay(100);

   //**********DISPLAY ON SETTING***********

    writeCmd(0x24);
    writeData(0x00,0x60);

    writeCmd(0x3D);
    writeData(0x00,0x40);

    writeCmd(0x34);
    writeData(0x00,0x38);

    writeCmd(0x35);
    writeData(0x00,0x38);

    writeCmd(0x24);
    writeData(0x00,0x38);

    delay(40);

    writeCmd(0x24);
    writeData(0x00,0x3C);

    writeCmd(0x16);
    writeData(0x00,0x1e); // bit2 scroll_on

    writeCmd(0x01);
    writeData(0x00,0x06);

    writeCmd(0x55);
    writeData(0x00,0x00);

    writeCmd(0x02);         // columns 0 - 239
    writeData(0x00,0x00);
    writeCmd(0x03);
    writeData(0x00,0x00);
    writeCmd(0x04);
    writeData(0x00,0x00);
    writeCmd(0x05);
    writeData(0x00,0xef);

    writeCmd(0x06);         // rows 0 - 399
    writeData(0x00,0x00);
    writeCmd(0x07);
    writeData(0x00,0x00);
    writeCmd(0x08);
    writeData(0x00,0x01);
    writeCmd(0x09);
    writeData(0x00,0x8f);

    //**********SCROLL***********

//    writeCmd(0x16);
//    writeData(0x10);

    delay(40);

    writeCmd(0x0E);
    writeData(0x00);
    writeCmd(0x0F);
    writeData(0x00);
    writeCmd(0x10);
    writeData(0x01);
    writeCmd(0x11);
    writeData(0x90);
    writeCmd(0x12);
    writeData(0x00);
    writeCmd(0x13);
    writeData(0x00);

    writeCmd(0x22);

    chipDeselect();
}

void Pixels::scrollCmd() {
    chipSelect();

    writeCmd(0x14);
    writeData(highByte(currentScroll));
    writeCmd(0x15);
    writeData(lowByte(currentScroll));

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

    writeCmdData(0x0003,x1);
    writeCmdData(0x0006,y1>>8);
    writeCmdData(0x0007,y1);
    writeCmdData(0x0005,x2);
    writeCmdData(0x0008,y2>>8);
    writeCmdData(0x0009,y2);
    writeCmd(0x22);
}

void Pixels::deviceWriteData(uint8_t high, uint8_t low) {
    writeData(high, low);
}
#endif
