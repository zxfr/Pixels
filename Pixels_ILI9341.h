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
 * Pixels port to ILI9341 controller, SPI mode (ElecFreaks TFT2.2SP Shield)
 * SPI is in bit banging mode, as the shield does not connect the hardware SPI (SCL=13, SDA=11)
 * to the display controller
 */

#include "Pixels.h"

#ifndef PIXELS_ILI9341_H
#define PIXELS_ILI9341_H
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

    int32_t setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
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

    writeCmd(0xCB);
    writeData(0x39);
    writeData(0x2C);
    writeData(0x00);
    writeData(0x34);
    writeData(0x02);

    writeCmd(0xCF);
    writeData(0x00);
    writeData(0XC1);
    writeData(0X30);

    writeCmd(0xE8);
    writeData(0x85);
    writeData(0x00);
    writeData(0x78);

    writeCmd(0xEA);
    writeData(0x00);
    writeData(0x00);

    writeCmd(0xED);
    writeData(0x64);
    writeData(0x03);
    writeData(0X12);
    writeData(0X81);

    writeCmd(0xF7);
    writeData(0x20);

    writeCmd(0xC0);    //Power control
    writeData(0x23);   //VRH[5:0]

    writeCmd(0xC1);    //Power control
    writeData(0x10);   //SAP[2:0];BT[3:0]

    writeCmd(0xC5);    //VCM control
    writeData(0x3e);   //Contrast
    writeData(0x28);

    writeCmd(0xC7);    //VCM control2
    writeData(0x86);   //--

    writeCmd(0x36);    // Memory Access Control
    writeData(0x48);   //C8	   //48 68竖屏//28 E8 横屏

    writeCmd(0x3A);
    writeData(0x55);

    writeCmd(0xB1);
    writeData(0x00);
    writeData(0x18);

    writeCmd(0xB6);    // Display Function Control
    writeData(0x08);
    writeData(0x82);
    writeData(0x27);
/*
    writeCmd(0xF2);    // 3Gamma Function Disable
    writeData(0x00);

    writeCmd(0x26);    //Gamma curve selected
    writeData(0x01);

    writeCmd(0xE0);    //Set Gamma
    writeData(0x0F);
    writeData(0x31);
    writeData(0x2B);
    writeData(0x0C);
    writeData(0x0E);
    writeData(0x08);
    writeData(0x4E);
    writeData(0xF1);
    writeData(0x37);
    writeData(0x07);
    writeData(0x10);
    writeData(0x03);
    writeData(0x0E);
    writeData(0x09);
    writeData(0x00);

    writeCmd(0XE1);    //Set Gamma
    writeData(0x00);
    writeData(0x0E);
    writeData(0x14);
    writeData(0x03);
    writeData(0x11);
    writeData(0x07);
    writeData(0x31);
    writeData(0xC1);
    writeData(0x48);
    writeData(0x08);
    writeData(0x0F);
    writeData(0x0C);
    writeData(0x31);
    writeData(0x36);
    writeData(0x0F);
*/
    writeCmd(0x11);    //Exit Sleep
    delay(120);

    writeCmd(0x29);    //Display on
    writeCmd(0x2c);

    chipDeselect();
}

void Pixels::scrollCmd() {
    chipSelect();
    int16_t s = (orientation > 1 ? deviceHeight - currentScroll : currentScroll) % deviceHeight;
    writeCmd(0x37);
    writeData(highByte(s));
    writeData(lowByte(s));
    chipDeselect();
}

void Pixels::setFillDirection(uint8_t direction) {
    fillDirection = direction;
}

void Pixels::quickFill (int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    chipSelect();
    
    int32_t counter = setRegion(x1, y1, x2, y2);
    if( counter == 0 ) {
        return;
    }

    registerSelect();

    uint8_t lo = lowByte(color);
    uint8_t hi = highByte(color);

    for (int16_t i = 0; i < counter / 20; i++) {
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
        writeData(hi);writeData(lo);
    }
    for (int32_t i = 0; i < counter % 20; i++) {
        writeData(hi);writeData(lo);
    }
    chipDeselect();
}

int32_t Pixels::setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    Bounds bb(x1, y1, x2, y2);
    if( !checkBounds(bb) ) {
        return 0;
    }

    writeCmd(0x2a);
    writeData(bb.x1>>8);
    writeData(bb.x1);
    writeData(bb.x2>>8);
    writeData(bb.x2);
    writeCmd(0x2b);
    writeData(y1>>8);
    writeData(bb.y1);
    writeData(bb.y2>>8);
    writeData(bb.y2);
    writeCmd(0x2c);

    return (int32_t)(bb.x2 - bb.x1 + 1) * (bb.y2 - bb.y1 + 1);
}
#endif
