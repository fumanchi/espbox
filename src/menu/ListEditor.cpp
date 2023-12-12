#include "TFT.hpp"

#include "myopeniconic220.h"
#include "GrundschriftRegular20.h"

#include "ListEditor.hpp"


ListEditor *ListEditor::CurrentListEditor = nullptr;


ListEditor::ListEditor(Menu *menu, Menu::Value *value, bool refresh)
  : Editor {menu, value ? value->label : ""},
    value {value},
    refresh {refresh},
    currentItem {-1},
    currentEntryIndex {0},
    
    running {false}
{
  // Serial.println(__PRETTY_FUNCTION__);
}

ListEditor::ListEditor(Menu *menu, std::string label, bool refresh)
  : Editor {menu, std::move(label)},
    value {nullptr},
    refresh {refresh},
    currentItem {-1},
    currentEntryIndex {0},
    
    running {false}
{
  // Serial.println(__PRETTY_FUNCTION__);
}

ListEditor::~ListEditor()
{
  if (ListEditor::CurrentListEditor == this)
  {
    this->menuIO.rotary.disableAcceleration();
    this->menuIO.rotary.setClickHandler(nullptr);
    this->menuIO.rotary.setDoubleClickHandler(nullptr); 
    this->menuIO.rotary.setLongClickTime(200);
    ListEditor::CurrentListEditor = nullptr;
  }
}

 
bool ListEditor::setup()
{
  ListEditor::CurrentListEditor = this;

  this->menuIO.rotary.disableAcceleration();
  this->menuIO.rotary.setClickHandler(ListEditor::Select);
  this->menuIO.rotary.setDoubleClickHandler(ListEditor::Exit); 
  this->update();  
  this->currentLocation = this->currentEntryIndex * 22;
  this->drawTitle();
  this->draw();      
  this->running = true;
 
  return true;
}

bool ListEditor::update(int index)
{
  bool retval = true;
  int numberOfItems = this->getNumberOfItems();
  this->menuIO.rotary.setBoundaries(0, numberOfItems - 1 + this->refresh);
  this->currentEntryIndex = index;
  if (this->value != nullptr)
  {
    const char *currentValue = this->value->getValue(this->menu, this->value);
    int currentValueLen = strlen(currentValue);
    Serial.printf("%s: numberOfItems=%d currentValue=%s\n", __PRETTY_FUNCTION__, numberOfItems, currentValue);
    for (int itemIndex = 0, end = numberOfItems; itemIndex < end && this->currentEntryIndex < 0; ++itemIndex)
    {
      const char *itemValue = this->getItem(itemIndex, Role::Value);
      // Serial.printf("%s: check for index %d (comparing itemValue=%s with currentValue=%s)\n", __PRETTY_FUNCTION__, itemIndex, itemValue, currentValue);
      if (strncmp(currentValue, itemValue, currentValueLen) == 0)
      {
        // Serial.printf("%s@%d\n", __PRETTY_FUNCTION__, __LINE__);
        if (itemValue[currentValueLen] == 0 || itemValue[currentValueLen] == ' ')
        {
          this->currentEntryIndex = itemIndex + this->refresh;
          // Serial.printf("%s@%d\n", __PRETTY_FUNCTION__, __LINE__);
        }
      }
    }
    this->currentItem = this->currentEntryIndex - this->refresh;
    if (this->currentEntryIndex < 0)
      this->currentEntryIndex = 0;
    this->menuIO.rotary.setEncoderValue(this->currentEntryIndex);
  }

  if (this->currentEntryIndex < 0)
    this->currentEntryIndex = 0;

  this->menuIO.rotary.setEncoderValue(this->currentEntryIndex);
  
  Serial.printf("%s: currentEntryIndex=%d\n", __PRETTY_FUNCTION__, this->currentEntryIndex);

  return retval;
}

bool ListEditor::loop()
{
  if (!this->running)
    return false;
      
  this->menuIO.rotary.loop();
  if (!this->running)
    return true;
  if (this->menuIO.rotary.encoderChanged())
    this->currentEntryIndex = this->menuIO.rotary.readEncoder();

  int16_t expectedLocation = this->currentEntryIndex * 22;
  if (int16_t dist = this->currentLocation - expectedLocation)
  {
    if (dist == 1 || dist == -1)
      this->currentLocation = expectedLocation;
    else
    {
      float step = ((float)dist / 8.);
      step = step < 0 ? floor(step) : ceil(step);

      this->currentLocation -= step;
    }
    //Serial.printf("this->currentLocation=%d\n", this->currentLocation);
    this->draw();
  }
  return true;
}

void ListEditor::select()
{
  Serial.printf("%s: refresh=%d currentEntryIndex=%d\n", __PRETTY_FUNCTION__, this->refresh, this->currentEntryIndex);
  if (this->refresh && this->currentEntryIndex == 0)
    this->update();
  else
  {
    this->currentItem = this->currentEntryIndex - this->refresh;
    Serial.printf("%s: Set currentitem to %d\n", __PRETTY_FUNCTION__, this->currentItem);
    if (!this->value)
    {
      this->exit(Result::Accept);
      return;
    }
  }
  this->draw();
}

void ListEditor::exit(Result result)
{
  this->result = result;
  if (this->result == Result::Accept)
  {
    // Serial.printf("%s: Accept...\n", __PRETTY_FUNCTION__);
    if (Menu::StringValue *stringValue = static_cast<Menu::StringValue*>(this->value))
    {
      stringValue->setValue(this->menu, this->value, this->getItem(this->currentEntryIndex - this->refresh, Role::Value));
    }
  }
  this->running = false;
}
    
void ListEditor::drawTitle()
{
  if (this->value)
    this->menu->drawTitle(this->value);
  else
    this->menu->drawTitle(this->label.c_str(), 0);  
}

void ListEditor::draw()
{
  auto &tft = this->menuIO.tft.getHandle();
  auto &sprite = this->menu->sprite;

  // tft.unloadFont();
  // tft.loadFont(GrundschriftRegular20);
  // tft.setTextColor(TFT_BLACK, TFT_WHITE);

  // tft.fillRect(35, 68, 170, 116, TFT_WHITE);
  // tft.setViewport(35, 68, 170, 116, true);
  
  // tft.setTextDatum(TL_DATUM); 

  sprite.unloadFont();
  sprite.loadFont(GrundschriftRegular20);
  sprite.setTextColor(TFT_BLACK, TFT_WHITE);

  sprite.fillSprite(TFT_WHITE);
  // sprite.setViewport(0, 0, 170, 116, true);
  
  // tft.setTextDatum(TL_DATUM); 
  sprite.setTextDatum(TL_DATUM); 

  int numberOfItems = this->getNumberOfItems();
  for (int16_t currentEntry = this->currentLocation / 22, entryIndex = std::max(0, currentEntry - 3), entryEnd = std::min(numberOfItems + this->refresh - 1, currentEntry + 3) + 1; entryIndex < entryEnd; ++entryIndex)
  {
    int16_t locationEntry = (entryIndex * 22) - (int16_t)this->currentLocation + (116 / 2);
    const char *label = refresh && entryIndex == 0 ? nullptr : this->getItem(entryIndex - this->refresh, Role::Label);
    if ((!this->refresh || entryIndex > 0) && ((entryIndex - refresh) == this->currentItem) && label)
    {
      // int labelWidth = tft.textWidth(label);
      int labelWidth = sprite.textWidth(label);
      // tft.drawRect(45, locationEntry, labelWidth + 4, 23, TFT_GREEN);
      sprite.drawRect(24, locationEntry - 1, labelWidth + 6, 25, TFT_GREEN);
    }
    // tft.drawString((entryIndex == 0 && this->refresh) ? "<refresh>" : (label ? label : "<>"), 47, locationEntry);
    sprite.drawString((entryIndex == 0 && this->refresh) ? "<refresh>" : (label ? label : "<>"), 27, locationEntry);
    // draw glyph for current value...
    if (entryIndex == this->currentEntryIndex)
    {
      uint16_t glyph = 0x0037;
      if (const char *glyphS = this->getItem(entryIndex - this->refresh, Role::Glyph))
        memcpy(&glyph, glyphS, 2);
        // glyph = ((uint16_t)glyphS[0]) << 8 + (uint16_t)glyphS[1];
      // tft.unloadFont();
      // tft.loadFont(myopeniconic220);
      // tft.setCursor(37, locationEntry);
      // tft.drawGlyph(0x0037);
      // tft.unloadFont();
      // tft.loadFont(GrundschriftRegular20);
      sprite.unloadFont();
      sprite.loadFont(myopeniconic220);
      sprite.setCursor(8, locationEntry);
      sprite.drawGlyph(glyph);
      sprite.unloadFont();
      sprite.loadFont(GrundschriftRegular20);
    }
    
  }

  // int numberOfItems = this->getNumberOfItems();
  // for (int16_t currentEntry = this->currentLocation / 15, entryIndex = std::max(0, currentEntry - 2), entryEnd = std::min(numberOfItems + this->refresh - 1, currentEntry + 2) + 1; entryIndex < entryEnd; ++entryIndex)
  // {
  //   int16_t locationEntry = (entryIndex * 15) - (int16_t)this->currentLocation;
  //   const char *label = refresh && entryIndex == 0 ? nullptr : this->getItem(entryIndex - this->refresh, Role::Label);
  //   if ((!this->refresh || entryIndex > 0) &&  entryIndex - refresh == this->currentItem && label)
  //   {
  //     int labelWidth = this->menuIO.u8g2.getStrWidth(label);
  //     this->menuIO.u8g2.drawFrame(10, 12 + 25 + locationEntry - 8 - 2, labelWidth + 4, 14);
  //   }
  //   this->menuIO.u8g2.drawUTF8(12, 12 + 25 + locationEntry, (entryIndex == 0 && this->refresh) ? "<refresh>" : (label ? label : "<>"));    
  //   if (entryIndex == this->currentEntryIndex)
  //   {
  //     this->menuIO.u8g2.setFont(u8g2_font_6x12_tf);
  //     this->menuIO.u8g2.drawGlyph(3, 12 + 25 + locationEntry, 183);
  //   }
  // }

  // tft.resetViewport();
  tft.drawLine(35, 71, 205, 71, TFT_BLACK);
  sprite.pushSprite(35, 73, 0, 0, 170, 116);
  tft.drawLine(35, 190, 205, 190, TFT_BLACK);
/*
  // Serial.printf("%s: currentEntryIndex=%d\n", __PRETTY_FUNCTION__, this->currentEntryIndex);
  this->menuIO.u8g2.setClipWindow(1, 11, 127, 62);  

  this->menuIO.u8g2.setDrawColor(0);  
  this->menuIO.u8g2.drawBox(1, 11, 126, 51);
  this->menuIO.u8g2.setDrawColor(1);
  this->menuIO.u8g2.setFont(u8g2_font_6x12_tf);

  int numberOfItems = this->getNumberOfItems();
  for (int16_t currentEntry = this->currentLocation / 15, entryIndex = std::max(0, currentEntry - 2), entryEnd = std::min(numberOfItems + this->refresh - 1, currentEntry + 2) + 1; entryIndex < entryEnd; ++entryIndex)
  {
    int16_t locationEntry = (entryIndex * 15) - (int16_t)this->currentLocation;
    const char *label = refresh && entryIndex == 0 ? nullptr : this->getItem(entryIndex - this->refresh, Role::Label);
    if ((!this->refresh || entryIndex > 0) &&  entryIndex - refresh == this->currentItem && label)
    {
      int labelWidth = this->menuIO.u8g2.getStrWidth(label);
      this->menuIO.u8g2.drawFrame(10, 12 + 25 + locationEntry - 8 - 2, labelWidth + 4, 14);
    }
    this->menuIO.u8g2.drawUTF8(12, 12 + 25 + locationEntry, (entryIndex == 0 && this->refresh) ? "<refresh>" : (label ? label : "<>"));    
    if (entryIndex == this->currentEntryIndex)
    {
      this->menuIO.u8g2.setFont(u8g2_font_6x12_tf);
      this->menuIO.u8g2.drawGlyph(3, 12 + 25 + locationEntry, 183);
    }
  }
  this->menuIO.u8g2.sendBuffer();  
*/
}

void ListEditor::Select(Button2&) 
{
  if (ListEditor::CurrentListEditor)
    ListEditor::CurrentListEditor->select();
};

void ListEditor::Exit(Button2&) 
{
  if (ListEditor::CurrentListEditor)
    ListEditor::CurrentListEditor->exit();
};
