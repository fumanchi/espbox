#pragma once

#ifndef TFT_HPP_
#define TFT_HPP

#include <TFT_eSPI.h> // Master copy here: https://github.com/Bodmer/TFT_eSPI
#include <SPI.h>


class TFT
{
public:
    TFT(uint8_t brightness = 50);
    ~TFT() = default;

    TFT_eSPI &getHandle();

    void setBrightness(uint8_t percent);
    void enableBacklight(bool on = true);

    void setup(); 

protected:
    TFT_eSPI tft;

    uint8_t brightness;

};

#endif /* TFT_HPP_ */
