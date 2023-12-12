#include "TFT_eSPI.h"

#include "MQTTClient.hpp"

#include "TimeControl.hpp"
        

namespace
{
    // from TFT_eSPI
    static uint32_t rgbTo24(uint8_t r, uint8_t g, uint8_t b)
    {
        uint32_t retval = 0;

        retval += r; retval <<= 8;
        retval += g; retval <<= 8;
        retval += b; retval <<= 8;

        return retval;
    }


    /***************************************************************************************
    ** Function name:           color16to24
    ** Description:             convert 16 bit colour to a 24 bit 888 colour value
    ***************************************************************************************/
    static uint32_t Color16to24(uint16_t color565)
    {
        uint8_t r = (color565 >> 8) & 0xF8; r |= (r >> 5);
        uint8_t g = (color565 >> 3) & 0xFC; g |= (g >> 6);
        uint8_t b = (color565 << 3) & 0xF8; b |= (b >> 5);

        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0);
    }

    /***************************************************************************************
    ** Function name:           color24to16
    ** Description:             convert 24 bit colour to a 16 bit 565 colour value
    ***************************************************************************************/
    static uint16_t Color24to16(uint32_t color888)
    {
        uint16_t r = (color888 >> 8) & 0xF800;
        uint16_t g = (color888 >> 5) & 0x07E0;
        uint16_t b = (color888 >> 3) & 0x001F;

        return (r | g | b);
    }

    static int16_t 
    ToMinutesSinceMidnight(const char *string)
    {
        int16_t retval = -1;
        if (string)
        {
            char *colon = nullptr;
            if (int16_t hours = std::strtoul(string, &colon, 10); errno == 0)
            {
                if ((colon = std::strchr(colon, ':')) != nullptr && errno == 0)
                {
                    int16_t minutes = std::strtoul(colon + 1, nullptr, 10);
                    retval = (int16_t)((hours * 60) + minutes);
                }
            }
        }
        return retval;
    }

    static char *MinutesSinceMidnightToString(uint16_t value, char *buffer)
    {
        uint8_t minutes = value % 60;
        uint8_t hours = (value - minutes) / 60;
        snprintf(buffer, 6, "%02u:%02u", hours, minutes);
        return buffer;
    }
}

TimeControl::TimeControl()
    :   timePointSettings
            {
                TimePointSetting {
                    (uint16_t)ToMinutesSinceMidnight("06:00"),
                    10,
                    80,
                    Color24to16(rgbTo24(200, 200, 0)),
                    String {F("early_time")}
                },
                TimePointSetting {
                    (uint16_t)ToMinutesSinceMidnight("06:30"),
                    25,
                    100,
                    Color24to16(rgbTo24(0, 200, 0)),
                    String {F("wakeup_time")}
                },
                TimePointSetting {
                    (uint16_t)ToMinutesSinceMidnight("08:00"),
                    25,
                    100,
                    Color24to16(rgbTo24(255, 255, 255)),
                    String {F("day_time")}
                },
                TimePointSetting {
                    (uint16_t)ToMinutesSinceMidnight("19:00"),
                    5,
                    50,
                    Color24to16(rgbTo24(200, 0, 0)),
                    String {F("night_time")}
                }
            },
            currentTimePoint {None}        
{

}

TimeControl::TimePoint TimeControl::getCurrentTimePoint() const
{
    return this->currentTimePoint;
}

bool TimeControl::setCurrentTimePoint(TimePoint timePoint)
{
    bool retval = false;
    if ((retval = (timePoint != this->currentTimePoint)))
    {
        this->currentTimePoint = timePoint;
        if (this->onTimePointChange)
            this->onTimePointChange(TimePointProperty::TimePoint, this->timePointSettings[this->currentTimePoint]);
    }
    return retval;
}

bool TimeControl::updateCurrentTimePoint()
{
    bool retval = false;

    struct tm info;
    if (getLocalTime(&info, 100))
    {
        uint16_t minutesSinceMidnight = ((info.tm_hour * 60) + info.tm_min);
        TimePoint timePoint = TimePoint::NightTime;
        for (uint8_t timePointNext = (uint8_t)TimePoint::EarlyTime; (timePointNext <= (uint8_t)TimePoint::NightTime) && (minutesSinceMidnight >= this->timePointSettings[timePointNext].minutesSinceMidnight); )
        {
            timePoint = (TimePoint)timePointNext++;
        }
        retval = this->setCurrentTimePoint(timePoint);
    }
    return retval;
}

TimeControl::TimePointSetting &TimeControl::getCurrentTimePointSetting()
{
    assert(this->currentTimePoint != TimePoint::None && this->currentTimePoint != TimePoint::N_);
    return this->timePointSettings[(uint8_t)this->currentTimePoint];
}

const TimeControl::TimePointSetting &TimeControl::getCurrentTimePointSetting() const
{
    assert(this->currentTimePoint != TimePoint::None && this->currentTimePoint != TimePoint::N_);
    return this->timePointSettings[(uint8_t)this->currentTimePoint];
}

void TimeControl::setup(MQTTClient &mqttClient, OnTimePointChange onTimePointChange)    
{
    this->onTimePointChange = onTimePointChange;
    static auto publishAll = [this, &mqttClient](MQTTClient &mqttClient, const TimePointSettings &timePointSettings) mutable 
        { 
            static char buffer[6];
            String prefix {F("time_settings/")};
            for (int i = 0, end = (int)TimePoint::N_; i < end; ++i)
            {
                String prefixTimePoint = prefix + String(i) + "_" + ToString((TimePoint)i) + "/";            
                mqttClient.publish(prefixTimePoint + F("label"), timePointSettings[i].label);
                mqttClient.loop();
                mqttClient.publish(prefixTimePoint + F("max_volume"), String {timePointSettings[i].maxVolume});        
                mqttClient.loop();
                mqttClient.publish(prefixTimePoint + F("time"), MinutesSinceMidnightToString(timePointSettings[i].minutesSinceMidnight, buffer)); 
                mqttClient.loop();
            }
        };
    mqttClient.publish(String{F("time_settings/update")}, String{F("false")});        
    publishAll(mqttClient, this->timePointSettings);
    mqttClient.subscribe(String{F("time_settings/update")}, [this, &mqttClient](String topic, String payload) mutable 
        { 
            if (MQTTClient::ToBool(payload))
            {
                // Serial.printf("%s: %s <- %s\n", __PRETTY_FUNCTION__, topic, payload);
                publishAll(mqttClient, this->timePointSettings);
            }
            mqttClient.publish(topic, String(F("false")));
            return false;
        });

    for (int i = 0, end = (int)TimePoint::N_; i < end; ++i)
    {
        String prefix {F("time_settings/")};
        auto &timePointSetting = this->timePointSettings[i];

        prefix += String(i) + "_" + ToString((TimePoint)i) + "/";
        mqttClient.subscribe(prefix + F("label"), [this, timePoint = (TimePoint)i, &timePointSetting](String , String payload) mutable 
                { 
                    timePointSetting.label = payload; 
                    if (this->onTimePointChange && timePoint == this->currentTimePoint)
                        this->onTimePointChange(TimePointProperty::Label, timePointSetting);
                    return true; 
                });
        mqttClient.subscribe(prefix + F("max_volume"), [this, timePoint = (TimePoint)i, &timePointSetting, &mqttClient](String topic, String payload) mutable 
                {
                    timePointSetting.maxVolume = payload.toInt(); 
                    if (this->onTimePointChange && timePoint == this->currentTimePoint)
                        this->onTimePointChange(TimePointProperty::MaxVolume, timePointSetting);
                    if (String payloadNew {timePointSetting.maxVolume}; payload != payload)
                        mqttClient.publish(topic, payloadNew); 
                    return false; 
                });        
        mqttClient.subscribe(prefix + F("brightness"), [this, timePoint = (TimePoint)i, &timePointSetting, &mqttClient](String topic, String payload) mutable 
                { 
                    timePointSetting.brightness = std::max(0L, std::min(100L, payload.toInt())); 
                    if (this->onTimePointChange && timePoint == this->currentTimePoint)
                        this->onTimePointChange(TimePointProperty::Brightness, timePointSetting);
                    if (String payloadNew {timePointSetting.brightness}; payloadNew != payload)
                        mqttClient.publish(topic, payloadNew); 
                    return false; 
                });        
        mqttClient.subscribe(prefix + F("color"), [this, timePoint = (TimePoint)i, &timePointSetting, &mqttClient](String topic, String payload) mutable 
                { 
                    timePointSetting.color = Color24to16(strtol(payload.c_str() + (int)(payload[0] == '#'), 0, 16));
                    uint32_t color888 = Color16to24(timePointSetting.color);
                    if (this->onTimePointChange && timePoint == this->currentTimePoint)
                        this->onTimePointChange(TimePointProperty::Color, timePointSetting);
                    char tmp[8]; snprintf(tmp, 8, "#%06X", color888);
                    mqttClient.publish(topic, tmp);
                    return false; 
                });
        mqttClient.subscribe(prefix + F("time"), [this, timePointCurrent = (TimePoint)i, &timePointSettings = this->timePointSettings, &mqttClient](String topic, String payload) mutable 
                { 
                    // Serial.printf("%s: %s <- %s\n", __PRETTY_FUNCTION__, topic, payload);
                    char buffer[6];
                    if (int16_t minutesSinceMidnightNew = ToMinutesSinceMidnight(payload.c_str()); minutesSinceMidnightNew > 0)
                    {
                        TimePoint timePointBefore = (TimePoint)((((uint8_t)timePointCurrent + TimePoint::N_) - 1) % TimePoint::N_);
                        TimePoint timePointAfter = (TimePoint)(((uint8_t)timePointCurrent + 1) % TimePoint::N_);
                        if (timePointCurrent > TimePoint::EarlyTime)
                            minutesSinceMidnightNew = std::max((uint16_t)minutesSinceMidnightNew, timePointSettings[timePointBefore].minutesSinceMidnight);
                        if (timePointCurrent < TimePoint::NightTime)
                            minutesSinceMidnightNew = std::min((uint16_t)minutesSinceMidnightNew, timePointSettings[timePointAfter].minutesSinceMidnight);
                        minutesSinceMidnightNew = std::min((uint16_t)minutesSinceMidnightNew, (uint16_t)((24 * 60 * 60) - 1));
                        timePointSettings[(uint8_t)timePointCurrent].minutesSinceMidnight = minutesSinceMidnightNew;

                        // Serial.printf("%s: this->currentTimePoint=%d\n", __PRETTY_FUNCTION__, (uint8_t)this->currentTimePoint);
                        // Serial.printf("%s: timePointCurrent=%d\n", __PRETTY_FUNCTION__, (uint8_t)timePointCurrent);
                        // Serial.printf("%s: timePointBefore=%d\n", __PRETTY_FUNCTION__, (uint8_t)timePointBefore);
                        // Serial.printf("%s: timePointSettings[this->currentTimePoint].minutesSinceMidnight=%d\n", __PRETTY_FUNCTION__, timePointSettings[(uint8_t)this->currentTimePoint].minutesSinceMidnight);
                        // Serial.printf("%s: timePointSettings[timePointCurrent].minutesSinceMidnight=%d\n", __PRETTY_FUNCTION__, timePointSettings[(uint8_t)timePointCurrent].minutesSinceMidnight);
                        // Serial.printf("%s: timePointSettings[timePointBefore].minutesSinceMidnight=%d\n", __PRETTY_FUNCTION__, timePointSettings[(uint8_t)timePointBefore].minutesSinceMidnight);
                        if (timePointCurrent == this->currentTimePoint || this->currentTimePoint == timePointBefore)
                        {
                            // the currentTimePoint might have changed...
                            this->currentTimePoint = TimePoint::None;

                            // struct tm info;
                            // if (getLocalTime(&info, 100))
                            // {
                            //     TimePointProperty propertyChanged = TimePointProperty::MinutesSinceMidnight;
                            //     uint16_t minutesSinceMidnight = ((info.tm_hour * 60) + info.tm_min);
                            //     if (timePointCurrent == this->currentTimePoint && minutesSinceMidnight < minutesSinceMidnightNew)
                            //     {
                            //         Serial.println("The current/changed time point has moved to a time in the future -> changed current time point to the one before...");
                            //         this->setCurrentTimePoint(timePointBefore);
                            //     }
                            //     else
                            //     if (this->currentTimePoint == timePointBefore && minutesSinceMidnight >= minutesSinceMidnightNew)
                            //     {
                            //         this->setCurrentTimePoint(timePointCurrent);
                            //         Serial.println("The next time point has moved to a time before now -> changed current time point to the next/changed one...");
                            //     }
                            //     else
                            //     {
                            //         Serial.println("The current time point has not changed...");
                            //         if (this->onTimePointChange)
                            //             this->onTimePointChange(TimePointProperty::MinutesSinceMidnight, this->getCurrentTimePointSetting());
                            //         else
                            //             Serial.println("onTimePointChange _not_ specified...");
                            //     }
                            // }
                        }
                    }
                    // Serial.printf("%s: %s -> %s (%u)\n", __PRETTY_FUNCTION__, topic, MinutesSinceMidnightToString(timePointSettings[timePointCurrent].minutesSinceMidnight, buffer), timePointSettings[timePointCurrent].minutesSinceMidnight);
                    MinutesSinceMidnightToString(timePointSettings[timePointCurrent].minutesSinceMidnight, buffer);
                    if (strcmp(buffer, payload.c_str()))
                        mqttClient.publish(topic, buffer);
                    return false;
                });        
    }

    // this->loop();
}

void TimeControl::loop()
{
    if (this->currentTimePoint == TimePoint::None)
        this->updateCurrentTimePoint();
    else
    {
        struct tm info;
        if (getLocalTime(&info, 100))
        {
            uint16_t minutesSinceMidnight = ((info.tm_hour * 60) + info.tm_min);
            TimePoint timePointAfter = (TimePoint)(((uint8_t)this->currentTimePoint + 1) % TimePoint::N_);
            if (minutesSinceMidnight >= this->timePointSettings[timePointAfter].minutesSinceMidnight && (timePointAfter != TimePoint::EarlyTime || minutesSinceMidnight < this->timePointSettings[TimePoint::NightTime].minutesSinceMidnight))
                this->setCurrentTimePoint(timePointAfter);
        }
    }        
}
        
void TimeControl::reconnect(MQTTClient &mqttClient)
{
    mqttClient.publish(String {F("time_settings/update")}, String {F("false")});
}

TimeControl &TimeControl::Instance()
{
    static TimeControl StaticInstance {};
    return StaticInstance;
}

const String &TimeControl::ToString(TimePoint timePoint)
{
    static const std::array<String, TimePoint::N_ + 2> Strings
        {
            String {F("<NONE>")},
            String {F("EarlyTime")},
            String {F("WakeUpTime")},
            String {F("DayTime")},
            String {F("NightTime")},
            String {F("<N>")}
        };

    return Strings[((int)timePoint) + 1];
}