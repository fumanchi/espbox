#pragma once

#ifndef WIFIPORTAL_HPP_
#define WIFIPORTAL_HPP_

#include <array>
#include <memory>

#ifdef FS_NO_GLOBALS
#ifdef FS_H
#error FS.h has already been included with FS_NO_GLOBALS defined! This does not work with the ESP_DoubleResetDetector!
#else // defined(FS_H)
#undef FS_NO_GLOBALS
#endif // defined(FS_H)
#endif // defined(FS_NO_GLOBALS)

#define _WIFIMGR_LOGLEVEL_ 3
#define ESP_WIFIPORTAL_USING_AMERICA        false
#define ESP_WIFIPORTAL_USING_EUROPE        true
#include "ESP_WifiPortal.hpp"

#include "Settings.hpp"

class TFTStatus;

class WifiPortal : public ESP_WifiPortal
{
public:
  WifiPortal(Settings &settings);

  virtual void logger(const String &line) override;

  void setup(bool initialConfig, TFTStatus &status);
  void setup(TFTStatus &status);
  void initParameters();
  void processParameters();
  void readParameters();
  
  virtual void setupWiFiManager(ESP_WiFiManager &wifiManager) override;
  virtual void processWiFiManager(ESP_WiFiManager &wifiManager) override;
  
  bool hasConfigFile() const;
  virtual bool readConfigFile() override;
  virtual bool writeConfigFile() override;

private: /* types */
  typedef std::array<Parameter, 14> Parameters;

private:
  TFTStatus *status;
  Settings &settings;
  std::unique_ptr<Parameters> parameters;

};


#endif /* WIFIPORTAL_HPP_ */

