
#include "Mp3Player.hpp"

#include "Mp3Player_ControlEntry.hpp"


Mp3Player::ControlEntry::ControlEntry(Menu *menu, Mp3Player *client)
  : parent {parent},
    client {client}
{
  assert(this->client);
}


uint16_t Mp3Player::ControlEntry::getGlyph() const
{
  return (uint16_t)this->action();
}

const std::string &Mp3Player::ControlEntry::getLabel() const
{
  switch (this->action())
  {
    case Action::Pause:
    {
      static const std::string string {"Pause"};
      return string;
    }
    case Action::Play:
    {
      static const std::string string {"Play"};
      return string;
    }
    case Action::PlayAll:
    {
      static const std::string string {"Alle"};
      return string;
    }
    case Action::Stop:
    {
      static const std::string string {"Stop"};
      return string;
    }
    default:
    {
      static const std::string string {":("};
      return string;
    }
  }
}

void Mp3Player::ControlEntry::call(Menu *menu)
{
  Action action = this->action();
  Serial.printf("%s: action=%lu\n", __PRETTY_FUNCTION__, (uint16_t)action);
  switch (action)
  {
    case Action::Pause:
      this->client->pause();
      break;
    case Action::Play:
      this->client->start();
      break;
    case Action::PlayAll:
      this->client->runState(Mp3Player::State {});
      break;
    default:
      Serial.printf("%s: ignored action=%lu\n", __PRETTY_FUNCTION__, (uint16_t)action);
      break;
  }
  menu->draw();
}

Mp3Player::ControlEntry::Action Mp3Player::ControlEntry::action() const
{
  Action action = Action::PlayAll;
  const Mp3Player::State &state = this->client->getCurrentState();
  if (state.isPlaying())
    action = Action::Pause;
  else
  if (state.hasFolder() || state.hasTrack())
    action = Action::Play;
  return action;
}