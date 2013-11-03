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
 * Pixels port to HX8340-B controller, SPI mode, ITDB02-2.2SP
 */

#include "Pixels.h"

#ifndef PIXELS_HX8340SPI_H
#define PIXELS_HX8340SPI_H

#define SCL 13  // SCK
#define SDA 11  // MOSI
#define RS 8  // not used
#define WR 9  // not used
#define CS 10  // SS
#define RST 7

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06
#define SPI_CLOCK_DIV64 0x07

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

#define SPI(X) SPDR=X;while(!(SPSR&_BV(SPIF)))

class PixelsHX8340SPI : public Pixels {
private:
    void deviceWriteCmd(uint8_t b);
    void deviceWriteData(uint8_t data);

    void beginSPI();
    void endSPI();
    void setSPIBitOrder(uint8_t bitOrder);
    void setSPIDataMode(uint8_t mode);
    void setSPIClockDivider(uint8_t rate);

protected:
    regtype *registerSCL;
    regtype *registerSDA;

    regsize bitmaskSCL;
    regsize bitmaskSDA;

    void deviceWriteData(uint8_t high, uint8_t low);

    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

public:
    PixelsHX8340SPI();
    PixelsHX8340SPI(uint16_t width, uint16_t height);

    void init();
};

PixelsHX8340SPI::PixelsHX8340SPI() {
    deviceWidth = 176;
    deviceHeight = 220;
}

PixelsHX8340SPI::PixelsHX8340SPI(uint16_t width, uint16_t height) : Pixels( width, height) {
    scrollSupported = true;
}

void PixelsHX8340SPI::init() {

    registerCS	= portOutputRegister(digitalPinToPort(CS));
    bitmaskCS	= digitalPinToBitMask(CS);
    registerSCL	= portOutputRegister(digitalPinToPort(SCL));
    bitmaskSCL	= digitalPinToBitMask(SCL);
    registerSDA	= portOutputRegister(digitalPinToPort(SDA));
    bitmaskSDA	= digitalPinToBitMask(SDA);

    digitalWrite(RST,LOW);
    delay(100);
    digitalWrite(RST,HIGH);
    delay(100);

    setSPIBitOrder(MSBFIRST);
    setSPIDataMode(SPI_MODE3);
  //  setSPIClockDivider(SPI_CLOCK_DIV64);
    beginSPI();

    pinMode(0,OUTPUT);
    pinMode(1,OUTPUT);
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
    pinMode(6,OUTPUT);
    pinMode(7,OUTPUT);
    pinMode(8,OUTPUT);
    pinMode(9,OUTPUT);
    pinMode(10,OUTPUT);
    pinMode(11,OUTPUT);
    pinMode(12,OUTPUT);
    pinMode(13,OUTPUT);

    CSELECT;

    deviceWriteCmd(0xC1);
    deviceWriteData(0xFF);
    deviceWriteData(0x83);
    deviceWriteData(0x40);
    deviceWriteCmd(0x11);

    delay(100);

    deviceWriteCmd(0xCA);
    deviceWriteData(0x70);
    deviceWriteData(0x00);
    deviceWriteData(0xD9);
    deviceWriteData(0x01);
    deviceWriteData(0x11);

    deviceWriteCmd(0xC9);
    deviceWriteData(0x90);
    deviceWriteData(0x49);
    deviceWriteData(0x10);
    deviceWriteData(0x28);
    deviceWriteData(0x28);
    deviceWriteData(0x10);
    deviceWriteData(0x00);
    deviceWriteData(0x06);

    delay(20);

    deviceWriteCmd(0xC2);
    deviceWriteData(0x60);
    deviceWriteData(0x71);
    deviceWriteData(0x01);
    deviceWriteData(0x0E);
    deviceWriteData(0x05);
    deviceWriteData(0x02);
    deviceWriteData(0x09);
    deviceWriteData(0x31);
    deviceWriteData(0x0A);

    deviceWriteCmd(0xc3);
    deviceWriteData(0x67);
    deviceWriteData(0x30);
    deviceWriteData(0x61);
    deviceWriteData(0x17);
    deviceWriteData(0x48);
    deviceWriteData(0x07);
    deviceWriteData(0x05);
    deviceWriteData(0x33);

    delay(10);

    deviceWriteCmd(0xB5);
    deviceWriteData(0x35);
    deviceWriteData(0x20);
    deviceWriteData(0x45);

    deviceWriteCmd(0xB4);
    deviceWriteData(0x33);
    deviceWriteData(0x25);
    deviceWriteData(0x4c);

    delay(10);

    deviceWriteCmd(0x3a);
    deviceWriteData(0x05);
    deviceWriteCmd(0x29);

    delay(10);

    deviceWriteCmd(0x33);
    deviceWriteData(0x00);
    deviceWriteData(0x00);
    deviceWriteData(0x00);
    deviceWriteData(0xdc);
    deviceWriteData(0x00);
    deviceWriteData(0x00);

    deviceWriteCmd(0x2a);
    deviceWriteData(0x00);
    deviceWriteData(0x00);
    deviceWriteData(0x00);
    deviceWriteData(0xaf);
    deviceWriteCmd(0x2b);
    deviceWriteData(0x00);
    deviceWriteData(0x00);
    deviceWriteData(0x00);
    deviceWriteData(0xdb);

    deviceWriteCmd(0x2c);

    CDESELECT;
}

void PixelsHX8340SPI::scrollCmd() {
    CSELECT;
    deviceWriteCmd(0x37);
    deviceWriteData(highByte(currentScroll), lowByte(currentScroll));
    CDESELECT;
}

void PixelsHX8340SPI::setFillDirection(uint8_t direction) {
    fillDirection = direction;
}

void PixelsHX8340SPI::quickFill (int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    CSELECT;

    setRegion(x1, y1, x2, y2);
    int32_t counter = (int32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    uint8_t lo = lowByte(color);
    uint8_t hi = highByte(color);

    for (int16_t i = 0; i < counter / 20; i++) {
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
        deviceWriteData(hi);deviceWriteData(lo);
    }
    for (int32_t i = 0; i < counter % 20; i++) {
        deviceWriteData(hi);deviceWriteData(lo);
    }

    CDESELECT;
}

void PixelsHX8340SPI::setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    deviceWriteCmd(0x2a);
    deviceWriteData(x1>>8);
    deviceWriteData(x1);
    deviceWriteData(x2>>8);
    deviceWriteData(x2);
    deviceWriteCmd(0x2b);
    deviceWriteData(y1>>8);
    deviceWriteData(y1);
    deviceWriteData(y2>>8);
    deviceWriteData(y2);
    deviceWriteCmd(0x2c);
}

void PixelsHX8340SPI::deviceWriteCmd(uint8_t cmd) {
    SPCR &= ~_BV(SPE); // Disable SPI to get control of the SCK pin.
    cbi(registerSDA, bitmaskSDA);
    cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
  //  delay(1);   // Insert extra time to the hight pulse, typically needed
    sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
    SPCR |= _BV(SPE);      // Enable SPI again

    SPDR = cmd;
    while (!(SPSR & _BV(SPIF)));
}

void PixelsHX8340SPI::deviceWriteData(uint8_t data) {
    SPCR &= ~_BV(SPE); // Disable SPI to get control of the SCK pin.
    sbi(registerSDA, bitmaskSDA);
    cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
  //  delay(1);   // Insert extra time to the hight pulse, typically needed
    sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
    SPCR |= _BV(SPE);      // Enable SPI again

    SPDR = data;
    while (!(SPSR & _BV(SPIF)));
}

void PixelsHX8340SPI::deviceWriteData(uint8_t high, uint8_t low) {
    deviceWriteData(high);
    deviceWriteData(low);
}

void PixelsHX8340SPI::beginSPI() {
  // Set direction register for SCK and MOSI pin.
  // MISO pin automatically overrides to INPUT.
  // When the SS pin is set as OUTPUT, it can be used as
  // a general purpose output port (it doesn't influence
  // SPI operations).

  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SS, OUTPUT);

  cbi(registerSCL, bitmaskSCL);
  cbi(registerSDA, bitmaskSDA);
  CDESELECT;

  // Warning: if the SS pin ever becomes a LOW INPUT then SPI
  // automatically switches to Slave, so the data direction of
  // the SS pin MUST be kept as OUTPUT.
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);
}

void PixelsHX8340SPI::endSPI() {
  SPCR &= ~_BV(SPE);
}

void PixelsHX8340SPI::setSPIBitOrder(uint8_t bitOrder) {
  if(bitOrder == LSBFIRST) {
    SPCR |= _BV(DORD);
  } else {
    SPCR &= ~(_BV(DORD));
  }
}

void PixelsHX8340SPI::setSPIDataMode(uint8_t mode) {
  SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
}

void PixelsHX8340SPI::setSPIClockDivider(uint8_t rate) {
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
}

#endif
