#pragma once

#ifndef TFTSTATUS_HPP_
#define TFTSTATUS_HPP_

#include <list>

#include <TFT_eSPI.h>


class TFT;

class TFTStatus
{
public:
    enum Section
    {
        TITLE   = 0x1,
        TEXT    = 0x2,
        BORDER  = 0x4,

        ALL     = TITLE | TEXT | BORDER
    };

public:
    TFTStatus(TFT &tft);
    ~TFTStatus() = default;

    void setup(const String &title, uint8_t max = 5);
    void update(Section section = Section::ALL);

    void setTitle(const String &title);
    void set(const String &line);
    void push(const String &line);


protected:
    void getCoord(int16_t x, int16_t y, float *xp, float *yp, int16_t r, float a);
    void renderFace();

protected:
    TFT &tft;
    
    String title;
    uint8_t max;
    std::list<String> buffer;

};

#endif /* TFTSTATUS_HPP_ */