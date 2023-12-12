#include <WiFi.h>

#include "Menu.hpp"
#include "Editor.hpp"

// #include "openiconic28.h"
#include "GrundschriftBold28.h"
#include "GrundschriftRegular20.h"
#include "myopeniconic228.h"


MenuIO::MenuIO(TFT &tft, RotaryEncoder &rotary)
  : tft {tft},
    rotary {rotary}
{
  
}

MenuIO::MenuIO(const MenuIO &other)
  : tft {other.tft},
    rotary {other.rotary}
{
    
}

Menu::SmartMenuIO::SmartMenuIO(Menu *menu)
  : MenuIO {*menu},
    menu {menu}
{
  if (this->menu)
    this->menu->unregisterIO();
}

Menu::SmartMenuIO::SmartMenuIO(SmartMenuIO &&other)
  : MenuIO {other},
    menu {nullptr}
{
  std::swap(this->menu, other.menu);  
}

Menu::SmartMenuIO::~SmartMenuIO()
{
  this->release();
}

bool Menu::SmartMenuIO::release()
{
  bool retval = this->menu != nullptr;
  if (retval)
  {
    this->menu->registerIO();
    this->menu = nullptr;
  }
  return retval;
}

Menu::Entry::Entry(Type type, uint16_t glyph, std::string label, callback_t callback)
  : parent {nullptr},

    type {type},
    glyph {glyph},
    label {std::move(label)},
    callback {callback}
{
  
}

Menu::Leaf::Leaf(uint16_t glyph, std::string label, callback_t callback)
  : Entry {Type::Leaf, glyph, std::move(label), callback}
{
      
}

bool Menu::Leaf::isSubMenu() { return false; }

Menu::Toggle::Toggle(StateEntry stateEntryEnabled, StateEntry stateEntryDisabled, bool state)
  : Toggle {std::move(stateEntryEnabled), std::move(stateEntryDisabled), nullptr, state}
{

}

Menu::Toggle::Toggle(StateEntry stateEntryEnabled, StateEntry stateEntryDisabled, callback_t callback, bool state)
  : Leaf {
        
        state ? stateEntryEnabled.first : stateEntryDisabled.first, 
        state ? stateEntryEnabled.second : stateEntryDisabled.second, 
        [](Menu *menu, Entry *entry)
        {
          if (Menu::Toggle *toggle = static_cast<Menu::Toggle*>(entry))
          {
            toggle->toggle();
            menu->draw();
            if (toggle->Toggle::callback)
              toggle->Toggle::callback(menu, entry);
          }
        } 
      },
    stateEntries {std::move(stateEntryEnabled), std::move(stateEntryDisabled)},
    callback {callback},
    state {state}
{

}

void Menu::Toggle::toggle()
{
  this->setState(!this->state);
}

void Menu::Toggle::setState(bool state)
{
  if (state != this->state)
  {
    this->state = state;
    this->Entry::glyph = this->stateEntries[(int)this->state].first;
    this->Entry::label = this->stateEntries[(int)this->state].second;
  }
}

bool Menu::Toggle::getState() const
{
  return this->state;
}

Menu::Value::Value(std::string label, callback_t callback)
  : Value {0, std::move(label), callback}
{
  
}

Menu::Value::Value(uint16_t glyph, std::string label, callback_t callback)
  : Entry {Type::Value, glyph, std::move(label), callback}
{
    
}

bool Menu::Value::isSubMenu()
{
  return false;
}

Menu::CheckValue::CheckValue(std::string label, getState_t getState, setState_t setState)
  : Value {std::move(label)},
    getState {getState},
    setState {setState}
{
}

const uint8_t *Menu::CheckValue::getValueFont()
{
  /* return u8g2_font_open_iconic_check_1x_t;  */
  return nullptr;
}

const char *Menu::CheckValue::getValue(Menu *menu, Value *value)
{
  static const char *On = "\x0040";
  static const char *Off = "\x0044";
  return this->getState(menu, static_cast<CheckValue*>(value)) ? On : Off;
}


Menu::StringValue::StringValue(std::string label, getStringValue_t getStringValue, setStringValue_t setStringValue, editCallback_t editCallback, callback_t callback)
  : StringValue {0, std::move(label), getStringValue, setStringValue, editCallback, callback}
{
      
}

Menu::StringValue::StringValue(uint16_t glyph, std::string label, getStringValue_t getStringValue, setStringValue_t setStringValue, editCallback_t editCallback, callback_t callback)
  : Value {glyph, std::move(label), callback},
    getStringValue {getStringValue},
    setStringValue {setStringValue},
    editCallback {editCallback}
{
      
}

const uint8_t *Menu::StringValue::getValueFont()
{
  /* return u8g2_font_6x12_tf; */
  return nullptr;
}

const char *Menu::StringValue::getValue(Menu *menu, Value *value)
{
  return this->getStringValue(menu, static_cast<StringValue*>(value));
}

void Menu::StringValue::setValue(Menu *menu, Value *value, const char *data)
{
  if (this->setStringValue)
    this->setStringValue(menu, static_cast<StringValue*>(value), data);
}


Menu::SubMenu::SubMenu(uint16_t glyph, std::string label, std::initializer_list<Entry*> entries, int8_t index, Direction direction, ExitCallback exitCallback)
  : Entry {Type::SubMenu, glyph, std::move(label), [](Menu *menu, Menu::Entry *entry) {menu->setCurrent(static_cast<SubMenu*>(entry));}},
    entries {entries.begin(), entries.end()},
    index {index},
    direction {direction},
    exitCallback {exitCallback}
{
  for (Entry *entry : this->entries)
    entry->parent = this;
}

bool Menu::SubMenu::isSubMenu() { return true; }

Menu::Root::Root(Menu *menu, std::string label, std::initializer_list<Entry*> entries, int8_t index, ExitCallback exitCallback)
  : SubMenu {0, std::move(label), entries, index, Direction::HORIZONTAL, exitCallback},
    menu {menu}
{
  
}

/* Menu::Menu(U8G2 &u8g2, Encoder &rotary, Button2 &button, std::string label, std::initializer_list<Entry*> entries, int8_t index)
  : MenuIO {u8g2, rotary, button},
  */
Menu::Menu(TFT &tft, RotaryEncoder &rotary, std::string label, std::initializer_list<Entry*> entries, int8_t index, SubMenu::ExitCallback exitCallback)
  : MenuIO {tft, rotary},

    backgroundColor {tft.getHandle().color565(200, 200, 200)},
    
    sprite {&tft.getHandle()},

    root {this, std::move(label), entries, index, std::move(exitCallback)},

    editors {},
    current {nullptr},

    dirty {true},
    index {0},
     
    locationCurrent {0},

    smartMenuIOGiven {nullptr}
{
//  this->setCurrent(&this->root);

  // Create the horizontal sprite
  this->sprite.setColorDepth(8); // 8 bit will work, but reduces effectiveness of anti-aliasing
  this->sprite.createSprite(170, 116);
}

static bool MenuExit(Menu *menu)
{
  menu->exit();
  delete menu;
  return true;
}

Menu::Menu(Menu *menu, std::string label, std::initializer_list<Entry*> entries, int8_t index, SubMenu::ExitCallback exitCallback)
  : MenuIO {*menu},

    backgroundColor {menu->backgroundColor},

  
    sprite {&tft.getHandle()},

    root {this, std::move(label), entries, index, exitCallback ? exitCallback : &MenuExit},

    editors {},
    current {nullptr},
    index {0},
     
    locationCurrent {0},

    smartMenuIOGiven {new SmartMenuIO {menu} }
{
  this->setCurrent(&this->root, 0, false);

  // Create the horizontal sprite
  this->sprite.setColorDepth(8); // 8 bit will work, but reduces effectiveness of anti-aliasing
  this->sprite.createSprite(170, 116);
}

Menu::~Menu()
{
  if (this->smartMenuIOGiven)
    delete this->smartMenuIOGiven;
  this->smartMenuIOGiven = nullptr;
}

void Menu::setCurrent(SubMenu *sub, int8_t index, bool draw)
{
  // Serial.printf("%s: sub=%p, index=%d draw=%s\n", __PRETTY_FUNCTION__, sub, index, draw ? "true" : "false");
  if (!sub)
    sub = &this->root;
  if (this->current == sub)
    return;
    
  this->current = sub;
  this->index = index == -1 ? (uint8_t)sub->index : index; // std::min((uint8_t)index, (uint8_t)(this->current->entries.size() - 1));
  this->rotary.setBoundaries(0, this->current->entries.size() - 1);    
  this->rotary.setEncoderValue(this->index);
  
  // this->dirty = true;
  
  if (sub->direction == SubMenu::Direction::HORIZONTAL)
    this->locationCurrent = this->index * 42;
  else
    this->locationCurrent = this->index * 15;

  // Serial.printf("%s: this->index=%u this->rotary.readEncoder()=%d boundary=[0,%u] this->locationCurrent=%lu\n", __PRETTY_FUNCTION__, this->index, this->rotary.readEncoder(), this->current->entries.size() - 1, this->locationCurrent);

  if (draw)
    this->draw();
}

Menu::SmartMenuIO Menu::getIO()
{
  return SmartMenuIO {this};
}

void Menu::setDirty()
{
  this->dirty = true;
}

void Menu::setBackground(uint16_t color)
{
  this->backgroundColor = color;
  if (Menu::CurrentInstance == this)
    this->dirty = true;
}

void Menu::begin()
{
  // Serial.println(__PRETTY_FUNCTION__);
  this->setCurrent(&this->root);
  this->registerIO();
}

void Menu::exit()
{
  // Serial.println(__PRETTY_FUNCTION__);
  CurrentInstance = nullptr;
}

void Menu::selected()
{
  if (Entry *entry = this->current ? this->current->entries[this->index] : nullptr)
  {
    if (entry->type == Entry::Type::Value)
    {
      StringValue *stringValue = static_cast<StringValue*>(entry);
      if (stringValue && stringValue->editCallback)
      {
        stringValue->editCallback(this, stringValue);
        return;
      }
    }
    if (entry->callback)
      entry->callback(this, entry);
  }
}

void Menu::back()
{
  if (this->current->type == Entry::Type::SubMenu)
  {
    SubMenu *last = this->current;
    if (SubMenu *sub = this->current->parent)
    {
      uint8_t index = 0;
      for (; index < sub->entries.size(); ++index)
        if (sub->entries[index] == this->current)
          break;
      this->setCurrent(sub, index, false);
    }

    if (last->exitCallback && last->exitCallback(this))
        return;
    this->draw();
  }
}

void Menu::drawTitle(const Entry *entry)
{
  if (entry)
    this->drawTitle(entry->label.empty() ? "root" : entry->label.c_str(), entry->glyph);  
}

void Menu::drawTitle(const char *label, uint16_t glyph)
{
  // Serial.println(__PRETTY_FUNCTION__);
  UpdateBlackout();

  auto &tft = this->tft.getHandle();

  // border
  tft.fillScreen(this->backgroundColor);
  tft.drawSmoothRoundRect(30, 30, 30, 30, 179, 179, TFT_BLACK, this->backgroundColor);
  tft.fillSmoothRoundRect(31, 31, 178, 178, 29, TFT_WHITE, TFT_BLACK);


  // setup
  tft.unloadFont();
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  if (glyph)
  {
    // glyph
    tft.setTextDatum(TL_DATUM);
    tft.loadFont(myopeniconic228);
    tft.setCursor(42, 37);
    tft.drawGlyph(glyph);
    tft.unloadFont();
  }
  
  // title  
  tft.loadFont(GrundschriftBold28);
  tft.setTextDatum(TC_DATUM); 
  tft.drawString(label, tft.width() / 2, 37);
  tft.unloadFont();

 /* 
  // title
  this->u8g2.setMaxClipWindow();
  this->u8g2.setDrawColor(1);
  this->u8g2.drawBox(0, 0, 128, 10);
  this->u8g2.setDrawColor(0);
  this->u8g2.setFont(u8g2_font_5x7_tf);
  int strWidth = u8g2.getStrWidth(label);
  if (!glyph)
  {
      this->u8g2.drawUTF8(64 - (strWidth / 2), 9, label);
      this->u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
  }
  else
  {
    this->u8g2.drawUTF8(64 - ((strWidth + 12)/ 2) + 12, 9, label);
    this->u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
    this->u8g2.drawGlyph(64 - ((strWidth + 12)/ 2), 9, glyph);
  }
  const auto glyphs = this->getTitleGlyphs();
  for (int i = 0; i < glyphs.first; ++i)
    this->u8g2.drawGlyph(128 - ((i + 1) * 10), 9, glyphs.second[i]);
    //this->u8g2.drawGlyph(128 - (glyphs.first - i) * 10, 9, glyphs.second[i]);
  this->u8g2.setDrawColor(1);
  
  // frame & clip...
  this->u8g2.drawFrame(0, 10, 128, 53);
  this->u8g2.setClipWindow(1, 11, 126, 62);  
*/
}

void Menu::draw()
{
  // static size_t counter = 0;
  // Serial.printf("%s: counter=%lu\n", __PRETTY_FUNCTION__, ++counter);
  UpdateBlackout();

  SubMenu *menu = this->current;
  auto &tft = this->tft.getHandle();

  if (this->dirty)
  {
    this->drawTitle(menu);
    this->dirty = false;
  }

  sprite.unloadFont();
  
  sprite.setTextColor(TFT_BLACK, TFT_WHITE);
  sprite.fillSprite(TFT_WHITE);

  sprite.setTextWrap(false, false);
  sprite.loadFont(myopeniconic228);
  sprite.setTextDatum(TC_DATUM);

  if (menu->direction == SubMenu::Direction::HORIZONTAL)
  {
    int16_t entryCurrent = ((int16_t)this->locationCurrent) / 42;
    // Serial.printf("this->locationCurrent=%d, entryCurrent=%d\n", this->locationCurrent, entryCurrent);
    for (int16_t entryIndex = std::max(0, entryCurrent - 3), entryEnd = std::min((uint8_t)this->current->entries.size() - 1, entryCurrent + 2) + 1; entryIndex < entryEnd; ++entryIndex)
    {
      int16_t locationEntry = (entryIndex * 42) - (int16_t)this->locationCurrent + (sprite.width() / 2) - 21;
      // Serial.printf("%c @ locationEntry=%d\n", this->current->entries[entryIndex]->glyph, locationEntry);

      
      //this->u8g2.setFont(this->current->entries[entryIndex]->font);
      sprite.setCursor(locationEntry, 0);
      
      sprite.drawGlyph(this->current->entries[entryIndex]->glyph);
      //this->u8g2.drawGlyph(64 + locationEntry, 0, this->current->entries[entryIndex]->glyph);    

      if (entryIndex == this->index)
      {
        //this->u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);

        sprite.setTextDatum(TC_DATUM);
        sprite.setCursor(locationEntry, 30);
        sprite.drawGlyph(this->current->entries[entryIndex]->isSubMenu() ? 0x0035 : 0x0038);
        // this->u8g2.drawGlyph(64 + locationEntry + (32-8) / 2, 16 + 32 + 2 + 8 + 6 , this->current->entries[entryIndex]->isSubMenu() ? 79 : 83);    
      }
    }
    
    // sprite.pushSprite(35, 85, 0, 0, 170, 60);

    int16_t dist = std::abs((int16_t)this->locationCurrent - ((int16_t)this->index * 42));
    // Serial.printf("this->locationCurrent=%d, expected=%d (index=%u)\n", this->locationCurrent, ((int16_t)this->index * 42), this->index);
    if (dist < 8)
    {
        const char *label = this->current->entries[this->index]->label.c_str();
        // tft.loadFont(GrundschriftRegular20);
        // // tft.setTextColor(TFT_BLACK);
        // uint8_t color = (30 * dist);
        // tft.setTextColor(tft.color565(color, color, color));
        // tft.setTextDatum(TC_DATUM);
        // tft.drawString(label, tft.width() / 2, 160);        
        sprite.loadFont(GrundschriftRegular20);
        // tft.setTextColor(TFT_BLACK);
        uint8_t color = (30 * dist);
        sprite.setTextColor(tft.color565(color, color, color));
        sprite.setTextDatum(TC_DATUM);
        sprite.drawString(label, sprite.width() / 2, 75);        
    }
    else
    {
        // tft.fillRect(35 + 2, 155, 170 - 4, 46, TFT_WHITE);        
        sprite.fillRect(2, 70, 170 - 4, 46, TFT_WHITE);        
    }
  }
  else
  {
/*/    
    // Vertival...

    // title
    // this->drawTitle(menu);
//    this->u8g2.drawBox(0, 0, 128, 10);
//    this->u8g2.setDrawColor(0);
//    this->u8g2.setFont(u8g2_font_5x7_tf);
//    const char *label = menu->label.c_str();
//    int strWidth = this->u8g2.getStrWidth(label);
//    this->u8g2.drawUTF8(64 - ((strWidth + 16)/ 2) + 16, 9, label);
//    this->u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
//    this->u8g2.drawGlyph(64 - ((strWidth + 16)/ 2), 9, menu->glyph);
//    this->u8g2.setDrawColor(1);
//    
//    // frame & clip...
//    this->u8g2.drawFrame(0, 10, 128, 53);   
    this->u8g2.setClipWindow(1, 11, 126, 62);
    
    this->u8g2.setFont(u8g2_font_6x12_tf);

    int16_t entryCurrent = this->locationCurrent / 15;
    // Serial.printf("entryCurrent=%d\n", entryCurrent);
    for (int16_t entryIndex = std::max(0, entryCurrent - 2), entryEnd = std::min((uint8_t)this->current->entries.size() - 1, entryCurrent + 2) + 1; entryIndex < entryEnd; ++entryIndex)
    {
      int16_t locationEntry = (entryIndex * 15) - (int16_t)this->locationCurrent;

      if (Value *value = static_cast<Value*>(this->current->entries[entryIndex]))
      {
        this->u8g2.drawUTF8(14, 12 + 25 + locationEntry, value->label.c_str());    
        this->u8g2.drawGlyph(56, 12 + 25 + locationEntry, ':');
        this->u8g2.setFont(value->getValueFont());
        this->u8g2.drawUTF8(64, 12 + 25 + locationEntry, value->getValue(this, value));    
        if (entryIndex == this->index)
        {
          this->u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
          // Serial.printf("%s: glyph=%d\n", __PRETTY_FUNCTION__, value->glyph);
          this->u8g2.drawGlyph(3, 12 + 25 + locationEntry, value->glyph != 0 ? value->glyph : (uint16_t)78);
          this->u8g2.setFont(u8g2_font_6x12_tf);  
          const char *val = value->getValue(this, value);
          int strWidth = this->u8g2.getStrWidth(val);
          if (strWidth > 68)
          {
            int dist = (strWidth - 60) * (sin((float)(millis() % (3141)) / 500.) + 1) / 2;
            this->u8g2.setClipWindow(64, 12 + 26 + locationEntry - 10, 126, 12 + 28 + locationEntry);
            this->u8g2.setDrawColor(0);
            this->u8g2.drawBox(64, 12 + 25 + locationEntry - 10, 126, 12 + 25 + locationEntry);
            this->u8g2.setDrawColor(1);
            this->u8g2.drawUTF8(64 - dist, 12 + 25 + locationEntry, val);    
            this->u8g2.setClipWindow(1, 12, 126, 62);
          }
        }
      }
    }
    */
    
//    for (int i = 0; i < 5; ++i)
//    {
//      if (i == 3)
//        this->u8g2.drawGlyph(3, 10 + i * 15, '>');
//      if (i == 3)
//        this->u8g2.drawUTF8(8, 10 + i * 15, "IP");
//      else
//        this->u8g2.drawUTF8(8, 10 + i * 15, "Testit");
//      this->u8g2.drawGlyph(50, 10 + i * 15, ':');
//      if (i == 3)
//        this->u8g2.drawUTF8(58, 10 + i * 15, "255.255.255.255");
//      else
//       this->u8g2.drawUTF8(58, 10 + i * 15, "<value>");
//    }
    
    // this->u8g2.setMaxClipWindow();
    
  }
  // Serial.println("sendBuffer\n");
  // this->u8g2.sendBuffer();   
  sprite.pushSprite(35, 85, 0, 0, 170, 111);


  this->dirty = false; 
}

void Menu::loop()
{
  if (!this->editors.empty())
  {
    if (!this->editors.back()->loop())
    {
      delete this->editors.back();
      this->editors.pop_back();
      if (!this->editors.empty())
      {
        // Serial.printf("%s: calling setup on current editor (editors.back())...\n", __PRETTY_FUNCTION__);
        this->editors.back()->setup();
      }
      else
        this->draw();
    }
    return;
  }
  this->rotary.loop();
  if (this != CurrentInstance || !this->editors.empty())
    return;
  
  if (this->rotary.encoderChanged() || this->index != this->rotary.readEncoder())
  {
    // Serial.printf("%s@%p: this->index %u -> %lu\n", __PRETTY_FUNCTION__, this, this->index, this->rotary.readEncoder());
    this->index = this->rotary.readEncoder();
  }
  // else
  // {
  //   if (Blackout < this->rotary.lastMovement())
  //   {
  //     if (Blackout == 0)
  //     {
  //       UpdateBlackout();
  //       this->dirty = true;
  //     }
  //   }
  // }

  int16_t locationExpected {(int16_t)(this->current->direction == SubMenu::Direction::HORIZONTAL ? (int16_t)this->index * 42 : (int16_t)this->index * 15)};
  if (int16_t dist = this->locationCurrent - locationExpected)
  {
    if (dist == 1 || dist == -1)
      this->locationCurrent = locationExpected;
    else
    {
      float step = ((float)dist / 8.);
      step = step < 0 ? floor(step) : ceil(step);
      // Serial.printf("%s: dist/8=%f step=%d\n", __PRETTY_FUNCTION__, ((float)dist / 8.), step);

      this->locationCurrent -= step;
    }
    this->draw();
  }
  else
  {
    if (this->dirty || this->current->direction == SubMenu::Direction::VERTICAL)
    {
      this->draw();
    }
  }
  
  // Serial.printf("Blackout=%d (millis() - Blackout)=%d BlackoutTimeout=%d\n", Blackout, (millis() - Blackout), BlackoutTimeout);
  if (Blackout != 0 && BlackoutTimeout > 0 && ((millis() - Blackout) > BlackoutTimeout))
  {
    Blackout = 0;
/*
    this->u8g2.setMaxClipWindow();
    this->u8g2.clearBuffer();
    this->u8g2.sendBuffer();
*/    
  }
}

void Menu::UpdateBlackout(int seconds)
{
  Blackout = millis();
  BlackoutTimeout = seconds < 0 ? BlackoutTimeoutDefault : (seconds * 1000);
}

void Menu::Selected(Button2&)
{
  // Serial.println(__PRETTY_FUNCTION__);
  if (Blackout == 0)
  {
    UpdateBlackout();
    if (CurrentInstance)
      CurrentInstance->dirty = true;
  }
  else
  if (CurrentInstance)
    CurrentInstance->selected();
}

void Menu::Back(Button2&)
{
  if (Blackout == 0)
  {
    UpdateBlackout();
    if (CurrentInstance)
      CurrentInstance->dirty = true;
  }
  else
  if (CurrentInstance)
    CurrentInstance->back();
}

void Menu::Screenshot(Button2&)
{
/*
  if (CurrentInstance)
    CurrentInstance->u8g2.writeBufferXBM(Serial);
*/
}

std::pair<int, uint16_t*> Menu::getTitleGlyphs()
{
  static uint16_t builtinTitleGlyph = 0;
  builtinTitleGlyph = WiFi.status() == WL_CONNECTED ? 247 : 121;
  return {1, &builtinTitleGlyph};  
}

bool Menu::registerIO()
{
  CurrentInstance = this;
  // Serial.printf("%s@%p: this->index=%d this->current=%p #editors=%d\n", __PRETTY_FUNCTION__, this, this->index, this->current, this->editors.size());
  this->rotary.disableAcceleration();
  if (this->current)
    this->rotary.setBoundaries(0, this->current->entries.size() - 1); 
  else
    this->rotary.setBoundaries(); 
  this->rotary.setEncoderValue(this->index);  
  Serial.printf("%s: Setup click handler...\n", __PRETTY_FUNCTION__);
  this->rotary.setClickHandler(&Menu::Selected);
  this->rotary.setDoubleClickHandler(&Menu::Back);
  this->rotary.setLongClickHandler(&Menu::Screenshot);
  
  this->dirty = true;
  if (this->current && this->editors.empty())
    this->draw();

  return true;
}

bool Menu::unregisterIO()
{
/*
  Serial.printf("%s@%p\n", __PRETTY_FUNCTION__, this);
  this->u8g2.setMaxClipWindow();
  this->u8g2.clearBuffer();
  this->u8g2.sendBuffer();    
*/

  this->rotary.setBoundaries();
  this->rotary.setEncoderValue(0);
  
  this->rotary.setClickHandler(nullptr);
  this->rotary.setDoubleClickHandler(nullptr);

  return true;
}

Menu *Menu::CurrentInstance = nullptr;
unsigned long Menu::Blackout = millis();
unsigned long Menu::BlackoutTimeout = 10 * 1000;
unsigned long Menu::BlackoutTimeoutDefault = 10 * 1000;