#pragma once

#ifndef TFTCLOCK_HPP_
#define TFTCLOCK_HPP_

#include <TFT_eSPI.h>

class TFT;

class TFTClock
{
public:
    TFTClock(TFT &tft);
    ~TFTClock() = default;

    void setBackgroundColor(uint16_t color);

    void setup();
    void loop();

protected:
    void getCoord(int16_t x, int16_t y, float *xp, float *yp, int16_t r, float a);
    void renderFace();

protected:
    TFT &tft;
    TFT_eSprite face;

    uint16_t backgroundColor;
};

#endif /* TFTCLOCK_HPP_ */