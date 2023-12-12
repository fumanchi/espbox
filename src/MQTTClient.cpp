#include <WiFi.h>

#include "MQTTClient.hpp"

MQTTClient::MQTTClient(const Settings &settings, const String &mqttName)
    :   settings {settings},
        chipId {[]{ uint32_t chipId = 0;  for(int i=0; i<17; i=i+8) chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i; return chipId; }()},
        mqttName {mqttName + "_" + String(this->chipId, HEX)},
        subscribtions {},
        needsResubscribtion {false}
{
    this->chipId = 0;
	for(int i=0; i<17; i=i+8)
	    this->chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;

    // WiFi.setAutoConnect(false);
    // WiFi.setAutoReconnect(true);
    WiFi.onEvent([this](WiFiEvent_t event, arduino_event_info_t info)
                {
                    // Serial.printf("[WiFi-event] event: %d\n", event);
                    switch (event)
                    {
                    case SYSTEM_EVENT_STA_GOT_IP:
                        // Serial.println("WiFi connected");
                        // Serial.println("IP address: ");
                        // Serial.println(WiFi.localIP());
                        this->connectToMqtt();
                        break;
                    case SYSTEM_EVENT_STA_DISCONNECTED:
                        // Serial.println("WiFi lost connection");
                        // this->onMqttDisconnect();
                        break;
                    default:
                        break;
                    }
                });
}

void MQTTClient::setup()
{
    this->mqttClient
        .onConnect([this](bool sessionPresent) { this->onMqttConnect(sessionPresent); })
        .onDisconnect([this](DisconnectReason reason) { this->onMqttDisconnect(reason); })
        .onSubscribe([this](uint16_t packetId, const SubscribeReturncode *codes, size_t len) { this->onMqttSubscribe(packetId, codes, len); })
        .onUnsubscribe([this](uint16_t packetId) { this->onMqttUnsubscribe(packetId); })
        .onMessage([this](const MessageProperties &properties, const char *topic, const uint8_t *payload, size_t len, size_t index, size_t total) { this->onMqttMessage(properties, topic, payload, len, index, total); })
        .onPublish([this](uint16_t packetId) { this->onMqttPublish(packetId); })
        .setServer(this->settings.MQTT_HOST.c_str(), this->settings.MQTT_PORT)
        .setCredentials(this->settings.MQTT_USER.c_str(), this->settings.MQTT_PASS.c_str());
    // Serial.printf("Broker: %s:%u (user=\"%s\" pass=\"%s\")...\n", this->settings.MQTT_HOST.c_str(), this->settings.MQTT_PORT, this->settings.MQTT_USER.c_str(), this->settings.MQTT_PASS.c_str());
}

void MQTTClient::loop()
{
    if (this->needsResubscribtion)
    {
        this->needsResubscribtion = false;
        this->resubscribe();
    }
    // this->publish();
    return this->mqttClient.loop();
}

bool MQTTClient::isConnected() const
{
    return this->mqttClient.connected();
}

bool MQTTClient::ready() const
{
    return this->mqttClient.connected() && this->mqttClient.queueSize() == 0;
}

bool MQTTClient::publish(const String &topic, const String &value, PublishedCallback publishedCallback)
{
    uint16_t retval = false;
    
    // if (this->mqttClient.connected())
    {
        std::vector<char> buffer;
        buffer.reserve(16 + topic.length() + 1);
        snprintf(buffer.data(), buffer.capacity(), "tonuinox/%06X/%s", this->chipId, topic.c_str());
        // Serial.printf("%s: %s ~> %s => %s\n", __FUNCTION__, topic.c_str(), buffer.data(), value.c_str());
        retval = this->mqttClient.publish(buffer.data(), 1, true, value.c_str());
        if (retval && publishedCallback)
            this->publishedCallbacks.emplace_back(retval, String(buffer.data()), publishedCallback);        
    }

    // if (!retval)
    //     this->publishings.emplace_back(Publishing {topic, value, publishedCallback});

    return retval != 0;
}

bool MQTTClient::subscribe(String topic, Subscriber subscriber, bool autocCofirm)
{
    if (topic.isEmpty() || !subscriber)
        return false;
    auto result = this->subscribtions.emplace(std::move(topic), Subscribtion {std::move(subscriber), autocCofirm});
    
    if (!result.second)
        return false;

    // Serial.printf("%s: succeeded to register subscribtion to topic: %s\n", __PRETTY_FUNCTION__, result.first->first.c_str());

    if (this->mqttClient.connected())
        return this->resubscribe(result.first->first, result.first->second.subscriber);

    this->needsResubscribtion = true;

    return true;
}

bool MQTTClient::resubscribe()
{
    bool retval = true;
    for (const auto &subscribtion : this->subscribtions)
        if (!(retval = this->resubscribe(subscribtion.first, subscribtion.second.subscriber)))
            break;

    return retval;
}

bool MQTTClient::resubscribe(const String &topic, const Subscriber &subscriber)
{
    assert(!topic.isEmpty());
    // Serial.printf("%s: topic=%s\n", __PRETTY_FUNCTION__, topic);
    bool retval = false;
    if (this->mqttClient.connected())
    {
        std::vector<char> buffer;
        buffer.reserve(16 + topic.length() + 1);
        snprintf(buffer.data(), buffer.capacity(), "tonuinox/%06X/%s", this->chipId, topic.c_str());
        // Serial.printf("%s: topic=%s ~> actual topic=%s\n", __PRETTY_FUNCTION__, topic, buffer.data());
        retval = this->mqttClient.subscribe(buffer.data(), 1);
        this->mqttClient.loop();
        yield();
    }
    return retval;
}

// void MQTTClient::publish()
// {
//     Serial.printf("%s: About tu publish %lu pending publications...\n", __PRETTY_FUNCTION__, this->publishings.size());
//     if (!this->publishings.empty())
//     for (int i = 0; !this->publishings.empty() && i < 10; ++i)
//     {
//         auto &publishing = this->publishings.back();
//         if (!this->publish(publishing.topic, publishing.value, publishing.publishedCallback))
//             break;
//         this->publishings.pop_back();
//     }
// }


void MQTTClient::connectToMqtt()
{
    // Serial.println("Connecting to MQTT...");
    if (!this->mqttClient.connect())
    {
        // Serial.printf("Connecting failed...");
        reconnectMqtt = true;
        lastReconnect = millis();
    }
    else
    {
        reconnectMqtt = false;
    }
}

// void MQTTClient::subscribe(const char *topic, uint8_t qos)
// {
//     static char topic[16 + 20];
//     snprintf(topic, sizeof(topic), "tonuinox/%06X/%s", this->chipId, topic);
//     this->mqttClient.subscribe(topic, qos);
// }

void MQTTClient::onMqttConnect(bool sessionPresent)
{
    // Serial.println("Connected to MQTT.");
    // Serial.print("Session present: ");
    // Serial.println(sessionPresent);

    this->needsResubscribtion = true;
    
    // uint16_t packetIdSub = mqttClient.subscribe("foo/bar", 2);
    // Serial.print("Subscribing at QoS 2, packetId: ");
    // Serial.println(packetIdSub);
    // mqttClient.publish("foo/bar", 0, true, "test 1");
    // Serial.println("Publishing at QoS 0");
    // uint16_t packetIdPub1 = mqttClient.publish("foo/bar", 1, true, "test 2");
    // Serial.print("Publishing at QoS 1, packetId: ");
    // Serial.println(packetIdPub1);
    // uint16_t packetIdPub2 = mqttClient.publish("foo/bar", 2, true, "test 3");
    // Serial.print("Publishing at QoS 2, packetId: ");
    // Serial.println(packetIdPub2);
}

void MQTTClient::onMqttDisconnect(DisconnectReason reason)
{
    // Serial.printf("Disconnected from MQTT: %u.\n", static_cast<uint8_t>(reason));

    if (WiFi.isConnected())
    {
        reconnectMqtt = true;
        lastReconnect = millis();
    }
}

void MQTTClient::onMqttSubscribe(uint16_t packetId, const SubscribeReturncode *codes, size_t len)
{
    // Serial.println("Subscribe acknowledged.");
    // Serial.print("  packetId: ");
    // Serial.println(packetId);
    // for (size_t i = 0; i < len; ++i)
    // {
    //     Serial.print("  SubscribeReturncode: ");
    //     Serial.println(static_cast<uint8_t>(codes[i]));
    // }
}

void MQTTClient::onMqttUnsubscribe(uint16_t packetId)
{
    // Serial.println("Unsubscribe acknowledged.");
    // Serial.print("  packetId: ");
    // Serial.println(packetId);
}

void MQTTClient::onMqttMessage(const MessageProperties &properties, const char *topicGiven, const uint8_t *payload, size_t len, size_t index, size_t total)
{
    // Serial.println("Publish received.");
    // Serial.print("  topic: ");
    // Serial.println(topicGiven);
    // Serial.print("  qos: ");
    // Serial.println(properties.qos);
    // Serial.print("  dup: ");
    // Serial.println(properties.dup);
    // Serial.print("  retain: ");
    // Serial.println(properties.retain);
    // Serial.print("  len: ");
    // Serial.println(len);
    // Serial.print("  index: ");
    // Serial.println(index);
    // Serial.print("  total: ");
    // Serial.println(total);

    assert(!strncmp(topicGiven, "tonuinox/", 9) && topicGiven[17] == '/');
    String topic {topicGiven + 16};
    auto lookup = this->subscribtions.find(topic);
    // Serial.printf("%s: topic=%s %s\n", __FUNCTION__, topic.c_str(), lookup != this->subscribtions.end() ? "found" : "_not_ found");
    if (lookup != this->subscribtions.end())
    {
        String value {payload, len};
        // Serial.printf("%s: topic=%s value=%s\n", __FUNCTION__, lookup->first.c_str(), value.c_str());
        // Serial.flush();
        if (lookup->second.subscriber(lookup->first, value) && lookup->second.autoConfirm)
            this->publish(lookup->first, value);
    }    
    // else
    // {
    //     Serial.printf("%s: #subscribtions=%lu\n", __PRETTY_FUNCTION__, this->subscribtions.size());
    //     for (const auto &subscribtion : this->subscribtions)
    //         Serial.printf("%s:\t\t%s\n", __PRETTY_FUNCTION__, subscribtion);
    // }
}

void MQTTClient::onMqttPublish(uint16_t packetId)
{
    // Serial.println("Publish acknowledged.");
    // Serial.print("  packetId: ");
    // Serial.println(packetId);
    // Serial.print(" queuesize:");
    // Serial.println(this->mqttClient.queueSize());

    if (auto lookup = std::find_if(this->publishedCallbacks.begin(), this->publishedCallbacks.end(), [packetId](const auto &publishing) { return std::get<0>(publishing) == packetId; }); lookup != this->publishedCallbacks.end())
    {
        std::get<2>(*lookup)();
        this->publishedCallbacks.erase(lookup);
    }
}
