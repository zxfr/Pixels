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
 * Hardware SPI layer (SCL=13, SDA=11 on Arduino)
 */

#include "Pixels.h"

#ifndef PIXELS_SPIHW_H
#define PIXELS_SPIHW_H

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

class SPIhw {
private:
    regtype *registerSCL;
    regtype *registerSDA;
    regtype *registerCSx;
    regsize bitmaskCSx;
    regsize bitmaskSCL;
    regsize bitmaskSDA;

    uint8_t _cs;

protected:
    void writeCmd(uint8_t b);
    void writeData(uint8_t data);

    void beginSPI();
    void endSPI();
    void setSPIBitOrder(uint8_t bitOrder);
    void setSPIDataMode(uint8_t mode);
    void setSPIClockDivider(uint8_t rate);

public:
    SPIhw();
    void initSPI(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t wr);
};

SPIhw::SPIhw() {
}

void SPIhw::initSPI(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t wr) {
    _cs = cs;

    registerSCL	= portOutputRegister(digitalPinToPort(scl));
    bitmaskSCL	= digitalPinToBitMask(scl);
    registerSDA	= portOutputRegister(digitalPinToPort(sda));
    bitmaskSDA	= digitalPinToBitMask(sda);
    registerCSx	= portOutputRegister(digitalPinToPort(10));
    bitmaskCSx	= digitalPinToBitMask(10);

    setSPIBitOrder(MSBFIRST);
    setSPIDataMode(SPI_MODE3);
  //  setSPIClockDivider(SPI_CLOCK_DIV64);
    beginSPI();
}

void SPIhw::writeCmd(uint8_t cmd) {
    SPCR &= ~_BV(SPE); // Disable SPI to get control of the SCK pin.
    cbi(registerSDA, bitmaskSDA);
    cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
  //  delay(1);   // Insert extra time to the hight pulse, typically needed
    sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
    SPCR |= _BV(SPE);      // Enable SPI again

    SPDR = cmd;
    while (!(SPSR & _BV(SPIF)));
}

void SPIhw::writeData(uint8_t data) {
    SPCR &= ~_BV(SPE); // Disable SPI to get control of the SCK pin.
    sbi(registerSDA, bitmaskSDA);
    cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
  //  delay(1);   // Insert extra time to the hight pulse, typically needed
    sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
    SPCR |= _BV(SPE);      // Enable SPI again

    SPDR = data;
    while (!(SPSR & _BV(SPIF)));
}

void SPIhw::beginSPI() {
  cbi(registerSCL, bitmaskSCL);
  cbi(registerSDA, bitmaskSDA);
  digitalWrite(_cs, HIGH);

  // Warning: if the SS pin ever becomes a LOW INPUT then SPI
  // automatically switches to Slave, so the data direction of
  // the SS pin MUST be kept as OUTPUT.
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);
}

void SPIhw::endSPI() {
  SPCR &= ~_BV(SPE);
}

void SPIhw::setSPIBitOrder(uint8_t bitOrder) {
  if(bitOrder == LSBFIRST) {
    SPCR |= _BV(DORD);
  } else {
    SPCR &= ~(_BV(DORD));
  }
}

void SPIhw::setSPIDataMode(uint8_t mode) {
  SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
}

void SPIhw::setSPIClockDivider(uint8_t rate) {
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
}
#endif
