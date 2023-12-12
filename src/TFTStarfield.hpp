#pragma once

#ifndef TFTSTARFIELD_HPP_
#define TFTSTARFIELD_HPP_

class TFT;

#define TFT_STARFIELD_NSTARS 1024

class TFTStarfield
{
public:
    TFTStarfield(TFT &tft);
    ~TFTStarfield() = default;

    void setup();
    void reset();
    void loop();

protected:
    // Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
    uint8_t __attribute__((always_inline)) rng()
    {
        zx++;
        za = (za^zc^zx);
        zb = (zb+za);
        zc = ((zc+(zb>>1))^za);
        return zc;
    }


protected:
    TFT &tft;

    uint8_t sx[TFT_STARFIELD_NSTARS];
    uint8_t sy[TFT_STARFIELD_NSTARS];
    uint8_t sz[TFT_STARFIELD_NSTARS];

    uint8_t za, zb, zc, zx;

};

#endif /* TFT_STARFIELD_HPP_ */