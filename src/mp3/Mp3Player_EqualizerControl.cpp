
#include "Mp3Player.hpp"

#include "Mp3Player_EqualizerControl.hpp"


Mp3Player::EqualizerControl::EqualizerControl(Menu *menu, std::string label, Mp3Player *client)
  : //ListEditor {menu, label, true},
    ListEditor {menu, label, false},
    client {client},
    currentValue {-1}
{  
}

int Mp3Player::EqualizerControl::getNumberOfItems() const
{
  return (int)DfMp3_Eq_Bass + 1;
}
  
const char *Mp3Player::EqualizerControl::getItem(int index, Role role) const
{
  static char stmp[10];

  if (role == Role::Glyph)
    return nullptr;
    
  if (role == Role::Label)
  {
    switch (index)
    {
      case DfMp3_Eq_Normal:
        strncpy(stmp, "normal", 9);        
        break;
      case DfMp3_Eq_Pop:
        strncpy(stmp, "pop", 9);        
        break;
      case DfMp3_Eq_Rock:
        strncpy(stmp, "rock", 9);        
        break;
      case DfMp3_Eq_Jazz:
        strncpy(stmp, "jazz", 9);        
        break;
      case DfMp3_Eq_Classic:
        strncpy(stmp, "classic", 9);        
        break;
      case DfMp3_Eq_Bass:
        strncpy(stmp, "bass", 9);        
        break;
    }
  }
  else
  {
    static char stmp[4];
    snprintf(stmp, 3, "%d", index);
  }
  return stmp;
}

bool Mp3Player::EqualizerControl::setup()
{
  this->currentItem = (int)this->client->getEq();
  this->currentEntryIndex = (int)this->client->getEq();
  return this->ListEditor::setup() && this->ListEditor::update(this->client->getEq());
}

bool Mp3Player::EqualizerControl::loop()
{
  // if (this->dirty)
  // {
  //   this->draw();
  //   this->dirty = false;
  // }
  return this->ListEditor::loop();
}

void Mp3Player::EqualizerControl::select()
{
  Serial.printf("%s: refresh=%d currentEntryIndex=%d\n", __PRETTY_FUNCTION__, this->refresh, this->currentEntryIndex);
  if (this->refresh && this->currentEntryIndex == 0)
    this->update();
  else
  {
    this->currentItem = this->currentEntryIndex - this->refresh;
    Serial.printf("%s: currentIndex=%d => this->currentItem=%d\n", __PRETTY_FUNCTION__, this->currentEntryIndex, this->currentItem);
    this->client->setEq((DfMp3_Eq)this->currentItem);
  }
  this->draw();
}

// bool Mp3Player::EqualizerControl::update(int index)
// {
//   Serial.printf("%s: index=%d\n", __PRETTY_FUNCTION__, index);
//   this->currentItem = (int)this->client->getEq();
//   return this->ListEditor::update(index);
// }

// void Mp3Player::EqualizerControl::exit(Result result)
// {
//   //
// }

// bool Mp3Player::EqualizerControl::loop()
// {
//   if (!this->IntegralEditor::loop())
//   {
//     Serial.printf("%s: IntegralEditor::loop() failed...\n", __PRETTY_FUNCTION__);
//     return false;
//   }

//   // Serial.printf("%s: this->client->getEqualizer()=%d this->current=%d\n", __PRETTY_FUNCTION__, this->client->getEqualizer(), this->current);
//   if (this->client->getEqualizer() != this->current)
//   {
//     this->client->setEqualizer(this->current);
//     this->current = this->client->getEqualizer();
//   }
//   return true;
// }

void Mp3Player::EqualizerControl::Spawn(Menu *menu, std::string label, Mp3Player *client)
{
  menu->editors.push_back(new EqualizerControl(menu, std::move(label), client));
  menu->editors.back()->setup();
}
