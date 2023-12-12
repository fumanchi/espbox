
#include "Mp3Player.hpp"

#include "Mp3Player_TrackSelection.hpp"


Mp3Player::TrackSelection::TrackSelection(Menu *menu, std::string label, Mp3Player *client, uint16_t folder, OnExitCallback onExitCallback)
  : //ListEditor {menu, label, true},
    ListEditor {menu, label, false},
    client {client},
    folder {folder},
    onExitCallback {onExitCallback},

    folderTrackCount {0},

    currentTrack {0},
    currentTrackMillis {0}
{  
  this->currentItem = -1;
}

uint16_t Mp3Player::TrackSelection::getFolder() const
{
  return this->folder;
}

Mp3Player::TrackSelection::Mode Mp3Player::TrackSelection::getCurrentMode() const
{
  Mode retval = Mode::SingleTrack;
  Serial.printf("%s@%d: this->currentItem=%d\n", __PRETTY_FUNCTION__, __LINE__, this->currentItem);
  if (this->currentItem < (int)Mode::SingleTrack)
    retval = (Mode)this->currentItem;
  return retval;
}

uint16_t Mp3Player::TrackSelection::getCurrentTrack() const
{
  uint16_t retval = 0;
  if (this->currentItem >= (int)Mode::SingleTrack)
    retval = this->currentItem - (int)Mode::SingleTrack + 1;
  return retval;
}

int Mp3Player::TrackSelection::getNumberOfItems() const
{
  return (int)Mode::SingleTrack + this->folderTrackCount; // this->client->getFolderTrackCount(this->folder);
}
  
const char *Mp3Player::TrackSelection::getItem(int index, Role role) const
{
  static char stmp[13];

  static auto StoreGlyph = [](uint16_t glyph) 
  {
    // stmp[0] = (glyph >> 8)  & 0xff;
    // stmp[1] = glyph & 0xff;
    memcpy(stmp, &glyph, 2);
    stmp[2] = 0;
  };

  switch (role)
  {
    case Role::Glyph:
    {
      StoreGlyph((index == (int)Mode::Back) ? 0x0036 : 0x0039);
      break;
    }
    case Role::Label:
    {
      switch (index)
      {
        case (int)Mode::Back:
          snprintf(stmp, sizeof(stmp), "<back>");
          break;
        case (int)Mode::Folder:
          snprintf(stmp, sizeof(stmp), "<folder>");
          break;
        case (int)Mode::Random:
          snprintf(stmp, sizeof(stmp), "<random>");
          break;
        default:
        {
          index = index - (int)Mode::SingleTrack;
          snprintf(stmp, sizeof(stmp), "File %0.2d", index + 1);
        }
        break;  
      }
      break;
    }
    case Role::Value:
    {
      snprintf(stmp, sizeof(stmp), "%d", index + 1);
      break;
    }
    default:
      // ignore
      break;
  }

  return stmp;
}

bool Mp3Player::TrackSelection::setup()
{
  this->folderTrackCount = this->client->getFolderTrackCount(this->folder);
  delay(100);

  this->currentTrack = this->currentEntryIndex = 0;  
  this->currentTrackMillis = millis();
  return this->ListEditor::setup();
}

bool Mp3Player::TrackSelection::loop()
{
  // Serial.printf("%s: xPortGetCoreID=%d\n", __PRETTY_FUNCTION__, xPortGetCoreID());

  this->client->loop();
  if (!this->ListEditor::loop())
  {
    Serial.printf("%s: ListEditor::loop() failed ~> done...\n", __PRETTY_FUNCTION__);
    return false;
  }

  int currentEntryIndex = this->currentEntryIndex;

  this->currentItem = currentEntryIndex;

  if (this->currentItem >= (int)Mode::SingleTrack)
  {
    uint8_t track = this->currentItem - (int)Mode::SingleTrack + 1;
    if (this->currentTrack == track) 
    {
      if (this->currentTrackMillis > 0 && (millis() - this->currentTrackMillis) > 1000)
      {
        Serial.printf("Playing folder track: %u, %u\n", this->folder, this->currentTrack);
        this->client->playFolderTrack(this->folder, this->currentTrack);
        for (int i = 0; i < 5 && !this->client->waitUntilBusy(500); ++i)
          this->client->playFolderTrack(this->folder, this->currentTrack);
        this->currentTrackMillis = 0;
      }
    }
    else
    {
      this->client->stop();
      this->currentTrack = track;  
      this->currentTrackMillis = millis();
    }
  }
  else
  {
    if (this->currentTrack != 0)
    {
      this->client->stop();
      this->currentTrack = 0; 
      this->currentTrackMillis = 0;
    }
  }
  return true;
}

void Mp3Player::TrackSelection::select()
{
  // Serial.printf("%s@%d: this->currentEntryIndex=%u this->currentItem=%d\n", __PRETTY_FUNCTION__, __LINE__, this->currentEntryIndex, this->currentItem);
  uint8_t index = this->currentEntryIndex;

  this->currentTrack = 0;  
  this->currentTrackMillis = 0;

  // if (index == 0 && (this->extras & Extra::RandomGlobalTrack))
  // {
  //   // random global track...
  //   // => neither current folder nor current track will be set...    
  // }
  // else
  // {
  //   index -= (this->extras & Extra::RandomGlobalTrack);
  //   if (index < this->client->getNumberOfFolders())
  //   {
  //     // track_selection!!!
  //     Serial.printf("Folder selection!!! folder=%d\n", index + 1);
  //     this->currentTrack = index + 1;
  //   }
  //   else
  //   {
  //     index -= this->client->getNumberOfFolders();
  //     Serial.printf("Track selection!!! track=%d\n", index + 1);
  //     this->currentGlobalTrack = index + 1;
  //   }
  // }

  // Serial.printf("%s: currentTrack=%d currentGlobalTrack=%d\n", __PRETTY_FUNCTION__, currentTrack, currentGlobalTrack);

  // this->client->stop();
  
  this->exit(Result::Accept);
  // Mp3Player::TrackSelection::Spawn()
}

void Mp3Player::TrackSelection::exit(Result result)
{
  if (this->onExitCallback)
    this->running = !this->onExitCallback(this, result);
}

void Mp3Player::TrackSelection::Spawn(Menu *menu, std::string label, Mp3Player *client, uint16_t folder, OnExitCallback onExitCallback)
{
  menu->editors.push_back(new TrackSelection(menu, std::move(label), client, folder, onExitCallback));
  menu->editors.back()->setup();
}
