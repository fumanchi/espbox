
#include "Mp3Player.hpp"

#include "Mp3Player_FolderSelection.hpp"


Mp3Player::FolderSelection::FolderSelection(Menu *menu, std::string label, Mp3Player *client, OnExitCallback onExitCallback, uint8_t extras)
  : //ListEditor {menu, label, true},
    ListEditor {menu, label, false},
    client {client},
    extras {extras},
    onExitCallback {onExitCallback},
    currentFolder {0},
    currentFolderMillis {0},
    currentGlobalTrack {0},
    currentGlobalTrackMillis {0}
{  
  this->currentItem = -1;
  Serial.printf("%s ~> extras=%u\n", __PRETTY_FUNCTION__, (u_int)this->extras);
}

Mp3Player::FolderSelection::Mode Mp3Player::FolderSelection::getCurrentMode() const
{
  Mp3Player::FolderSelection::Mode retval = Mode::RandomGlobalTrack;
  if (this->currentFolder > 0)
    retval = Mode::Folder;
  if (this->currentGlobalTrack > 0)
    retval = Mode::Track;
  return retval;
}

uint16_t Mp3Player::FolderSelection::getCurrentFolder() const
{
  return this->currentFolder;
}

uint16_t Mp3Player::FolderSelection::getCurrentTrack() const
{
  return this->currentGlobalTrack;
}


int Mp3Player::FolderSelection::getNumberOfItems() const
{
  int retval = this->client->getNumberOfFolders();
  if (this->extras & Extra::RandomGlobalTrack)
    ++retval;
  if (this->extras & Extra::LoopGlobalTrack)
    retval += this->client->getTotalTrackCount();
  Serial.printf("%s ~> %d (seconds=%d)\n", __PRETTY_FUNCTION__, retval, millis()/1000);
  return retval;
}
  
const char *Mp3Player::FolderSelection::getItem(int index, Role role) const
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
    case Role::Label:
    {
      if ((this->extras & Extra::RandomGlobalTrack) && index == 0)
      {
        if (role == Role::Glyph)
          StoreGlyph(0x0039);
        else
          snprintf(stmp, 12, "<zufällig>");
      }
      else
      {
        index -= (this->extras & Extra::RandomGlobalTrack);
        if (index < this->client->getNumberOfFolders())
        {
          if (role == Role::Glyph)
            StoreGlyph(0x0037);
          else
          {            
            snprintf(stmp, 12, "Folder %0.2d", index + 1);
          }
        }
        else
        {
          if (role == Role::Glyph)
            StoreGlyph(0x0039);
          else
          {
            index -= this->client->getNumberOfFolders();
            snprintf(stmp, 12, "File %0.2d", index + 1);
          }
        }
      }
      break;
    }
    case Role::Value:
    {
      snprintf(stmp, 12, "%d", index + 1);
      break;
    }
  }

  return stmp;
}

bool Mp3Player::FolderSelection::setup()
{
  this->currentFolder = this->currentEntryIndex = 0;  
  this->currentFolderMillis = millis();
  return this->ListEditor::setup();
}

bool Mp3Player::FolderSelection::loop()
{
  // Serial.printf("%s: xPortGetCoreID=%d\n", __PRETTY_FUNCTION__, xPortGetCoreID());

  this->client->loop();
  if (!this->ListEditor::loop())
  {
    Serial.printf("%s: ListEditor::loop() failed...\n", __PRETTY_FUNCTION__);
    return false;
  }

  int currentEntryIndex = this->currentEntryIndex;
  // Serial.printf("%s@%d: currentEntryIndex=%d\n", __PRETTY_FUNCTION__, __LINE__, currentEntryIndex);
  if (this->extras & Extra::RandomGlobalTrack)
  {
    if (currentEntryIndex == 0)
    {
      if (this->currentFolder || this->currentGlobalTrack)
      {
        this->currentFolder = 0; 
        this->currentFolderMillis = 0;
        this->currentGlobalTrack = 0; 
        this->currentGlobalTrackMillis = 0;
        this->client->stop();
      }
      return true;
    }
    currentEntryIndex -= 1;
  }  

  // Serial.printf("%s@%d: currentEntryIndex=%d\n", __PRETTY_FUNCTION__, __LINE__, currentEntryIndex);
  if (currentEntryIndex < this->client->getNumberOfFolders())
  {
    if (this->currentFolder == currentEntryIndex + 1) 
    {
      if (this->currentFolderMillis > 0 && (millis() - this->currentFolderMillis) > 1000)
      {
        Serial.printf("Playing folder track: %d, 1\n", this->currentFolder);
        this->client->loopFolder(this->currentFolder);
        for (int i = 0; i < 5 && !this->client->waitUntilBusy(500); ++i)
          this->client->loopFolder(this->currentFolder);
        this->currentFolderMillis = 0;
      }
    }
    else
    {
      this->currentFolder = currentEntryIndex + 1;  
      this->currentFolderMillis = millis();
      this->currentGlobalTrack = 0; 
      this->currentGlobalTrackMillis = 0;
      Serial.printf("%s ~> %d (seconds=%d)\n", __PRETTY_FUNCTION__, __LINE__, millis()/1000);
      this->client->stop();
    }
    return true;
  }
  // currentEntryIndex = currentEntryIndex - this->client->getNumberOfFolders() + 1;
  Serial.printf("%s@%d: currentEntryIndex=%d\n", __PRETTY_FUNCTION__, __LINE__, currentEntryIndex);
  if (this->currentGlobalTrack == currentEntryIndex) 
  {
    if (this->currentGlobalTrackMillis > 0 && (millis() - this->currentGlobalTrackMillis) > 500)
    {
      Serial.printf("Playing gloabl track: %d\n", this->currentGlobalTrack);
      this->client->playGlobalTrack(this->currentGlobalTrack);
      for (int i = 0; i < 5 && !this->client->waitUntilBusy(500); ++i)
        this->client->playGlobalTrack(this->currentGlobalTrack);
      this->currentGlobalTrackMillis = 0;
    }
  }
  else
  {
    this->currentFolder = 0;  
    this->currentFolderMillis = 0;
    this->currentGlobalTrack = currentEntryIndex; 
    this->currentGlobalTrackMillis = millis();
    this->client->stop();
  }
  return true;
}

void Mp3Player::FolderSelection::select()
{
  uint8_t index = this->currentEntryIndex;

  this->currentFolder = 0;  
  this->currentFolderMillis = 0;
  this->currentGlobalTrack = 0; 
  this->currentGlobalTrackMillis = 0;

  if (index == 0 && (this->extras & Extra::RandomGlobalTrack))
  {
    // random global track...
    // => neither current folder nor current track will be set...    
  }
  else
  {
    index -= (this->extras & Extra::RandomGlobalTrack);
    if (index < this->client->getNumberOfFolders())
    {
      // track_selection!!!
      Serial.printf("Folder selection!!! folder=%d\n", index + 1);
      this->currentFolder = index + 1;
    }
    else
    {
      index -= this->client->getNumberOfFolders();
      Serial.printf("Track selection!!! track=%d\n", index + 1);
      this->currentGlobalTrack = index + 1;
    }
  }

  Serial.printf("%s: currentFolder=%d currentGlobalTrack=%d\n", __PRETTY_FUNCTION__, currentFolder, currentGlobalTrack);

  this->client->pause();
  
  this->exit(Result::Accept);
  // Mp3Player::FileSelection::Spawn()
}

void Mp3Player::FolderSelection::exit(Result result)
{
  if (this->onExitCallback)
    this->running = !this->onExitCallback(this, result);
}

void Mp3Player::FolderSelection::Spawn(Menu *menu, std::string label, Mp3Player *client, OnExitCallback onExitCallback, uint8_t extras)
{
  Serial.printf("%s ~> extras=%u\n", __PRETTY_FUNCTION__, extras);
  menu->editors.push_back(new FolderSelection(menu, std::move(label), client, onExitCallback, extras));
  menu->editors.back()->setup();
}
