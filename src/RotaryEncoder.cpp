#include "RotaryEncoder.hpp"


RotaryEncoder::RotaryEncoder()
    : encoder{nullptr},
      button{nullptr}

    //   maxEncoderValue {0}
{
}

RotaryEncoder::~RotaryEncoder()
{
    this->reset();
}

RotaryEncoder::Encoder &RotaryEncoder::getEncoder()
{
    assert(this->encoder != nullptr);
    return *this->encoder;
}

RotaryEncoder::Button &RotaryEncoder::getButton()
{
    assert(this->button != nullptr);
    return *this->button;
}


void RotaryEncoder::reset()
{
    // Serial.printf("%s: RotaryEncoder@%p\n", __PRETTY_FUNCTION__, this);
    if (this->encoder)
        delete this->encoder;
    this->encoder = nullptr;
    if (this->button)
        delete this->button;
    this->button = nullptr;
}

void RotaryEncoder::setup(const Settings &settings)
{
    this->reset();

    Serial.printf("%s: RotaryEncoder: ROTARY_PIN1=%u ROTARY_PIN2=%u\n", __PRETTY_FUNCTION__, settings.ROTARY_PIN1, settings.ROTARY_PIN2);
    encoder = new Encoder{settings.ROTARY_PIN1, settings.ROTARY_PIN2, -1, settings.ROTARY_CLICKS_PER_STEP};
    encoder->begin();
    encoder->setup([] { RotaryEncoder::Instance().readEncoder_ISR(); });
    encoder->correctionOffset = 0;

    Serial.printf("%s: RotaryEncoder: BUTTON_PIN=%u\n", __PRETTY_FUNCTION__, settings.BUTTON_PIN);
    button = new Button2{};
    button->reset();
    button->begin(settings.BUTTON_PIN);
    button->setTripleClickHandler([](Button2 &button) { esp_restart(); });
}

void RotaryEncoder::loop()
{
    button->loop();

    // // Loop and read the count
    // if (encoder->encoderChanged())
    //     Serial.println("Encoder count = " + String((int32_t)encoder->readEncoder()));
}

void IRAM_ATTR RotaryEncoder::readEncoder_ISR()
{
    this->encoder->readEncoder_ISR();
}

RotaryEncoder &RotaryEncoder::Instance()
{
    static RotaryEncoder SingletonInstance;
    return SingletonInstance;
}
