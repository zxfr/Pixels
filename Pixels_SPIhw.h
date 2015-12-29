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

#if defined(ESP8266)
 #include <SPI.h>
#endif

#ifdef PIXELS_MAIN
#error Pixels_SPIhw.h must be included before Pixels_<CONTROLLER>.h
#endif

#ifndef PIXELS_SPIHW_H
#define PIXELS_SPIHW_H

#ifndef ESP8266
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

  #define SPI(X) SPDR=X;while(!(SPSR&_BV(SPIF)))

#endif 

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR


class SPIhw {
private:
    uint8_t pinSCL;
    uint8_t pinSDA;
    uint8_t pinWR;
    uint8_t pinCS;
    uint8_t pinRST;

    regtype *registerSCL;
    regtype *registerSDA;
    regtype *registerWR;
    regsize bitmaskSCL;
    regsize bitmaskSDA;
    regsize bitmaskWR;

    void beginSPI();
    void endSPI();

    bool eightBit;
    bool hwCS;

protected:
    void reset() {
        digitalWrite(pinRST,LOW);
        delay(100);
        digitalWrite(pinRST,HIGH);
        delay(100);
    }

    void writeCmd(uint8_t b);
    __attribute__((noinline)) void writeData(uint8_t data); // noinline saves 4-5kb sketch code in the case. An impact to performance is to be learned.

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
    void setSPIBitOrder(uint8_t bitOrder);
    void setSPIDataMode(uint8_t mode);
    void setSPIClockDivider(uint8_t rate);
    void setHwCs(bool useHwCs) {
      hwCS = useHwCs;
    }
//    void setSPIEightBit(bool bits) {
//        eightBit = bits;
//    }

    /**
     * Overrides SPI pins
     * @param scl
     * @param sda
     * @param cs chip select
     * @param rst reset
     * @param wr write pin; if not omitted (and not equals to 255) - switches to eight bit mode transfer
     */
    inline void setSpiPins(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr = 255) {
        pinSCL = scl;
        pinSDA = sda;
        pinCS = cs;
        pinRST = rst;
        pinWR = wr;
        eightBit = wr != 255;
        hwCS = true;
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
    registerWR	= portOutputRegister(digitalPinToPort(pinWR));
    bitmaskWR	= digitalPinToBitMask(pinWR);
    registerCS	= portOutputRegister(digitalPinToPort(pinCS));
    bitmaskCS	= digitalPinToBitMask(pinCS);

    pinMode(pinSCL,OUTPUT);
    pinMode(pinSDA,OUTPUT);
    pinMode(pinWR,OUTPUT);
    pinMode(pinRST,OUTPUT);
    pinMode(pinCS,OUTPUT);

    reset();

    #if defined(ESP8266)

        SPI.setClockDivider(SPI_CLOCK_DIV4);
        //setSPIClockDivider(SPI_CLOCK_DIV16);
        setSPIBitOrder(MSBFIRST);
        setSPIDataMode(SPI_MODE0);
       
        beginSPI();
        

    #else

      setSPIBitOrder(MSBFIRST);
      setSPIDataMode(SPI_MODE3);
    //  setSPIClockDivider(SPI_CLOCK_DIV64);
      beginSPI();

    #endif
}

void SPIhw::writeCmd(uint8_t cmd) {

#if defined(ESP8266)
  if (eightBit) {
    cbi(GPOC,bitmaskWR); //wr low
  }
  if (hwCS){
    SPI.write(cmd);
  } else {
     cbi(GPOC,bitmaskCS); //cs low 
     SPI.write(cmd); 
     sbi(GPOS,bitmaskCS); //cs high
  } 
#else  
    if ( eightBit ) {
        *registerWR &= ~bitmaskWR;
    } else {
        SPCR &= ~_BV(SPE); // Disable SPI to get control of the SCK pin.
        cbi(registerSDA, bitmaskSDA);
        cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
      //  delay(1);   // Insert extra time to the hight pulse, typically needed
        sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
        SPCR |= _BV(SPE);      // Enable SPI again
    }
  #if defined(TEENSYDUINO)
    SPI0_SR = SPI_SR_TCF;
    SPI0_PUSHR = cmd;
    while (!(SPI0_SR & SPI_SR_TCF)) ; // wait
  #else
    SPDR = cmd;
    while (!(SPSR & _BV(SPIF)));
  #endif
#endif    
}

void SPIhw::writeData(uint8_t data) {

 #if defined(ESP8266)
    if (eightBit) {
      sbi(GPOS,bitmaskWR);
    }
    if (hwCS){
      SPI.write(data);
    } else {
       cbi(GPOC,bitmaskCS); //cs low 
       SPI.write(data); 
       sbi(GPOS,bitmaskCS); //cs high
    } 
 #else      
    if ( eightBit ) {
        *registerWR |= bitmaskWR;
    } else {
        SPCR &= ~_BV(SPE); // Disable SPI to get control of the SCK pin.
        sbi(registerSDA, bitmaskSDA);
        cbi(registerSCL, bitmaskSCL);   // Pull SPI SCK high
      //  delay(1);   // Insert extra time to the hight pulse, typically needed
        sbi(registerSCL, bitmaskSCL);   // Pull SPI SCK low
        SPCR |= _BV(SPE);      // Enable SPI again
    }

  #if defined(TEENSYDUINO)
      SPI0_SR = SPI_SR_TCF;
      SPI0_PUSHR = data;
      while (!(SPI0_SR & SPI_SR_TCF)) ; // wait  
  #else
      SPDR = data;
      while (!(SPSR & _BV(SPIF)));
  #endif
#endif
}

void SPIhw::beginSPI() {
 

  #if defined(ESP8266)
    SPI.setHwCs(hwCS);
    SPI.begin();    
  #else  

    cbi(registerSCL, bitmaskSCL);
    cbi(registerSDA, bitmaskSDA);
    digitalWrite(pinCS, HIGH);
      // Warning: if the SS pin ever becomes a LOW INPUT then SPI
      // automatically switches to Slave, so the data direction of
      // the SS pin MUST be kept as OUTPUT.

      SPCR |= _BV(MSTR);
      SPCR |= _BV(SPE);

    #if defined(TEENSYDUINO)
      uint32_t ctar = SPI_CTAR_FMSZ(7) |
              SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR;

      SPI0_MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
      SPI0_CTAR0 = ctar;
      SPI0_CTAR1 = ctar | SPI_CTAR_FMSZ(8);
      SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F);  
    #else
      DDRB = DDRB | B00000001; // PB0 as OUTPUT
      PORTB = PORTB | B00000001; // PB0 as HIGH
    #endif
  #endif    
}

void SPIhw::endSPI() {
  #if defined(ESP8266)
    SPI.end();  
  #else  
    SPCR &= ~_BV(SPE);
  #endif
}

void SPIhw::setSPIBitOrder(uint8_t bitOrder) {

  #if defined(ESP8266)
    SPI.setBitOrder(bitOrder);
  #else  

    if(bitOrder == LSBFIRST) {
      SPCR |= _BV(DORD);
    } else {
      SPCR &= ~(_BV(DORD));
    }
  #endif  
}

void SPIhw::setSPIDataMode(uint8_t mode) {
  #if defined(ESP8266)
    SPI.setDataMode(mode);
  #else  
    SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
  #endif
}

void SPIhw::setSPIClockDivider(uint8_t rate) {
   #if defined(ESP8266)
    SPI.setClockDivider(rate);
  #else  
    SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
    SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
   #endif
}
#endif
