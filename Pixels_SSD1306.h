/*
 * Pixels. Graphics library for TFT displays.
 *
 * Copyright (C) 2012-2015
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
 * Pixels port to TEMPLATE controller
 */

#define PIXELS_FRAMEBUFFER 1

#include "Pixels.h"

#ifndef PIXELS_SSD1306_H
#define PIXELS_SSD1306_H
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

    int32_t setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void quickFill(int b, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void setFillDirection(uint8_t direction);

    void scrollCmd();

    uint8_t* frameBuffer;
    int xx1, yy1, xx2, yy2; // current region
    int mx;
    int my;

//    boolean checkBounds(Bounds& bb);
    void endGfxOperation(boolean force);

public:

    Pixels() : PixelsBase(128, 64) {
        scrollSupported = true;
//      setSpiPins(uint8_t scl, uint8_t sda, uint8_t cs, uint8_t rst, uint8_t wr = 255)
        setSpiPins(13, 11, 10, 5, 2);
        frameBuffer = new uint8_t[deviceWidth/8 * deviceHeight];
    }

    Pixels(uint16_t width, uint16_t height) : PixelsBase(width, height) {
        scrollSupported = true;
        setSpiPins(13, 11, 10, 5, 2); // dummy code in PPI case
        frameBuffer = new uint8_t[deviceWidth/8 * deviceHeight];
    }

    void init();
    int8_t drawBitmap(int16_t x, int16_t y, int16_t width, int16_t height, prog_uint16_t* data);
};

#if defined(PIXELS_ANTIALIASING_H)
#undef PixelsBase
#endif

void Pixels::init() {

    initInterface();

    reset();
    delay(10);
    reset();
    delay(10);

    boolean extvcc = false;

    chipSelect();

    writeCmd(0xAE);         // DISPLAYOFF
    writeCmd(0xD5);         // SETDISPLAYCLOCKDIV
    writeCmd(0x80);         // the suggested ratio 0x80
    writeCmd(0xA8);         // SETMULTIPLEX
    writeCmd(deviceWidth - 1);       // deviceHeight - 1 = 31
    writeCmd(0xD3);         // SETDISPLAYOFFSET
    writeCmd(0x00);         // offset = 0
    writeCmd(0x40);         // SETSTARTLINE to 0
    writeCmd(0x8D);         // CHARGEPUMP
    if (extvcc) {
      writeCmd(0x10);
    } else {
      writeCmd(0x14);
    }
    writeCmd(0x20);         // MEMORYMODE
    writeCmd(0x00);         // 0x0 act like ks0108

    writeCmd(0xA0 | 0x1);   // SEGREMAP
    writeCmd(0xC8);         // COMSCANDEC
    writeCmd(0xDA);         // SETCOMPINS

    if ( deviceWidth < 64 ) {
      writeCmd(0x02);
      writeCmd(0x81);       // SETCONTRAST
      writeCmd(0x8F);
    } else {
      writeCmd(0x12); // ----------
      writeCmd(0x81);       // SETCONTRAST
      if (extvcc) {
          writeCmd(0x9F);
      } else {
          writeCmd(0xCF);
      }
    }

    writeCmd(0xD9);         // SETPRECHARGE
    if (extvcc) {
      writeCmd(0x22);
    } else {
      writeCmd(0xF1);
    }
    writeCmd(0xDB);         // SETVCOMDETECT
    writeCmd(0x40);
    writeCmd(0xA4);         // DISPLAYALLON_RESUME
    writeCmd(0xA6);         // NORMALDISPLAY

    writeCmd(0xAF);         // DISPLAYON

    // define region display memory region to write to

    writeCmd(0x21);     // COLUMNADDR
    writeCmd(0);       // Column start address (0 = reset)
    writeCmd(127);       // Column end address (127 = reset)

    writeCmd(0x22); // PAGEADDR
    writeCmd(0);       // Page start address (0 = reset)
    writeCmd(deviceWidth/8 - 1);       // Page end address

    chipDeselect();

    for ( int i = 0; i < deviceWidth * deviceHeight / 8; i++ ) {
        frameBuffer[i] = 0;
    }
}

void Pixels::scrollCmd() {
    // NOP
    // scrolling is emulated with the frame buffer
}

void Pixels::setFillDirection(uint8_t direction) {
    fillDirection = direction;
}

void Pixels::quickFill (int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    uint8_t r = ((0xf800 & color)>>11) * 255 / 31;
    uint8_t g = ((0x7e0 & color)>>5) * 255 / 63;
    uint8_t b = (0x1f & color) * 255 / 31;
    boolean w = r > 127 || g > 127 || b > 127;;

    if( !setRegion(x1, y1, x2, y2) ) {
        return;
    }

    for ( int y = yy1; y <= yy2; y++ ) {
        for ( int x = xx1; x <= xx2; x++ ) {
            int idx = deviceHeight - y - 1 + (x/8) * deviceHeight;
            if ( idx < 0 || idx >= deviceWidth/8 * deviceHeight ) {
                continue;
            }
            if (w) {
                frameBuffer[idx] |= (1 << (x&7));
            } else {
                frameBuffer[idx] &= ~(1 << (x&7));
            }
        }
    }
}

void Pixels::deviceWriteData(uint8_t high, uint8_t low) {
    int color = (((uint16_t)high) << 8) + low;
    uint8_t r = ((0xf800 & color)>>11) * 255 / 31;
    uint8_t g = ((0x7e0 & color)>>5) * 255 / 63;
    uint8_t b = (0x1f & color) * 255 / 31;
    boolean w = r > 127 || g > 127 || b > 127;;

    int idx = deviceHeight - 1 - my + (mx/8) * deviceHeight;

    if ( idx >= 0 && idx < deviceWidth/8 * deviceHeight ) {
        if (w) {
            frameBuffer[idx] |= (1 << (mx&7));
        } else {
            frameBuffer[idx] &= ~(1 << (mx&7));
        }
    }

    my++;
    if ( my > yy2 ) {
        my = yy1;
        mx++;
        if ( mx > xx2 ) { // yy2 ?????????
            mx--;
            return;
        }
    }
}

int32_t Pixels::setRegion(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    Bounds bb(x1, y1, x2, y2);
    if( !checkBounds(bb) ) {
        return 0;
    }

    xx1 = bb.x1;
    yy1 = bb.y1;
    xx2 = bb.x2;
    yy2 = bb.y2;
    mx = bb.x1;
    my = bb.y1;

    return 1;
}

void Pixels::endGfxOperation(boolean force) {
    gfxOpNestingDepth--;


    if ( !force ) {
        if ( gfxOpNestingDepth > 0 ) {
            return;
        }

        gfxOpNestingDepth = 0;
    }

    chipSelect();

//    writeCmd(0x21);     // COLUMNADDR
//    writeCmd(0);       // Column start address (0 = reset)
//    writeCmd(127);       // Column end address (127 = reset)

//    writeCmd(0x22); // PAGEADDR
//    writeCmd(0);       // Page start address (0 = reset)
//    writeCmd(deviceWidth/8 - 1);       // Page end address

    if (currentScroll != 0) {

        int cs = orientation < 2 ? currentScroll : deviceHeight - currentScroll;

        for ( int x = 0; x < deviceWidth/8; x++ ) {
            for ( int y = deviceHeight - cs; y < deviceHeight; y++ ) {
                int idx  = x * deviceHeight + y;
                if ( idx < 0 || idx >= deviceWidth/8 * deviceHeight ) {
                    continue;
                }

                writeData(frameBuffer[idx]);
            }
            for ( int y = 0; y < deviceHeight - cs; y++ ) {
                int idx  = x * deviceHeight + y;
                if ( idx < 0 || idx >= deviceWidth/8 * deviceHeight ) {
                    continue;
                }

                writeData(frameBuffer[idx]);
            }
        }
    } else {
        for ( int i = 0; i < deviceWidth * deviceHeight / 8; i++ ) {
            writeData(frameBuffer[i]);
        }
    }
}

int8_t Pixels::drawBitmap(int16_t x, int16_t y, int16_t width, int16_t height, prog_uint16_t* data) {

    Bounds bb(x, y, x+width-1, y+height-1);
    if( !transformBounds(bb) ) {
        return 1;
    }

    if( !checkBounds(bb) ) {
        return 1;
    }

    beginGfxOperation();
    setRegion(bb.x1, bb.y1, bb.x2, bb.y2);

    int sc = currentScroll;
    if ( sc == 0 ) {
        sc = deviceHeight;
    }

    switch( orientation ) {
    case PORTRAIT:
        {
            for ( int16_t i = bb.x1; i <= bb.x2; i++ ) {
                for ( int16_t j = bb.y1; j <= bb.y2; j++ ) {
                    int16_t px = pgm_read_word_near(data + (j - y) * width + i - x);
                    setCurrentPixel(px);
                }
            }
        }
        break;
    case LANDSCAPE:
        {
            int h1 = height - max(0, (y + height) - deviceWidth) - 1;
            int w1 = width - max(0, (x + width) - sc) - 1;
            int w = bb.x2 - bb.x1 + 1;
            int h = bb.y2 - bb.y1 + 1;
            for ( int16_t i = 0; i < w; i++ ) {
                for ( int16_t j = h - 1; j >= 0; j-- ) {
                    int16_t px = pgm_read_word_near(data + (h1 - i) * width + (w1 - j));
                    setCurrentPixel(px);
                }
            }
        }
        break;
    case PORTRAIT_FLIP:
        {
            int h = bb.y2 - bb.y1 + 1;
            int w = bb.x2 - bb.x1 + 1;
            int cutH = y < 0 ? 0 : height - h;
            int cutW = x < 0 ? 0 : width - w;
            for ( int16_t i = 0; i < w; i++ ) {
                for ( int16_t j = 0; j < h; j++ ) {
                    int16_t px = pgm_read_word_near(data + (height - j - 1 - cutH) * width + (width - i - 1 - cutW));
                    setCurrentPixel(px);
                }
            }
        }
        break;
    case LANDSCAPE_FLIP:
        {
            int h1 = height - max(0, (y + height) - deviceWidth) - 1;
            int w1 = width - max(0, (x + width) - sc) - 1;
            int w = bb.x2 - bb.x1 + 1;
            int h = bb.y2 - bb.y1 + 1;
            for ( int16_t i = w - 1; i >= 0; i-- ) {
                for ( int16_t j = 0; j < h; j++ ) {
                    int16_t px = pgm_read_word_near(data + (h1 - i) * width + (w1 - j));
                    setCurrentPixel(px);
                }
            }
        }
        break;
    }

    endGfxOperation(false);
    return 0;
}

//boolean Pixels::checkBounds(Bounds& bb) {
//    if (bb.x2 < bb.x1) {
//        swap(bb.x1, bb.x2);
//    }
//    if (bb.y2 < bb.y1) {
//        swap(bb.y1, bb.y2);
//    }

//    if ( bb.x2 < 0 ) {
//        return false;
//    }
//    if ( bb.x1 < 0 ) {
//        bb.x1 = 0;
//    }
//    if ( bb.x1 >= deviceWidth ) {
//        return false;
//    }
//    if ( bb.x2 >= deviceWidth ) {
//        bb.x2 = deviceWidth - 1;
//    }

//    if ( bb.y1 < 0 ) {
//        if ( bb.y2 < 0 ) {
//            return false;
//        }
//        bb.y1 = 0;
//    }
//    if ( !relativeOrigin ) {
//        if ( bb.y1 >= deviceHeight ) {
//            return false;
//        }
//        if ( bb.y2 >= deviceHeight ) {
//            bb.y2 = deviceHeight - 1;
//        }
//    } else if ( orientation > 1 ) {
//        if ( bb.y1 >= deviceHeight ) {
//            return false;
//        }
//        if ( bb.y2 >= deviceHeight ) {
//            bb.y2 = deviceHeight - 1;
//        }
//        int s = (deviceHeight - currentScroll) % deviceHeight;
//        if ( bb.y2 < s ) {
//            return false;
//        }
//        if ( bb.y1 < s ) {
//            bb.y1 = s;
//        }
////        if ( bb.y2 >= s ) {
////            if ( bb.y1 >= s ) {
////                return false;
////            }
////            bb.y2 = s - 1;
////        }
//    } else {
//        int16_t s = currentScroll > 0 ? currentScroll : deviceHeight;
//        if ( bb.y2 >= s ) {
//            if ( bb.y1 >= s ) {
//                return false;
//            }
//            bb.y2 = s - 1;
//        }
//    }

//    return true;
//}


#endif
