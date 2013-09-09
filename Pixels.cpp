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


Pixels::Pixels() {
    deviceWidth = 240;
    deviceHeight = 320;
    this->width = 240;
    this->height = 320;
    orientation = PORTRAIT;

    relativeOrigin = true;

    currentScroll = 0;
    scrollSupported = true;
    scrollEnabled = true;

    lineWidth = 1;
    fillDirection = 0;

    setBackground(0,0,0);
    setColor(0xFF,0xFF,0xFF);
}

Pixels::Pixels(uint16_t width, uint16_t height) {
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

void Pixels::setColor(uint8_t r, uint8_t g, uint8_t b) {
    setColor(RGB(r, g, b));
}

void Pixels::setColor(RGB color) {
    foreground = color;
}

RGB Pixels::getColor() {
    return foreground;
}

void Pixels::setBackground(uint8_t r, uint8_t g, uint8_t b) {
    setBackground(RGB(r, g, b));
}

void Pixels::setBackground(RGB color) {
    background = color;
}

RGB Pixels::getBackground() {
    return background;
}


void Pixels::setOrientation( uint8_t direction ){

    orientation = direction;

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
        landscape = false;
        break;
    default:
        width = deviceWidth;
        height = deviceHeight;
        landscape = false;
        orientation = PORTRAIT;
        break;
    }
}

uint8_t Pixels::getOrientation() {
    return orientation;
}

void Pixels::enableAntialiasing(boolean enable){
    antialiasing = enable;
}

boolean Pixels::isAntialiased(){
    return antialiasing;
}

void Pixels::setOriginRelative() {
    relativeOrigin = true;
}

void Pixels::setOriginAbsolute() {
    relativeOrigin = false;
}

boolean Pixels::isOriginRelative() {
    return relativeOrigin;
}

void Pixels::enableScroll(boolean enable) {
    scrollEnabled = enable;
}

boolean Pixels::canScroll() {
    return scrollEnabled & scrollSupported;
}


/*  Graphic primitives */

void Pixels::clear() {
    RGB sav = getColor();
    setColor(background);
    fillRectangle(0, 0, width, height);
    setColor(sav);
}

RGB Pixels::getPixel(int16_t x, int16_t y) {
    if ( x < 0 || x >= width || y < 0 || y >= height ) {
        return  getBackground();
    }
    return getBackground();
}

void Pixels::setLineWidth(double width) {
    lineWidth = width;
}

double Pixels::getLineWidth() {
    return lineWidth;
}

void Pixels::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

//    if ( landscape ) {
//        int16_t buf = x1;
//        x1 = y1;
//        y1 = buf;
//        buf = x2;
//        x2 = y2;
//        y2 = buf;
//    }

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
                    e2 = 2 * err;
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

void Pixels::drawRectangle(int16_t x, int16_t y, int16_t width, int16_t height) {
    drawLine(x, y, x+width-2, y);
    drawLine(x+width-1, y, x+width-1, y+height-2);
    drawLine(x+1, y+height-1, x+width-1, y+height-1);
    drawLine(x, y+1, x, y+height-1);
}

void Pixels::fillRectangle(int16_t x, int16_t y, int16_t width, int16_t height) {
    int16_t color = foreground.convertTo565();
    quickFill(color, x, y, x+width-1, y+height-1, false);
}

void Pixels::drawRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t r) {

    if ( r < 1 ) {
        drawRectangle(x, y, width, height);
        return;
    }

    int16_t radius = r;
    if ( radius > height / 2 ) {
        radius = height/2;
    }
    if ( radius > width / 2 ) {
        radius = width/2;
    }

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x, y, width, height, radius, radius, 0);
    } else {
        drawLine(x + radius, y + height, x + width - radius, y + height);
        drawLine(x + radius, y, x + width - radius, y );
        drawLine(x + width, y + radius, x + width, y + height - radius);
        drawLine(x, y + radius, x, y + height - radius);

        int16_t shiftX = width - radius * 2;
        int16_t shiftY = height - radius * 2;
        int16_t f = 1 - radius;
        int16_t ddF_x = 1;
        int16_t ddF_y = -radius * 2;
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

void Pixels::fillRoundRectangle(int16_t x, int16_t y, int16_t width, int16_t height, int16_t r) {

    if ( r < 1 ) {
        fillRectangle(x, y, width, height);
        return;
    }

    int16_t radius = r;
    if ( radius > height / 2 ) {
        radius = height/2;
    }
    if ( radius > width / 2 ) {
        radius = width/2;
    }

    int16_t corr = 0;
    for (int16_t j = 0; j < height+1; j++ ) {
        if ( j < radius ||  j > height - radius ) {
            corr = radius;
        } else {
            corr = 0;
        }
        drawLine( x + corr, y+j, x+width-corr, y+j );
    }

    int16_t shiftX = width - radius * 2;
    int16_t shiftY = height - radius * 2;
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -radius * 2;
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

        drawLine(xx + shiftX, yy - y1, xx + shiftX + x1, yy - y1);
        drawLine(xx - x1, yy - y1, xx, yy - y1);
        drawLine(xx + shiftX, yy - x1, xx + shiftX + y1, yy - x1);
        drawLine(xx - y1, yy - x1, xx, yy - x1);

        drawLine(xx + shiftX, yy + y1 + shiftY,  xx + x1 + shiftX, yy + y1 + shiftY);
        drawLine(xx + shiftX, yy + x1 + shiftY, xx + shiftX + y1, yy + x1 + shiftY);
        drawLine(xx - x1, yy + y1 + shiftY, xx, yy + y1 + shiftY);
        drawLine(xx - y1, yy + x1 + shiftY, xx, yy + x1 + shiftY);
    }

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x, y, width, height, radius, radius, true);
    }
}

void Pixels::drawCircle(int16_t x, int16_t y, int16_t r) {

    if ( antialiasing ) {
        drawCircleAntialiaced(x, y, r, false);
    } else {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x1 = 0;
        int16_t y1 = r;

        drawPixel(x, y + r);
        drawPixel(x, y - r);
        drawPixel(x + r, y);
        drawPixel(x - r, y);

        while (x1 < y1) {
            if (f >= 0) {
                y1--;
                ddF_y += 2;
                f += ddF_y;
            }
            x1++;
            ddF_x += 2;
            f += ddF_x;
            drawPixel(x + x1, y + y1);
            drawPixel(x - x1, y + y1);
            drawPixel(x + x1, y - y1);
            drawPixel(x - x1, y - y1);
            drawPixel(x + y1, y + x1);
            drawPixel(x - y1, y + x1);
            drawPixel(x + y1, y - x1);
            drawPixel(x - y1, y - x1);
        }
    }
}

void Pixels::fillCircle(int16_t x, int16_t y, int16_t r) {
    int16_t yy;
    int16_t xx;

    for (yy = -r; yy <= r; yy++) {
        for (xx = -r; xx <= r; xx++) {
            if ((xx * xx) + (yy * yy) <= (r * r)) {
                drawPixel(x+xx, y+yy);
            }
        }
    }

    if ( antialiasing ) {
        drawCircleAntialiaced(x, y, r, true);
    }
}

void Pixels::drawOval(int16_t x, int16_t y, int16_t width, int16_t height) {

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x, y, width, height, width/2, height/2, 0);
    } else {
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
            drawLine(xx, yy, xx, yy + height);
            return;
        }
        if (height == 1) {
            drawLine(xx, yy, xx + width, yy);
            return;
        }

        oh = oi = oj = ok = 0xFFFF;

        if (width > height) {
            ix = 0;
            iy = rx * 64;

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
            iy = ry * 64;

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

void Pixels::fillOval(int16_t xx, int16_t yy, int16_t width, int16_t height) {

    int16_t rx = width/2;
    int16_t ry = height/2;

    int16_t x = xx + width/2;
    int16_t y = yy + height/2;

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

    if (rx == 0) {
        drawLine(x, y - ry, x, y + ry);
        return;
    }

    if (ry == 0) {
        drawLine(x - rx, y, x + rx, y);
        return;
    }

    oh = oi = oj = ok = 0xFFFF;

    if (rx > ry) {
        ix = 0;
        iy = rx * 64;

        do {
            h = (ix + 32) >> 6;
            i = (iy + 32) >> 6;
            j = (h * ry) / rx;
            k = (i * ry) / rx;

            if ((ok != k) && (oj != k)) {
                xph = x + h;
                xmh = x - h;
                if (k > 0) {
                    drawLine(xmh, y + k, xph, y + k);
                    drawLine(xmh, y - k, xph, y - k);
                } else {
                    drawLine(xmh, y, xph, y);
                }
                ok = k;
            }
            if ((oj != j) && (ok != j) && (k != j)) {
                xmi = x - i;
                xpi = x + i;
                if (j > 0) {
                    drawLine(xmi, y + j, xpi, y + j);
                    drawLine(xmi, y - j, xpi, y - j);
                } else {
                    drawLine(xmi, y, xpi, y);
                }
                oj = j;
            }

            ix = ix + iy / rx;
            iy = iy - ix / rx;

        } while (i > h);
    } else {
        ix = 0;
        iy = ry * 64;

        do {
            h = (ix + 32) >> 6;
            i = (iy + 32) >> 6;
            j = (h * rx) / ry;
            k = (i * rx) / ry;

            if ((oi != i) && (oh != i)) {
                xmj = x - j;
                xpj = x + j;
                if (i > 0) {
                    drawLine(xmj, y + i, xpj, y + i);
                    drawLine(xmj, y - i, xpj, y - i);
                } else {
                    drawLine(xmj, y, xpj, y);
                }
                oi = i;
            }
            if ((oh != h) && (oi != h) && (i != h)) {
                xmk = x - k;
                xpk = x + k;
                if (h > 0) {
                    drawLine(xmk, y + h, xpk, y + h);
                    drawLine(xmk, y - h, xpk, y - h);
                } else {
                    drawLine(xmk, y, xpk, y);
                }
                oh = h;
            }

            ix = ix + iy / ry;
            iy = iy - ix / ry;

        } while (i > h);
    }

    if ( antialiasing ) {
        drawRoundRectangleAntialiased(x-rx, y-ry, rx*2, ry*2, rx, ry, true);
    }
}

int8_t Pixels::drawBitmap(int16_t x, int16_t y, int16_t width, int16_t height, int16_t* data) {
    setRegion(x, y, x+width, y+height);
    int16_t ptr = 0;
    for ( int16_t j = 0; j < height; j++ ) {
        for ( int16_t i = 0; i < width; i++ ) {
            int16_t px = data[ptr++];
            setCurrentPixel(px);
        }
    }
    return 0;
}

int8_t Pixels::loadBitmap(int16_t x, int16_t y, int16_t sx, int16_t sy, String path) {
    int16_t* data = loadFileBytes( path );
    return drawBitmap(x, y, sx, sy, data);
}

/*  -------   Antialiasing ------- */

void Pixels::drawLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    boolean steep = (y2 > y1 ? y2 - y1 : y1 - y2) > (x2 > x1 ? x2 - x1 : x1 - x2);
    if (steep) {
        int16_t tmp = x1;
        x1 = y1;
        y1 = tmp;
        tmp = x2;
        x2 = y2;
        y2 = tmp;
    }
    if (x1 > x2) {
        int16_t tmp = x1;
        x1 = x2;
        x2 = tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    int16_t deltax = x2 - x1;
    int16_t deltay = y2 - y1;
    double gradient = 1.0 * deltay / deltax;

    int16_t xend = x1; // round(x1);
    double yend = y1 + gradient * (xend - x1);
    double xgap = rfpart(x1 + 0.5);
    int16_t xpxl1 = xend;
    int16_t ypxl1 = ipart(yend);
    putColor(xpxl1, ypxl1, steep, rfpart(yend)*xgap);
    putColor(xpxl1, ypxl1 + 1, steep, fpart(yend)*xgap);
    double intery = yend + gradient;

    xend = x2; // round(x2);
    yend = y2 + gradient * (xend - x2);
    xgap = rfpart(x2 + 0.5);
    int16_t xpxl2 = xend;
    int16_t ypxl2 = ipart(yend);
    putColor(xpxl2, ypxl2, steep, rfpart(yend)*xgap);
    putColor(xpxl2, ypxl2 + 1, steep, fpart(yend)*xgap);

    for ( int16_t x = xpxl1 + 1; x < xpxl2 - 1; x++ ) {
          putColor(x, ipart(intery), steep, rfpart(intery));
          putColor(x, ipart(intery) + 1, steep, fpart(intery));
          intery += gradient;
    }
}

void Pixels::drawFatLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

    double wd = lineWidth;

    int16_t dx = abs(x2 - x1);
    int16_t sx = x1 < x2 ? 1 : -1;
    int16_t dy = abs(y2 - y1);
    int16_t sy = y1 < y2 ? 1 : -1;
    int16_t err = dx - dy;

    int16_t e2;
    int16_t x;
    int16_t y;

    double ed = dx + dy == 0 ? 1 : sqrt((double) dx * dx + (double) dy * dy);

    wd = (wd + 1) / 2;
    while ( true ) {
        putColor(x1, y1, false, 1 - max(0, abs(err-dx+dy)/ed - wd + 1));
        e2 = err;
        x = x1;
        boolean out = false;
        if (2 * e2 >= -dx) { /* x step */
            for (e2 += dy, y = y1; e2 < ed * wd && (y2 != y || dx > dy); e2 += dx) {
                putColor(x1, y += sy, false, 1 - max(0, abs(e2)/ed - wd + 1));
            }
            if (x1 == x2) {
                out = true;
            }
            e2 = err;
            err -= dy;
            x1 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            for (e2 = dx - e2; e2 < ed * wd && (x2 != x || dx < dy); e2 += dy) {
                putColor(x += sx, y1, false, 1 - max(0, abs(e2)/ed - wd + 1));
            }
            if (y1 == y2) {
                out = true;
            }
            err += dx;
            y1 += sy;
        }
        if ( out ) {
            break;
        }
    }
}

void Pixels::drawRoundRectangleAntialiased(int16_t x, int16_t y, int16_t width, int16_t height, int16_t rx, int16_t ry, boolean bordermode) {

    int16_t i;
    int32_t a2, b2, ds, dt, dxt, t, s, d;
    int16_t xp, yp, xs, ys, dyt, od, xx, yy, xc2, yc2;
    float cp;
    double sab;
    double weight, iweight;

    if ((rx < 0) || (ry < 0)) {
        return;
    }

    if (rx == 0) {
        drawLine(x, y - ry, x, y + ry);
        return;
    }

    if (ry == 0) {
        drawLine(x - rx, y, x + rx, y);
        return;
    }

    a2 = rx * rx;
    b2 = ry * ry;

    ds = 2 * a2;
    dt = 2 * b2;

    xc2 = 2 * x;
    yc2 = 2 * y;

    sab = sqrt((double)(a2 + b2));
    od = (int)round(sab*0.01) + 1;
    dxt = (int)round((double)a2 / sab) + od;

    t = 0;
    s = -2 * a2 * ry;
    d = 0;

    xp = x + rx;
    yp = y;

    drawLine(x + rx, y + height, x + width - rx, y + height);
    drawLine(x + rx, y, x + width - rx, y );
    drawLine(x + width, y + ry, x + width, y + height - ry);
    drawLine(x, y + ry, x, y + height - ry);

    for (i = 1; i <= dxt; i++) {
        xp--;
        d += t - b2;

        if (d >= 0) {
            ys = yp - 1;
        } else if ((d - s - a2) > 0) {
            if ((2 * d - s - a2) >= 0) {
                ys = yp + 1;
            } else {
                ys = yp;
                yp++;
                d -= s + a2;
                s += ds;
            }
        } else {
            yp++;
            ys = yp + 1;
            d -= s + a2;
            s += ds;
        }

        t -= dt;

        if (s != 0) {
            cp = (float) abs(d) / (float) abs(s);
            if (cp > 1.0) {
                cp = 1.0f;
            }
        } else {
            cp = 1.0f;
        }

        weight = cp;
        iweight = 1 - weight;

        if( bordermode ) {
            iweight = yp > ys ? 1 : iweight;
            weight = ys > yp ? 1 : weight;
        }

        /* Upper half */
        xx = xc2 - xp;
        putColor(xp, yp, false, iweight);
        putColor(xx+width, yp, false, iweight);

        putColor(xp, ys, false, weight );
        putColor(xx+width, ys, false, weight);

        /* Lower half */
        yy = yc2 - yp;
        putColor(xp, yy+height, false, iweight);
        putColor(xx+width, yy+height, false, iweight);

        yy = yc2 - ys;
        putColor(xp, yy+height, false, weight);
        putColor(xx+width, yy+height, false, weight);
    }

    /* Replaces original approximation code dyt = abs(yp - yc); */
    dyt = (int)round((double)b2 / sab ) + od;

    for (i = 1; i <= dyt; i++) {
        yp++;
        d -= s + a2;

        if (d <= 0) {
            xs = xp + 1;
        } else if ((d + t - b2) < 0) {
            if ((2 * d + t - b2) <= 0) {
                xs = xp - 1;
            } else {
                xs = xp;
                xp--;
                d += t - b2;
                t -= dt;
            }
        } else {
            xp--;
            xs = xp - 1;
            d += t - b2;
            t -= dt;
        }

        s += ds;

        if (t != 0) {
            cp = (float) abs(d) / (float) abs(t);
            if (cp > 1.0) {
                cp = 1.0f;
            }
        } else {
            cp = 1.0f;
        }

        weight = cp;
        iweight = 1 - weight;

        /* Left half */
        xx = xc2 - xp;
        yy = yc2 - yp;
        putColor(xp, yp, false, iweight);
        putColor(xx+width, yp, false, iweight);

        putColor(xp, yy+height, false, iweight);
        putColor(xx+width, yy+height, false, iweight);

        /* Right half */
        xx = xc2 - xs;
        putColor(xs, yp, false, weight);
        putColor(xx+width, yp, false, weight);

        putColor(xs, yy+height, false, weight);
        putColor(xx+width, yy+height, false, weight);
    }
}

void Pixels::drawCircleAntialiaced( int16_t x, int16_t y, int16_t radius, boolean bordermode )	{
    drawRoundRectangleAntialiased(x-radius, y-radius, radius*2, radius*2, radius, radius, bordermode);
}

/* TEXT */


int16_t Pixels::setFont(prog_uchar font[]) {
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

void Pixels::print(int16_t xx, int16_t yy, String text, int8_t kerning[]) {
	printString(xx, yy, text, 0, kerning);
}

void Pixels::cleanText(int16_t xx, int16_t yy, String text, int8_t kerning[]) {
	printString(xx, yy, text, 1, kerning);
}

void Pixels::printString(int16_t xx, int16_t yy, String text, boolean clean, int8_t kerning[]) {

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
                found = true;

                width = 0xff & pgm_read_byte_near(currentFont + ptr + 4);

                int16_t marginLeft = 0x7f & pgm_read_byte_near(currentFont + ptr + 5);
                int16_t marginTop = 0xff & pgm_read_byte_near(currentFont + ptr + 6);
                int16_t marginRight = 0x7f & pgm_read_byte_near(currentFont + ptr + 7);
                int16_t effWidth = width - marginLeft - marginRight;

                int16_t ctr = 0;

                if ( fontType == ANTIALIASED_FONT ) {

                    boolean vraster = (0x80 & pgm_read_byte_near(currentFont + ptr + 5)) > 0;

                    if ( vraster ) {
                        int16_t marginBottom = marginRight;
                        int16_t effHeight = glyphHeight - marginTop - marginBottom;

                        for ( int16_t i = 0; i < length - 8; i++ ) {
                            int16_t b = 0xff & pgm_read_byte_near(currentFont + ptr + 8 + i);
                            int16_t x = ctr / effHeight;
                            int16_t y = ctr % effHeight;

                            if ( (0xc0 & b) > 0 ) {
                                int16_t len = 0x3f & b;
                                ctr += len;
                                if ( (0x80 & b) > 0 ) {
                                    if ( clean ) {
                                        setColor(background.red, background.green, background.blue);
                                    } else {
                                        setColor(fg.red, fg.green, fg.blue);
                                    }
                                    while ( y + len > effHeight ) {
                                        vLine(x1 + marginLeft + x, yy + marginTop + y, yy + marginTop + effHeight - 1);
                                        len -= effHeight - y;
                                        y = 0;
                                        x++;
                                    }
                                    vLine(x1 + marginLeft + x, yy + marginTop + y, yy + marginTop + y + len - 1);
                                }
                            } else {
                                if ( clean ) {
                                    setColor(background.red, background.green, background.blue);
								} else {
                                    uint8_t opacity = (0xff & (b * 4));
                                    RGB cl = computeColor(fg, opacity);
                                    setColor(cl);
                                }
                                drawPixel(x1 + marginLeft + x, yy + marginTop + y);
                                ctr++;
                            }
                        }

                    } else {

                        for ( int16_t i = 0; i < length - 8; i++ ) {
                            int16_t b = 0xff & pgm_read_byte_near(currentFont + ptr + 8 + i);
                            int16_t x = ctr % effWidth;
                            int16_t y = ctr / effWidth;

                            if ( (0xc0 & b) > 0 ) {
                                int16_t len = 0x3f & b;
                                ctr += len;
                                if ( (0x80 & b) > 0 ) {
                                    if ( clean ) {
                                        setColor(background.red, background.green, background.blue);
									} else {
                                        setColor(fg.red, fg.green, fg.blue);
                                    }
                                    while ( x + len > effWidth ) {
                                        hLine(x1 + marginLeft + x, yy + marginTop + y, x1 + marginLeft + effWidth - 1);
                                        len -= effWidth - x;
                                        x = 0;
                                        y++;
                                    }
                                    hLine(x1 + marginLeft + x, yy + marginTop + y, x1 + marginLeft + x + len - 1);
                                }
                            } else {
                                if ( clean ) {
                                    setColor(background.red, background.green, background.blue);
								} else {
                                    uint8_t opacity = (0xff & (b * 4));
                                    RGB cl = computeColor(fg, opacity);
                                    setColor(cl);
                                }
                                drawPixel(x1 + marginLeft + x, yy + marginTop + y);
                                ctr++;
                            }
                        }
                    }

                } else if ( fontType == BITMASK_FONT ) {

                    if ( clean ) {
                        setColor(background.red, background.green, background.blue);
//					} else {
//                        setColor(fg.red, fg.green, fg.blue );
					}

                    boolean compressed = (pgm_read_byte_near(currentFont + ptr + 7) & 0x80) > 0;
                    if ( compressed ) {
                        boolean vraster = (pgm_read_byte_near(currentFont + ptr + 5) & 0x80) > 0;
                        if ( vraster ) {
                            int16_t marginBottom = marginRight;
                            int16_t effHeight = glyphHeight - marginTop - marginBottom;

                            for ( int16_t i = 0; i < length - 8; i++ ) {
                                int16_t len = 0x7f & pgm_read_byte_near(currentFont + ptr + 8 + i);
                                boolean color = (0x80 & pgm_read_byte_near(currentFont + ptr + 8 + i)) > 0;
                                if ( color ) {
                                    int16_t x = ctr / effHeight;
                                    int16_t y = ctr % effHeight;
                                    while ( y + len > effHeight ) {
                                        vLine(x1 + marginLeft + x, yy + marginTop + y, yy + marginTop + effHeight);
                                        len -= effHeight - y;
                                        ctr += effHeight - y;
                                        y = 0;
                                        x++;
                                    }
                                    vLine(x1 + marginLeft + x, yy + marginTop + y, yy + marginTop + y + len);
                                }
                                ctr += len;
                            }
                        } else {
                            for ( int16_t i = 0; i < length - 8; i++ ) {
                                int16_t len = 0x7f & pgm_read_byte_near(currentFont + ptr + 8 + i);
                                boolean color = (0x80 & pgm_read_byte_near(currentFont + ptr + 8 + i)) > 0;
                                if ( color ) {
                                    int16_t x = ctr % effWidth;
                                    int16_t y = ctr / effWidth;
                                    while ( x + len > effWidth ) {
                                        hLine(x1 + marginLeft + x, yy + marginTop + y, x1 + marginLeft + effWidth);
                                        len -= effWidth - x;
                                        ctr += effWidth - x;
                                        x = 0;
                                        y++;
                                    }
                                    hLine(x1 + marginLeft + x, yy + marginTop + y, x1 + marginLeft + x + len);
                                }
                                ctr += len;
                            }
                        }
                    } else {
                        for ( int16_t i = 0; i < length - 8; i++ ) {
                            int16_t b = 0xff & pgm_read_byte_near(currentFont + ptr + 8 + i);
                            int16_t x = i * 8 % effWidth;
                            int16_t y = i * 8 / effWidth;
                            for ( int16_t j = 0; j < 8; j++ ) {
                                if ( x + j == effWidth ) {
                                    x = -j;
                                    y++;
                                }
                                int16_t mask = 1 << (7 - j);
                                if ( (b & mask) == 0 ) {
                                    vLine(x1 + marginLeft + x + j, yy + marginTop + y, yy + marginTop + y + 1);
                                }
                            }
                        }
                    }
                }
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

int16_t Pixels::getTextLineHeight() {
    if ( currentFont == NULL ) {
        return 0;
    }

    int16_t fontType = pgm_read_byte_near(currentFont + 2);
    if ( fontType != ANTIALIASED_FONT && fontType != BITMASK_FONT ) {
        return 0;
    }

    return pgm_read_byte_near(currentFont + 3);
}

int16_t Pixels::getTextBaseline() {
    if ( currentFont == NULL ) {
        return 0;
    }

    int16_t fontType = pgm_read_byte_near(currentFont + 2);
    if ( fontType != ANTIALIASED_FONT && fontType != BITMASK_FONT ) {
        return 0;
    }

    return pgm_read_byte_near(currentFont + 4);
}

int16_t Pixels::getTextWidth(String text, int8_t kerning[]) {
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
                Serial.print( c );
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


/* Low level */

void Pixels::putColor(int16_t x, int16_t y, boolean steep, double alpha) {
    if ( x < 0 || x >= width || y < 0 || y >= height ) {
        return;
    }
    if ( steep ) {
        int16_t tmp = x;
        x = y;
        y = tmp;
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

RGB Pixels::computeColor(RGB bg, double alpha) {
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

RGB Pixels::computeColor(RGB fg, uint8_t opacity) {
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

void Pixels::scroll(int16_t dy, int8_t flags) {
    scroll(dy, 0, deviceWidth, flags);
}

void Pixels::scroll(int16_t dy, int16_t x1, int16_t x2, int8_t flags) {

    if(!canScroll()) {
        return;
    }

    int16_t mdy = dy > 0 ? dy : -dy;

    if (mdy > 1 && (flags & SCROLL_SMOOTH) > 0) {

        int16_t easingLen = 7;
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

        cbi(registerCS, bitmaskCS);
        deviceWriteCmd(0x6A);
        deviceWriteData(highByte(currentScroll), lowByte(currentScroll));
        sbi(registerCS, bitmaskCS);

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


void Pixels::drawPixel(int16_t x, int16_t y) {

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
            x = (x + deviceHeight + currentScroll) % deviceHeight;
        } else {
            y = (y + deviceHeight + currentScroll) % deviceHeight;
        }
    }

    cbi(registerCS, bitmaskCS);
    setRegion(x, y, x, y);
    setCurrentPixel(foreground);
    sbi(registerCS, bitmaskCS);
}

void Pixels::hLine(int16_t x1, int16_t y, int16_t x2) {
    int16_t color = foreground.convertTo565();
    quickFill(color, x1, y, x2, y, false);
}

void Pixels::vLine(int16_t x, int16_t y1, int16_t y2) {
    int16_t color = foreground.convertTo565();
    quickFill(color, x, y1, x, y2, false);
}


void Pixels::deviceWriteCmd(uint8_t b) {
    cbi(registerRS, bitmaskRS);
    deviceWrite(0x00, b);
}

void Pixels::deviceWriteData(uint8_t hi, uint8_t lo) {
    sbi(registerRS, bitmaskRS);
    deviceWrite(hi, lo);
}

void Pixels::deviceWriteCmdData(uint8_t cmd, uint16_t data) {
     deviceWriteCmd(cmd);
     deviceWriteData(highByte(data), lowByte(data));
}

void Pixels::init() {

    int16_t pinRS = 38;
    int16_t pinWR = 39;
    int16_t pinCS = 40;
    int16_t pinRST = 41;
    int16_t pinRD = -1;

    DDRA = 0xFF;

    registerRS	= portOutputRegister(digitalPinToPort(pinRS));
    registerWR	= portOutputRegister(digitalPinToPort(pinWR));
    registerCS	= portOutputRegister(digitalPinToPort(pinCS));
    registerRST	= portOutputRegister(digitalPinToPort(pinRST));
    if ( pinRD > 0 ) {
        registerRD	= portOutputRegister(digitalPinToPort(pinRD));
    }

    bitmaskRS	= digitalPinToBitMask(pinRS);
    bitmaskWR	= digitalPinToBitMask(pinWR);
    bitmaskCS	= digitalPinToBitMask(pinCS);
    bitmaskRST	= digitalPinToBitMask(pinRST);
    if ( pinRD > 0 ) {
        bitmaskRD	= digitalPinToBitMask(pinRD);
    }

    pinMode(pinRS,OUTPUT);
    pinMode(pinWR,OUTPUT);
    pinMode(pinCS,OUTPUT);
    pinMode(pinRST,OUTPUT);

    sbi(registerRST, bitmaskRST);
    delay(5);
    cbi(registerRST, bitmaskRST);
    delay(15);
    sbi(registerRST, bitmaskRST);
    delay(15);

    cbi(registerCS, bitmaskCS);

    deviceWriteCmdData(0xE5, 0x78F0);
    deviceWriteCmdData(0x01, 0x0100);
    deviceWriteCmdData(0x02, 0x0700); // line inversion
    deviceWriteCmdData(0x03, 0x1030); // write direction; alternatively 1038
    deviceWriteCmdData(0x04, 0x0000);
    deviceWriteCmdData(0x08, 0x0302);
    deviceWriteCmdData(0x09, 0x0000);
    deviceWriteCmdData(0x0A, 0x0000);

    deviceWriteCmdData(0x0C, 0x0000);
    deviceWriteCmdData(0x0D, 0x0000);
    deviceWriteCmdData(0x0F, 0x0000);

    // Power control
    deviceWriteCmdData(0x10, 0x0000);
    deviceWriteCmdData(0x11, 0x0007);
    deviceWriteCmdData(0x12, 0x0000);
    deviceWriteCmdData(0x13, 0x0000);
    deviceWriteCmdData(0x07, 0x0001);
    delay(220);
    deviceWriteCmdData(0x10, 0x1090);
    deviceWriteCmdData(0x11, 0x0227);
    delay(60);
    deviceWriteCmdData(0x12, 0x001F);
    delay(60);
    deviceWriteCmdData(0x13, 0x1500);

//  Power control alternative
//    deviceWriteCmdData(0x0010, 0x0000);
//    deviceWriteCmdData(0x0011, 0x0007);
//    deviceWriteCmdData(0x0012, 0x0000);
//    deviceWriteCmdData(0x0013, 0x0000);
//    delay(1000);
//    deviceWriteCmdData(0x0010, 0x14B0);
//    delay(500);
//    deviceWriteCmdData(0x0011, 0x0007);
//    delay(500);
//    deviceWriteCmdData(0x0012, 0x008E);
//    deviceWriteCmdData(0x0013, 0x0C00);

    deviceWriteCmdData(0x29, 0x0027); // 0x0015 ?
    deviceWriteCmdData(0x2B, 0x000D); // Frame rate
    delay(50);

    // Gamma tuning
    deviceWriteCmdData(0x0030, 0x0000);
    deviceWriteCmdData(0x0031, 0x0107);
    deviceWriteCmdData(0x0032, 0x0000);
    deviceWriteCmdData(0x0035, 0x0203);
    deviceWriteCmdData(0x0036, 0x0402);
    deviceWriteCmdData(0x0037, 0x0000);
    deviceWriteCmdData(0x0038, 0x0207);
    deviceWriteCmdData(0x0039, 0x0000);
    deviceWriteCmdData(0x003C, 0x0203);
    deviceWriteCmdData(0x003D, 0x0403);

    deviceWriteCmdData(0x20, 0x0000); // GRAM horizontal Address
    deviceWriteCmdData(0x21, 0x0000); // GRAM Vertical Address

    deviceWriteCmdData(0x50, 0x0000); // Window Horizontal RAM Address Start (R50h)
    deviceWriteCmdData(0x51, 0x00EF); // Window Horizontal RAM Address End (R51h)
    deviceWriteCmdData(0x52, 0x0000); // Window Vertical RAM Address Start (R52h)
    deviceWriteCmdData(0x53, 0x013F); // Window Vertical RAM Address End (R53h)
    deviceWriteCmdData(0x60, 0xA700); // Driver Output Control (R60h) - Gate Scan Line
    deviceWriteCmdData(0x61, 0x0003); // Driver Output Control (R61h) - enable VLE
//    deviceWriteCmdData(0x6A, 0x0000); // set initial scrolling

    deviceWriteCmdData(0x90, 0x0010); // Panel Interface Control 1 (R90h)
    deviceWriteCmdData(0x92, 0x0600);
    deviceWriteCmdData(0x07, 0x0133); // RGB565 color

    sbi (registerCS, bitmaskCS);
}

void Pixels::setFillDirection(uint8_t direction) {
    fillDirection = direction;
//    if ( order ) {
//        deviceWriteCmdData(0x03, 0x1030);
//    } else {
//        deviceWriteCmdData(0x03, 0x1030);
//    }
}

void Pixels::quickFill(int color, int16_t x1, int16_t y1, int16_t x2, int16_t y2, boolean valid) {

    if (x2 < x1) {
        swap(x1, x2);
    }

    if (y2 < y1) {
        swap(y1, y2);
    }

    if (!valid) {
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
                        quickFill(color, x1, y1, x2, deviceHeight-1, true);
                        quickFill(color, x1, 0, x2, y2, true);
                    } else {
                        quickFill(color, x1, y1, x2, y2, true);
                    }
                    break;
                case LANDSCAPE:
                case LANDSCAPE_FLIP:
                    x1 += currentScroll;
                    x2 += currentScroll;
                    x1 %= deviceHeight;
                    x2 %= deviceHeight;
                    if ( x1 > x2 ) {
                        quickFill(color, x1, y1, deviceHeight-1, y2, true);
                        quickFill(color, 0, y1, x2, y2, true);
                    } else {
                        quickFill(color, x1, y1, x2, y2, true);
                    }
                    break;
//                case PORTRAIT_FLIP:
//                    y1 += currentScroll;
//                    y2 += currentScroll;
//                    y1 %= deviceHeight;
//                    y2 %= deviceHeight;
//                    if ( y1 > y2 ) {
//                        quickFill(color, x1, y1, x2, deviceHeight-1, true);
//                        quickFill(color, x1, 0, x2, y2, true);
//                    } else {
//                        quickFill(color, x1, deviceHeight-y1-1, x2, deviceHeight-y2-1, true);
//                    }
//                    break;
//                case LANDSCAPE_FLIP:
//                    x1 += currentScroll;
//                    x2 += currentScroll;
//                    x1 %= deviceHeight;
//                    x2 %= deviceHeight;
//                    if ( x1 > x2 ) {
//                        quickFill(color, x1, y1, deviceHeight-1, y2, true);
//                        quickFill(color, 0, y1, x2, y2, true);
//                    } else {
//                        quickFill(color, deviceHeight-x1-1, y1, deviceHeight-x2-1, y2, true);
//                    }
//                    break;
                }
                return;
            }
        }
    }

    cbi(registerCS, bitmaskCS);

    setRegion(x1, y1, x2, y2);
    int32_t counter = (int32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    sbi(registerRS, bitmaskRS);

    uint8_t lo = lowByte(color);
    uint8_t hi = highByte(color);

    if ( lo == hi ) {
        PORTA = color;
        for (int16_t i = 0; i < counter / 20; i++) {
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
            deviceWriteTwice(lo);
        }
        for (int32_t i = 0; i < counter % 20; i++) {
            deviceWriteTwice(lo);
        }
    } else {
        for (int16_t i = 0; i < counter / 20; i++) {
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
            deviceWrite(hi, lo);
        }
        for (int32_t i = 0; i < counter % 20; i++) {
            deviceWrite(hi, lo);
        }
    }
    sbi(registerCS, bitmaskCS);
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

    deviceWriteCmdData(0x20,x1);
    deviceWriteCmdData(0x21,y1);
    deviceWriteCmdData(0x50,x1);
    deviceWriteCmdData(0x52,y1);
    deviceWriteCmdData(0x51,x2);
    deviceWriteCmdData(0x53,y2);
    deviceWriteCmd(0x22);
}

void Pixels::resetRegion() {
    setRegion(0, 0, deviceWidth, deviceHeight);
}

void Pixels::setCurrentPixel(int16_t color) {
    deviceWriteData(highByte(color), lowByte(color));
}

void Pixels::setCurrentPixel(RGB color) {
    int16_t c = color.convertTo565();
    deviceWriteData(highByte(c), lowByte(c));
}
