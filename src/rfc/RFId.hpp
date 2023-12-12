#ifndef RFId_H
#define RFId_H

#include <functional>

#include <SPI.h>
#include <MFRC522.h>

#include "../mp3/Mp3Player.hpp"

#include "RFIdTag.hpp"


class RFId
{
public:
    typedef std::function<void(int value)> ProgressListener;

public:
    RFId();
    ~RFId() = default;

    void setup(const Settings &settings);

    bool readCard(RFIdTag &tag);
    bool writeCard(const RFIdTag &tag, bool force = false);
    bool clearCard();
    bool checkCardPresent();
    // bool waitCard(int timeout, Menu *menu, const String &title, uint16_t glyph = 0, const String &text = "");
    bool waitCard(int timeout, ProgressListener progressListener = {});
    // bool waitCardLeft(int timeout, Menu *menu, const String &title, uint16_t glyph = 0, const String &text = "");
    bool waitCardLeft(int timeout = 0, ProgressListener progressListener = {});

    MFRC522 mfrc522;
};

#endif /* RFId_H */