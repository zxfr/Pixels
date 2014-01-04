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
#include "SPIsw.h"

#ifndef PIXELS_ST7735_SPI_SW_H
#define PIXELS_ST7735_SPI_SW_H

class PixelsST7735SPIsw : public Pixels, SPIsw {
private:
    uint8_t _scl;
    uint8_t _sda;
    uint8_t _wr;
    uint8_t _cs;
    uint8_t _rst;

protected:
    void deviceWriteData(uint8_t high, uint8_t low);

    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

public:
    PixelsST7735SPIsw(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr);
    PixelsST7735SPIsw(uint16_t width, uint16_t height,uint8_t scl,
                    uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr);

    void init();
};

PixelsST7735SPIsw::PixelsST7735SPIsw(uint8_t scl, uint8_t sda,
                                 uint8_t cs, uint8_t rst, uint8_t wr) : Pixels(128, 160, cs) {
    scrollSupported = true;
    _scl = scl;
    _sda = sda;
    _wr = wr;
    _cs = cs;
    _rst = rst;
}

PixelsST7735SPIsw::PixelsST7735SPIsw(uint16_t width, uint16_t height, uint8_t scl,
                                 uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr) : Pixels( width, height, cs) {
    scrollSupported = true;
    _scl = scl;
    _sda = sda;
    _wr = wr;
    _cs = cs;
    _rst = rst;
}

void PixelsST7735SPIsw::init() {

    digitalWrite(_rst,LOW);
    delay(100);
    digitalWrite(_rst,HIGH);
    delay(100);

    pinMode(_scl,OUTPUT);
    pinMode(_sda,OUTPUT);
    pinMode(_wr,OUTPUT);
    pinMode(_rst,OUTPUT);
    pinMode(_cs,OUTPUT);

    initSPI(_scl, _sda, _cs, _wr);

    CSELECT;
    writeCmd(0x11);
    delay(12);

    writeCmd(0xB1);
    writeData(0x01);
    writeData(0x2C);
    writeData(0x2D);
    writeCmd(0xB2);
    writeData(0x01);
    writeData(0x2C);
    writeData(0x2D);
    writeCmd(0xB3);
    writeData(0x01);
    writeData(0x2C);
    writeData(0x2D);
    writeData(0x01);
    writeData(0x2C);
    writeData(0x2D);

    writeCmd(0xB4);
    writeData(0x07);

    writeCmd(0xC0);
    writeData(0xA2);
    writeData(0x02);
    writeData(0x84);
    writeCmd(0xC1);
    writeData(0xC5);
    writeCmd(0xC2);
    writeData(0x0A);
    writeData(0x00);
    writeCmd(0xC3);
    writeData(0x8A);
    writeData(0x2A);
    writeCmd(0xC4);
    writeData(0x8A);
    writeData(0xEE);

    writeCmd(0xC5);
    writeData(0x0E);

    writeCmd(0x36);
    writeData(0xC8);

    writeCmd(0xe0);
    writeData(0x0f);
    writeData(0x1a);
    writeData(0x0f);
    writeData(0x18);
    writeData(0x2f);
    writeData(0x28);
    writeData(0x20);
    writeData(0x22);
    writeData(0x1f);
    writeData(0x1b);
    writeData(0x23);
    writeData(0x37);
    writeData(0x00);

    writeData(0x07);
    writeData(0x02);
    writeData(0x10);
    writeCmd(0xe1);
    writeData(0x0f);
    writeData(0x1b);
    writeData(0x0f);
    writeData(0x17);
    writeData(0x33);
    writeData(0x2c);
    writeData(0x29);
    writeData(0x2e);
    writeData(0x30);
    writeData(0x30);
    writeData(0x39);
    writeData(0x3f);
    writeData(0x00);
    writeData(0x07);
    writeData(0x03);
    writeData(0x10);

    writeCmd(0x2a);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x7f);
    writeCmd(0x2b);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x9f);

    writeCmd(0xF0);
    writeData(0x01);
    writeCmd(0xF6);
    writeData(0x00);

    writeCmd(0x3A);
    writeData(0x05);
    writeCmd(0x29);

    CDESELECT;
}

void PixelsST7735SPIsw::scrollCmd() {
    CSELECT;
    // the feature seems to be undocumented in the datasheet
    writeCmd(0x37);
    writeData(highByte(deviceHeight - currentScroll));
    writeData(lowByte(deviceHeight - currentScroll));
    CDESELECT;
}

void PixelsST7735SPIsw::setFillDirection(uint8_t direction) {
    fillDirection = direction;
}

void PixelsST7735SPIsw::quickFill (int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    CSELECT;

    setRegion(x1, y1, x2, y2);
    int32_t counter = (int32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

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

    CDESELECT;
}

void PixelsST7735SPIsw::setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    writeCmd(0x2a);
    writeData(x1>>8);
    writeData(x1);
    writeData(x2>>8);
    writeData(x2);
    writeCmd(0x2b);
    writeData(y1>>8);
    writeData(y1);
    writeData(y2>>8);
    writeData(y2);
    writeCmd(0x2c);
}

void PixelsST7735SPIsw::deviceWriteData(uint8_t high, uint8_t low) {
    writeData(high);
    writeData(low);
}
#endif
