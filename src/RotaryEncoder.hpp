#pragma once

#ifndef ROTARYENCODER_HPP_
#define ROTARYENCODER_HPP_

/* #include <ESP32Encoder.h> */
#include <AiEsp32RotaryEncoder.h>
#include <Button2.h>

#include "Settings.hpp"


class RotaryEncoder
{
public:
    using Encoder = AiEsp32RotaryEncoder;
    using Button = Button2;

    typedef std::function<void(Button2 &btn)> ButtonCallbackFunction;
    typedef std::function<byte()> ButtonStateCallbackFunction;

private:
    RotaryEncoder();
    
public:
    ~RotaryEncoder();

    Encoder &getEncoder();
    Button &getButton();

    // rotary encoder
    void setBoundaries(long minEncoderValue = -100, long maxEncoderValue = 100, bool circleValues = false);

    long readEncoder();
    void setEncoderValue(long newValue); 
    long encoderChanged();

    unsigned long getAcceleration();
	void setAcceleration(unsigned long acceleration);
	void disableAcceleration();

    // button
    void setClickHandler(ButtonCallbackFunction callback);
    void setLongClickHandler(ButtonCallbackFunction callback);
    void setDoubleClickHandler(ButtonCallbackFunction callback); 
    void setLongClickTime(unsigned int ms);

    void reset();
    void setup(const Settings &settings);
    void loop();

    static RotaryEncoder &Instance();

private:
   	void IRAM_ATTR readEncoder_ISR();

private:
    Encoder *encoder;
    Button *button;

    // long maxEncoderValue;
};

inline void RotaryEncoder::setBoundaries(long minEncoderValue, long maxEncoderValue, bool circleValues)
{
    assert(this->encoder);
    this->encoder->setBoundaries(minEncoderValue, maxEncoderValue, circleValues);
}

inline long RotaryEncoder::readEncoder()
{
    assert(this->encoder);
    return this->encoder->readEncoder();
}

inline void RotaryEncoder::setEncoderValue(long newValue)
{
    assert(this->encoder);
    this->encoder->setEncoderValue(newValue);
}

inline long RotaryEncoder::encoderChanged()
{
    assert(this->encoder);
    return this->encoder->encoderChanged();
}

inline unsigned long RotaryEncoder::getAcceleration()
{
    assert(this->encoder);
    return this->encoder->getAcceleration();
}	

inline void RotaryEncoder::setAcceleration(unsigned long acceleration)
{
    assert(this->encoder);
    this->encoder->setAcceleration(acceleration);
}	

inline void RotaryEncoder::disableAcceleration()
{
    assert(this->encoder);
    this->encoder->disableAcceleration();
}

inline void RotaryEncoder::setClickHandler(ButtonCallbackFunction callback)
{
    assert(this->button);
    this->button->setClickHandler(callback);
}

inline void RotaryEncoder::setLongClickHandler(ButtonCallbackFunction callback)
{
    assert(this->button);
    this->button->setLongClickHandler(callback);
}

inline void RotaryEncoder::setDoubleClickHandler(ButtonCallbackFunction callback)
{
    assert(this->button);
    this->button->setDoubleClickHandler(callback);
}

inline void RotaryEncoder::setLongClickTime(unsigned int ms)
{
    assert(this->button);
    this->button->setLongClickTime(ms);
}

#endif /* ROTARYENCODER_HPP_ */