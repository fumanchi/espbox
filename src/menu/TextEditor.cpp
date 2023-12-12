#include "TextEditor.hpp"

#include "QuestionSelection.hpp"


TextEditor *TextEditor::CurrentTextEditor = nullptr;


TextEditor::TextEditor(Menu *menu, Menu::StringValue *stringValue)
  : Editor {menu, stringValue->label},
    stringValue {stringValue},
    string {stringValue->getStringValue(menu, stringValue)},
    running {false},
    dirty {Dirty::Title | Dirty::Selector | Dirty::Value},
    result {Result::None},
    index {0},
    locationCurrent {0},
    lastChange {-1u},
    questionResult {0}
{
  Serial.println(__PRETTY_FUNCTION__);
}

TextEditor::~TextEditor()
{
  if (TextEditor::CurrentTextEditor == this)
  {
    this->menuIO.rotary.getButton().setClickHandler(nullptr);
    this->menuIO.rotary.getButton().setDoubleClickHandler(nullptr); 
    this->menuIO.rotary.getButton().setLongClickTime(200);
    TextEditor::CurrentTextEditor = nullptr;
  }
}

 
bool TextEditor::setup()
{
  TextEditor::CurrentTextEditor = this;

  if (this->questionResult != 0)
  {
    Serial.printf("%s: result=%s\n", __PRETTY_FUNCTION__, this->questionResult > 0 ? "YES" : "NO");
    if (this->questionResult == 1)
    {
      this->stringValue->setStringValue(this->menu, this->stringValue, this->string.c_str());
    }
    return true;
  }
   
  Serial.println(__PRETTY_FUNCTION__);
  this->menuIO.rotary.getEncoder().setAcceleration(100);
  this->menuIO.rotary.getEncoder().setBoundaries(33, 126);
  this->menuIO.rotary.getEncoder().setEncoderValue(64);

  this->menuIO.rotary.getButton().setClickHandler(&TextEditor::Click);
  this->menuIO.rotary.getButton().setLongClickDetectedHandler(&TextEditor::Backspace);
  this->menuIO.rotary.getButton().setLongClickDetectedRetriggerable(true);
  this->menuIO.rotary.getButton().setLongClickTime(400);
  this->menuIO.rotary.getButton().setDoubleClickHandler(&TextEditor::Exit);

  this->index = menuIO.rotary.readEncoder();
  this->locationCurrent = 6 + index * (2 + 12);

  this->lastChange = millis();
  this->running = true;

  this->dirty = Dirty::All;
  this->draw();

  return true;
}

bool TextEditor::loop()
{
  if (!this->running)
    return false;
      
  this->menuIO.rotary.getButton().loop();
  if (this->menuIO.rotary.encoderChanged())
  {
    this->index = menuIO.rotary.readEncoder();
    Serial.printf("%s: index=%d\n", __PRETTY_FUNCTION__, index);
  }

  int16_t locationExpected {(int16_t)(6 + index * (2 + 12))};
  if (int16_t dist = this->locationCurrent - locationExpected)
  {
    this->dirty |= Dirty::Selector;

    if (dist == 1 || dist == -1)
      this->locationCurrent = locationExpected;
    else
    {
      float step = ((float)dist / 4.);
      step = step < 0 ? floor(step) : ceil(step);

      this->locationCurrent -= step;
    }
  }   

  this->draw();      

  return true;
}

void TextEditor::Spawn(Menu *menu, Menu::Value *value)
{
  Serial.println(__PRETTY_FUNCTION__);
  menu->editors.push_back(new TextEditor(menu, static_cast<Menu::StringValue*>(value)));
  menu->editors.back()->setup();
}

void TextEditor::click()
{
  char characterCurrent = (locationCurrent - 6) / (2 + 12);
  this->string.concat(characterCurrent);
  this->dirty |= Dirty::Value;
  this->lastChange = millis();
}

void TextEditor::backspace()
{
  Serial.println(__PRETTY_FUNCTION__);
  this->string.remove(this->string.length() - 1);
  this->dirty |= Dirty::Value;
  this->lastChange = millis();
}

void TextEditor::exit()
{
  Serial.printf("%s: About to spawn QuestionSelec...\n", __PRETTY_FUNCTION__);
  Serial.printf("%s: Setting this->menu->editor...\n", __PRETTY_FUNCTION__);
  this->running = false;
  TextEditor::CurrentTextEditor = nullptr;
  this->menu->editors.push_back(new QuestionSelection
    {
      this->menu, 
      {
        QuestionSelection::Entry{1, "Yes"},
        QuestionSelection::Entry{-1, "No"}
      },
      "Accept input?",
      &this->questionResult
    });
  this->menu->editors.back()->setup();
  Serial.printf("%s: done...\n", __PRETTY_FUNCTION__);
}
    
void TextEditor::drawTitle()
{
  /*
  this->menuIO.u8g2.setMaxClipWindow();
  this->menuIO.u8g2.clearBuffer();
  
  // title
  this->menuIO.u8g2.drawBox(0, 0, 128, 10);
  this->menuIO.u8g2.setDrawColor(0);
  this->menuIO.u8g2.setFont(u8g2_font_5x7_tf);
  const char *label = this->stringValue->label.c_str();
  int strWidth = menuIO.u8g2.getStrWidth(label);
  this->menuIO.u8g2.drawStr(64 - ((strWidth + 16)/ 2) + 16, 9, label);
  this->menuIO.u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
  this->menuIO.u8g2.drawGlyph(64 - ((strWidth + 16)/ 2), 9, stringValue->glyph);
  this->menuIO.u8g2.setDrawColor(1);
  
  // frame & clip...
  this->menuIO.u8g2.drawFrame(0, 10, 128, 53);
  this->menuIO.u8g2.setClipWindow(1, 11, 126, 62);  
  */
}

void TextEditor::drawSelector(bool clear)
{
  /*
  int16_t characterCurrent = (locationCurrent - 6) / (2 + 12);

  if (clear)
  {
    this->menuIO.u8g2.setDrawColor(0); 
    this->menuIO.u8g2.drawBox(1, 22-8, 126, 18);
    this->menuIO.u8g2.setDrawColor(1);
  }
    
  this->menuIO.u8g2.setFontMode(1);
  this->menuIO.u8g2.setFont(u8g2_font_6x12_tf);
  for (int16_t character = std::max(33, characterCurrent - 5), characterLast = std::min(126, characterCurrent + 5) + 1; character < characterLast; ++character)
  {
    int16_t locationEntry = (character * (2 + 12)) - locationCurrent;

    this->menuIO.u8g2.drawGlyph(64 + locationEntry, 22, character);    
    if (character == index)
    {
      this->menuIO.u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
      this->menuIO.u8g2.drawGlyph(64 + locationEntry - 1, 22 + 8 , 79); 
      this->menuIO.u8g2.setFont(u8g2_font_6x12_tf);
    }
  }
  */
};

void TextEditor::drawString(bool clear)
{
  /*
  if (clear)
  {
    this->menuIO.u8g2.setDrawColor(0); 
    this->menuIO.u8g2.drawBox(4, 45, 120, 10);
    this->menuIO.u8g2.setDrawColor(1); 
  }
  else
    this->menuIO.u8g2.drawFrame(3, 44, 122, 12);
    
  this->menuIO.u8g2.setClipWindow(4, 46, 125, 46 + 8);  

  this->menuIO.u8g2.setFont(u8g2_font_6x12_tf);
  int strWidth = menuIO.u8g2.getStrWidth(this->string.c_str());
  if (strWidth > 118)
  {
    if ((millis() - this->lastChange) > 4000)
    {
      this->menuIO.u8g2.drawStr(5 + (118 - strWidth) * (sin((float)((millis() - this->lastChange) % (3141)) / 500.) + 1) / 2, 46 + 8, this->string.c_str());
    }
    else
      this->menuIO.u8g2.drawStr(5 + (118 - strWidth), 46 + 8, this->string.c_str());
  }
  else
    this->menuIO.u8g2.drawStr(5, 46 + 8, this->string.c_str());
  */
}

void TextEditor::draw()
{
/*
  if (this->dirty & Dirty::Title)
  {
//    menuIO.u8g2.clearBuffer();
    drawTitle();
    drawSelector(false);
    drawString(false);
  }
  else
  {
    this->menuIO.u8g2.setClipWindow(1, 11, 126, 62);  
    if (this->dirty & Dirty::Selector)
    {
      this->drawSelector(true);
//      menuIO.u8g2.updateDisplayArea(0, 11, 128, 30);
    // menuIO.u8g2.sendWindow(1, 11, 126, 30);
    }
    
    if ((millis() - this->lastChange) > 4000)
      this->dirty |= Dirty::Value;
      
    if (this->dirty & Dirty::Value)
    {
      this->drawString(true);
//      menuIO.u8g2.updateDisplayArea(0, 11, 128, 30);
      // menuIO.u8g2.sendWindow(1, 11, 126, 30);
    }
  }
  if (this->dirty != 0)
    menuIO.u8g2.sendBuffer();  
  this->dirty = 0;
*/
};
  
void TextEditor::Click(Button2&) 
{
  Serial.println(__PRETTY_FUNCTION__);
  if (TextEditor::CurrentTextEditor)
    TextEditor::CurrentTextEditor->click();
};

void TextEditor::Backspace(Button2&) 
{
  Serial.println(__PRETTY_FUNCTION__);
  if (TextEditor::CurrentTextEditor)
    TextEditor::CurrentTextEditor->backspace();
};

void TextEditor::Exit(Button2&) 
{
  Serial.println(__PRETTY_FUNCTION__);
  if (TextEditor::CurrentTextEditor)
    TextEditor::CurrentTextEditor->exit();
};
