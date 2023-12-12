
#include "Mp3Player_VolumeControl.hpp"


Mp3Player::VolumeControl::VolumeControl(Menu *menu, std::string label, Mp3Player *client, long max, long min)
  : IntegralEditor {menu, std::move(label), min, max, client->getVolume()},
    client {client}
{

}

// bool Mp3Player::VolumeControl::setup()
// {
//  this->current = this->client->getVolume();
//  return this->IntegralEditor::setup();
// }

bool Mp3Player::VolumeControl::loop()
{
  if (!this->IntegralEditor::loop())
  {
    Serial.printf("%s: IntegralEditor::loop() failed...\n", __PRETTY_FUNCTION__);
    return false;
  }

  // Serial.printf("%s: this->client->getVolume()=%d this->current=%d\n", __PRETTY_FUNCTION__, this->client->getVolume(), this->current);
  if (this->client->getVolume() != this->current)
  {
    Serial.printf("%s: this->client->setVolume(%d)\n", __PRETTY_FUNCTION__, this->current);
    this->client->setVolume(this->current);
    this->client->loop();
    delay(10);
    this->current = this->client->getVolume();
    Serial.printf("%s: this->client->getVolume() = %d\n", __PRETTY_FUNCTION__, this->current);
  }
  return true;
}

void Mp3Player::VolumeControl::Spawn(Menu *menu, std::string label, Mp3Player *client, long max, long min)
{
  menu->editors.push_back(new VolumeControl(menu, std::move(label), client, max, min));
  menu->editors.back()->setup();
}
