// this example will play a track and then 
// every five seconds play another track
//
// it expects the sd card to contain these three mp3 files
// but doesn't care whats in them
//
// sd:/mp3/0001.mp3
// sd:/mp3/0002.mp3
// sd:/mp3/0003.mp3

#ifdef FS_NO_GLOBALS
#ifdef FS_H
#error FS.h has already been included with FS_NO_GLOBALS defined! This does not work with the ESP_DoubleResetDetector!
#else // defined(FS_H)
#undef FS_NO_GLOBALS
#endif // defined(FS_H)
#endif // defined(FS_NO_GLOBALS)

#ifndef ARDUINO_LOOP_STACK_SIZE
#ifndef CONFIG_ARDUINO_LOOP_STACK_SIZE
#define ARDUINO_LOOP_STACK_SIZE 8192
#else
#define ARDUINO_LOOP_STACK_SIZE CONFIG_ARDUINO_LOOP_STACK_SIZE
#endif
#endif

#include <Arduino.h>

#include "Settings.hpp"

#include "WifiPortal.hpp"

#include "MQTTClient.hpp"

#include "TFT.hpp"
#include "TFTStatus.hpp"
#include "TFTClock.hpp"
#include "TFTStarfield.hpp"

#include "GrundschriftRegular20.h"

#include "RotaryEncoder.hpp"

#include "rfc/RFId.hpp"

#include "mp3/Mp3Player.hpp"
#include "mp3/Mp3Player_ControlEntry.hpp"
#include "mp3/Mp3Player_VolumeControl.hpp"
#include "mp3/Mp3Player_EqualizerControl.hpp"
#include "mp3/Mp3Player_FolderSelection.hpp"
#include "mp3/Mp3Player_TrackSelection.hpp"

#include "TimeControl.hpp"

Settings settings;

WifiPortal wifiPortal {settings};

MQTTClient mqttClient {settings, "tonuinox"};

TFT tft;
TFTClock tftClock {tft};
TFTStarfield tftStarfield {tft};

RFId rfid;

#include "menu/Menu.hpp"
#include "menu/QuestionSelection.hpp"

enum class Mode
{
  Blackout,
  ScreenSaver,
  Clock,
  Menu
};

Mode currentMode = Mode::Menu;
size_t timeout = 0;

class MainMenu;
MainMenu *mainMenu;

class MainMenu : public Menu
{
public:
  Menu::SubMenu *subMenuRFID = nullptr;

  static auto ProgressListenerFactory(int timeout, Menu *menu, String title, uint16_t glyph, String text)
  {
    return [menu, timeout, glyph, title, text, textOffset = 37, textWidth = 0](int currentValue) mutable
        {
          TFT_eSPI &tft = menu->tft.getHandle();
          if (currentValue == 0)
            menu->drawTitle(title.c_str(), glyph);

          tft.unloadFont();
          tft.loadFont(GrundschriftRegular20);
          tft.setTextColor(TFT_BLACK, TFT_WHITE);
          tft.setTextDatum(TL_DATUM);
          if (currentValue == 0)
          {
            tft.drawString(text, textOffset, 80);
          }
          if (timeout)
          {
            String valueText {timeout - currentValue};
            if (!textWidth) // textWidth still 0 => first iteration _and_ timeout given...
            {
              textOffset +=  tft.textWidth(text) + 5;
              textWidth = tft.textWidth(valueText);
            }
            tft.fillRect(textOffset - 2, 75, textWidth + 4, 20, TFT_WHITE);
            tft.drawString(valueText, textOffset, 80);
          }
        };
  };


  MainMenu(TFT &tft)
    : Menu  {tft, RotaryEncoder::Instance(), "Main", {
        new Mp3Player::ControlEntry {this, &Mp3Player::Instance()},
        new Menu::SubMenu{0x00A2, "Ton", 
          {
            new Menu::Leaf{0x00D6, "Lautstärke", [](Menu *menu, Menu::Entry*) { Serial.println(__PRETTY_FUNCTION__); Mp3Player::VolumeControl::Spawn(menu, "volume", &Mp3Player::Instance(), 20 /* settings.maxVolume */); }},
            new Menu::Leaf{0x002A, "Klang", [](Menu *menu, Menu::Entry*) 
              {
              Serial.println(__PRETTY_FUNCTION__); Mp3Player::EqualizerControl::Spawn(menu, "Equalizer", &Mp3Player::Instance()); 
              }}
          }
        },
        ( 
          new Menu::SubMenu{0x004C, "RFId", 
            {
              new Menu::Leaf{0x0066, "card info", [](Menu *menu, Menu::Entry*)
                {          
                  if (!rfid.waitCard(10, ProgressListenerFactory(10, menu, "card info", 0x0066, "Insert card...")))
                  {
                    menu->drawTitle(menu->current);
                    menu->draw();
                    return;
                  }
                  if (!(rfid.mfrc522.PICC_IsNewCardPresent() && rfid.mfrc522.PICC_ReadCardSerial()))
                  {
                    rfid.mfrc522.PCD_Init();
                    delay(40);
                    if (!(rfid.mfrc522.PICC_IsNewCardPresent() && rfid.mfrc522.PICC_ReadCardSerial()))
                    {
                      menu->drawTitle(menu->current);
                      menu->draw();
                      return;
                    }
                  }
                  RFIdTag tag;
                  if (rfid.readCard(tag) && tag)
                  { 
                    // rfid.waitCardLeft(0, ProgressListenerFactory(0, menu, "card info", 0x0066, String("PlayFolder (") + String {detail->getFolder()} + ")"));
                  }
                  else
                  {
                    rfid.waitCardLeft(0, ProgressListenerFactory(0, menu, "card info", 0x0066, "Unknown card..."));
                  }
                  menu->drawTitle(menu->current);
                  menu->draw();
                  return;
                }
              },
              new Menu::Leaf{0x003A, "erase card", [](Menu *menu, Menu::Entry*)
                { 
                  // if (!rfid.waitCard(10, menu, "erase card", 0x003A, "Insert card..."))
                  if (!rfid.waitCard(10, ProgressListenerFactory(10, menu, "erase card", 0x003A, "Insert card...")))
                  {
                    menu->drawTitle(menu->current);
                    menu->draw();
                    return;
                  }
                  Serial.printf("%s@%d: menu->editors.size=%d\n", __PRETTY_FUNCTION__, __LINE__, menu->editors.size());
                  menu->editors.push_back(new QuestionSelection
                    {
                      menu, 
                      {
                        QuestionSelection::Entry{1, "Yes"},
                        QuestionSelection::Entry{0, "No"}
                      },
                      "Sure?",
                      [](int selection) -> bool 
                      { 
                        if (selection == 1)
                        {
                          Menu *menu = Menu::CurrentInstance;
                          rfid.waitCardLeft(2, ProgressListenerFactory(2, menu, "Erase card...", 0x003A, rfid.clearCard() ? "Succeeded..." : "Failed..."));
                          rfid.waitCardLeft(0, ProgressListenerFactory(0, menu, "Erase card...", 0x003A, "Remove card..."));
                          // rfid.waitCardLeft(2, menu, "erase card", 0x003A, rfid.clearCard() ? "Succeeded..." : "Failed...");
                          // rfid.waitCardLeft(0, menu, "erase card", 0x003A, "Remove card...");
                        }
                        Serial.printf("%s@%d: selection=%d menu->editors.size=%d\n", __PRETTY_FUNCTION__, __LINE__, selection, Menu::CurrentInstance->editors.size());
                        return false;
                      }
                    });
                  Serial.printf("%s@%d: menu->editors.size=%d\n", __PRETTY_FUNCTION__, __LINE__, menu->editors.size());
                  menu->editors.back()->setup();
                }
              }

            },  0,  Menu::SubMenu::Direction::HORIZONTAL            
        })
      }, 0 /* index */,
      [](Menu *menu) -> bool { /* exit callback */
        menu->unregisterIO();
        if (wifiPortal.networking)
          currentMode = Mode::Clock;
        else
          currentMode = Mode::ScreenSaver;
        timeout = 0;
        return true;
      }},
      subMenuRFID {(Menu::SubMenu*)this->root.entries[1]}
  {
    Serial.printf("%s: subMenuRFID=%p\n", __PRETTY_FUNCTION__, subMenuRFID);
  }

  void setup()
  {
    this->Menu::begin();
    Serial.printf("%s: subMenuRFID=%p\n", __PRETTY_FUNCTION__, subMenuRFID);
  }

};

MainMenu menu {tft};
Menu *mfrc522Wizard = nullptr;

// #include "soc/soc.h"
// #include "soc/rtc_cntl_reg.h"

// void esp32Setup() 
// {
//   Serial.println("Initializing ESP32...");
//   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
// }


/*
*   A hook callback for external ram allocation fails.
*/
// void heap_caps_alloc_failed_hook(size_t requested_size, uint32_t caps, const char *function_name)
// {
//     Serial.printf("[E] %s was called but failed to allocate %d bytes with 0x%X capabilities. \n", function_name, requested_size, caps);
// }

void setup() 
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  heap_caps_malloc_extmem_enable(500);
  //esp_err_t error = heap_caps_register_failed_alloc_callback(heap_caps_alloc_failed_hook);

  // Serial.println("initializing esp32...");
  // esp32Setup();

  Serial.println("initializing tft...");
  tft.setup();

  {
    TFTStatus status {tft};
    status.setup(F("Initializing"));

    status.push(F("initialize filesystem..."));
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

    if (wifiPortal.hasConfigFile())
    {
      if (wifiPortal.readConfigFile())
        status.push(F("config file read..."));

      Serial.println("activate MQTT...");
      mqttClient.setup();
      mqttClient.subscribe("name", [](const String &topic, String value) 
        { 
          menu.root.label = value.c_str();
          if (Menu::CurrentInstance == &menu && menu.current == &menu.root)
            menu.setDirty();
          return true; 
        });      
      mqttClient.subscribe("color", [](const String &topic, String value) 
        {
          uint16_t color565 = tft.getHandle().color24to16(strtol(value.c_str() + (int)(value[0] == '#'), 0, 16));
          uint32_t color888 = tft.getHandle().color16to24(color565);
          char tmp[8]; snprintf(tmp, 8, "#%06X", color888);
          mqttClient.publish(topic, tmp);
          menu.setBackground(color565);
          return false; 
        });
      mqttClient.publish("reset", "false");
      mqttClient.subscribe("reset", [](const String &topic, String value) 
        {
          if (MQTTClient::ToBool(value))
          {
            mqttClient.publish(topic, "false", []{ESP.restart();});          
          }
          return false; 
        });
      mqttClient.publish("control/volume", String {Mp3Player::Instance().getVolume()});
      mqttClient.subscribe("control/volume", [](String topic, String value) 
        {
          uint8_t volume = (uint8_t)strtol(value.c_str(), nullptr, 10);
          Mp3Player::Instance().setVolume(volume);
          mqttClient.publish(topic, String(volume));
          return false; 
        });
      mqttClient.loop();
      mqttClient.publish("control/stop", "false");
      mqttClient.subscribe("control/stop", [](String topic, String value) 
        {
          if (MQTTClient::ToBool(value))
            Mp3Player::Instance().stop();
          mqttClient.publish(topic, "false");
          return false; 
        });
      mqttClient.loop();
      mqttClient.publish("control/pause", "false");    
      mqttClient.subscribe("control/pause", [](String topic, String value) 
        {
          if (MQTTClient::ToBool(value))
            Mp3Player::Instance().pause();
          mqttClient.publish(topic, "false");
          return false; 
        });
      mqttClient.loop();
      mqttClient.publish("control/play", "false");
      mqttClient.subscribe("control/play", [](String topic, String value) 
        {
          if (MQTTClient::ToBool(value))
            Mp3Player::Instance().start();
          mqttClient.publish(topic, "false");
          return false; 
        });
      mqttClient.loop();
      mqttClient.publish("control/next", "false");
      mqttClient.subscribe("control/next", [](String topic, String value) 
        {
          if (MQTTClient::ToBool(value))
            Mp3Player::Instance().nextTrack();
          mqttClient.publish(topic, "false");
          return false; 
        });
      mqttClient.loop();
      mqttClient.publish("control/prev", "false");
      mqttClient.subscribe("control/prev", [](String topic, String value) 
        {
          if (MQTTClient::ToBool(value))
            Mp3Player::Instance().prevTrack();
          mqttClient.publish(topic, "false");
          return false; 
        });
      mqttClient.loop();

      TimeControl::Instance().setup(mqttClient, [](TimeControl::TimePointProperty property, const TimeControl::TimePointSetting &timePointSetting) 
        {
          using Property = TimeControl::TimePointProperty;
          if (property == Property::Brightness || property == Property::All)
            tft.setBrightness(timePointSetting.brightness);
          if (property == Property::Color || property == Property::All)
            tftClock.setBackgroundColor(timePointSetting.color);
          if (property == Property::MaxVolume || property == Property::All)
            Mp3Player::Instance().setMaxVolume(timePointSetting.maxVolume);
          Serial.printf("%s: TimeControl notifies property=%u timepoint=%s\n", __PRETTY_FUNCTION__, property, timePointSetting.label);
        });

      Serial.println("activate wifi portal...");
      status.push(F("spawning wifi portal..."));
      wifiPortal.setup(status);
    }
    else
    {
      status.push(F("config file missing..."));

      Serial.println("activate wifi portal...");
      status.push(F("spawning wifi portal..."));
      wifiPortal.setup(true, status);
    }
    if (WiFi.status() != WL_CONNECTED)
      wifiPortal.readConfigFile();

    status.push(F("initializing mp3 player..."));
    Serial.println("initializing mp3 player...");
    Mp3Player::Instance().setup(settings);

    status.push(F("initializing tft clock..."));
    Serial.println("initializing tft clock...");
    tftClock.setup();

    status.push(F("initializing mrfc522..."));
    Serial.println("initializing mrfc522...");
    rfid.setup(settings);

    status.push(F("initializing encoder..."));
    Serial.println("initializing encoder...");
    RotaryEncoder::Instance().setup(settings);

    status.push(F("initializing complete..."));
    Serial.println("initializing complete...");

  } // status dies...

  // menu.setup();
}


void loop() 
{
  wifiPortal.loop();
  RotaryEncoder::Instance().loop();
  if (wifiPortal.networking)
  {
    mqttClient.loop();
    TimeControl::Instance().loop();
  }
  Mp3Player::Instance().loop();

  // Serial.printf("%s: Menu::CurrentInstance == &menu && menu.current == menu.subMenuRFID -> (%p == %p && %p == %p) = %s\n", __PRETTY_FUNCTION__, Menu::CurrentInstance, &menu, menu.current, menu.subMenuRFID, (Menu::CurrentInstance == &menu && menu.current == menu.subMenuRFID) ? "true" : "false");
  if (Menu::CurrentInstance != mfrc522Wizard && !(Menu::CurrentInstance == &menu && menu.current == menu.subMenuRFID))
  if (rfid.mfrc522.PICC_IsNewCardPresent()) 
  {
    Serial.printf("%s: New card...\n", __PRETTY_FUNCTION__);
    if (rfid.mfrc522.PICC_ReadCardSerial()) 
    {
      Serial.printf("%s: Succeded to read card serial... (PICC_ReadCardSerial)\n", __PRETTY_FUNCTION__);
      RFIdTag tag;
      if (!rfid.readCard(tag))
      {
        Serial.printf("%s: Failed to read card tag...\n", __PRETTY_FUNCTION__);
      }
      else
      {
        Serial.printf("%s: Succeded to read card tag...\n", __PRETTY_FUNCTION__);
        if (tag)
        { 
          tag.dump();
          Mp3Player::Instance().runState(tag);
          if (currentMode == Mode::Blackout)
            tft.enableBacklight(true);
          Menu *menu = Menu::CurrentInstance;
          rfid.waitCardLeft(2, MainMenu::ProgressListenerFactory(2, menu, "Playing...", 0x004C, "..."));
          timeout = 0;  
        }
        else
        {
          Serial.printf("%s: Unknown card tag...\n", __PRETTY_FUNCTION__);
          if (Menu::CurrentInstance->editors.empty())
          {
            Serial.printf("%s: Spawning RFId Wizard...\n", __PRETTY_FUNCTION__);
            
            mfrc522Wizard = new Menu 
              {
                Menu::CurrentInstance, 
                "RFId...", 
                {
                  new Menu::Leaf{0x006C, "folder", [](Menu *menu, Menu::Entry*) 
                    { 
                      using FolderSelection = Mp3Player::FolderSelection;
                      Serial.println(__PRETTY_FUNCTION__);
                      FolderSelection::Spawn(menu, std::string{"Auswahl..."}, &Mp3Player::Instance(), [](FolderSelection *editor, Editor::Result result) -> bool
                        { 
                          bool retval = true;
                          switch (editor->getCurrentMode())
                          {
                            case FolderSelection::Mode::Folder:
                            {
                              using TrackSelection = Mp3Player::TrackSelection;
                              auto folder = editor->getCurrentFolder();
                              TrackSelection::Spawn(Menu::CurrentInstance, (String {"Folder "} + String {folder}).c_str(), &Mp3Player::Instance(), folder, [](TrackSelection *editor, Editor::Result result) -> bool
                                {    
                                  bool retval = true;
                                  using Mode = TrackSelection::Mode;
                                  Serial.printf("%s@%d: folder=%u editor->getCurrentMode()=%u\n", __PRETTY_FUNCTION__, __LINE__, editor->getFolder(), editor->getCurrentMode());
                                  switch (editor->getCurrentMode())
                                  {
                                    case Mode::Back:
                                      break;
                                    case Mode::Folder:
                                    case Mode::Random:
                                    {
                                      Serial.printf("%s@%d: editor->getFolder()=%u\n", __PRETTY_FUNCTION__, __LINE__, editor->getFolder());
                                      Menu::CurrentInstance->editors.push_back(new QuestionSelection
                                        {
                                          Menu::CurrentInstance, 
                                          {
                                            QuestionSelection::Entry{1, "Keep Playing"},
                                            QuestionSelection::Entry{0, "Play Once"}
                                          },
                                          "Mode?",
                                          [folder = editor->getFolder(), mode = editor->getCurrentMode()](int selection) -> bool 
                                          { 
                                            Serial.printf("%s@%d: folder=%u selection=%s\n", __PRETTY_FUNCTION__, __LINE__, folder, selection ? "repeat" : "no repeat");
                                            RFIdTag tag {folder, 0, {}};
                                            tag[RFIdTag::Flag::REPEAT] = (bool)selection;
                                            tag[RFIdTag::Flag::RANDOM] = mode == Mode::Random;
                                            tag.dump(); 
                                            if (!rfid.writeCard(tag))
                                            {
                                              rfid.waitCardLeft(2, MainMenu::ProgressListenerFactory(2, Menu::CurrentInstance, "Card...", 0x003A, "Failed..."));
                                              return false;
                                            }
                                            rfid.waitCardLeft(2, MainMenu::ProgressListenerFactory(2, Menu::CurrentInstance, "Card...", 0x003A, "Succeeded..."));
                                            rfid.waitCardLeft(0, MainMenu::ProgressListenerFactory(0, Menu::CurrentInstance, "Card...", 0x003A, "Remove card..."));
                                            return true;
                                          }
                                        });
                                      Menu::CurrentInstance->editors.back()->setup();
                                      break;
                                    }
                                    // case Mode::Random:
                                    // {
                                    //   Serial.printf("%s@%d: editor->getFolder()=%u\n", __PRETTY_FUNCTION__, __LINE__, editor->getFolder());
                                    //   Menu::CurrentInstance->editors.push_back(new QuestionSelection
                                    //     {
                                    //       Menu::CurrentInstance, 
                                    //       {
                                    //         QuestionSelection::Entry{1, "Keep Playing"},
                                    //         QuestionSelection::Entry{0, "Play Once"}
                                    //       },
                                    //       "Mode?",
                                    //       [folder = editor->getFolder()](int selection) -> bool 
                                    //       { 
                                    //         Serial.printf("%s@%d: folder=%u selection=%s\n", __PRETTY_FUNCTION__, __LINE__, folder, selection ? "repeat" : "no repeat");
                                    //         RFIdTag tag {folder, 0, {RFIdTag::Flag::RANDOM}};
                                    //         tag[RFIdTag::Flag::REPEAT] = (bool)selection;
                                    //         tag.dump(); 
                                    //         if (!rfid.writeCard(tag))
                                    //         {
                                    //           rfid.waitCardLeft(2, MainMenu::ProgressListenerFactory(2, Menu::CurrentInstance, "Card...", 0x003A, "Failed..."));
                                    //           return false;
                                    //         }
                                    //         rfid.waitCardLeft(2, MainMenu::ProgressListenerFactory(2, Menu::CurrentInstance, "Card...", 0x003A, "Succeeded..."));
                                    //         rfid.waitCardLeft(0, MainMenu::ProgressListenerFactory(0, Menu::CurrentInstance, "Card...", 0x003A, "Remove card..."));
                                    //         return true;
                                    //       }
                                    //     });
                                    //   Menu::CurrentInstance->editors.back()->setup();
                                    //   break;
                                    // }
                                    case Mode::SingleTrack:
                                    {
                                      Menu::CurrentInstance->editors.push_back(new QuestionSelection
                                        {
                                          Menu::CurrentInstance, 
                                          {
                                            QuestionSelection::Entry{1, "Keep Playing"},
                                            QuestionSelection::Entry{0, "Play Once"}
                                          },
                                          "Mode?",
                                          [folder = editor->getFolder(), track = editor->getCurrentTrack()](int selection) -> bool 
                                          { 
                                            Serial.printf("%s@%d: folder=%u track=%u selection=%s\n", __PRETTY_FUNCTION__, __LINE__, folder, track, selection ? "repeat" : "no repeat");
                                            RFIdTag tag {folder, track, {}};
                                            tag[RFIdTag::Flag::REPEAT] = (bool)selection;
                                            tag.dump(); 
                                            rfid.writeCard(tag); 
                                            return true;
                                          }
                                        });
                                      Menu::CurrentInstance->editors.back()->setup();
                                      break;
                                    }
                                  }

                                  return true;
                                });
                              break;
                            }
                            case FolderSelection::Mode::Track:
                            {
                              Menu::CurrentInstance->editors.push_back(new QuestionSelection
                                {
                                  Menu::CurrentInstance, 
                                  {
                                    QuestionSelection::Entry{1, "Loop"},
                                    QuestionSelection::Entry{0, "Single"}
                                  },
                                  "Mode?",
                                  [track = editor->getCurrentTrack()](int selection) -> bool 
                                  { 
                                    Serial.printf("%s: selection=%d\n", __PRETTY_FUNCTION__, selection);
                                    RFIdTag tag {0, track, {}};
                                    tag[RFIdTag::Flag::LOOP] = (bool)selection;
                                    tag.dump(); 
                                    rfid.writeCard(tag); 
                                    return false;
                                  }
                                });
                              Menu::CurrentInstance->editors.back()->setup();
                              break;
                            }
                            case FolderSelection::Mode::RandomGlobalTrack:
                            {
                              Menu::CurrentInstance->editors.push_back(new QuestionSelection
                                {
                                  Menu::CurrentInstance, 
                                  {
                                    QuestionSelection::Entry{1, "Keep Playing"},
                                    QuestionSelection::Entry{0, "Play Once"}
                                  },
                                  "Mode?",
                                  [track = editor->getCurrentTrack()](int selection) -> bool 
                                  { 
                                    Serial.printf("%s: selection=%d\n", __PRETTY_FUNCTION__, selection);
                                    RFIdTag tag {0, 0, {RFIdTag::Flag::RANDOM}};
                                    tag[RFIdTag::Flag::REPEAT] = (bool)selection;
                                    tag.dump(); 
                                    rfid.writeCard(tag); 
                                    return false;
                                  }
                                });
                              Menu::CurrentInstance->editors.back()->setup();
                              break;
                            }
                          }
                          return retval;
                        },
                        Mp3Player::FolderSelection::AllExtras
                      );
                    }
                  // },
                  // new Menu::Leaf{0x0068, "tracks", [](Menu *menu, Menu::Entry*)
                  //   { 
                  //   //   ListEditor::Spawn<SonosAPIClient::PlaylistSelection>(menu, std::string{"play playlist"}, sonosAPIClient,
                  //   //   [](SonosAPIClient &client, const String &playlist) -> void 
                  //   //   { 
                  //   //     NFCTag tag; 
                  //   //     tag.cookie = NFCTag::MagicCookie;
                  //   //     tag.version = 1;
                  //   //     tag.type = NFCTag::Type::SonosPlayPlaylist; 
                  //   //     tag.detail = new NFCTag::SonosPlayHash {playlist}; 
                  //   //     tag.dump(); 
                  //   //     nfc.writeCard(tag); 
                  //   //   }
                  //   // );
                  //   }
                  }
                }
              };
            mfrc522Wizard->begin();
            currentMode = Mode::Menu;
            timeout = millis();
            //mfrc522Wizard->draw();
          }
        }
      }
    }
  }

  auto *button = &RotaryEncoder::Instance().getButton();
  switch (currentMode)
  {
    case Mode::Blackout:
      if (timeout == 0)
      {
        Serial.println("Switched to Blackout....");
        tft.enableBacklight(false);
        timeout = millis();
        tft.getHandle().fillScreen(TFT_BLACK);
        Serial.printf("%s@%d: set click handler....\n", __PRETTY_FUNCTION__, __LINE__);
        button->setClickHandler([](Button2 &button)
          {
            Serial.printf("%s@%d: unset click handler....\n", __PRETTY_FUNCTION__, __LINE__);
            button.setClickHandler(nullptr);
            if (wifiPortal.networking)
              currentMode = Mode::Clock;
            else
              currentMode = Mode::Menu;
            tft.enableBacklight(true);
            timeout = 0;
          });
      }
      break;
    case Mode::ScreenSaver:
      if (timeout == 0)
      {
        Serial.println("Switched to Screensaver....");
        timeout = millis();
        tftStarfield.reset();

        Serial.printf("%s@%d: set click handler....\n", __PRETTY_FUNCTION__, __LINE__);
        button->setClickHandler([](Button2 &button)
          {
            Serial.printf("%s@%d: unset click handler....\n", __PRETTY_FUNCTION__, __LINE__);
            button.setClickHandler(nullptr);
            if (wifiPortal.networking)
              currentMode = Mode::Clock;
            else
              currentMode = Mode::Menu;
            timeout = 0;
          });
      }
      tftStarfield.loop();
      if (millis() - timeout >= 20 * 1000) 
      {
          Serial.printf("%s@%d: unset click handler....\n", __PRETTY_FUNCTION__, __LINE__);
          button->setClickHandler(nullptr);
          currentMode = Mode::Blackout;
          timeout = 0;
      }
      break;
    case Mode::Clock:
      tftClock.loop();
      // loop_clock();
      if (timeout == 0)
      {
        Serial.println("Switched to Clock....");
        timeout = millis();
        button->setLongClickHandler([](Button2 &button)
          {
            button.setLongClickHandler(nullptr);
            menu.setup();
            currentMode = Mode::Menu;
            timeout = 0;
          });
      }
      if (millis() - timeout >= 60 * 1000) 
      {
          button->setLongClickHandler(nullptr);
          currentMode = Mode::ScreenSaver;
          timeout = 0;
      }
      break;
    case Mode::Menu:
      if (timeout == 0)
      {
        Serial.println("Switched to Menu....");
        Serial.printf("Menu::CurrentInstance=%p timeout=%ul\n", (void*)Menu::CurrentInstance, timeout);
        timeout = millis();
        menu.setup();
        Serial.printf("Menu::CurrentInstance=%p timeout=%ul\n", (void*)Menu::CurrentInstance, timeout);
      }
      // if (Menu::CurrentInstance)
      // {
      //   Menu::CurrentInstance->loop();
      // }
      if (Menu::CurrentInstance)
      {
        if (Menu::CurrentInstance == mfrc522Wizard && !rfid.checkCardPresent())
        {
          Serial.printf("%s: card removed....\n", __PRETTY_FUNCTION__);
          Menu::CurrentInstance->back();
          return;
        }
        Menu::CurrentInstance->loop();
      }
      // Serial.printf("%s: Menu::CurrentInstance == &menu && menu.current == menu.subMenuRFID -> (%p == %p && %p == %p) = %s\n", __PRETTY_FUNCTION__, Menu::CurrentInstance, &menu, menu.current, menu.subMenuRFID, (Menu::CurrentInstance == &menu && menu.current == menu.subMenuRFID) ? "true" : "false");
      // if (rfid.mfrc522.PICC_IsNewCardPresent()) 
      // {
      //   if (!rfid.mfrc522.PICC_ReadCardSerial()) 
        
      // }
      if (Menu::CurrentInstance == &menu && !Menu::Blackout) 
      {
        Menu::CurrentInstance->back();
        if (wifiPortal.networking)
          currentMode = Mode::Clock;
        else
          currentMode = Mode::ScreenSaver;
        timeout = 0;
      }
      break;
  }
}
