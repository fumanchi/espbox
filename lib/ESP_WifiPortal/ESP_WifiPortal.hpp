#pragma once

// Use LittleFS
#ifdef FS_NO_GLOBALS
#ifdef FS_H
#error FS.h has already been included with FS_NO_GLOBALS defined! This does not work with the ESP_DoubleResetDetector!
#else // defined(FS_H)
#undef FS_NO_GLOBALS
#endif // defined(FS_H)
#endif // defined(FS_NO_GLOBALS)

#include <Arduino.h>

#ifndef ESP_ESP_WIFIPORTAL_HPP
#define ESP_ESP_WIFIPORTAL_HPP

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(2, 0, 0))  //(ESP_ARDUINO_VERSION_MAJOR < 2)
    #error This code is intended to run on the ESP32 platform >= 2.0
#endif

#include <string>
#include <type_traits>

// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#ifndef _WIFIMGR_LOGLEVEL_ 
#define _WIFIMGR_LOGLEVEL_    1
#endif

// To not display stored SSIDs and PWDs on Config Portal, select false. Default is true
// Even the stored Credentials are not display, just leave them all blank to reconnect and reuse the stored Credentials 
//#define DISPLAY_STORED_CREDENTIALS_IN_CP        false

// Default is 30s, using 20s now
#ifndef ESP_WIFIPORTAL_TIME_BETWEEN_MODAL_SCANS
#define ESP_WIFIPORTAL_TIME_BETWEEN_MODAL_SCANS          20000UL
#endif

// Default is 60s, using 30s now
#ifndef ESP_WIFIPORTAL_TIME_BETWEEN_MODELESS_SCANS
#define ESP_WIFIPORTAL_TIME_BETWEEN_MODELESS_SCANS       30000UL
#endif


#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>

// From v1.1.1
#include <WiFiMulti.h>


#include "FS.h"
#include "LittleFS.h"       // https://github.com/espressif/arduino-esp32/tree/master/libraries/LittleFS


#ifndef ESP_WIFIPORTAL_DRD_DEBUG
#define ESP_WIFIPORTAL_DRD_DEBUG false
#endif

#define ESP_DRD_USE_LITTLEFS    true
#define ESP_DRD_USE_SPIFFS      false
#define ESP_DRD_USE_EEPROM      false
#pragma push_macro("DOUBLERESETDETECTOR_DEBUG")
#define DOUBLERESETDETECTOR_DEBUG ESP_WIFIPORTAL_DRD_DEBUG
#include <ESP_DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector
#pragma pop_macro("DOUBLERESETDETECTOR_DEBUG")

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#ifndef ESP_WIFIPORTAL_DRD_TIMEOUT
#define ESP_WIFIPORTAL_DRD_TIMEOUT 10
#endif

// RTC Memory Address for the DoubleResetDetector to use
#ifndef ESP_WIFIPORTAL_DRD_ADDRESS
#define ESP_WIFIPORTAL_DRD_ADDRESS 0
#endif

#define ESP_WIFIPORTAL_MIN_AP_PASSWORD_SIZE    8

#define ESP_WIFIPORTAL_CONFIG_FILENAME F("/ESP_WIFIPORTAL_cred.dat")


// Use false if you don't like to display Available Pages in Information Page of Config Portal
// Comment out or use true to display Available Pages in Information Page of Config Portal
// Must be placed before #include <ESPAsync_WiFiManager.h>
#ifndef ESP_WIFIPORTAL_USE_AVAILABLE_PAGES
#define ESP_WIFIPORTAL_USE_AVAILABLE_PAGES     false
#endif

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal from sketch. Valid only if DHCP is used.
// You'll loose the feature of dynamically changing from DHCP to static IP, or vice versa
// You have to explicitly specify false to disable the feature.
//#define ESP_WIFIPORTAL_USE_STATIC_IP_CONFIG_IN_CP         false

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#ifndef ESP_WIFIPORTAL_USE_ESP_WIFIMANAGER_NTP
#define ESP_WIFIPORTAL_USE_ESP_WIFIMANAGER_NTP     true
#endif

// Just use enough to save memory. On ESP8266, can cause blank ConfigPortal screen
// if using too much memory
#ifndef ESP_WIFIPORTAL_USING_AFRICA
#define ESP_WIFIPORTAL_USING_AFRICA         false
#endif
#ifndef ESP_WIFIPORTAL_USING_AMERICA
#define ESP_WIFIPORTAL_USING_AMERICA        true
#endif
#ifndef ESP_WIFIPORTAL_USING_ANTARCTICA
#define ESP_WIFIPORTAL_USING_ANTARCTICA     false
#endif
#ifndef ESP_WIFIPORTAL_USING_ASIA
#define ESP_WIFIPORTAL_USING_ASIA           false
#endif
#ifndef ESP_WIFIPORTAL_USING_ATLANTIC
#define ESP_WIFIPORTAL_USING_ATLANTIC           false
#endif
#ifndef ESP_WIFIPORTAL_USING_AUSTRALIA
#define ESP_WIFIPORTAL_USING_AUSTRALIA           false
#endif
#ifndef ESP_WIFIPORTAL_USING_EUROPE
#define ESP_WIFIPORTAL_USING_EUROPE           false
#endif
#ifndef ESP_WIFIPORTAL_USING_INDIAN
#define ESP_WIFIPORTAL_USING_INDIAN           false
#endif
#ifndef ESP_WIFIPORTAL_USING_PACIFIC
#define ESP_WIFIPORTAL_USING_PACIFIC           false
#endif
#ifndef ESP_WIFIPORTAL_USING_ETC_GMT
#define ESP_WIFIPORTAL_USING_ETC_GMT           false
#endif

// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#ifndef ESP_WIFIPORTAL_USE_CLOUDFLARE_NTP
#define ESP_WIFIPORTAL_USE_CLOUDFLARE_NTP          false
#endif

#ifndef ESP_WIFIPORTAL_USING_CORS_FEATURE
#define ESP_WIFIPORTAL_USING_CORS_FEATURE          true
#endif

//////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(ESP_WIFIPORTAL_USE_STATIC_IP_CONFIG_IN_CP) && !ESP_WIFIPORTAL_USE_STATIC_IP_CONFIG_IN_CP)
  // Force DHCP to be true
  #if defined(ESP_WIFIPORTAL_USE_DHCP_IP)
    #undef ESP_WIFIPORTAL_USE_DHCP_IP
  #endif
  #define ESP_WIFIPORTAL_USE_DHCP_IP     true
#else
  // You can select DHCP or Static IP here
  #define ESP_WIFIPORTAL_USE_DHCP_IP     true
  //#define ESP_WIFIPORTAL_USE_DHCP_IP     false
#endif

#if (ESP_WIFIPORTAL_USE_DHCP_IP)
  // Use DHCP
  #if (_WIFIMGR_LOGLEVEL_ > 3)
    #warning Using DHCP IP
  #endif
  

#else
  // Use static IP
  #if (_WIFIMGR_LOGLEVEL_ > 3)
    #warning Using static IP
  #endif
  
  IPAddress stationIP   = IPAddress(192, 168, 2, 232);
  IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

#ifndef ESP_WIFIPORTAL_USE_CONFIGURABLE_DNS
#define ESP_WIFIPORTAL_USE_CONFIGURABLE_DNS      true
#endif



#ifndef ESP_WIFIPORTAL_USE_CUSTOM_AP_IP
#define ESP_WIFIPORTAL_USE_CUSTOM_AP_IP          false
#endif



// Must be placed before #include <ESP_WiFiManager.h>, or default port 80 will be used
#ifndef ESP_WIFIPORTAL_HTTP_PORT
//#define ESP_WIFIPORTAL_HTTP_PORT     8080
#define ESP_WIFIPORTAL_HTTP_PORT           80
#endif

#ifndef ESP_WIFIPORTAL_SSID_MAX_LEN
#define ESP_WIFIPORTAL_SSID_MAX_LEN            32
#endif
#ifndef ESP_WIFIPORTAL_PASS_MAX_LEN
#define ESP_WIFIPORTAL_PASS_MAX_LEN            64
#endif
#ifndef ESP_WIFIPORTAL_NUM_WIFI_CREDENTIALS
#define ESP_WIFIPORTAL_NUM_WIFI_CREDENTIALS      2
#endif

// Assuming max 491 chars
#ifndef ESP_WIFIPORTAL_TZNAME_MAX_LEN
#define ESP_WIFIPORTAL_TZNAME_MAX_LEN            50
#endif
#ifndef ESP_WIFIPORTAL_TIMEZONE_MAX_LEN
#define ESP_WIFIPORTAL_TIMEZONE_MAX_LEN          50
#endif

// #pragma push_macro("HTTP_PORT")
#define HTTP_PORT ESP_WIFIPORTAL_HTTP_PORT
#pragma push_macro("TIME_BETWEEN_MODAL_SCANS")
#define TIME_BETWEEN_MODAL_SCANS ESP_WIFIPORTAL_TIME_BETWEEN_MODAL_SCANS
#pragma push_macro("TIME_BETWEEN_MODELESS_SCANS")
#define TIME_BETWEEN_MODELESS_SCANS ESP_WIFIPORTAL_TIME_BETWEEN_MODELESS_SCANS
#pragma push_macro("USE_AVAILABLE_PAGES")
#define USE_AVAILABLE_PAGES ESP_WIFIPORTAL_USE_AVAILABLE_PAGES
#pragma push_macro("USE_DHCP_IP")
#define USE_DHCP_IP ESP_WIFIPORTAL_USE_DHCP_IP
#ifdef ESP_WIFI_USE_STATIC_IP_CONFIG_IN_CP
#pragma push_macro("USE_STATIC_IP_CONFIG_IN_CP")
#define USE_STATIC_IP_CONFIG_IN_CP ESP_WIFI_USE_STATIC_IP_CONFIG_IN_CP
#endif
#pragma push_macro("USE_AVAILABLE_PAGES")
#define USE_ESP_WIFIMANAGER_NTP ESP_WIFIPORTAL_USE_ESP_WIFIMANAGER_NTP
#pragma push_macro("USING_AFRICA")
#define USING_AFRICA ESP_WIFIPORTAL_USING_AFRICA
#pragma push_macro("USING_AMERICA")
#define USING_AMERICA ESP_WIFIPORTAL_USING_AMERICA
#pragma push_macro("USING_ANTARCTICA")
#define USING_ANTARCTICA ESP_WIFIPORTAL_USING_ANTARCTICA
#pragma push_macro("USING_ASIA")
#define USING_ASIA ESP_WIFIPORTAL_USING_ASIA
#pragma push_macro("USING_ATLANTIC")
#define USING_ATLANTIC ESP_WIFIPORTAL_USING_ATLANTIC
#pragma push_macro("USING_ATLANTIC")
#define USING_AUSTRALIAESP_WIFIPORTAL_USING_AUSTRALIA
#pragma push_macro("USING_ATLANTIC")
#define USING_EUROPE ESP_WIFIPORTAL_USING_EUROPE
#pragma push_macro("USING_ATLANTIC")
#define USING_INDIANESP_WIFIPORTAL_USING_INDIAN
#pragma push_macro("USING_ATLANTIC")
#define USING_PACIFIC ESP_WIFIPORTAL_USING_PACIFIC
#pragma push_macro("USING_ATLANTIC")
#define USING_ETC_GMT ESP_WIFIPORTAL_USING_ETC_GMT
#pragma push_macro("USING_CLOUDFLARE_NTP")
#define USING_CLOUDFLARE_NTP ESP_WIFIPORTAL_USING_CLOUDFLARE_NTP
#pragma push_macro("USING_CORS_FEATURE")
#define USE_CORS_FEATUREESP_WIFIPORTAL_USE_CORS_FEATURE
// #include <ESP_WiFiManager.h>              //https://github.com/khoih-prog/ESP_WiFiManager
#include <ESP_WiFiManager.hpp>              //https://github.com/khoih-prog/ESP_WiFiManager
#pragma pop_macro("HTTP_PORT")
#pragma pop_macro("TIME_BETWEEN_MODAL_SCANS")
#pragma pop_macro("TIME_BETWEEN_MODELESS_SCANS")
#pragma pop_macro("USE_AVAILABLE_PAGES")
#pragma pop_macro("USE_DHCP_IP")
#ifdef ESP_WIFI_USE_STATIC_IP_CONFIG_IN_CP
#pragma pop_macro("USE_STATIC_IP_CONFIG_IN_CP")
#endif
#pragma pop_macro("USE_AVAILABLE_PAGES")
#pragma pop_macro("USING_AFRICA")
#pragma pop_macro("USING_AMERICA")
#pragma pop_macro("USING_ANTARCTICA")
#pragma pop_macro("USING_ASIA")
#pragma pop_macro("USING_ATLANTIC")
#pragma pop_macro("USING_ATLANTIC")
#pragma pop_macro("USING_ATLANTIC")
#pragma pop_macro("USING_ATLANTIC")
#pragma pop_macro("USING_ATLANTIC")
#pragma pop_macro("USING_ATLANTIC")
#pragma pop_macro("USING_CLOUDFLARE_NTP")
#pragma pop_macro("USING_CORS_FEATURE")


static IPAddress APStaticIP  = IPAddress(192, 168, 100, 1);
static IPAddress APStaticGW  = IPAddress(192, 168, 100, 1);
static IPAddress APStaticSN  = IPAddress(255, 255, 255, 0);

static const IPAddress StationIP   = IPAddress(0, 0, 0, 0);
static const IPAddress GatewayIP   = IPAddress(192, 168, 2, 1);
static const IPAddress NetMask     = IPAddress(255, 255, 255, 0);

#if ESP_WIFIPORTAL_USE_CONFIGURABLE_DNS
static const IPAddress Dns1IP      = GatewayIP;
static const IPAddress Dns2IP      = IPAddress(8, 8, 8, 8);
#endif

class ESP_WifiPortal
{
public: /* types */
 class Parameter : public ESP_WMParameter
  {
  public:
    template<typename T>
    Parameter(const char *id, const char *placeholder, T &value, const int& length, 
                    const char *custom = "", const int& labelPlacement = WFM_LABEL_BEFORE)
      : ESP_WMParameter {id, placeholder, Convert(value), length, custom, labelPlacement},
        getter {[&value]() -> const char* { return Convert(value); }},
        setter {[&value](const char *valueGiven) mutable 
                  { 
                    Serial.printf("%s@%d: valueGiven=%s\n", __PRETTY_FUNCTION__, __LINE__, valueGiven);
                    if constexpr (std::is_integral<T>::value)
                      value = static_cast<T>(atoll(valueGiven)); 
                    else
                      value = valueGiven;
                  }}
    {

    }

    const char *value()
    {
      return this->getter();
    }

    void setValue(const char *value)
    {
      this->setter(value);
    }

    void apply(const char *value = 0)
    {
      this->setter(value ? value : this->getValue());
    }

  private:
    template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
    static const char *Convert(T value) 
    {
      static char buffer[9];
      if constexpr (std::is_unsigned<T>::value)
        snprintf(buffer, 9, "%lu", value);
      else
        snprintf(buffer, 9, "%ld", value);
      return buffer;
    }

    static const char *Convert(const String &value) 
    {
      return value.c_str();
    }

  private: /* types */
    typedef std::function<const char*()> Getter;
    typedef std::function<void(const char*)> Setter;

  private:
    Getter getter;
    Setter setter;
  };

public:

    ESP_WifiPortal(bool initialConfig = false)
        : networking {true},
          initialConfig {initialConfig}
    {

    }

    virtual void logger(const String &line)
    {
      LOGERROR(line);
    }

    virtual void logger(std::initializer_list<String> text)
    {
      for (const String &line : text)
        this->logger(line);
    }

    uint8_t connectMultiWiFi()
    {
        // For ESP32, this better be 0 to shorten the connect time.
        // For ESP32-S2/C3, must be > 500
        #if ( USING_ESP32_S2 || USING_ESP32_C3 )
            #define WIFI_MULTI_1ST_CONNECT_WAITING_MS           500L
        #else
            // For ESP32 core v1.0.6, must be >= 500
            #define ESP_WIFIPORTAL_WIFI_MULTI_1ST_CONNECT_WAITING_MS           800L
        #endif

        #define ESP_WIFIPORTAL_WIFI_MULTI_CONNECT_WAITING_MS                   500L

        uint8_t status;

        LOGERROR(F("ConnectMultiWiFi with :"));

        if ((this->Router_SSID != "") && (this->Router_Pass != ""))
        {
            LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
            LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass );
            this->wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
        }

        for (uint8_t i = 0; i < ESP_WIFIPORTAL_NUM_WIFI_CREDENTIALS; ++i)
        {
            // Don't permit NULL SSID and password len < ESP_WIFIPORTAL_MIN_AP_PASSWORD_SIZE (8)
            if ((config.WiFi_Creds[i].wifi_ssid[0] != 0) && (strlen(config.WiFi_Creds[i].wifi_pw.data()) >= ESP_WIFIPORTAL_MIN_AP_PASSWORD_SIZE))
            {
                LOGERROR3(F("* Additional SSID = "), config.WiFi_Creds[i].wifi_ssid.data(), F(", PW = "), config.WiFi_Creds[i].wifi_pw.data());
            }
        }

        LOGERROR(F("Connecting MultiWifi..."));
        logger(F("connecting MultiWifi..."));

        #if !ESP_WIFIPORTAL_USE_DHCP_IP
        configWiFi(this->STA_IPconfig);
        #endif

        int i = 0;
        status = this->wifiMulti.run();
        delay(ESP_WIFIPORTAL_WIFI_MULTI_1ST_CONNECT_WAITING_MS);

        while ((i++ < 20 ) && (status != WL_CONNECTED))
        {
            status = WiFi.status();

            if ( status == WL_CONNECTED )
                break;
            else
                delay(ESP_WIFIPORTAL_WIFI_MULTI_CONNECT_WAITING_MS);
        }

        if ( status == WL_CONNECTED )
        {
            logger(F("connected..."));
            IPAddress ipAddress = WiFi.localIP();
            logger(String(F("IP: ")) + 
                  String(ipAddress[0]) + String(".") +
                  String(ipAddress[1]) + String(".") +
                  String(ipAddress[2]) + String(".") +
                  String(ipAddress[3]));
            logger(String(F("SSID:")) + WiFi.SSID());
            logger(String(F("RSSI=")) + WiFi.RSSI());
            logger(String(F("Channel:")) + String(WiFi.channel()));
            LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
            LOGERROR1(F("WiFi connected after time: "), i);
            LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
            LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
        }
        else
        {
            LOGERROR(F("WiFi not connected"));
            logger(F("_not_ connected..."));
            // logger(F("rebooting..."));
            // delay(1000);
            // To avoid unnecessary DRD
            if (this->drd)
              this->drd->loop();
            // ESP.restart();

            this->networking = false;
            WiFi.mode(WIFI_OFF);
        }
        return status;
    }

    bool check_WiFi()
    {
        if (this->networking && (WiFi.status() != WL_CONNECTED))
        {
            Serial.println(F("\nWiFi lost. Reconnect..."));
            this->connectMultiWiFi();
        }

        return this->networking && WiFi.status() == WL_CONNECTED;
    }  

    void check_status()
    {
        static ulong checkstatus_timeout  = 0;
        static ulong checkwifi_timeout    = 0;
        
        ulong current_millis = millis();

        #define WIFICHECK_INTERVAL    1000L

        #if USE_ESP_WIFIMANAGER_NTP
        #define HEARTBEAT_INTERVAL    60000L
        #else
        #define HEARTBEAT_INTERVAL    10000L
        #endif

        // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
        if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
        {
            check_WiFi();
            checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
        }

        // // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
        // if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
        // { 
        //     heartBeatPrint();
        //     checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
        // }
    }

    int calcChecksum(uint8_t* address, uint16_t sizeToCalc)
    {
        uint16_t checkSum = 0;
        
        for (uint16_t index = 0; index < sizeToCalc; ++index)
            checkSum += *(((byte*)address) + index);

        return checkSum;
    }

    bool loadConfigData()
    {
        memset((void *)&this->config, 0, sizeof(Config));
        memset((void *)&this->STA_IPconfig, 0, sizeof(STA_IPconfig));

        logger(F("loading credential file..."));
        LOGERROR1(F("* Laoding credential file = "), ESP_WIFIPORTAL_CONFIG_FILENAME);
        File file = LittleFS.open(ESP_WIFIPORTAL_CONFIG_FILENAME, FILE_READ);
        if (file)
        {
            LOGERROR3(F("* Opened credential file = "), ESP_WIFIPORTAL_CONFIG_FILENAME, F(", size = "), file.size());
            if (file.size() != sizeof(Config) + sizeof(STA_IPconfig))
            {
              LOGERROR3(F("* Failed to read credential file = "), ESP_WIFIPORTAL_CONFIG_FILENAME, F(", size = "), file.size());
              LOGERROR1(F("* \t\texpected = "), sizeof(Config) + sizeof(STA_IPconfig));
              file.close();
              return false;
            }
            file.readBytes((char *)&this->config, sizeof(Config));
            file.readBytes((char *)&this->STA_IPconfig, sizeof(STA_IPconfig));
            file.close();

            logger(F("succeded..."));

            if (this->config.checksum != calcChecksum((uint8_t*)&this->config, sizeof(Config) - sizeof(Config::checksum)))
                return false;
    
            this->STA_IPconfig.dump();
            return true;
        }
        else
        {
            logger(F("failed..."));
            return false;
        }
    }

    void saveConfigData()
    {
        logger("persisting credentials...");
        File file = LittleFS.open(ESP_WIFIPORTAL_CONFIG_FILENAME, FILE_WRITE);
        if (file)
        {
            this->config.checksum = calcChecksum((uint8_t*) &config, sizeof(config) - sizeof(config.checksum));
            file.write((uint8_t*)&this->config, sizeof(Config));
            file.write((uint8_t*)&this->STA_IPconfig, sizeof(this->STA_IPconfig));
            logger("succeded...");

            this->STA_IPconfig.dump();
            file.close();
        }
        else
        {
        }
    }


    void wifi_manager() 
    {
        logger("wifi portal requested...");
        Serial.println(F("\nConfig Portal requested."));

        //Local intialization. Once its business is done, there is no need to keep it around
        ESP_WiFiManager ESP_wifiManager("WiFiPortal");

        //Check if there is stored WiFi router/password credentials.
        //If not found, device will remain in configuration mode until switched off via webserver.
        Router_SSID = ESP_wifiManager.WiFi_SSID();
        Router_Pass = ESP_wifiManager.WiFi_Pass();
        if (!initialConfig && (Router_SSID != "") && (Router_Pass != ""))
        {
            // If valid AP credential and not DRD, set timeout 120s.
            ESP_wifiManager.setConfigPortalTimeout(120);
            Serial.println(F("Got stored Credentials. Timeout 120s"));
            logger("got stored credentials...");
        }
        else
        {
            ESP_wifiManager.setConfigPortalTimeout(0);
            Serial.print(F("No timeout : "));
            if (this->initialConfig)
            {
                Serial.println(F("DRD or No stored Credentials.."));
                logger("DRD detected...");
            }
            else
            {
                Serial.println(F("No stored Credentials."));
                logger("no stored credentials...");
            }
        }


        //Local intialization. Once its business is done, there is no need to keep it around

        // Extra parameters to be configured
        // After connecting, parameter.getValue() will get you the configured value
        // Format: <ID> <Placeholder text> <default value> <length> <custom HTML> <label placement>
        // (*** we are not using <custom HTML> and <label placement> ***)

        this->setupWiFiManager(ESP_wifiManager);
     
        // Sets timeout in seconds until configuration portal gets turned off.
        // If not specified device will remain in configuration mode until
        // switched off via webserver or device is restarted.
        //ESP_wifiManager.setConfigPortalTimeout(120);

        ESP_wifiManager.setMinimumSignalQuality(-1);

        // From v1.0.10 only
        // Set config portal channel, default = 1. Use 0 => random channel from 1-13
        ESP_wifiManager.setConfigPortalChannel(0);
        //////
        
        #if ESP_WIFIPORTAL_USE_CUSTOM_AP_IP
        //set custom ip for portal
        // New in v1.4.0
        ESP_wifiManager.setAPStaticIPConfig(WM_AP_IPconfig);
        //////
        #endif
        
        #if !USE_DHCP_IP    
            // Set (static IP, Gateway, Subnetmask, DNS1 and DNS2) or (IP, Gateway, Subnetmask). New in v1.0.5
            // New in v1.4.0
            ESP_wifiManager.setSTAStaticIPConfig(this->STA_IPconfig);
            //////
        #endif

        // New from v1.1.1
        #if USING_CORS_FEATURE
        ESP_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
        #endif

        // Start an access point
        // and goes into a blocking loop awaiting configuration.
        // Once the user leaves the portal with the exit button
        // processing will continue
        // SSID to uppercase
        this->ssid.toUpperCase();
        this->password = "My" + ssid;

        logger(F("spawning portal..."));
        Serial.print(F("Starting configuration portal @ "));
        Serial.print(F("IP: 192.168.4.1"));
 
        #if defined(HTTP_PORT_TO_USE)
            Serial.print(F(":")); Serial.print(HTTP_PORT_TO_USE);
            logger(F("IP: 192.168.4.1"));
            // logger(String {itoa(HTTP_PORT_TO_USE)}});
        #endif

        logger(String(F("SSID: ")) + ssid);
        logger(String(F("PWD: ")) + password);
        Serial.print(F(", SSID = "));
        Serial.print(ssid);
        Serial.print(F(", PWD = "));
        Serial.println(password);

        ESP_wifiManager.setCredentials(this->config.WiFi_Creds[0].wifi_ssid.data(), this->config.WiFi_Creds[0].wifi_pw.data(), this->config.WiFi_Creds[1].wifi_ssid.data(), this->config.WiFi_Creds[1].wifi_pw.data());

        if (!ESP_wifiManager.startConfigPortal((const char *)ssid.c_str(), password.c_str()))
        {
            Serial.println(F("Not connected to WiFi but continuing anyway."));
        }
        else
        {
            // If you get here you have connected to the WiFi
            Serial.println(F("Connected...yeey :)"));
            Serial.print(F("Local IP: "));
            Serial.println(WiFi.localIP());
        }

        // Only clear then save data if CP entered and with new valid Credentials
        // No CP => stored getSSID() = ""
        Serial.printf("%s@%d: ESP_wifiManager.getSSID(0)=\"%s\" ESP_wifiManager.getSSID(1)=\"%s\"\n", __PRETTY_FUNCTION__, __LINE__, ESP_wifiManager.getSSID(0), ESP_wifiManager.getSSID(1));
        if (String(ESP_wifiManager.getSSID(0)) != "" && String(ESP_wifiManager.getSSID(1)) != "")
        {
            // Stored  for later usage, from v1.1.0, but clear first
            memset(&this->config, 0, sizeof(Config));
            for (uint8_t i = 0; i < ESP_WIFIPORTAL_NUM_WIFI_CREDENTIALS; i++)
            {
                String tempSSID = ESP_wifiManager.getSSID(i);
                String tempPW   = ESP_wifiManager.getPW(i);
            
                if (strlen(tempSSID.c_str()) < this->config.WiFi_Creds[i].wifi_ssid.size() - 1)
                    strcpy(this->config.WiFi_Creds[i].wifi_ssid.data(), tempSSID.c_str());
                else
                    strncpy(this->config.WiFi_Creds[i].wifi_ssid.data(), tempSSID.c_str(), this->config.WiFi_Creds[i].wifi_ssid.size() - 1);
            
                if (strlen(tempPW.c_str()) < config.WiFi_Creds[i].wifi_pw.size() - 1)
                    strcpy(this->config.WiFi_Creds[i].wifi_pw.data(), tempPW.c_str());
                else
                    strncpy(this->config.WiFi_Creds[i].wifi_pw.data(), tempPW.c_str(), this->config.WiFi_Creds[i].wifi_pw.size() - 1);  
            
                // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
                if (this->config.WiFi_Creds[i].wifi_ssid[0] && (strlen(this->config.WiFi_Creds[i].wifi_pw.data()) >= ESP_WIFIPORTAL_MIN_AP_PASSWORD_SIZE))
                {
                    LOGERROR3(F("* Add SSID = "), this->config.WiFi_Creds[i].wifi_ssid.data(), F(", PW = "), this->config.WiFi_Creds[i].wifi_pw.data());
                    this->wifiMulti.addAP(this->config.WiFi_Creds[i].wifi_ssid.data(), this->config.WiFi_Creds[i].wifi_pw.data());
                }
            }
        
        #if USE_ESP_WIFIMANAGER_NTP      
            String tempTZ   = ESP_wifiManager.getTimezoneName();
            if (strlen(tempTZ.c_str()) < this->config.TZ_Name.size() - 1)
                strcpy(this->config.TZ_Name.data(), tempTZ.c_str());
            else
                strncpy(this->config.TZ_Name.data(), tempTZ.c_str(), this->config.TZ_Name.size() - 1);

            const char *TZ_Result = ESP_wifiManager.getTZ(this->config.TZ_Name.data());        
            if (strlen(TZ_Result) < this->config.TZ.size() - 1)
                strcpy(this->config.TZ.data(), TZ_Result);
            else
                strncpy(this->config.TZ.data(), TZ_Result, this->config.TZ_Name.size() - 1);
                
            if (strlen(this->config.TZ_Name.data()) > 0)
            {
                LOGERROR3(F("Saving current TZ_Name ="), this->config.TZ_Name.data(), F(", TZ = "), this->config.TZ.data());
                configTzTime(this->config.TZ.data(), "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
            }
            else
            {
                LOGERROR(F("Current Timezone Name is not set. Enter Config Portal to set."));
            }
        #endif

            ESP_wifiManager.getSTAStaticIPConfig(this->STA_IPconfig);
            
            saveConfigData();
        }

        // // Getting posted form values and overriding local variables parameters
        // // Config file is written regardless the connection state
        // for (auto &parameter : this->parameters)
        //     strcpy(parameter._value, parameter.getValue());
        // // for (auto &aio_data : AIO_SERVER_TOTAL_DATA) 
        // // {              
        // //     strcpy(aio_data.second._value, DATA_FIELD[aio_data.first]->getValue());
        // // }
        this->processWiFiManager(ESP_wifiManager);
        
        // Writing JSON config file to flash for next boot
        writeConfigFile();
    }

  virtual bool readConfigFile()
  {
    return true;
  }

  // {
  //   // this opens the config file in read-mode
  //   File f = FileFS.open(this->configFile.c_str(), "r");
  //   if (!f)
  //   {
  //     Serial.println(F("Config File not found"));
  //     return false;
  //   }
  //   else
  //   {
  //     // we could open the file
  //     size_t size = f.size();
  //     // Allocate a buffer to store contents of the file.
  //     std::unique_ptr<char[]> buf(new char[size + 1]);

  //     // Read and store file contents in buf
  //     f.readBytes(buf.get(), size);
  //     // Closing file
  //     f.close();
  //     // Using dynamic JSON buffer which is not the recommended memory model, but anyway
  //     // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

  //     DynamicJsonDocument json(1024);
  //     auto deserializeError = deserializeJson(json, buf.get());
      
  //     if (deserializeError)
  //     {
  //       Serial.println(F("JSON parseObject() failed"));
  //       return false;
  //     }
      
  //     serializeJson(json, Serial);
      
  //     // Parse all config file parameters, override
  //     // local config variables with parsed values
  //     for (Parameter &parameter : this->parameters)
  //     {           
  //       if (json.containsKey(parameter._id))
  //       {
  //         strcpy(parameter._value, json[parameter._id]);
  //       }
  //     }   
  //   }

  //   Serial.println(F("\nConfig File successfully parsed"));
  //   return true;
  // }

  virtual bool writeConfigFile()
  {
    return true;
  }

  void setup(bool initialConfig)
  {
    this->initialConfig = initialConfig;
    this->setup();
  }

  // Setup function
  virtual void setup()
  {
    // Format FileFS if not yet
    if (!LittleFS.begin(true))
    {
      Serial.println(F("LittleFS already tried to format."));
      if (!FileFS.begin())
      {     
        // prevents debug info from the library to hide err message.
        delay(100);
        
        Serial.println(F("LittleFS failed! Stay forever"));
        while (true)
        {
          delay(100);
        }
      }
    }

    this->AP_IPconfig.init();
    this->STA_IPconfig.init();
    
    if (!this->readConfigFile())
    {
      Serial.println(F("Can't read Config File, using default values"));
    }

    this->drd = new DoubleResetDetector(ESP_WIFIPORTAL_DRD_TIMEOUT, ESP_WIFIPORTAL_DRD_ADDRESS);

    logger("initializing DRD...");
    if (!this->drd)
    {
      Serial.println(F("Can't instantiate. Disable DRD feature"));
    }
    else if (this->drd->detectDoubleReset())
    {
      // DRD, disable timeout.
      logger("double reset detected...");
      Serial.println(F("Open Config Portal without Timeout: Double Reset Detected"));
      this->initialConfig = true;
    }
  
    if (this->initialConfig)
    {
      this->loadConfigData();
      this->wifi_manager();
    }
    else
    {   
      // Pretend CP is necessary as we have no AP Credentials
      this->initialConfig = true;

      // Load stored data, the addAP ready for MultiWiFi reconnection
      logger("loading config data...");
      if (this->loadConfigData())
      {
#if USE_ESP_WIFIMANAGER_NTP      
        if (strlen(this->config.TZ_Name.data()) > 0 )
        {
          logger(String(F("Zone: ")) + this->config.TZ_Name.data());
          LOGERROR3(F("Current TZ_Name ="), this->config.TZ_Name.data(), F(", TZ = "), config.TZ.data());

          configTzTime(this->config.TZ.data(), "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
        }
        else
        {
          Serial.println(F("Current Timezone is not set. Enter Config Portal to set."));
        } 
#endif
        
        for (uint8_t i = 0; i < ESP_WIFIPORTAL_NUM_WIFI_CREDENTIALS; i++)
        {
          // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
          if (this->config.WiFi_Creds[i].wifi_ssid[0] && (strlen(this->config.WiFi_Creds[i].wifi_pw.data()) >= ESP_WIFIPORTAL_MIN_AP_PASSWORD_SIZE))
          {
            LOGERROR3(F("* Add SSID = "), this->config.WiFi_Creds[i].wifi_ssid.data(), F(", PW = "), this->config.WiFi_Creds[i].wifi_pw.data());
            this->wifiMulti.addAP(this->config.WiFi_Creds[i].wifi_ssid.data(), this->config.WiFi_Creds[i].wifi_pw.data());
            this->initialConfig = false;
          }
        }
      }

      if (this->initialConfig)
      {
        Serial.println(F("Open Config Portal without Timeout: No stored WiFi Credentials"));
        this->wifi_manager();
      }
      else if (!this->check_WiFi()) 
      {
        Serial.println(F("ConnectMultiWiFi in setup"));
        logger("connecting to WiFi...");
        this->networking = false;
        WiFi.mode(WIFI_OFF);
      }
    }
  }

  // Loop function
  void loop()
  {
    // Call the double reset detector loop method every so often,
    // so that it can recognise when the timeout expires.
    // You can also call drd.stop() when you wish to no longer
    // consider the next reset as a double reset.
    if (this->drd)
    {
      this->drd->loop();
      if (!this->drd->waitingForDRD())
      {
        delete this->drd;
        this->drd = nullptr;
      }
    }

    // this is just for checking if we are connected to WiFi
    if (this->networking)
      this->check_status();
  }

public:
    bool networking;

protected:
    virtual void setupWiFiManager(ESP_WiFiManager &wifiManager) = 0;
    virtual void processWiFiManager(ESP_WiFiManager &wifiManager) = 0;

private: /* types */
    DoubleResetDetector *drd;

    struct WiFi_Credentials_String
    {
        String wifi_ssid;
        String wifi_pw;
    };

    struct Config
    {
        struct WiFi_Credentials
        {
            std::array<char, ESP_WIFIPORTAL_SSID_MAX_LEN> wifi_ssid;
            std::array<char, ESP_WIFIPORTAL_PASS_MAX_LEN> wifi_pw;
        };

        std::array<WiFi_Credentials, ESP_WIFIPORTAL_NUM_WIFI_CREDENTIALS>  WiFi_Creds;
        std::array<char, ESP_WIFIPORTAL_TZNAME_MAX_LEN> TZ_Name;     // "America/Toronto"
        std::array<char, ESP_WIFIPORTAL_TIMEZONE_MAX_LEN> TZ;        // "EST5EDT,M3.2.0,M11.1.0"
        uint16_t checksum;
    };

    struct WiFi_AP_IPConfig : ::WiFi_AP_IPConfig
    {
      WiFi_AP_IPConfig() = default;

      void init()
      {
          this->_ap_static_ip = APStaticIP;
          this->_ap_static_gw = APStaticGW;
          this->_ap_static_sn = APStaticSN;
      }

    };

    struct WiFi_STA_IPConfig : ::WiFi_STA_IPConfig
    {
        WiFi_STA_IPConfig() = default;

        void init()
        {
            this->_sta_static_ip   = StationIP;
            this->_sta_static_gw   = GatewayIP;
            this->_sta_static_sn   = NetMask;
            #if ESP_WIFIPORTAL_USE_CONFIGURABLE_DNS
            this->_sta_static_dns1 = Dns1IP;
            this->_sta_static_dns2 = Dns2IP;
            #endif
        }

        void apply() const
        {
            #if ESP_WIFIPORTAL_USE_CONFIGURABLE_DNS
                // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
                WiFi.config(this->_sta_static_ip, this->_sta_static_gw, this->_sta_static_sn, this->_sta_static_dns1, this->_sta_static_dns2);  
            #else
                // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
                WiFi.config(this->_sta_static_ip, this->_sta_static_gw, this->_sta_static_sn);
            #endif 
        }

        void dump() const
        {
            LOGERROR1(F("stationIP ="), this->_sta_static_ip.toString());
            LOGERROR1(F("gatewayIP ="), this->_sta_static_gw.toString());
            LOGERROR1(F("netMask ="), this->_sta_static_sn.toString());
            #if ESP_WIFIPORTAL_USE_CONFIGURABLE_DNS
                LOGERROR3(F("dns1IP ="), this->_sta_static_dns1.toString(), ", dns2IP =", this->_sta_static_dns2.toString());
            #endif
        }


    };

private:
    WiFiMulti wifiMulti;

    Config config;

    WiFi_AP_IPConfig  AP_IPconfig;
    WiFi_STA_IPConfig STA_IPconfig;

    // For Config Portal

    // Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
    bool initialConfig;

    // SSID and PW for Config Portal
    String ssid = "ESP_" + String(ESP_getChipId(), HEX);
    String password;

    // SSID and PW for your Router
    String Router_SSID;
    String Router_Pass;
};


#endif // ESP_ESP_WIFIPORTAL_HPP