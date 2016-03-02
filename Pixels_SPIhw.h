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
#include <SPI.h>

#ifdef PIXELS_MAIN
#error Pixels_SPIhw.h must be included before Pixels_<CONTROLLER>.h
#endif

#ifndef PIXELS_SPIHW_H
#define PIXELS_SPIHW_H


#define WIDTH  320
#define HEIGHT 240

// At all other speeds, SPI.beginTransaction() will use the fastest available clock
#define SPICLOCK 30000000
#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0D
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR    0x30
#define ILI9341_MADCTL   0x36
#define ILI9341_VSCRSADD 0x37
#define ILI9341_PIXFMT   0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

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

//#undef chipDeselect
//#define chipDeselect()

class SPIhw {
private:
    uint8_t pinSCL;
    uint8_t pinSDA;
    uint8_t pinWR;
    uint8_t pinCS;
    uint8_t pinRST;
    
    /* JK */
    uint8_t cs = 10;
    uint8_t dc = 9;
    uint8_t rst = 2;
    uint8_t mosi = 11;
    uint8_t sclk = 13;
    uint8_t miso = 12;

    regtype *registerSCL;
    regtype *registerSDA;
    regtype *registerWR;
    regsize bitmaskSCL;
    regsize bitmaskSDA;
    regsize bitmaskWR;

    void beginSPI();
    void endSPI();

    bool eightBit;

    uint32_t ctar0;
    uint32_t ctar1;

    void updatectars();
    int spiModeRequest;

protected:
    uint8_t  _rst;
    uint8_t _cs, _dc;
    uint8_t pcs_data, pcs_command;
    uint8_t _miso, _mosi, _sclk;
    
    void reset() {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(5);
        digitalWrite(_rst, LOW);
        delay(20);
        digitalWrite(_rst, HIGH);
        delay(150);
        
        /*
        digitalWrite(pinRST,LOW);
        delay(100);
        digitalWrite(pinRST,HIGH);
        delay(100);
        */
    }

    /* JK ADDITION */
    void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
    __attribute__((always_inline)) {
        writecommand_cont(ILI9341_CASET); // Column addr set
        writedata16_cont(x0);   // XSTART
        writedata16_cont(x1);   // XEND
        writecommand_cont(ILI9341_PASET); // Row addr set
        writedata16_cont(y0);   // YSTART
        writedata16_cont(y1);   // YEND
    }
    void waitFifoNotFull(void) {
        uint32_t sr;
        uint32_t tmp __attribute__((unused));
        do {
            sr = KINETISK_SPI0.SR;
            if (sr & 0xF0) tmp = KINETISK_SPI0.POPR;  // drain RX FIFO
        } while ((sr & (15 << 12)) > (3 << 12));
    }
    void waitFifoEmpty(void) {
        uint32_t sr;
        uint32_t tmp __attribute__((unused));
        do {
            sr = KINETISK_SPI0.SR;
            if (sr & 0xF0) tmp = KINETISK_SPI0.POPR;  // drain RX FIFO
        } while ((sr & 0xF0F0) > 0);             // wait both RX & TX empty
    }
    void waitTransmitComplete(void) __attribute__((always_inline)) {
        uint32_t tmp __attribute__((unused));
        while (!(KINETISK_SPI0.SR & SPI_SR_TCF)) ; // wait until final output done
        tmp = KINETISK_SPI0.POPR;                  // drain the final RX FIFO word
    }
    void waitTransmitComplete(uint32_t mcr) __attribute__((always_inline)) {
        uint32_t tmp __attribute__((unused));
        while (1) {
            uint32_t sr = KINETISK_SPI0.SR;
            if (sr & SPI_SR_EOQF) break;  // wait for last transmit
            if (sr &  0xF0) tmp = KINETISK_SPI0.POPR;
        }
        KINETISK_SPI0.SR = SPI_SR_EOQF;
        SPI0_MCR = mcr;
        while (KINETISK_SPI0.SR & 0xF0) {
            tmp = KINETISK_SPI0.POPR;
        }
    }
    void writecommand_cont(uint8_t c) __attribute__((always_inline)) {
        KINETISK_SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
        waitFifoNotFull();
    }
    void writedata8_cont(uint8_t c) __attribute__((always_inline)) {
        KINETISK_SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
        waitFifoNotFull();
    }
    void writedata16_cont(uint16_t d) __attribute__((always_inline)) {
        KINETISK_SPI0.PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
        waitFifoNotFull();
    }
    void writecommand_last(uint8_t c) __attribute__((always_inline)) {
        uint32_t mcr = SPI0_MCR;
        KINETISK_SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_EOQ;
        waitTransmitComplete(mcr);
    }
    void writedata8_last(uint8_t c) __attribute__((always_inline)) {
        uint32_t mcr = SPI0_MCR;
        KINETISK_SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_EOQ;
        waitTransmitComplete(mcr);
    }
    void writedata16_last(uint16_t d) __attribute__((always_inline)) {
        uint32_t mcr = SPI0_MCR;
        KINETISK_SPI0.PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1) | SPI_PUSHR_EOQ;
        waitTransmitComplete(mcr);
    }
    /* END JK ADDITION */
    
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
        
        /* JK ADDITION */
        _sclk = scl;
        _mosi = sda;
        _cs = cs;
        _rst = rst;
        _dc = wr;
        _miso = 12;
        /* JK ADDITION END */
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

/* JK ADDITION */
static const uint8_t init_commands[] = {
    4, 0xEF, 0x03, 0x80, 0x02,
    4, 0xCF, 0x00, 0XC1, 0X30,
    5, 0xED, 0x64, 0x03, 0X12, 0X81,
    4, 0xE8, 0x85, 0x00, 0x78,
    6, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02,
    2, 0xF7, 0x20,
    3, 0xEA, 0x00, 0x00,
    2, ILI9341_PWCTR1, 0x23, // Power control
    2, ILI9341_PWCTR2, 0x10, // Power control
    3, ILI9341_VMCTR1, 0x3e, 0x28, // VCM control
    2, ILI9341_VMCTR2, 0x86, // VCM control2
    2, ILI9341_MADCTL, 0x48, // Memory Access Control
    2, ILI9341_PIXFMT, 0x55,
    3, ILI9341_FRMCTR1, 0x00, 0x18,
    4, ILI9341_DFUNCTR, 0x08, 0x82, 0x27, // Display Function Control
    2, 0xF2, 0x00, // Gamma Function Disable
    2, ILI9341_GAMMASET, 0x01, // Gamma curve selected
    16, ILI9341_GMCTRP1, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00, // Set Gamma
    16, ILI9341_GMCTRN1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F, // Set Gamma
    0
};
/* JK ADDITION END */

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
    digitalWrite(pinCS, HIGH);
    
    if ((_mosi == 11 || _mosi == 7) && (_miso == 12 || _miso == 8) && (_sclk == 13 || _sclk == 14)) {
        SPI.setMOSI(_mosi);
        SPI.setMISO(_miso);
        SPI.setSCK(_sclk);
    }
    else {
        return;
    }
    
    SPI.begin();
    
    if (SPI.pinIsChipSelect(_cs, _dc)) {
        pcs_data = SPI.setCS(_cs);
        pcs_command = pcs_data | SPI.setCS(_dc);
    }
    else {
        pcs_data = 0;
        pcs_command = 0;
        return;
    }
    
    reset();

    SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
    const uint8_t *addr = init_commands;
    
    while (1) {
        uint8_t count = *addr++;
        if (count-- == 0) break;
        writecommand_cont(*addr++);
        while (count-- > 0) {
            writedata8_cont(*addr++);
        }
    }
    
    writecommand_last(ILI9341_SLPOUT);    // Exit Sleep
    SPI.endTransaction();
    
    delay(120);
    SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
    writecommand_last(ILI9341_DISPON);    // Display on
    SPI.endTransaction();

    
    // beginSPI();

    // if ( spiModeRequest > 0 ) {
    //     setSPIDataMode(spiModeRequest-1);
    // }

    // setSPIBitOrder(MSBFIRST);
    // setSPIDataMode(SPI_MODE0);
    // setSPIClockDivider(SPI_CLOCK_DIV64);

}

void SPIhw::writeCmd(uint8_t cmd) {
    /* JK ADDITION */
    SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
    writecommand_last(cmd);
    SPI.endTransaction();
    /* JK ADDITION END */
    
    /*
     #if defined(TEENSYDUINO)
     chipSelect();
     #endif
     
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
     chipDeselect();
     #else
     SPDR = cmd;
     while (!(SPSR & _BV(SPIF)));
     #endif
     */
}

void SPIhw::writeData(uint8_t data) {
    /* JK ADDITION */
    SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
    writedata8_last(data);
    SPI.endTransaction();
    /* JK ADDITION END */
    
     /*
     #if defined(TEENSYDUINO)
     chipSelect();
     #endif
     
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
     chipDeselect();
     #else
     SPDR = data;
     while (!(SPSR & _BV(SPIF)));
     #endif
     */
}

void SPIhw::beginSPI() {

    digitalWrite(pinCS, HIGH);
    pinMode(pinCS, OUTPUT);

    cbi(registerSCL, bitmaskSCL);
    cbi(registerSDA, bitmaskSDA);
    digitalWrite(pinCS, HIGH);

#if defined(TEENSYDUINO)
    // Warning: if the SS pin ever becomes a LOW INPUT then SPI
    // automatically switches to Slave, so the data direction of
    // the SS pin MUST be kept as OUTPUT.

    /* JK ADDITION */
    uint32_t ctar = SPI_CTAR_FMSZ(7) | SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0) | SPI_CTAR_DBR;
    SIM_SCGC6 |= SIM_SCGC6_SPI0;
    SPI0_MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
    SPI0_CTAR0 = (SPI_CTAR_FMSZ(7) | SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0)) & ~SPI_CTAR_LSBFE;
    SPI0_CTAR1 = (SPI_CTAR_FMSZ(15) | SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0)) & ~SPI_CTAR_LSBFE;
    SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F);
    /* JK ADDITION END */
    
    /*
    SIM_SCGC6 |= SIM_SCGC6_SPI0;
    SPI0_MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
    SPI0_CTAR0 = (SPI_CTAR_FMSZ(7) |
    SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0)) & ~SPI_CTAR_LSBFE;
    SPI0_CTAR1 = (SPI_CTAR_FMSZ(15) |
    SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0)) & ~SPI_CTAR_LSBFE;
    SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F);
    */

#else
    SPCR |= _BV(MSTR);
    SPCR |= _BV(SPE);

    DDRB = DDRB | B00000001; // PB0 as OUTPUT
    PORTB = PORTB | B00000001; // PB0 as HIGH
#endif
}

void SPIhw::endSPI() {
#if defined(TEENSYDUINO)
    /* JK ADDITION */
    SPI.end();
    /* JK ADDITION END */
    //SPI0_MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
#else
  SPCR &= ~_BV(SPE);
#endif
}

void SPIhw::setSPIBitOrder(uint8_t bitOrder) {
    if (bitOrder == LSBFIRST) {
        SPCR |= _BV(DORD);
    } else {
      SPCR &= ~(_BV(DORD));
    }
}

void SPIhw::setSPIDataMode(uint8_t mode) {
#if defined(TEENSYDUINO)
    spiModeRequest = mode + 1;
    SIM_SCGC6 |= SIM_SCGC6_SPI0;
    SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
#else
    SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
#endif
}

void SPIhw::setSPIClockDivider(uint8_t rate) {
#if defined(TEENSYDUINO)
    // TODO
#else
    SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
    SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
#endif
}

#endif // PIXELS_SPIHW_H
