#pragma once

#ifndef MQTTCLIENT_HPP_
#define MQTTCLIENT_HPP_

#include <initializer_list>
#include <vector>
#include <list>
#include <unordered_map>
#include <functional>

#include <WString.h>

#include <espMqttClientAsync.h>

#include "Settings.hpp"

namespace std
{
    template<>
    struct hash<String> : public unary_function<String, bool>
    {
        std::size_t operator()(String const& s) const noexcept
        {
            std::size_t retval = 0;
            static std::hash<char> Hasher;
            for (const char &c: s)
                retval = retval * 31 + Hasher(c);
            return retval;
        }

    };

    template<>
    struct equal_to<String> : public unary_function<String, bool>
    {
        bool operator()(const String& x, const String& y) const
        {
            return x.operator==(y);
        }
    };

}

class MQTTClient
{
public: /* types */      
    typedef std::function<bool(String, String)> Subscriber;
    typedef std::function<void()> PublishedCallback;

public:
    MQTTClient(const Settings &settings, const String &mqttName);
    ~MQTTClient() = default;

    void setup();
    void loop();

    bool isConnected() const;
    bool ready() const; // isConnected() && queueSize() == 0

    bool connect(bool blocking);
    bool disconnect();

    bool publish(const String &topic, const String &value, PublishedCallback publishedCallback = nullptr);
    bool subscribe(String topic, Subscriber subscriber, bool autocCofirm = true);


    static bool ToBool(const String &value);

private:
    using DisconnectReason = espMqttClientTypes::DisconnectReason;
    using SubscribeReturncode = espMqttClientTypes::SubscribeReturncode;
    using MessageProperties = espMqttClientTypes::MessageProperties;

private:
    void connectToMqtt();
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(DisconnectReason reason);
    void onMqttSubscribe(uint16_t packetId, const SubscribeReturncode* codes, size_t len);
    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(const MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);

private:
    struct Subscribtion
    {
        Subscriber subscriber;
        bool autoConfirm;
    };

private:
    bool resubscribe();
    bool resubscribe(const String &topic, const Subscriber &subscriber);

    // void publish();

    void mqttCallback(char* topic, uint8_t* payload, unsigned int length);

private:
    const Settings &settings;
    uint32_t chipId;
    String mqttName;

    mutable espMqttClientAsync mqttClient;
    bool reconnectMqtt = false;
    uint32_t lastReconnect = 0;


    typedef std::unordered_map<String, Subscribtion> Subscribtions;
    Subscribtions subscribtions;
    bool needsResubscribtion;

    // struct Publishing
    // {
    //     String topic;
    //     String value;
    //     PublishedCallback publishedCallback;
    // };    
    // std::vector<Publishing> publishings;

    typedef std::list<std::tuple<uint16_t, String, PublishedCallback>> PublishedCallbacks;
    PublishedCallbacks publishedCallbacks;

    std::vector<char> buffer;
};

inline bool MQTTClient::ToBool(const String &value)
{
    bool retval = false;
    retval |= value.length() == 1 && value[0] == '1';
    retval |= value.length() == 2 && value.equalsIgnoreCase("on");
    retval |= value.length() == 3 && value.equalsIgnoreCase("yes");
    retval |= value.length() == 4 && value.equalsIgnoreCase("true");
    return retval;
}

#endif /* MQTTCLIENT_HPP_ */