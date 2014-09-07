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
 * Pixels port to ILI9163 controller, SPI mode (Arduino Robot LCD)
 */

#include "Pixels.h"

#ifndef PIXELS_ILI9163_H
#define PIXELS_ILI9163_H
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
//    {
//        writeData(high, low);
//    }

    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

public:
    Pixels() : PixelsBase(128, 160) { // ElecFreaks TFT2.2SP shield as default
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

// ILI9163 LCD Controller Commands
#define NOP                     0x00
#define SOFT_RESET              0x01
#define GET_RED_CHANNEL         0x06
#define GET_GREEN_CHANNEL       0x07
#define GET_BLUE_CHANNEL        0x08
#define GET_PIXEL_FORMAT        0x0C
#define GET_POWER_MODE          0x0A
#define GET_ADDRESS_MODE        0x0B
#define GET_DISPLAY_MODE        0x0D
#define GET_SIGNAL_MODE         0x0E
#define GET_DIAGNOSTIC_RESULT   0x0F
#define ENTER_SLEEP_MODE        0x10
#define EXIT_SLEEP_MODE         0x11
#define ENTER_PARTIAL_MODE      0x12
#define ENTER_NORMAL_MODE       0x13
#define EXIT_INVERT_MODE        0x20
#define ENTER_INVERT_MODE       0x21
#define SET_GAMMA_CURVE         0x26
#define SET_DISPLAY_OFF         0x28
#define SET_DISPLAY_ON          0x29
#define SET_COLUMN_ADDRESS      0x2A
#define SET_PAGE_ADDRESS        0x2B
#define WRITE_MEMORY_START      0x2C
#define WRITE_LUT               0x2D
#define READ_MEMORY_START       0x2E
#define SET_PARTIAL_AREA        0x30
#define SET_SCROLL_AREA         0x33
#define SET_TEAR_OFF            0x34
#define SET_TEAR_ON             0x35
#define SET_ADDRESS_MODE        0x36
#define SET_SCROLL_START        0X37
#define EXIT_IDLE_MODE          0x38
#define ENTER_IDLE_MODE         0x39
#define SET_PIXEL_FORMAT        0x3A
#define WRITE_MEMORY_CONTINUE   0x3C
#define READ_MEMORY_CONTINUE    0x3E
#define SET_TEAR_SCANLINE       0x44
#define GET_SCANLINE            0x45
#define READ_ID1                0xDA
#define READ_ID2                0xDB
#define READ_ID3                0xDC
#define FRAME_RATE_CONTROL1     0xB1
#define FRAME_RATE_CONTROL2     0xB2
#define FRAME_RATE_CONTROL3     0xB3
#define DISPLAY_INVERSION       0xB4
#define SOURCE_DRIVER_DIRECTION 0xB7
#define GATE_DRIVER_DIRECTION   0xB8
#define POWER_CONTROL1          0xC0
#define POWER_CONTROL2          0xC1
#define POWER_CONTROL3          0xC2
#define POWER_CONTROL4          0xC3
#define POWER_CONTROL5          0xC4
#define VCOM_CONTROL1           0xC5
#define VCOM_CONTROL2           0xC6
#define VCOM_OFFSET_CONTROL     0xC7
#define WRITE_ID4_VALUE         0xD3
#define NV_MEMORY_FUNCTION1     0xD7
#define NV_MEMORY_FUNCTION2     0xDE
#define POSITIVE_GAMMA_CORRECT  0xE0
#define NEGATIVE_GAMMA_CORRECT  0xE1
#define GAM_R_SEL               0xF2



void Pixels::init() {

    initInterface();

    chipSelect();


    writeCmd(EXIT_SLEEP_MODE);

    writeCmd(SET_PIXEL_FORMAT);
    writeData(0x05); // 16 bits per pixel

    writeCmd(SET_GAMMA_CURVE);
    writeData(0x04); // Select gamma curve 3

    writeCmd(GAM_R_SEL);
    writeData(0x01); // Gamma adjustment enabled

    writeCmd(POSITIVE_GAMMA_CORRECT);
    writeData(0x3f); // 1st Parameter
    writeData(0x25); // 2nd Parameter
    writeData(0x1c); // 3rd Parameter
    writeData(0x1e); // 4th Parameter
    writeData(0x20); // 5th Parameter
    writeData(0x12); // 6th Parameter
    writeData(0x2a); // 7th Parameter
    writeData(0x90); // 8th Parameter
    writeData(0x24); // 9th Parameter
    writeData(0x11); // 10th Parameter
    writeData(0x00); // 11th Parameter
    writeData(0x00); // 12th Parameter
    writeData(0x00); // 13th Parameter
    writeData(0x00); // 14th Parameter
    writeData(0x00); // 15th Parameter

    writeCmd(NEGATIVE_GAMMA_CORRECT);
    writeData(0x20); // 1st Parameter
    writeData(0x20); // 2nd Parameter
    writeData(0x20); // 3rd Parameter
    writeData(0x20); // 4th Parameter
    writeData(0x05); // 5th Parameter
    writeData(0x00); // 6th Parameter
    writeData(0x15); // 7th Parameter
    writeData(0xa7); // 8th Parameter
    writeData(0x3d); // 9th Parameter
    writeData(0x18); // 10th Parameter
    writeData(0x25); // 11th Parameter
    writeData(0x2a); // 12th Parameter
    writeData(0x2b); // 13th Parameter
    writeData(0x2b); // 14th Parameter
    writeData(0x3a); // 15th Parameter

    writeCmd(FRAME_RATE_CONTROL1);
    writeData(0x08); // DIVA = 8
    writeData(0x08); // VPA = 8

    writeCmd(DISPLAY_INVERSION);
    writeData(0x07); // NLA = 1, NLB = 1, NLC = 1 (all on Frame Inversion)

    writeCmd(POWER_CONTROL1);
    writeData(0x0a); // VRH = 10:  GVDD = 4.30
    writeData(0x02); // VC = 2: VCI1 = 2.65

    writeCmd(POWER_CONTROL2);
    writeData(0x02); // BT = 2: AVDD = 2xVCI1, VCL = -1xVCI1, VGH = 5xVCI1, VGL = -2xVCI1

    writeCmd(VCOM_CONTROL1);
    writeData(0x50); // VMH = 80: VCOMH voltage = 4.5
    writeData(0x5b); // VML = 91: VCOML voltage = -0.225

    writeCmd(VCOM_OFFSET_CONTROL);
    writeData(0x40); // nVM = 0, VMF = 64: VCOMH output = VMH, VCOML output = VML

    writeCmd(SET_COLUMN_ADDRESS);
    writeData(0x00); // XSH
    writeData(0x00); // XSL
    writeData(0x00); // XEH
    writeData(0x7f); // XEL (128 pixels x)
    delay(250); // Wait for the screen to wake up

    writeCmd(SET_PAGE_ADDRESS);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    writeData(0x9f); // 160 pixels y

//    // Select display orientation
    writeCmd(SET_ADDRESS_MODE);
    writeData(0xC8); // 0xC8 - BGR

    // Set the display to on
    writeCmd(SET_DISPLAY_ON);

    chipDeselect();
}

void Pixels::scrollCmd() {
    chipSelect();
    // the feature seems to be undocumented in the datasheet
    writeCmd(0x37);
    writeData(highByte(deviceHeight - currentScroll));
    writeData(lowByte(deviceHeight - currentScroll));
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

void Pixels::deviceWriteData(uint8_t high, uint8_t low) {
    writeData(high);
    writeData(low);
}
#endif
