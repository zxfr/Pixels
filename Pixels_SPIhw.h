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
    uint8_t pinSCL;
    uint8_t pinSDA;
    uint8_t pinWR;
    uint8_t pinCS;
    uint8_t pinRST;

    regtype *registerSCL;
    regtype *registerSDA;
    regsize bitmaskSCL;
    regsize bitmaskSDA;

    void beginSPI();
    void endSPI();
    void setSPIBitOrder(uint8_t bitOrder);
    void setSPIDataMode(uint8_t mode);
    void setSPIClockDivider(uint8_t rate);

protected:
    void reset() {
        digitalWrite(pinRST,LOW);
        delay(100);
        digitalWrite(pinRST,HIGH);
        delay(100);
    }

    void writeCmd(uint8_t b);
    void writeData(uint8_t data);

    void writeData(uint8_t hi, uint8_t lo) {
        writeData(hi);
        writeData(lo);
    }

    void writeDataTwice(uint8_t b) {
        writeData(b);
        writeData(b);
    }

    void writeCmdData(uint8_t cmd, uint16_t data) {
        writeCmd(cmd);
        writeData(highByte(data));
        writeData(lowByte(data));
    }

public:
    /**
     * Overrides SPI pins
     * @param scl
     * @param sda
     * @param cs chip select
     * @param rst reset
     * @param wr write pin
     */
    inline void setSpiPins(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr) {
        pinSCL = scl;
        pinSDA = sda;
        pinCS = cs;
        pinRST = rst;
        pinWR = wr;
//        registerCS	= portOutputRegister(digitalPinToPort(cs));
//        bitmaskCS	= digitalPinToBitMask(cs);
    }

    /**
     * Overrides PPI pins
     * @param cs chip select
     */
    inline void setPpiPins(uint8_t rs, uint8_t wr, uint8_t cs, uint8_t rst, uint8_t rd) {
    }

    inline void registerSelect() {
    }

    void initInterface();
};

void SPIhw::initInterface() {
    registerSCL	= portOutputRegister(digitalPinToPort(pinSCL));
    bitmaskSCL	= digitalPinToBitMask(pinSCL);
    registerSDA	= portOutputRegister(digitalPinToPort(pinSDA));
    bitmaskSDA	= digitalPinToBitMask(pinSDA);
    registerCS	= portOutputRegister(digitalPinToPort(pinCS));
    bitmaskCS	= digitalPinToBitMask(pinCS);

    pinMode(pinSCL,OUTPUT);
    pinMode(pinSDA,OUTPUT);
    pinMode(pinWR,OUTPUT);
    pinMode(pinRST,OUTPUT);
    pinMode(pinCS,OUTPUT);

    reset();

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
  digitalWrite(pinCS, HIGH);

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
