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
 * Commercial use of the library is possible for licensees of "Pixelmeister (for industry)" product.
 *
 * This library includes some code portions and algoritmic ideas derived from works of
 * - Andreas Schiffler -- aschiffler at ferzkopp dot net (SDL_gfx Project)
 * - K. Townsend http://microBuilder.eu (lpc1343codebase Project)
 */

/*
 * Currently supported platforms:
 *
 * 1. Reference platform: Arduino Mega, TFT_PQ 2.4 (ILI9325 controller), ITDB02 MEGA Shield v1.1
 *
 * More platforms coming soon
 */

#ifndef PIXELS_H
#define PIXELS_H

#if defined(__AVR__)
    #include <Arduino.h>

    #define regtype volatile uint8_t
    #define regsize uint8_t

    #define cbi(reg, bitmask) *reg &= ~bitmask
    #define sbi(reg, bitmask) *reg |= bitmask
    #define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
    #define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);


#elif defined(__SAM3X8E__)
    #include <Arduino.h>
    #define PROGMEM
    #define prog_uchar const unsigned char
    #define pgm_read_byte(x)        (*((char *)x))
    #define pgm_read_word(x)        ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
    #define pgm_read_byte_near(x)   (*((char *)x))
    #define pgm_read_byte_far(x)    (*((char *)x))
//  #define pgm_read_word(x)        (*((short *)(x & 0xfffffffe)))
//  #define pgm_read_word_near(x)   (*((short *)(x & 0xfffffffe))
//  #define pgm_read_word_far(x)    (*((short *)(x & 0xfffffffe)))
    #define pgm_read_word_near(x)   ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))
    #define pgm_read_word_far(x)    ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x))))
    #define PSTR(x)  x
#else
    #define PROGMEM
    #define prog_uchar byte

    #define regtype volatile uint32_t
    #define regsize uint16_t
#endif

#define swap(a, b) {int16_t buf = a; a = b; b = buf;}

#define deviceWrite(hi, lo) PORTA = hi; pulse_low(registerWR, bitmaskWR); PORTA = lo; pulse_low(registerWR, bitmaskWR)
#define deviceWriteTwice(b) PORTA = b; pulse_low(registerWR, bitmaskWR); pulse_low(registerWR, bitmaskWR)




// #include <UTFT.h>

#define BITMASK_FONT 1
#define ANTIALIASED_FONT 2
#define HEADER_LENGTH 5

#define SCROLL_SMOOTH 1
#define SCROLL_CLEAN 2

#define FILL_TOPDOWN 0
#define FILL_LEFTRIGHT 0
#define FILL_DOWNTOP 1
#define FILL_RIGHTLEFT 2

#define ORIGIN_RELATIVE true // origin relative to a current scroll position
#define ORIGIN_ABSOLUTE false // origin matches physical device pixel coordinates


#define ipart(X) ((uint16_t)(X))
#define round(X) ((uint16_t)(((double)(X))+0.5))
#define fpart(X) (((double)(X))-(double)ipart(X))
#define rfpart(X) (1.0-fpart(X))


class RGB {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RGB(uint8_t r, uint8_t g, uint8_t b);
    RGB();

    RGB convert565toRGB(uint16_t color);
    uint16_t convertRGBto565(RGB color);
    uint16_t convertTo565();
};


class Pixels {
private:
    /* device */
    uint16_t deviceWidth;
    uint16_t deviceHeight;

    /* currently selected font */
    prog_uchar* currentFont;

    regtype *registerRS; // register select
    regtype *registerCS; // chip select
    regtype *registerWR; // write strobe
    regtype *registerRD; // read strobe
    regtype *registerRST; // reset

    regsize bitmaskRS;
    regsize bitmaskCS;
    regsize bitmaskWR;
    regsize bitmaskRD;
    regsize bitmaskRST;

    RGB foreground;
    RGB background;

    double lineWidth;
    int16_t currentScroll;
    int16_t maxScroll;
    int16_t scrollWidth;
    uint8_t fillDirection;
    boolean scrollSupported;
    boolean antialiasing;
    boolean landscape;

    void printString(int16_t xx, int16_t yy, String text, boolean clean, int8_t kerning[] = NULL);

    void setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setCurrentPixel(RGB color);
    void setCurrentPixel(int16_t color);
    void quickFill(int b, int32_t counter);
    void putColor(int16_t x, int16_t y, boolean steep, double weight);
    RGB computeColor(RGB, double weight);

    void resetRegion();

    void hLine(int16_t x1, int16_t y1, int16_t x2);
    void vLine(int16_t x1, int16_t y1, int16_t y2);

    void deviceWriteCmd(uint8_t b);
    void deviceWriteData(uint8_t hi, uint8_t lo);
    void deviceWriteCmdData(uint8_t cmd, uint16_t data);

    void drawCircleAntialiaced(int16_t x, int16_t y, int16_t radius, boolean bordermode);
    void drawFatLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void drawLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void drawRoundRectangleAntialiased(int16_t x, int16_t y, int16_t width, int16_t height, int16_t rx, int16_t ry, boolean bordermode);

    int16_t* loadFileBytes(String);

public:
    Pixels();
    Pixels(uint16_t width, uint16_t height);

    void init();
    void setLandscape();
    void setPortrait();
    boolean isLandscape();

    void enableAntialiasing(boolean enable);
    boolean isAntialiased();

    void setLineWidth(double width);
    double getLineWidth();

    void clear();

    void setBackground(uint8_t r, uint8_t g, uint8_t b);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setBackground(RGB color);
    void setColor(RGB color);
    RGB getBackground();
    RGB getColor();

    void setOrigin(boolean relative);

    RGB getPixel(int16_t x, int16_t y);
    void drawPixel(int16_t x, int16_t y);
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

    void drawCircle(int16_t x, int16_t y, int16_t radius);
    void drawOval(int16_t x, int16_t y, int16_t width, int16_t height);
    void drawRectangle(int16_t x, int16_t y, int16_t width, int16_t height);
    void drawRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t r);

    void setFillDirection(uint8_t direction);

    void fillCircle(int16_t x, int16_t y, int16_t radius);
    void fillOval(int16_t x, int16_t y, int16_t width, int16_t height);
    void fillRectangle(int16_t x, int16_t y, int16_t width, int16_t height);
    void fillRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t r);

    int8_t drawBitmap(int16_t x, int16_t y, int16_t width, int16_t height, int data[]);
    int8_t loadBitmap(int16_t x, int16_t y, int16_t width, int16_t height, String path);

    void scroll(int16_t dy, int16_t x1, int16_t x2, int8_t flags);

    int setFont(prog_uchar font[]);
    void print(int16_t xx, int16_t yy, String text, int8_t kerning[] = NULL);
    void clean(int16_t xx, int16_t yy, String text, int8_t kerning[] = NULL);
    int16_t getLineHeight();
    int16_t getBaseline();
    int16_t getTextWidth(String text, int8_t kerning[] = NULL);
};

#endif
