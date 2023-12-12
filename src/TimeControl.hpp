#pragma once

#ifndef TIMECONTROL_HPP_
#define TIMECONTROL_HPP_

class MQTTClient;

class TimeControl
{
public:
    enum TimePoint : int8_t
    {
        None = -1,

        EarlyTime,
        WakeUpTime,
        DayTime,
        NightTime,

        N_
    };

    enum class TimePointProperty : uint8_t
    {
        None,

        MinutesSinceMidnight,
        MaxVolume,
        Brightness,
        Color,
        Label,

        All,
        TimePoint = All
    };
    

    struct TimePointSetting
    {
        uint16_t minutesSinceMidnight;
        uint8_t maxVolume;
        uint8_t brightness;
        uint16_t color;
        String label;
    };

    using OnTimePointChange = void(*)(TimePointProperty, const TimePointSetting&);

private:
    TimeControl();

public:
    ~TimeControl() = default;

    TimePoint getCurrentTimePoint() const;
    bool setCurrentTimePoint(TimePoint timePoint);
    bool updateCurrentTimePoint();

    TimePointSetting &getCurrentTimePointSetting();
    const TimePointSetting &getCurrentTimePointSetting() const;

    void setup(MQTTClient &mqttClient, OnTimePointChange onTimePointChange);
    void loop();

    void reconnect(MQTTClient &mqttClient);

    static TimeControl &Instance();

    static const String &ToString(TimePoint timepoint);
    
private:
    using TimePointSettings = std::array<TimePointSetting, N_>;
    TimePointSettings timePointSettings;

    TimePoint currentTimePoint;

    OnTimePointChange onTimePointChange;

};

#endif