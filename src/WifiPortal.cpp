#ifdef FS_NO_GLOBALS
#ifdef FS_H
#error FS.h has already been included with FS_NO_GLOBALS defined! This does not work with the ESP_DoubleResetDetector!
#else // defined(FS_H)
#undef FS_NO_GLOBALS
#endif // defined(FS_H)
#endif // defined(FS_NO_GLOBALS)

#include "WifiPortal.hpp"
#include <ESP_WiFiManager.h>              //https://github.com/khoih-prog/ESP_WiFiManager

#include "ArduinoJson.hpp"
using namespace ArduinoJson;

#include "TFTStatus.hpp"


WifiPortal::WifiPortal(Settings &settings)
    : status{nullptr},
      settings{settings}
{
}

void WifiPortal::logger(const String &line)
{
    if (this->status)
        this->status->push(line);
    else
        this->ESP_WifiPortal::logger(line);
}

void WifiPortal::setup(bool initialConfig, TFTStatus &status)
{
    this->status = &status;
    this->ESP_WifiPortal::setup(initialConfig);
    this->status = nullptr;
}

void WifiPortal::setup(TFTStatus &status)
{
    this->status = &status;
    this->ESP_WifiPortal::setup();
    this->status = nullptr;
}

void WifiPortal::initParameters()
{
    Serial.println(F(__PRETTY_FUNCTION__));
    // setup parameters given the current settings...
    this->parameters.reset(
        new Parameters{
            // Parameter {"TFT_MOSI", "TFT MOSI", this->settings.TFT_MOSI, 3},
            // Parameter {"TFT_SCLK", "TFT SCLK", this->settings.TFT_SCLK, 3},
            // Parameter {"TFT_CS", "TFT CS", this->settings.TFT_CS, 3},
            // Parameter {"TFT_DC", "TFT DC", this->settings.TFT_DC, 3},
            // Parameter {"TFT_RST", "TFT RST", settings.TFT_RST, 3},
            // Parameter {"TFT_SPI_FREQUENCY", "TFT SPI fequency", this->settings.TFT_SPI_FREQUENCY, 3},

            Parameter{"BUTTON_PIN", "Button Pin", this->settings.BUTTON_PIN, 3},

            Parameter{"ROTARY_PIN1", "Rotary encoder pin1", this->settings.ROTARY_PIN1, 3},
            Parameter{"ROTARY_PIN2", "Rotary encoder pin2", this->settings.ROTARY_PIN2, 3},
            Parameter{"ROTARY_CLICKS_PER_STEP", "Rotary encoder clicks per step", this->settings.ROTARY_CLICKS_PER_STEP, 9},

            Parameter{"MRFC522_SS", "MFRC522 SS", this->settings.MRFC522_SS, 3},
            Parameter{"MRFC522_RST", "MFRC522 RST", this->settings.MRFC522_RST, 3},
            Parameter{"MFRC522_SPI_FREQUENCY", "MFRC522 SPI bus fequency", this->settings.MFRC522_SPI_FREQUENCY, 9},

            Parameter{"DFMP3_TX", "DFMP3 serial TX", this->settings.DFMP3_TX, 3},
            Parameter{"DFMP3_RX", "DFMP3 serial RX", this->settings.DFMP3_RX, 3},
            Parameter{"DFMP3_BUSY", "DFMP3 busy", this->settings.DFMP3_BUSY, 3},

            Parameter{"MQTT_HOST", "MQTT broker host", this->settings.MQTT_HOST, 25},
            Parameter{"MQTT_PORT", "MQTT broker port", this->settings.MQTT_PORT, 6},
            Parameter{"MQTT_USER", "MQTT user", this->settings.MQTT_USER, 15},
            Parameter{"MQTT_PASS", "MQTT password", this->settings.MQTT_PASS, 15}
            
            });
}

void WifiPortal::processParameters()
{
    if (this->parameters)
        for (Parameter &parameter : *this->parameters)
            parameter.apply();
}

void WifiPortal::setupWiFiManager(ESP_WiFiManager &wifiManager)
{
    this->initParameters();
    Serial.println(F(__PRETTY_FUNCTION__));
    for (Parameter &parameter : *this->parameters)
        wifiManager.addParameter(&parameter);
    Serial.println(F(__PRETTY_FUNCTION__));
}

void WifiPortal::processWiFiManager(ESP_WiFiManager &wifiManager)
{
    this->processParameters();
}

bool WifiPortal::hasConfigFile() const
{
    return FileFS.exists("/settings.json");
}

bool WifiPortal::readConfigFile()
{
    // this opens the config file in read-mode
    File file = FileFS.open("/settings.json", FILE_READ);
    if (!file)
    {
        Serial.println(F("Config File not found"));
        return false;
    }
    else
    {
        // // we could open the file
        // size_t size = f.size();
        // // Allocate a buffer to store contents of the file.
        // std::unique_ptr<char[]> buf(new char[size + 1]);

        // // Read and store file contents in buf
        // file.readBytes(buf.get(), size);
        // // Closing file
        // file.close();
        // // Using dynamic JSON buffer which is not the recommended memory model, but anyway
        // // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

        // DynamicJsonDocument json(1024);
        // auto deserializeError = deserializeJson(json, buf.get());

        // if (deserializeError)
        // {
        //   Serial.println(F("JSON parseObject() failed"));
        //   return false;
        // }

        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, file);
        if (deserializeError)
        {
            Serial.println(F("JSON parseObject() failed"));
            return false;
        }

        serializeJsonPretty(json, Serial);

        // Parse all config file parameters, override
        // local config variables with parsed values
        if (!this->parameters)
            this->initParameters();
        if (this->parameters)
            for (Parameter &parameter : *this->parameters)
            {
                const char *parameterId = parameter.getID();
                Serial.printf("\t\tLooking for parameter \"%s\"... ", parameterId);
                if (json.containsKey(parameterId))
                {
                    if (const char *value = json[parameterId]; value && value[0])
                    {
                        Serial.print(F("found... ~> "));
                        Serial.println(value);
                        parameter.setValue(value);
                    }
                    else
                        Serial.println(F("not found... (empty)"));
                }
                else
                    Serial.println(F("not found..."));
            }
    }

    Serial.println(F("\nConfig File successfully parsed"));
    return true;
}

bool WifiPortal::writeConfigFile()
{
    Serial.println(F("Saving Config File"));

    // JSONify local configuration parameters
    DynamicJsonDocument json(1024);
    if (this->parameters)
        for (Parameter &parameter : *this->parameters)
        {
            json[parameter.getID()] = String{parameter.value()};
        }

    // Open file for writing
    File f = LittleFS.open("/settings.json", FILE_WRITE);
    if (!f)
    {
        Serial.println(F("Failed to open Config File for writing"));
        return false;
    }

    serializeJsonPretty(json, Serial);
    // Write data to file and close it
    serializeJson(json, f);

    f.close();

    Serial.println(F("\nConfig File successfully saved"));
    return true;
}
