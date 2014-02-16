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

#include "Pixels.h"

#ifdef PIXELS_MAIN
#error Pixels_Antialiasing.h must be included before Pixels_<CONTROLLER>.h
#endif

#ifndef PIXELS_ANTIALIASING_H
#define PIXELS_ANTIALIASING_H

class PixelsAntialiased : public PixelsBase {
protected:
    virtual void drawCircleAntialiaced(int16_t x, int16_t y, int16_t radius, boolean bordermode);
    virtual void drawFatLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    virtual void drawLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    virtual void drawRoundRectangleAntialiased(int16_t x, int16_t y, int16_t width, int16_t height, int16_t rx, int16_t ry, boolean bordermode);

public:
    virtual void enableAntialiasing(boolean enable) {
        antialiasing = enable;
    }

    PixelsAntialiased(uint16_t width, uint16_t height) : PixelsBase( width, height) {
    }
};

void PixelsAntialiased::drawLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {

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

void PixelsAntialiased::drawFatLineAntialiased(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
#ifdef ENABLE_FAT_LINES
// the code still needs to be completed. Problems by line caps

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
#else
    drawLineAntialiased(x1, y1, x2, y2);
#endif
}

void PixelsAntialiased::drawRoundRectangleAntialiased(int16_t x, int16_t y, int16_t width, int16_t height, int16_t rx, int16_t ry, boolean bordermode) {

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
        vLine(x, y - ry, y + ry);
        return;
    }

    if (ry == 0) {
        hLine(x - rx, y, x + rx);
        return;
    }

    a2 = rx * rx;
    b2 = ry * ry;

    ds = a2 << 1;
    dt = b2 << 1;

    xc2 = x << 1;
    yc2 = y << 1;

    sab = sqrt((double)(a2 + b2));
    od = (int)round(sab*0.01) + 1;
    dxt = (int)round((double)a2 / sab) + od;

    t = 0;
    s = -2 * a2 * ry;
    d = 0;

    xp = x + rx;
    yp = y;

    hLine(x + rx, y + height, x + width - rx);
    hLine(x + rx, y, x + width - rx );
    vLine(x + width, y + ry, y + height - ry);
    vLine(x, y + ry, y + height - ry);

    for (i = 1; i <= dxt; i++) {
        xp--;
        d += t - b2;

        if (d >= 0) {
            ys = yp - 1;
        } else if ((d - s - a2) > 0) {
            if (((d << 1) - s - a2) >= 0) {
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
            if (((d << 1) + t - b2) <= 0) {
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

void PixelsAntialiased::drawCircleAntialiaced( int16_t x, int16_t y, int16_t radius, boolean bordermode )	{
    drawRoundRectangleAntialiased(x-radius, y-radius, radius<<1, radius<<1, radius, radius, bordermode);
}
#endif
