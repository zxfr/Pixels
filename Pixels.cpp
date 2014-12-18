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

#include "Pixels.h"

RGB::RGB(uint8_t r, uint8_t g, uint8_t b) {
    red = r;
    green = g;
    blue = b;
}

RGB::RGB() {
}

RGB RGB::convert565toRGB(uint16_t color) {
    uint8_t r = ((0xf800 & color)>>11) * 255 / 31;
    uint8_t g = ((0x7e0 & color)>>5) * 255 / 63;
    uint8_t b = (0x1f & color) * 255 / 31;
    return RGB(r, g, b);
}

uint16_t RGB::convertRGBto565(RGB color) {
    return ((color.red / 8) << 11) | ((color.green / 4) << 5) | (color.blue / 8);
}

uint16_t RGB::convertTo565() {
    return ((red / 8) << 11) | ((green / 4) << 5) | (blue / 8);
}

regtype *registerCS; // chip select
regsize bitmaskCS;

PixelsBase::PixelsBase(uint16_t width, uint16_t height) {
    deviceWidth = width < height ? width : height;
    deviceHeight = width > height ? width : height;
    this->width = width;
    this->height = height;
    orientation = width > height ? LANDSCAPE : PORTRAIT;

    relativeOrigin = true;

    currentScroll = 0;
    scrollSupported = true;
    scrollEnabled = true;

    lineWidth = 1;
    fillDirection = 0;

    setBackground(0,0,0);
    setColor(0xFF,0xFF,0xFF);
}

void PixelsBase::setOrientation( uint8_t direction ){

    orientation = direction;
    landscape = false;

    switch ( orientation ) {
    case LANDSCAPE_FLIP:
    case LANDSCAPE:
        width = deviceHeight;
        height = deviceWidth;
        landscape = true;
        break;
    case PORTRAIT_FLIP:
        width = deviceWidth;
        height = deviceHeight;
        break;
    default:
        width = deviceWidth;
        height = deviceHeight;
        orientation = PORTRAIT;
        break;
    }
}

/*  Graphic primitives */

void PixelsBase::clear() {
    RGB sav = getColor();
    setColor(background);
    fillRectangle(0, 0, width, height);
    setColor(sav);
}

RGB PixelsBase::getPixel(int16_t x, int16_t y) {
    return getBackground();
}

void PixelsBase::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    if (y1 == y2 && lineWidth == 1) {
        hLine(x1, y1, x2);
    } else if (x1 == x2 && lineWidth == 1) {
        vLine(x1, y1, y2);
    } else {
        if ( lineWidth == 1 ) {
            if ( antialiasing ) {
                drawLineAntialiased(x1, y1, x2, y2);
            } else {
                int16_t dx;
                int16_t dy;
                int16_t sx;
                int16_t sy;

                if ( x2 > x1 ) {
                    dx = x2 - x1;
                    sx = 1;
                } else {
                    dx = x1 - x2;
                    sx = -1;
                }

                if ( y2 > y1 ) {
                    dy = y2 - y1;
                    sy = 1;
                } else {
                    dy = y1 - y2;
                    sy = -1;
                }

                int16_t x = x1;
                int16_t y = y1;
                int16_t err = dx - dy;
                int16_t e2;
                while (true) {
                    drawPixel(x, y);
                    if (x == x2 && y == y2) {
                        break;
                    }
                    e2 = err << 1;
                    if (e2 > -dy) {
                        err = err - dy;
                        x = x + sx;
                    }
                    if (e2 < dx) {
                        err = err + dx;
                        y = y + sy;
                    }
                }
            }
        } else {
            drawFatLineAntialiased(x1, y1, x2, y2);
        }
    }
}

void PixelsBase::drawRectangle(int16_t x, int16_t y, int16_t width, int16_t height) {
    hLine(x, y, x+width-2);
    vLine(x+width-1, y, y+height-2);
    hLine(x+1, y+height-1, x+width-1);
    vLine(x, y+1, y+height-1);
}

void PixelsBase::fillRectangle(int16_t x, int16_t y, int16_t width, int16_t height) {
    fill(foreground.convertTo565(), x, y, x+width-1, y+height-1);
}

void PixelsBase::drawRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t radius) {

    if ( radius < 1 ) {
        drawRectangle(x, y, width, height);
        return;
    }

    if ( radius > height >> 2 ) {
        radius = height >> 2;
    }
    if ( radius > width >> 1 ) {
        radius = width >> 1;
    }

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x, y, width, height, radius, radius, 0);
    } else {
        height--;
        width--;

        hLine(x + radius, y + height, x + width - radius);
        hLine(x + radius, y, x + width - radius );
        vLine(x + width, y + radius, y + height - radius);
        vLine(x, y + radius, y + height - radius);

        int16_t shiftX = width - (radius << 1);
        int16_t shiftY = height - (radius << 1);
        int16_t f = 1 - radius;
        int16_t ddF_x = 1;
        int16_t ddF_y = - (radius << 1);
        int16_t x1 = 0;
        int16_t y1 = radius;

        int16_t xx = x + radius;
        int16_t yy = y + radius;

        while (x1 < y1) {
            if (f >= 0) {
                y1--;
                ddF_y += 2;
                f += ddF_y;
            }
            x1++;
            ddF_x += 2;
            f += ddF_x;

            drawPixel(xx + x1 + shiftX, yy + y1 + shiftY);
            drawPixel(xx - x1, yy + y1 + shiftY);
            drawPixel(xx + x1 + shiftX, yy - y1);
            drawPixel(xx - x1, yy - y1);
            drawPixel(xx + y1 + shiftX, yy + x1 + shiftY);
            drawPixel(xx - y1, yy + x1 + shiftY);
            drawPixel(xx + y1 + shiftX, yy - x1);
            drawPixel(xx - y1, yy - x1);
        }
    }
}

void PixelsBase::fillRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t radius) {

    if ( radius < 1 ) {
        fillRectangle(x, y, width, height);
        return;
    }

    if ( radius > height >> 1 ) {
        radius = height >> 1;
    }
    if ( radius > width >> 1 ) {
        radius = width >> 1;
    }

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x, y, width-1, height-1, radius, radius, true);
    }

    fillRectangle(x + radius, y + height - radius, width - (radius << 1), radius);
    fillRectangle(x, y + radius, width, height - (radius << 1));
    fillRectangle(x + radius, y, width - (radius << 1), radius);

    height--;
    width--;

    int16_t shiftX = width - (radius << 1);
    int16_t shiftY = height - (radius << 1);
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -(radius << 1);
    int16_t x1 = 0;
    int16_t y1 = radius;

    int16_t xx = x + radius;
    int16_t yy = y + radius;

    while (x1 < y1) {
        if (f >= 0) {
            y1--;
            ddF_y += 2;
            f += ddF_y;
        }
        x1++;
        ddF_x += 2;
        f += ddF_x;

        hLine(xx + shiftX, yy - y1, xx + shiftX + x1);
        hLine(xx - x1, yy - y1, xx);
        hLine(xx + shiftX, yy - x1, xx + shiftX + y1);
        hLine(xx - y1, yy - x1, xx);

        hLine(xx + shiftX, yy + y1 + shiftY,  xx + x1 + shiftX);
        hLine(xx + shiftX, yy + x1 + shiftY, xx + shiftX + y1);
        hLine(xx - x1, yy + y1 + shiftY, xx);
        hLine(xx - y1, yy + x1 + shiftY, xx);
    }
}

void PixelsBase::drawCircle(int16_t x, int16_t y, int16_t r) {

    drawOval(x-r, y-r, x+r, y-r);

//    if ( antialiasing ) {
//        drawCircleAntialiaced(x, y, r, false);
//    } else {
//        int16_t f = 1 - r;
//        int16_t ddF_x = 1;
//        int16_t ddF_y = -2 * r;
//        int16_t x1 = 0;
//        int16_t y1 = r;

//        drawPixel(x, y + r);
//        drawPixel(x, y - r);
//        drawPixel(x + r, y);
//        drawPixel(x - r, y);

//        while (x1 < y1) {
//            if (f >= 0) {
//                y1--;
//                ddF_y += 2;
//                f += ddF_y;
//            }
//            x1++;
//            ddF_x += 2;
//            f += ddF_x;
//            drawPixel(x + x1, y + y1);
//            drawPixel(x - x1, y + y1);
//            drawPixel(x + x1, y - y1);
//            drawPixel(x - x1, y - y1);
//            drawPixel(x + y1, y + x1);
//            drawPixel(x - y1, y + x1);
//            drawPixel(x + y1, y - x1);
//            drawPixel(x - y1, y - x1);
//        }
//    }
}

void PixelsBase::fillCircle(int16_t x, int16_t y, int16_t r) {
    int16_t yy;
    int16_t xx;

    fillOval(x-r, y-r, x+r, y-r);

//    if ( antialiasing ) {
//        drawCircleAntialiaced(x, y, r, true);
//    }

//    for (yy = -r; yy <= r; yy++) {
//        for (xx = -r; xx <= r; xx++) {
//            if ((xx * xx) + (yy * yy) <= (r * r)) {
//                drawPixel(x+xx, y+yy);
//            }
//        }
//    }
}

void PixelsBase::drawOval(int16_t x, int16_t y, int16_t width, int16_t height) {

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x, y, width, height, width/2, height/2, 0);
    } else {
        height--;
        width--;

        int16_t ix, iy;
        int16_t h, i, j, k;
        int16_t oh, oi, oj, ok;
        int16_t xmh, xph, ypk, ymk;
        int16_t xmi, xpi, ymj, ypj;
        int16_t xmj, xpj, ymi, ypi;
        int16_t xmk, xpk, ymh, yph;

        int16_t rx = width / 2;
        int16_t ry = height / 2;

        int16_t xx = x + rx;
        int16_t yy = y + ry;

        if ((width <= 0) || (height <= 0)) {
            return;
        }

        if (width == 1) {
            vLine(xx, yy, yy + height - 1);
            return;
        }
        if (height == 1) {
            hLine(xx, yy, xx + width - 1);
            return;
        }

        oh = oi = oj = ok = 0xFFFF;

        if (width > height) {
            ix = 0;
            iy = rx << 6;

            do {
                h = (ix + 32) >> 6;
                i = (iy + 32) >> 6;
                j = (h * ry) / rx;
                k = (i * ry) / rx;

                if (((ok != k) && (oj != k)) || ((oj != j) && (ok != j)) || (k != j)) {
                    xph = xx + h;
                    xmh = xx - h;
                    if (k > 0) {
                        ypk = yy + k;
                        ymk = yy - k;
                        drawPixel(xmh, ypk);
                        drawPixel(xph, ypk);
                        drawPixel(xmh, ymk);
                        drawPixel(xph, ymk);
                    } else {
                        drawPixel(xmh, yy);
                        drawPixel(xph, yy);
                    }
                    ok = k;
                    xpi = xx + i;
                    xmi = xx - i;
                    if (j > 0) {
                        ypj = yy + j;
                        ymj = yy - j;
                        drawPixel(xmi, ypj);
                        drawPixel(xpi, ypj);
                        drawPixel(xmi, ymj);
                        drawPixel(xpi, ymj);
                    } else {
                        drawPixel(xmi, yy);
                        drawPixel(xpi, yy);
                    }
                    oj = j;
                }

                ix = ix + iy / rx;
                iy = iy - ix / rx;

            } while (i > h);
        } else {
            ix = 0;
            iy = ry << 6;

            do {
                h = (ix + 32) >> 6;
                i = (iy + 32) >> 6;
                j = (h * rx) / ry;
                k = (i * rx) / ry;

                if (((oi != i) && (oh != i)) || ((oh != h) && (oi != h) && (i != h))) {
                    xmj = xx - j;
                    xpj = xx + j;
                    if (i > 0) {
                        ypi = yy + i;
                        ymi = yy - i;
                        drawPixel(xmj, ypi);
                        drawPixel(xpj, ypi);
                        drawPixel(xmj, ymi);
                        drawPixel(xpj, ymi);
                    } else {
                        drawPixel(xmj, yy);
                        drawPixel(xpj, yy);
                    }
                    oi = i;
                    xmk = xx - k;
                    xpk = xx + k;
                    if (h > 0) {
                        yph = yy + h;
                        ymh = yy - h;
                        drawPixel(xmk, yph);
                        drawPixel(xpk, yph);
                        drawPixel(xmk, ymh);
                        drawPixel(xpk, ymh);
                    } else {
                        drawPixel(xmk, yy);
                        drawPixel(xpk, yy);
                    }
                    oh = h;
                }

                ix = ix + iy / ry;
                iy = iy - ix / ry;

            } while (i > h);
        }
    }
}

void PixelsBase::fillOval(int16_t xx, int16_t yy, int16_t width, int16_t height) {

    height--;
    width--;

    int16_t rx = width / 2;
    int16_t ry = height / 2;

    int16_t x = xx + rx;
    int16_t y = yy + ry;

    int16_t ix, iy;
    int16_t h, i, j, k;
    int16_t oh, oi, oj, ok;
    int16_t xmh, xph;
    int16_t xmi, xpi;
    int16_t xmj, xpj;
    int16_t xmk, xpk;

    if ((rx < 0) || (ry < 0)) {
        return;
    }

    if (width < 2) {
        vLine(xx, yy, yy + height);
        return;
    }

    if (height < 2) {
        hLine(xx, yy, xx + width);
        return;
    }

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x-rx, y-ry, rx<<1, ry<<1, rx, ry, true);
    }

    oh = oi = oj = ok = 0xFFFF;

    if (rx > ry) {
        ix = 0;
        iy = rx << 6;

        do {
            h = (ix + 32) >> 6;
            i = (iy + 32) >> 6;
            j = (h * ry) / rx;
            k = (i * ry) / rx;

            if ((ok != k) && (oj != k)) {
                xph = x + h;
                xmh = x - h;
                if (k > 0) {
                    hLine(xmh, y + k, xph);
                    hLine(xmh, y - k, xph);
                } else {
                    hLine(xmh, y, xph);
                }
                ok = k;
            }
            if ((oj != j) && (ok != j) && (k != j)) {
                xmi = x - i;
                xpi = x + i;
                if (j > 0) {
                    hLine(xmi, y + j, xpi);
                    hLine(xmi, y - j, xpi);
                } else {
                    hLine(xmi, y, xpi);
                }
                oj = j;
            }

            ix = ix + iy / rx;
            iy = iy - ix / rx;

        } while (i > h);
    } else {
        ix = 0;
        iy = ry << 6;

        do {
            h = (ix + 32) >> 6;
            i = (iy + 32) >> 6;
            j = (h * rx) / ry;
            k = (i * rx) / ry;

            if ((oi != i) && (oh != i)) {
                xmj = x - j;
                xpj = x + j;
                if (i > 0) {
                    hLine(xmj, y + i, xpj);
                    hLine(xmj, y - i, xpj);
                } else {
                    hLine(xmj, y, xpj);
                }
                oi = i;
            }
            if ((oh != h) && (oi != h) && (i != h)) {
                xmk = x - k;
                xpk = x + k;
                if (h > 0) {
                    hLine(xmk, y + h, xpk);
                    hLine(xmk, y - h, xpk);
                } else {
                    hLine(xmk, y, xpk);
                }
                oh = h;
            }

            ix = ix + iy / ry;
            iy = iy - ix / ry;

        } while (i > h);
    }
}

void PixelsBase::drawIcon(int16_t xx, int16_t yy, prog_uchar* data) {

    int16_t fontType = BITMASK_FONT;
    if ( pgm_read_byte_near(data + 1) == 'a' ) {
        fontType = ANTIALIASED_FONT;
    }

    int16_t length = ((0xFF & (int32_t)pgm_read_byte_near(data + 2)) << 8) + (0xFF & (int32_t)pgm_read_byte_near(data + 3));

    int16_t height =  pgm_read_byte_near(data + 4);

    drawGlyph(fontType, false, xx, yy, height, data, 1, length-1);
}

void PixelsBase::cleanIcon(int16_t xx, int16_t yy, prog_uchar* data) {

    int16_t fontType = BITMASK_FONT;
    if ( pgm_read_byte_near(data + 1) == 'a' ) {
        fontType = ANTIALIASED_FONT;
    }

    int16_t length = ((0xFF & (int32_t)pgm_read_byte_near(data + 2)) << 8) + (0xFF & (int32_t)pgm_read_byte_near(data + 3));

    int16_t height =  pgm_read_byte_near(data + 4);

    drawGlyph(fontType, true, xx, yy, height, data, 1, length-1);
}

int8_t PixelsBase::drawBitmap(int16_t x, int16_t y, int16_t width, int16_t height, prog_uint16_t* data) {
    chipSelect();
    setRegion(x, y, x+width-1, y+height-1);

    switch( orientation ) {
    case PORTRAIT:
        {
            int32_t ptr = 0;
            int32_t size = width * height;
            while ( ptr++ < size ) {
                int16_t px = pgm_read_word_near(data + ptr);
                setCurrentPixel(px);
            }
        }
        break;
    case LANDSCAPE_FLIP:
        for ( int16_t i = width - 1; i >= 0; i-- ) {
            for ( int16_t j = 0; j < height; j++ ) {
                int16_t px = pgm_read_word_near(data + j * width + i);
                setCurrentPixel(px);
            }
        }
        break;
    case LANDSCAPE:
        for ( int16_t i = 0; i < width; i++ ) {
            for ( int16_t j = height - 1; j >= 0; j-- ) {
                int16_t px = pgm_read_word_near(data + j * width + i);
                setCurrentPixel(px);
            }
        }
        break;
    case PORTRAIT_FLIP:
        {
            int32_t ptr = width * height;
            while ( ptr-- >= 0 ) {
                int16_t px = pgm_read_word_near(data + ptr);
                setCurrentPixel(px);
            }
        }

        break;
    }

    chipDeselect();
    return 0;
}

int8_t PixelsBase::drawCompressedBitmap(int16_t x, int16_t y, uint8_t* data) {

    if ( data == NULL ) {
        return -1;
    }

    if ( pgm_read_byte_near(data + 0) != 'Z' ) {
        // Unknown compression method
        return -2;
    }

    int32_t compressedLen = ((0xFF & (int32_t)pgm_read_byte_near(data + 1)) << 16) + ((0xFF & (int32_t)pgm_read_byte_near(data + 2)) << 8) + (0xFF & (int32_t)pgm_read_byte_near(data + 3));
    if ( compressedLen < 0 ) {
        // Unknown compression method or compressed data inconsistence
        return -3;
    }

    int32_t resultLen = ((0xFF & (int32_t)pgm_read_byte_near(data + 4)) << 16) + ((0xFF & (int32_t)pgm_read_byte_near(data + 5)) << 8) + (0xFF & (int32_t)pgm_read_byte_near(data + 6));
    if ( resultLen < 0 ) {
        // Unknown compression method or compression format error
        return resultLen;
    }

    uint8_t windowLen = 0xFF & (int16_t)pgm_read_byte_near(data + 7);
    if ( windowLen < 0 || windowLen > 254 ) {
        // corrupted content
        return -5;
    }

    int16_t width = ((0xFF & (int32_t)pgm_read_byte_near(data + 8)) << 8) + (0xFF & (int32_t)pgm_read_byte_near(data + 9));
    if ( width < 0 ) {
        // Unknown compression method or compression format error (width parameter is invalid)
        return -6;
    }

    int16_t height = ((0xFF & (int32_t)pgm_read_byte_near(data + 10)) << 8) + (0xFF & (int32_t)pgm_read_byte_near(data + 11));
    if ( height < 0 ) {
        // Unknown compression method or compression format error (height parameter is invalid)
        return -7;
    }

    uint8_t window[windowLen];
    int16_t wptr = 0;

    int32_t ctr = 0;

    uint8_t buf;
    bool bufEmpty = true;

    int* raster;
    int rasterPtr = 0;
    int rasterLine = 0;

    if ( orientation != PORTRAIT ) {
        raster = new int[width];
    }

    chipSelect();

    if( orientation == PORTRAIT ) {
        setRegion(x, y, x + width - 1, y + height - 1);
    } else {
        rasterLine = y;
    }

    BitStream bs( data, compressedLen, 96 );
    while ( true ) {

        uint8_t bit = bs.readBit();
        if ( bit == 0 ) { // literal
            uint8_t bits = bs.readBits(8);
            if ( bufEmpty ) {
                buf = bits;
                bufEmpty = false;
            } else {
                uint16_t px = buf;
                px <<= 8;
                px |= bits;
                if ( orientation == PORTRAIT ) {
                    setCurrentPixel(px);
                } else {
                    raster[rasterPtr++] = px;
                    if ( rasterPtr == width ) {
                        setRegion(x, rasterLine, x + width - 1, rasterLine);
                        rasterLine++;
                        if( orientation == LANDSCAPE ) {
                            for ( int i = 0; i < width; i++ ) {
                                setCurrentPixel(raster[i]);
                            }
                        } else {
                            for ( int i = width - 1; i >= 0; i-- ) {
                                setCurrentPixel(raster[i]);
                            }
                        }
                        rasterPtr = 0;
                    }
                }
                bufEmpty = true;
            }
            ctr++;
            window[wptr++] = bits;
            if ( wptr >= windowLen ) {
                wptr -= windowLen;
            }
        } else {
            uint8_t offset = (uint8_t)bs.readNumber() - 1;
            uint8_t matchCount = (uint8_t)bs.readNumber() - 1;

            while( matchCount-- > 0 ) {
                int16_t p1 = wptr - offset;
                while ( p1 < 0 ) {
                    p1 += windowLen;
                }
                while ( p1 >= windowLen ) {
                    p1 -= windowLen;
                }
                int16_t p2 = wptr;
                while ( p2 >= windowLen ) {
                    p2 -= windowLen;
                }
                wptr++;
                ctr++;

                if ( bufEmpty ) {
                    buf = window[p1];
                    bufEmpty = false;
                } else {
                    uint16_t px = buf;
                    px <<= 8;
                    px |= window[p1];
                    if ( orientation == PORTRAIT ) {
                        setCurrentPixel(px);
                    } else {
                        raster[rasterPtr++] = px;
                        if ( rasterPtr == width ) {
                            setRegion(x, rasterLine, x + width - 1, rasterLine);
                            rasterLine++;
                            if( orientation == LANDSCAPE ) {
                                for ( int i = 0; i < width; i++ ) {
                                    setCurrentPixel(raster[i]);
                                }
                            } else {
                                for ( int i = width - 1; i >= 0; i-- ) {
                                    setCurrentPixel(raster[i]);
                                }
                            }
                            rasterPtr = 0;
                        }
                    }
                    bufEmpty = true;
                }
                window[p2] = window[p1];
            }

            while ( wptr >= windowLen ) {
                wptr -= windowLen;
            }
        }
        if ( ctr > resultLen ) {
            break;
        }
    }

    chipDeselect();

    return 0;
}


int8_t PixelsBase::loadBitmap(int16_t x, int16_t y, int16_t sx, int16_t sy, String path) {
    int16_t* data = loadFileBytes( path );
    return 0; // drawBitmap(x, y, sx, sy, data);
}

/*  -------   Antialiasing ------- */
/* To be overriden with Pixels_Antialiasing.h */

void PixelsBase::drawLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {}

void PixelsBase::drawFatLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {}

void PixelsBase::drawRoundRectangleAntialiased(int16_t x, int16_t y, int16_t width, int16_t height, int16_t rx, int16_t ry, boolean bordermode) {}

void PixelsBase::drawCircleAntialiaced( int16_t x, int16_t y, int16_t radius, boolean bordermode )	{}

/* TEXT */


int PixelsBase::setFont(prog_uchar font[]) {
    int16_t p1 = pgm_read_byte_near(font + 0);
    int16_t p2 = pgm_read_byte_near(font + 1);
    if ( p1 != 'Z' || p2 != 'F' ) {
//			Serial.print("Invalid font prefix ");
//			Serial.print( p1 );
//			Serial.print( " " );
//			Serial.println( p2 );
        currentFont = NULL;
        return -1;
    }
    int16_t fontType = pgm_read_byte_near(font + 2);
    if ( fontType != ANTIALIASED_FONT && fontType != BITMASK_FONT ) {
//			Serial.println("Unsupported font type");
        currentFont = NULL;
        return -1;
    }
    currentFont = font;
    return 0;
}

void PixelsBase::print(int16_t xx, int16_t yy, String text, int8_t kerning[]) {
    printString(xx, yy, text, 0, kerning);
}

void PixelsBase::cleanText(int16_t xx, int16_t yy, String text, int8_t kerning[]) {
    printString(xx, yy, text, 1, kerning);
}

void PixelsBase::printString(int16_t xx, int16_t yy, String text, boolean clean, int8_t kerning[]) {

    if ( currentFont == NULL ) {
        return;
    }

    int16_t fontType = pgm_read_byte_near(currentFont + 2);
    if ( fontType != ANTIALIASED_FONT && fontType != BITMASK_FONT ) {
        return;
    }

    RGB fg = foreground;

    int16_t kernPtr = 0;
    int16_t kern = -100; // no kerning

    int16_t glyphHeight = pgm_read_byte_near(currentFont + 3);

    int16_t x1 = xx;

    for (int16_t t = 0; t < text.length(); t++) {
        char c = text.charAt(t);

        int16_t width = 0;
        boolean found = false;
        int16_t ptr = HEADER_LENGTH;
        while ( 1 ) {
            char cx = (char)(((int)pgm_read_byte_near(currentFont + ptr + 0) << 8) + pgm_read_byte_near(currentFont + ptr + 1));
            if ( cx == 0 ) {
              break;
            }
            int16_t length = (((int)(pgm_read_byte_near(currentFont + ptr + 2) & 0xff) << 8) + (int)(pgm_read_byte_near(currentFont + ptr + 3) & 0xff));

            if ( cx == c ) {
                if ( length < 8 ) {
//						Serial.print( "Invalid "  );
//						Serial.print( c );
//						Serial.println( " glyph definition. Font corrupted?" );
                    break;
                }
                drawGlyph(fontType, clean, x1, yy, glyphHeight, currentFont, ptr, length);
                width = 0xff & pgm_read_byte_near(currentFont + ptr + 4);
                found = true;
                break;
            }
            ptr += length;
        }

        if ( kerning != NULL && kerning[kernPtr] > -100 ) {
            kern = kerning[kernPtr];
            if (kerning[kernPtr+1] > -100) {
                kernPtr++;
            }
        }

        if ( found ) {
            x1 += width;
            if ( kern > -100 ) {
                x1+= kern;
            }
        }
    }

    setColor(fg);
}

int16_t PixelsBase::getTextLineHeight() {
    if ( currentFont == NULL ) {
        return 0;
    }

//    int16_t fontType = pgm_read_byte_near(currentFont + 2);
//    if ( fontType != ANTIALIASED_FONT && fontType != BITMASK_FONT ) {
//        return 0;
//    }

    return pgm_read_byte_near(currentFont + 3);
}

int16_t PixelsBase::getTextBaseline() {
    if ( currentFont == NULL ) {
        return 0;
    }

//    int16_t fontType = pgm_read_byte_near(currentFont + 2);
//    if ( fontType != ANTIALIASED_FONT && fontType != BITMASK_FONT ) {
//        return 0;
//    }

    return pgm_read_byte_near(currentFont + 4);
}

int16_t PixelsBase::getTextWidth(String text, int8_t kerning[]) {
    if ( currentFont == NULL ) {
        return 0;
    }

    int16_t kernPtr = 0;
    int16_t kern = -100; // no kerning
    int16_t x1 = 0;

    for (int16_t t = 0; t < text.length(); t++) {
        char c = text.charAt(t);

        int16_t width = 0;
        boolean found = false;
        int16_t ptr = HEADER_LENGTH;
        while ( 1 ) {
            char cx = (char)(((int)pgm_read_byte_near(currentFont + ptr + 0) << 8) + pgm_read_byte_near(currentFont + ptr + 1));
            if ( cx == 0 ) {
              break;
            }
            int16_t length = (((int)(pgm_read_byte_near(currentFont + ptr + 2) & 0xff) << 8) + (int)(pgm_read_byte_near(currentFont + ptr + 3) & 0xff));

            if ( cx == c ) {
                if ( length < 8 ) {
//						Serial.print( "Invalid "  );
//						Serial.print( c );
//						Serial.println( " glyph definition. Font corrupted?" );
                    break;
                }
//                Serial.print( c );
                found = true;
                width = 0xff & pgm_read_byte_near(currentFont + ptr + 4);
            }

            ptr += length;
        }

        if ( kerning != NULL && kerning[kernPtr] > -100 ) {
            kern = kerning[kernPtr];
            if (kerning[kernPtr+1] > -100) {
                kernPtr++;
            }
        }

        if ( found ) {
            x1 += width;
            if ( kern > -100 ) {
                x1+= kern;
            }
        }
    }

    return x1;
}

void PixelsBase::drawGlyph(int16_t fontType, boolean clean, int16_t x1, int16_t yy,
                           int16_t glyphHeight, prog_uchar* data, int16_t ptr, int16_t length) {
    RGB fg = foreground;

    int16_t width = 0xff & pgm_read_byte_near(data + ptr + 4);

    int16_t marginLeft = 0x7f & pgm_read_byte_near(data + ptr + 5);
    int16_t marginTop = 0xff & pgm_read_byte_near(data + ptr + 6);
    int16_t marginRight = 0x7f & pgm_read_byte_near(data + ptr + 7);
    int16_t effWidth = width - marginLeft - marginRight;

    int16_t ctr = 0;

    if ( fontType == ANTIALIASED_FONT ) {

        boolean vraster = (0x80 & pgm_read_byte_near(data + ptr + 5)) > 0;

        if ( vraster ) {
            int16_t marginBottom = marginRight;
            int16_t effHeight = glyphHeight - marginTop - marginBottom;

            int16_t y = 0;
            for ( int16_t i = 0; i < length - 8; i++ ) {
                int16_t b = 0xff & pgm_read_byte_near(data + ptr + 8 + i);
                int16_t x = ctr / effHeight;

                if ( (0xc0 & b) > 0 ) {
                    int16_t yt = y;
                    int16_t len = 0x3f & b;
                    ctr += len;
                    y += len;
                    if ( (0x80 & b) > 0 ) {
                        if ( clean ) {
                            setColor(background.red, background.green, background.blue);
                        } else {
                            setColor(fg.red, fg.green, fg.blue);
                        }
                        while ( yt + len > effHeight ) {
                            vLine(x1 + marginLeft + x, yy + marginTop + yt, yy + marginTop + effHeight - 1);
                            len -= effHeight - yt;
                            yt = 0;
                            x++;
                        }
                        vLine(x1 + marginLeft + x, yy + marginTop + yt, yy + marginTop + yt + len - 1);
                    }
                } else {
                    if ( clean ) {
                        setColor(background.red, background.green, background.blue);
                    } else {
                        uint8_t opacity = (0xff & (b << 2));
                        RGB cl = computeColor(fg, opacity);
                        setColor(cl);
                    }
                    drawPixel(x1 + marginLeft + x, yy + marginTop + y);
                    ctr++;
                    y++;
                }
                while ( y >= effHeight ) {
                    y -= effHeight;
                }
            }

        } else {

            int16_t x = 0;
            for ( int16_t i = 0; i < length - 8; i++ ) {
                int16_t b = 0xff & pgm_read_byte_near(data + ptr + 8 + i);
                int16_t y = ctr / effWidth;

                if ( (0xc0 & b) > 0 ) {
                    int16_t xt = x;
                    int16_t len = 0x3f & b;
                    ctr += len;
                    x += len;
                    if ( (0x80 & b) > 0 ) {
                        if ( clean ) {
                            setColor(background.red, background.green, background.blue);
                        } else {
                            setColor(fg.red, fg.green, fg.blue);
                        }
                        while ( xt + len > effWidth ) {
                            hLine(x1 + marginLeft + xt, yy + marginTop + y, x1 + marginLeft + effWidth - 1);
                            len -= effWidth - xt;
                            xt = 0;
                            y++;
                        }
                        hLine(x1 + marginLeft + xt, yy + marginTop + y, x1 + marginLeft + xt + len - 1);
                    }
                } else {
                    if ( clean ) {
                        setColor(background.red, background.green, background.blue);
                    } else {
                        uint8_t opacity = (0xff & (b << 2));
                        RGB cl = computeColor(fg, opacity);
                        setColor(cl);
                    }
                    drawPixel(x1 + marginLeft + x, yy + marginTop + y);
                    ctr++;
                    x++;
                }
                while ( x >= effWidth ) {
                    x -= effWidth;
                }
            }
        }

        setColor(fg);

    } else if ( fontType == BITMASK_FONT ) {

        if ( clean ) {
            setColor(background.red, background.green, background.blue);
//					} else {
//                        setColor(fg.red, fg.green, fg.blue );
        }

        boolean compressed = (pgm_read_byte_near(data + ptr + 7) & 0x80) > 0;
        if ( compressed ) {
            boolean vraster = (pgm_read_byte_near(data + ptr + 5) & 0x80) > 0;
            if ( vraster ) {
                int16_t marginBottom = marginRight;
                int16_t effHeight = glyphHeight - marginTop - marginBottom;
                int16_t y = 0;
                for ( int16_t i = 0; i < length - 8; i++ ) {
                    int16_t len = 0x7f & pgm_read_byte_near(data + ptr + 8 + i);
                    boolean color = (0x80 & pgm_read_byte_near(data + ptr + 8 + i)) > 0;
                    if ( color ) {
                        int16_t x = ctr / effHeight;
                        int16_t yt = y;
                        while ( yt + len > effHeight ) {
                            vLine(x1 + marginLeft + x, yy + marginTop + yt, yy + marginTop + effHeight);
                            int16_t dy = effHeight - yt;
                            len -= dy;
                            ctr += dy;
                            y += dy;
                            yt = 0;
                            x++;
                        }
                        vLine(x1 + marginLeft + x, yy + marginTop + yt, yy + marginTop + yt + len);
                    }
                    ctr += len;
                    y += len;
                    while ( y >= effHeight ) {
                        y -= effHeight;
                    }
                }
            } else {
                int16_t x = 0;
                for ( int16_t i = 0; i < length - 8; i++ ) {
                    int16_t len = 0x7f & pgm_read_byte_near(data + ptr + 8 + i);
                    boolean color = (0x80 & pgm_read_byte_near(data + ptr + 8 + i)) > 0;
                    int16_t xt = x;
                    int16_t y = ctr / effWidth;
                    if ( color ) {
                        while ( xt + len > effWidth ) {
                            hLine(x1 + marginLeft + xt, yy + marginTop + y, x1 + marginLeft + effWidth);
                            int16_t dx = effWidth - xt;
                            len -= dx;
                            ctr += dx;
                            x += dx;
                            xt = 0;
                            y++;
                        }
                        hLine(x1 + marginLeft + xt, yy + marginTop + y, x1 + marginLeft + xt + len);
                    }
                    ctr += len;
                    x += len;
                    while ( x >= effWidth ) {
                        x -= effWidth;
                    }
                }
            }
        } else {

            int16_t x = 0;
            int16_t offset = 0;
            for ( int16_t i = 0; i < length - 8; i++, offset += 8, x += 8 ) {
                int16_t b = 0xff & pgm_read_byte_near(data + ptr + 8 + i);

                while ( x >= effWidth ) {
                    x -= effWidth;
                }

                int16_t xt = x;
                int16_t y = offset / effWidth;

                for ( uint8_t j = 0; j < 8; j++ ) {
                    if ( xt + j == effWidth ) {
                        xt = -j;
                        y++;
                    }
                    int16_t mask = 1 << (7 - j);
                    if ( (b & mask) == 0 ) {
                        vLine(x1 + marginLeft + xt + j, yy + marginTop + y, yy + marginTop + y + 1);
                    }
                }
            }
        }
    }
}

/* Low level */

void PixelsBase::putColor(int16_t x, int16_t y, boolean steep, double alpha) {

    if ( steep ) {
        int16_t tmp = x;
        x = y;
        y = tmp;
    }

    if ( x < 0 || x >= width || y < 0 || y >= height ) {
        return;
    }

    RGB result;
    if ( alpha != 1 ) {
        RGB bg = getPixel(x, y);
        result = computeColor(bg, alpha);
        RGB sav = getColor();
        setColor(result);
        drawPixel(x, y);
        setColor(sav);
    } else {
        drawPixel(x, y);
    }
}


RGB PixelsBase::computeColor(RGB bg, double alpha) {
    if ( alpha < 0 ) {
        alpha = 0;
        return bg;
    }
    if ( alpha > 1 ) {
        alpha = 1;
    }
    int16_t sr = (int)(bg.red * (1 - alpha) + foreground.red * alpha);
    int16_t sg = (int)(bg.green * (1 - alpha) + foreground.green * alpha);
    int16_t sb = (int)(bg.blue * (1 - alpha) + foreground.blue * alpha);
    return RGB(sr, sg, sb);
}

RGB PixelsBase::computeColor(RGB fg, uint8_t opacity) {
    int32_t sr = (int32_t)fg.red * (255 - opacity) + background.red * opacity;
    int32_t sg = (int32_t)fg.green * (255 - opacity) + background.green * opacity;
    int32_t sb = (int32_t)fg.blue * (255 - opacity) + background.blue * opacity;
    sr /= 200;
    sg /= 200;
    sb /= 200;
    if ( sr > 255 ) {
        sr = 255;
    }
    if ( sg > 255 ) {
        sg = 255;
    }
    if ( sb > 255 ) {
        sb = 255;
    }
    return RGB((uint8_t)sr, (uint8_t)sg, (uint8_t)sb);
}

void PixelsBase::scroll(int16_t dy, int8_t flags) {
    scroll(dy, 0, deviceWidth, flags);
}

void PixelsBase::scroll(int16_t dy, int16_t x1, int16_t x2, int8_t flags) {

    if(!canScroll()) {
        return;
    }

    int16_t mdy = dy > 0 ? dy : -dy;

    if (mdy > 1 && (flags & SCROLL_SMOOTH) > 0) {

        int16_t easingLen = 5;
        if ( mdy / 2 < easingLen) {
            easingLen = mdy / 2;
        }

        int16_t dlx = (flags & SCROLL_CLEAN) > 0 ? 8 : 15;
        int16_t factor = 3;

        int16_t step = dy < 0 ? -1 : 1;
        for ( int16_t i = 0; i < easingLen; i++ ) {
            delay(dlx+(easingLen-i)*(easingLen-i)*factor/2);
            scroll(step, x1, x2, flags & SCROLL_CLEAN);
        }
        for ( int16_t i = 0; i < mdy - easingLen*2; i++ ) {
            scroll(step, x1, x2, flags & SCROLL_CLEAN);
            delay(dlx+factor);
        }
        for ( int16_t i = 1; i <= easingLen; i++ ) {
            scroll(step, x1, x2, flags & SCROLL_CLEAN);
            delay(dlx+i*i*factor/2);
        }

    } else {

        if ( orientation > LANDSCAPE ) {
            dy = -dy;
        }

        currentScroll += dy;

        while ( currentScroll < 0 ) {
            currentScroll += deviceHeight;
        }

        currentScroll %= deviceHeight;
        flipScroll = (deviceHeight - currentScroll) % deviceHeight;

        scrollCmd();

        if ( (flags & SCROLL_CLEAN) > 0 ) {

            scrollCleanMode = true;
            RGB sav = getColor();
            setColor(getBackground());

            boolean changed = false;
            if ( relativeOrigin ) {
                relativeOrigin = false;
                changed = true;
            }

            int16_t savScroll = currentScroll;

            if ( orientation > LANDSCAPE ) {
                currentScroll = flipScroll;
                dy = -dy;
            }

            switch ( orientation ) {
            case PORTRAIT:
            case PORTRAIT_FLIP:
                if ( dy < 0 ) {
                    fillRectangle(0, 0, deviceWidth, mdy);
                } else {
                    fillRectangle(0, deviceHeight-mdy, deviceWidth, mdy);
                }
                break;
            case LANDSCAPE:
            case LANDSCAPE_FLIP:
                if ( dy < 0 ) {
                    fillRectangle(0, 0, mdy, deviceWidth);
                } else {
                    fillRectangle(deviceHeight-mdy, 0, mdy, deviceWidth);
                }
                break;
            }

            currentScroll = savScroll;

            if ( changed ) {
                relativeOrigin = true;
            }
            setColor(sav);
            scrollCleanMode = false;
        }
    }
}


void PixelsBase::drawPixel(int16_t x, int16_t y) {

    if ( x < 0 || y < 0 || x >= width || y >= height ) {
        return;
    }

    if ( relativeOrigin ) {
        if ( currentScroll != 0 ) {
            if ( landscape ) {
                int edge = currentScroll;
                if ( !scrollCleanMode && x == edge || x > edge ) {
                    return;
                }
            } else {
                int edge = currentScroll;
                if ( !scrollCleanMode && y == edge || y > edge ) {
                    return;
                }
            }
        }
    } else {
        if ( landscape ) {
            x = (x + deviceHeight + currentScroll);
            while (x > deviceHeight ) {
                x -= deviceHeight;
            }
        } else {
            y = (y + deviceHeight + currentScroll);
            while (y > deviceHeight ) {
                y -= deviceHeight;
            }
        }
    }

    chipSelect();
    setRegion(x, y, x, y);
    setCurrentPixel(foreground);
    chipDeselect();
}

void PixelsBase::fill(int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    if (x2 < x1) {
        swap(x1, x2);
    }

    if (y2 < y1) {
        swap(y1, y2);
    }

    if ( x1 >= width ) {
        return;
    }

    if ( y1 >= height ) {
        return;
    }

    if ( x1 < 0 ) {
        if ( x2 < 0 ) {
            return;
        }
        x1 = 0;
    }

    if ( y1 < 0 ) {
        if ( y2 < 0 ) {
            return;
        }
        y1 = 0;
    }

    if ( relativeOrigin || currentScroll == 0 ) {
        if ( currentScroll != 0 ) {
            if ( landscape ) {
                int edge = currentScroll;
                if (x2 >= edge) {
                    if ( x1 >= edge ) {
                        return;
                    } else  {
                        x2 = edge - 1;
                    }
                }
                if (y2 >= height) {
                    y2 = height - 1;
                }
            } else {
                int edge = currentScroll;
                if (y2 >= edge) {
                    if ( y1 >= edge ) {
                        return;
                    } else  {
                        y2 = edge - 1;
                    }
                }
                if (x2 >= width) {
                    x2 = width - 1;
                }
            }
        } else {
            if (x2 >= width) {
                x2 = width - 1;
            }
            if (y2 >= height) {
                y2 = height - 1;
            }
        }
    } else {
        if ( x2 >= width ) {
            x2 = width - 1;
        }
        if ( y2 >= height ) {
            y2 = height - 1;
        }

        if ( currentScroll != 0 ) {
            switch ( orientation ) {
            case PORTRAIT:
            case PORTRAIT_FLIP:
                y1 += currentScroll;
                y2 += currentScroll;
                y1 %= deviceHeight;
                y2 %= deviceHeight;
                if ( y1 > y2 ) {
                    quickFill(color, x1, y1, x2, deviceHeight-1);
                    quickFill(color, x1, 0, x2, y2);
                } else {
                    quickFill(color, x1, y1, x2, y2);
                }
                break;
            case LANDSCAPE:
            case LANDSCAPE_FLIP:
                x1 += currentScroll;
                x2 += currentScroll;
                x1 %= deviceHeight;
                x2 %= deviceHeight;
                if ( x1 > x2 ) {
                    quickFill(color, x1, y1, deviceHeight-1, y2);
                    quickFill(color, 0, y1, x2, y2);
                } else {
                    quickFill(color, x1, y1, x2, y2);
                }
                break;
            }
            return;
        }
    }

    quickFill(color, x1, y1, x2, y2);
}

void PixelsBase::hLine(int16_t x1, int16_t y, int16_t x2) {
    fill(foreground.convertTo565(), x1, y, x2, y);
}

void PixelsBase::vLine(int16_t x, int16_t y1, int16_t y2) {
    fill(foreground.convertTo565(), x, y1, x, y2);
}

void PixelsBase::resetRegion() {
    setRegion(0, 0, deviceWidth, deviceHeight);
}

void PixelsBase::setCurrentPixel(int16_t color) {
    deviceWriteData(highByte(color), lowByte(color));
}

void PixelsBase::setCurrentPixel(RGB color) {
    int16_t c = color.convertTo565();
    deviceWriteData(highByte(c), lowByte(c));
}
