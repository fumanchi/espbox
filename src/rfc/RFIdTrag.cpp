#include "RFIdTag.hpp"

const uint32_t RFIdTag::MagicCookie = 27337;
const uint32_t RFIdTag::NoCookie = 0;


namespace 
{
    void dump_byte_array(const byte * buffer, byte bufferSize)
    {
        for (byte i = 0; i < bufferSize; i++) 
        {
            Serial.print(buffer[i] < 0x10 ? " 0" : " ");
            Serial.print(buffer[i], HEX);
        }
    };

}


RFIdTag::RFIdTag()
    : State {},
    cookie {NoCookie},
    version {0}
{
    Serial.printf("%s@%d\n", __PRETTY_FUNCTION__, __LINE__);
}

RFIdTag::RFIdTag(uint16_t folder, uint16_t track, std::initializer_list<Flag> flagsGiven)
    : State {folder, track, flagsGiven},
    cookie {MagicCookie},
    version {1}
{
    Serial.printf("%s@%d #flagsGiven=%d\n", __PRETTY_FUNCTION__, __LINE__, flagsGiven.size());
}

RFIdTag::RFIdTag(const RFIdTag &other)
    : RFIdTag {}
{
    this->operator=(other);
}

RFIdTag::operator bool() const
{
    return this->cookie == MagicCookie && this->version != 0;
}

RFIdTag &RFIdTag::operator=(const RFIdTag &other)
{
    this->State::operator=(other);
    this->cookie = other.cookie;
    this->version = other.version;
    return *this;
}

bool RFIdTag::operator[](Flag flag) const
{
    return this->State::operator[](flag);
}

RFIdTag::FlagReference RFIdTag::operator[](Flag flag)
{
    return this->State::operator[](flag);
}

bool RFIdTag::read(const byte *buffer, byte bufferSize) const
{
    bool retval = bufferSize >= RFIdTag::Size();
    if (retval)
    {
        int offset = 0; 
        memcpy((void*)&this->cookie, buffer + offset, sizeof(RFIdTag::cookie));
        if ((retval = this->cookie == MagicCookie))
        {
            offset += sizeof(RFIdTag::cookie);
            memcpy((void*)&this->version, buffer + offset, sizeof(RFIdTag::version));
            if ((retval = this->version >= 1))
            {
                offset += sizeof(RFIdTag::version);
                offset += 1; /* reserved */
                memcpy((void*)&this->folder, buffer + offset, sizeof(RFIdTag::folder));
                offset += sizeof(RFIdTag::folder);
                memcpy((void*)&this->track, buffer + offset, sizeof(RFIdTag::track));
                offset += sizeof(RFIdTag::track);
                memcpy((void*)&this->flags, buffer + offset, sizeof(RFIdTag::flags));
            }
        }
    }
    this->dump();
    return retval;
}

bool RFIdTag::write(byte *buffer, byte bufferSize) const
{
    bool retval = this->operator bool();
    if (retval)
    {
        memcpy(buffer, (const void*)&this->cookie, 4);
        buffer[4] = this->version;
        buffer[5] = 0u; /* reserved */
        memcpy(&buffer[6], (const void*)&this->folder, 2);
        memcpy(&buffer[8], (const void*)&this->track, 2);
        buffer[10] = this->flags;
    }
    return retval;
}

void RFIdTag::dump() const
{
    Serial.printf("RFIdTag cookie=%u version=%d folder=%u track=%u flags=%u\n", this->cookie, this->version, this->folder, this->track, this->flags);
}

uint8_t RFIdTag::Size()
{
    return sizeof(RFIdTag::cookie) + sizeof(RFIdTag::version) + 1 /* reserved */ + sizeof(RFIdTag::folder) + sizeof(RFIdTag::track) + sizeof(RFIdTag::flags);
}
