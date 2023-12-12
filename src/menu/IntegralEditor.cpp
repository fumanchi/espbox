#include "IntegralEditor.hpp"

#include "TFT.hpp"

#include "GrundschriftRegular20.h"

// #include "TFT_eSPI.h"

IntegralEditor *IntegralEditor::CurrentIntegralEditor = nullptr;

IntegralEditor::IntegralEditor(Menu *menu, Menu::Value *value, long min, long max)
  : Editor {menu, value->label},
    value {value},
    min {min},
    max {max},    
    current {(min + max) / 2},
    
    running {false}
{

}

IntegralEditor::IntegralEditor(Menu *menu, std::string label, long min, long max)
  : IntegralEditor {menu, std::move(label), min, max, (min + max) / 2}
{
    
}

IntegralEditor::IntegralEditor(Menu *menu, std::string label, long min, long max, long current)
  : Editor {menu, std::move(label)},
    value {nullptr},
    min {min},
    max {max},    
    current {current},
    
    running {false}
{
  Serial.printf("%s@%u: current=%ld\n", __PRETTY_FUNCTION__, __LINE__, this->current);
}

IntegralEditor::~IntegralEditor()
{
  if (IntegralEditor::CurrentIntegralEditor == this)
  {
    this->menuIO.rotary.getButton().setClickHandler(nullptr);
    this->menuIO.rotary.getButton().setDoubleClickHandler(nullptr); 
    this->menuIO.rotary.getButton().setLongClickTime(200);
    IntegralEditor::CurrentIntegralEditor = nullptr;
  }
}

 
bool IntegralEditor::setup()
{
  IntegralEditor::CurrentIntegralEditor = this;
  
  this->menuIO.rotary.setAcceleration(500);
  this->menuIO.rotary.setBoundaries(this->min, this->max);
  if (this->value)
    this->current = (int64_t)(String {this->value->getValue(this->menu, this->value)}.toDouble());
  this->menuIO.rotary.setEncoderValue(this->current);  

  this->menuIO.rotary.getButton().setClickHandler(&IntegralEditor::Click);
  this->menuIO.rotary.getButton().setDoubleClickHandler(&IntegralEditor::Exit);
  
  this->drawTitle();
  this->draw();      
  this->running = true;
  return true;
}

bool IntegralEditor::loop()
{
  if (!this->running)
    return false;
      
  this->menuIO.rotary.getButton().loop();
  if (this->menuIO.rotary.encoderChanged())
  {
    this->current = this->menuIO.rotary.readEncoder();
    this->draw();      
  }
  return true;
}

void IntegralEditor::Spawn(Menu *menu, Menu::Value *value, long min, long max)
{
  menu->editors.push_back(new IntegralEditor(menu, value, min, max));
  menu->editors.back()->setup();
}

void IntegralEditor::exit(Result result)
{
  this->result = result;
  if (this->result == Result::Accept)
  {
    if (this->value)
    if (Menu::StringValue *stringValue = static_cast<Menu::StringValue*>(this->value))
      stringValue->setValue(this->menu, stringValue, String(this->current).c_str());
  }
  this->running = false;
}
    
void IntegralEditor::drawTitle()
{
  if (this->value)
    this->menu->drawTitle(this->value);
  else
    this->menu->drawTitle(this->label.c_str());

  TFT_eSPI &tft = this->menu->tft.getHandle();
  tft.unloadFont();
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextDatum(TC_DATUM);
  tft.loadFont(GrundschriftRegular20);
}

void IntegralEditor::draw()
{
  TFT_eSPI &tft = this->menu->tft.getHandle();

  tft.fillRect(55, 85, 130, 40, TFT_WHITE);  
  tft.drawString(String{this->current}, 120, 90);
/*  
  this->menuIO.u8g2.setDrawColor(0); 
  this->menuIO.u8g2.drawBox(4, 25, 120, 20);
  this->menuIO.u8g2.setDrawColor(1);    
    
  this->menuIO.u8g2.setFont(u8g2_font_10x20_tn);
  String string(this->current);
  int strWidth = menuIO.u8g2.getStrWidth(string.c_str());
  this->menuIO.u8g2.drawStr(64 - strWidth / 2, 43, string.c_str());
  menuIO.u8g2.sendBuffer();  
*/
}

void IntegralEditor::Click(Button2&) 
{
  Serial.println(__PRETTY_FUNCTION__);
  if (IntegralEditor::CurrentIntegralEditor)
    IntegralEditor::CurrentIntegralEditor->exit(Result::Accept);
}

void IntegralEditor::Exit(Button2&) 
{
  Serial.println(__PRETTY_FUNCTION__);
  if (IntegralEditor::CurrentIntegralEditor)
    IntegralEditor::CurrentIntegralEditor->exit(Result::Cancel);
}
