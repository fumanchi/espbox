#ifndef RFIdTAG_H
#define RFIdTAG_H

#include <Arduino.h>
#include <stdint.h>

// #include <crc.h>

#include "../mp3/Mp3Player.hpp"

class RFIdTag : public Mp3Player::State
{
  public: /* types */
    using State = Mp3Player::State;
    using Flag = State::Flag;

    static const uint32_t MagicCookie;
    static const uint32_t NoCookie;  

  public: 
    RFIdTag();
    RFIdTag(uint16_t folder, uint16_t track, std::initializer_list<Flag> flags = {});
    
    RFIdTag(const RFIdTag &other);
    ~RFIdTag() = default;

    operator bool() const;
    RFIdTag &operator=(const RFIdTag &other);

    bool operator[](Flag flag) const;
    FlagReference operator[](Flag flag);

    bool read(const byte *buffer, byte bufferSize) const;
    bool write(byte *buffer, byte bufferSize) const;
    
    void dump() const;
    
    static uint8_t Size();

  public: /* members */
    uint32_t cookie;  
    byte version;
};

#endif