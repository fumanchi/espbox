
#include "Mp3Player.hpp"

namespace
{
  uint16_t fnv1a_16(const uint8_t *data, uint8_t size) 
  {
    uint16_t hash = 0x811C;
    
    for (uint8_t i = 0; i < size; ++i)
    {
        hash ^= (uint16_t)data[i];
        hash *= 0x0101;
    }
    
    return hash;
  }

  template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  uint16_t fnv1a_16(T value) 
  {
    return fnv1a_16((const uint8_t*)&value, (uint8_t)sizeof(T));
  }
}

Mp3Player::State::FlagReference::FlagReference(Flag flag, uint8_t &storage)
    :   flag {flag},
        storage {storage}
{
}

Mp3Player::State::FlagReference::FlagReference(const FlagReference &other)
    :   flag {other.flag},
        storage {other.storage}
{
}

Mp3Player::State::FlagReference::operator bool() const
{
    return (this->storage >> this->flag) & 1u;
}

Mp3Player::State::FlagReference &Mp3Player::State::FlagReference::operator=(bool value)
{
  if (value) 
    this->storage |= (1u << this->flag);
  else
    this->storage &= ~(1u << this->flag);
  return *this;
}

Mp3Player::State::State()
  : folder {0},
    track {0},
    flags {0}
{
  
}

Mp3Player::State::State(uint16_t folder, uint16_t track, std::initializer_list<Flag> flagsGiven)
  : folder {folder},
    track {track},
    flags {0}
{
  for (uint8_t flagGiven : flagsGiven) 
    this->flags |= (1u << flagGiven);
}

Mp3Player::State::State(const State &other)
    : State {}
{
    this->operator=(other);
}


Mp3Player::State &Mp3Player::State::operator=(const State &other)
{
    this->folder = other.folder;
    this->track = other.track;
    this->flags = other.flags;
    return *this;
}

bool Mp3Player::State::operator[](Flag flag) const
{
    return (this->flags >> flag) & 1u;
}

Mp3Player::State::FlagReference Mp3Player::State::operator[](Flag flag)
{
    return FlagReference {flag, this->flags};
}

Mp3Player::State::Mode Mp3Player::State::getMode() const
{
  return (Mode)(((this->folder != 0) * HasFolder) + ((this->track != 0) * HasTrack));
}

bool Mp3Player::State::hasFolder() const
{
    return this->folder != 0;
}

uint16_t Mp3Player::State::getFolder() const
{
    return this->folder;
}

bool Mp3Player::State::hasTrack() const
{
    return this->track != 0;
}

uint16_t Mp3Player::State::getTrack() const
{
    return this->track;
}

Mp3Player::Mp3Player()
    : DfMp3{Serial1},
      numberOfFolders{-1},
      totalTrackCount{-1},
      folderTrackCount{-1},

      currentVolume {1},
      maxVolume {20},

      currentState {},
      currentTrackIndex {0},

      randomSeed {0}
{

}

void Mp3Player::setup(const Settings &settings, uint8_t volume, uint8_t maxVolume)
{
  Serial.printf("%s@%d...\n", __PRETTY_FUNCTION__, __LINE__);
  pinMode(settings.DFMP3_BUSY, INPUT_PULLUP);
  Serial.printf("%s@%d...\n", __PRETTY_FUNCTION__, __LINE__);
  attachInterrupt(digitalPinToInterrupt(settings.DFMP3_BUSY), &Mp3Player::BusyStateISR, CHANGE);
  Serial.printf("%s@%d...\n", __PRETTY_FUNCTION__, __LINE__);
  busy = digitalRead(settings.DFMP3_BUSY);
  Serial.printf("%s@%d...\n", __PRETTY_FUNCTION__, __LINE__);
  
  // dfmp3.begin();
  // for boards that support hardware arbitrary pins
  Serial.printf("%s@%d... this->DfMp3::begin(%u, %u)\n", __PRETTY_FUNCTION__, __LINE__, settings.DFMP3_RX, settings.DFMP3_TX);
  this->DfMp3::begin(settings.DFMP3_RX, settings.DFMP3_TX); // RX, TX
  Serial.printf("%s@%d...\n", __PRETTY_FUNCTION__, __LINE__);

  // during development, it's a good practice to put the module
  // into a known state by calling reset().
  // You may hear popping when starting and you can remove this
  // call to reset() once your project is finalized
  // this->DfMp3::setVolume(0);
  // delay(100);
  this->DfMp3::reset();
  Serial.printf("%s@%d...\n", __PRETTY_FUNCTION__, __LINE__);
  delay(200);

  uint16_t version = this->DfMp3::getSoftwareVersion();
  Serial.print("version ");
  Serial.println(version);
  this->DfMp3::setPlaybackSource(DfMp3_PlaySource_Sd);

  if (maxVolume)
    this->maxVolume = maxVolume;
  this->currentVolume = std::min(volume, this->maxVolume);

  this->DfMp3::setVolume(this->currentVolume);
  delay(100);
  this->DfMp3::loop();

  // uint16_t count = this->DfMp3::getTotalTrackCount(DfMp3_PlaySource_Sd);
  // this->DfMp3::loop();
  // Serial.print("files ");
  // Serial.println(count);


  Serial.println("starting...");
  this->updateNumberOfFolderCount();
  Serial.printf("Mp3Player: Found %d folders...\n", this->numberOfFolders);
  delay(900);
  this->updateTotalTrackCount();
  Serial.printf("Mp3Player: Found %d global tracks...\n", this->totalTrackCount);
  /*
  // start the first track playing
  Serial.print("Got ");
  Serial.print(this->DfMp3::getFolderTrackCount(3));
  this->DfMp3::loop();
  Serial.println(" files in folder #3");
  this->DfMp3::setPlaybackSource(DfMp3_PlaySource_Sd);
  this->DfMp3::playFolderTrack(2, 1); // sd:/##/###track name
  // while (this->DfMp3::getStatus().state == DfMp3_StatusState_Idle)
  // {
  //   Serial.print("Status ");
  //   Serial.println(this->DfMp3::getStatus().state);
  //   this->loop();
  //   delay(10);
  // }
  Serial.print("Status ");
  Serial.println(this->DfMp3::getStatus().state);
  Serial.print("Playing track #");
  Serial.println(this->DfMp3::getCurrentTrack(DfMp3_PlaySource_Sd));
  // this->DfMp3::playMp3FolderTrack(1);  // sd:/mp3/0001.mp3
  */
}

void Mp3Player::loop()
{
  this->DfMp3::loop();
  // Serial.printf("%s: BusyState=%d/%d\n", __PRETTY_FUNCTION__, (int)this->busy, (int)digitalRead(27));
}

void Mp3Player::updateNumberOfFolderCount()
{
  delay(900);
  this->numberOfFolders = this->DfMp3::getTotalFolderCount();
  if (this->numberOfFolders == 0)
  {
    Serial.printf("Mp3Player: getTotalFolderCount not supported...\n");
    std::pair<uint8_t, uint8_t> minmax{0, std::numeric_limits<uint8_t>().max()};
    while (minmax.first != minmax.second)
    {
      this->DfMp3::loop();
      Serial.printf("Mp3Player: minmax=(%u, %u)\n", minmax.first, minmax.second);
      if ((minmax.second - minmax.first) == 1)
      {
        if (this->DfMp3::getFolderTrackCount(minmax.second))
          minmax.first = minmax.second;
        else
          minmax.second = minmax.first;
      }
      else
      {
        uint8_t check = (minmax.first + minmax.second) / 2;
        if (this->DfMp3::getFolderTrackCount(check))
          minmax.first = check;
        else
          minmax.second = check;
      }
      delay(10);
      // this->waitUntilIdle();
    }
    this->numberOfFolders = minmax.second;
  }
}

uint16_t Mp3Player::getNumberOfFolders() const
{
  return this->numberOfFolders;
}

void Mp3Player::updateTotalTrackCount()
{
  this->totalTrackCount = this->DfMp3::getTotalTrackCount(DfMp3_PlaySource_Sd);
}

uint16_t Mp3Player::getTotalTrackCount() const
{
  return this->totalTrackCount;
}

uint16_t Mp3Player::getFolderTrackCount()
{
  if (this->folderTrackCount < 0 && this->currentState.hasFolder())
    this->folderTrackCount = this->DfMp3::getFolderTrackCount(this->currentState.getFolder());
  return (uint16_t)this->folderTrackCount;
}

uint8_t Mp3Player::getVolume() const
{
  return this->currentVolume;
}

uint8_t Mp3Player::setVolume(uint8_t volume)
{
  this->currentVolume = std::min(volume, this->maxVolume);
  this->DfMp3::setVolume(this->currentVolume);
  return this->currentVolume;
}

uint8_t Mp3Player::getMaxVolume() const
{
  return this->maxVolume;
}

uint8_t Mp3Player::setMaxVolume(uint8_t maxVolume)
{
  if (maxVolume < this->maxVolume && this->currentVolume > maxVolume)
      this->DfMp3::setVolume(this->currentVolume = maxVolume);
  this->maxVolume = maxVolume;
  return this->currentVolume;
}

bool Mp3Player::waitUntilIdle(size_t timeout)
{  
  if (this->busy)
  for (size_t start = millis(); this->busy && (millis() - start < timeout); ) 
  {
    delay(10);
    this->loop();
  }
  return !this->busy;
}

bool Mp3Player::waitUntilBusy(size_t timeout)
{  
  if (!this->busy)
  for (size_t start = millis(); !this->busy && (millis() - start < timeout); ) 
  {
    delay(10);
    this->loop();
  }
  return this->busy;
}

void Mp3Player::runState(const State &state)
{
  Serial.printf("%s@%d: this->currentTrackIndex=%u\n", __PRETTY_FUNCTION__, __LINE__, this->currentTrackIndex);
  if (this->currentTrackIndex)
  {
    this->stop();
    this->waitUntilIdle();
  }
  assert(this->currentTrackIndex == 0);
  this->currentTrackIndex = 0;

  // reset folderTrackCount on folder switch
  if (this->currentState.hasFolder() && this->currentState.getFolder() != state.getFolder())
    this->folderTrackCount = -1;

  this->currentState = state;
  if (this->currentState[State::RANDOM])
  {
    this->randomSeed = fnv1a_16(millis());
    Serial.printf("%s: randomSeed=%u\n", __PRETTY_FUNCTION__, this->randomSeed);
  }
  using Mode = State::Mode;
  Serial.printf("%s@%d: this->currentState.getMode()=%u\n", __PRETTY_FUNCTION__, __LINE__, this->currentState.getMode());
  switch (this->currentState.getMode())
  {
    case Mode::AllGlobalTracks:
      if (this->currentState[State::RANDOM])
        this->playRandomTrackFromAll();
      else
        this->playGlobalTrack(1);
      break;
    case Mode::SingleFolder:
      if (this->currentState[State::RANDOM])
        this->playRandomTrackFromFolder();
      else
        this->playFolderTrack();
      break;
    case Mode::SingleGlobalTrack:
      this->DfMp3::playGlobalTrack(this->currentState.getTrack());
      break;
    case Mode::SingleFolderTrack:
      this->DfMp3::playFolderTrack(this->currentState.getFolder(), this->currentState.getTrack());
      break;
  }

}

void Mp3Player::playRandomTrackFromAll()
{
  if (this->currentTrackIndex)
  {
    this->stop();
    this->waitUntilIdle();
  }
  assert(this->currentTrackIndex == 0);
  this->currentTrackIndex = 0;

  this->playNextRandomTrackFromAll();
  // if (this->getTotalTrackCount())
  // {
  //   Serial.printf("%s: currentTrack=%u actual track=%u/%u\n", __PRETTY_FUNCTION__, this->currentTrackIndex, (fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getTotalTrackCount()) + 1, this->getTotalTrackCount());
  //   this->DfMp3::playGlobalTrack((fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getTotalTrackCount()) + 1);
  // }
}

void Mp3Player::playNextRandomTrackFromAll()
{
  if (++this->currentTrackIndex >= this->getTotalTrackCount())
    this->currentTrackIndex = this->currentState[State::LOOP];

  if (this->currentTrackIndex)
  {
    this->DfMp3::playGlobalTrack((fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getTotalTrackCount()) + 1);
    Serial.printf("%s: currentTrack=%u actual track=%u/%u\n", __PRETTY_FUNCTION__, this->currentTrackIndex, (fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getTotalTrackCount()) + 1, this->getTotalTrackCount());
  }
}

void Mp3Player::playRandomTrackFromFolder()
{
  assert(this->currentState.hasFolder());
  if (this->currentTrackIndex)
  {
    this->stop();
    this->waitUntilIdle();
  }
  assert(this->currentTrackIndex == 0);
  this->currentTrackIndex = 0;

  this->playNextRandomTrackFromFolder();
  // if (this->getFolderTrackCount())
  // {
  //   Serial.printf("%s: currentTrack=%u actual track=%u/%u\n", __PRETTY_FUNCTION__, this->currentTrackIndex, (fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getFolderTrackCount()) + 1, this->getFolderTrackCount());
  //   this->DfMp3::playFolderTrack(this->currentState.getFolder(), (fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getFolderTrackCount()) + 1);
  // }
}

void Mp3Player::playNextRandomTrackFromFolder()
{
  assert(this->currentState.hasFolder());
  if (++this->currentTrackIndex >= this->getFolderTrackCount())
    this->currentTrackIndex = (uint16_t)this->currentState[State::LOOP];

  if (this->currentTrackIndex)
  {
    Serial.printf("%s: currentTrack=%u actual track=%u/%u\n", __PRETTY_FUNCTION__, this->currentTrackIndex, (fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getFolderTrackCount()) + 1, this->getFolderTrackCount());
    this->DfMp3::playFolderTrack(this->currentState.getFolder(), (fnv1a_16(this->currentTrackIndex + this->randomSeed) % this->getFolderTrackCount()) + 1);
  }
}

void Mp3Player::playFolderTrack()
{
  if (this->currentTrackIndex)
  {
    this->stop();
    this->waitUntilIdle();
  }
  assert(this->currentTrackIndex == 0);
  this->currentTrackIndex = 0;

  this->playNextFolderTrack(); // add 1 & play
}

void Mp3Player::playNextFolderTrack()
{
  if (++this->currentTrackIndex >> this->getFolderTrackCount())
    this->currentTrackIndex = this->currentState[State::LOOP];

  if (this->currentTrackIndex)
    this->DfMp3::playFolderTrack(this->currentState.getFolder(), this->currentTrackIndex);
}

void Mp3Player::onError(uint16_t errorCode)
{
}

void Mp3Player::onPlayFinished(PlaySources source, uint16_t track)
{
  using Mode = State::Mode;

  switch (this->currentState.getMode())
  {
  case Mode::AllGlobalTracks:
    if (this->currentState[State::RANDOM])
    {
      // if (++this->currentTrackIndex > this->getTotalTrackCount())
      //   this->currentTrackIndex = this->currentState[State::LOOP];
      // if (this->currentTrackIndex)
      //   this->playRandomTrackFromAll();
      this->playNextRandomTrackFromAll();
    }
    else
    {
      // if (++this->currentTrackIndex > this->getTotalTrackCount())
      //   this->currentTrackIndex = this->currentState[State::LOOP];
      // if (this->currentTrackIndex)
      //   this->playGlobalTrack(this->currentTrackIndex);
      if (++track > this->getTotalTrackCount())
        track = this->currentState[State::LOOP];
      if (track)
        this->playGlobalTrack(track);
    }
    break;
  case Mode::SingleFolder:
    if (this->currentState[State::RANDOM])
      this->playNextRandomTrackFromFolder();
    else
      this->playNextFolderTrack();
    break;
  case Mode::SingleGlobalTrack:
    if (this->currentState[State::LOOP])
      playGlobalTrack(this->currentState.getTrack());
    break;
  case Mode::SingleFolderTrack:
    if (this->currentState[State::LOOP])
      playFolderTrack(this->currentState.getFolder(), this->currentState.getTrack());
    break;
  }
}

void Mp3Player::onPlaySourceOnline(PlaySources source)
{
}

void Mp3Player::onPlaySourceInserted(PlaySources source)
{
  if (source == DfMp3_PlaySources_Sd)
  {
    this->updateNumberOfFolderCount();
    delay(900);
    this->updateTotalTrackCount();
    delay(900);
  }
}

void Mp3Player::onPlaySourceRemoved(PlaySources source)
{
  if (source == DfMp3_PlaySources_Sd)
    this->numberOfFolders = -1;
}

Mp3Player &Mp3Player::Instance()
{
  static Mp3Player SingletonInstance;
  return SingletonInstance;
}

void Mp3Player::PrintlnSourceAction(DfMp3_PlaySources source, const char *action)
{
  if (source & DfMp3_PlaySources_Sd)
  {
    Serial.print("SD Card, ");
  }
  if (source & DfMp3_PlaySources_Usb)
  {
    Serial.print("USB Disk, ");
  }
  if (source & DfMp3_PlaySources_Flash)
  {
    Serial.print("Flash, ");
  }
  Serial.println(action);
}

void Mp3Player::OnError(DfMp3 &mp3, uint16_t errorCode)
{
  Mp3Player::Instance().onError(errorCode);

  // see DfMp3_Error for code meaning
  Serial.println();
  Serial.print("Com Error ");
  Serial.println(errorCode);
}

void Mp3Player::OnPlayFinished(DfMp3 &mp3, PlaySources source, uint16_t track)
{
  Mp3Player::Instance().onPlayFinished(source, track);
  // Serial.print("Play finished for #");
  // Serial.println(track);

  // // start next track
  // track += 1;
  // Serial.print("Playing #");
  // Serial.println(track);
  // // this example will just start back over with 1 after track 3
  // if (false || track > 3)
  // {
  //   track = 1;
  // }
  // dfmp3.playMp3FolderTrack(track); // sd:/mp3/0001.mp3, sd:/mp3/0002.mp3, sd:/mp3/0003.mp3
}

void Mp3Player::OnPlaySourceOnline(DfMp3 &mp3, DfMp3_PlaySources source)
{
  Mp3Player::Instance().onPlaySourceOnline(source);
  PrintlnSourceAction(source, "online");
}

void Mp3Player::OnPlaySourceInserted(DfMp3 &mp3, DfMp3_PlaySources source)
{
  Mp3Player::Instance().onPlaySourceInserted(source);
  PrintlnSourceAction(source, "inserted");
}

void Mp3Player::OnPlaySourceRemoved(DfMp3 &mp3, DfMp3_PlaySources source)
{
  Mp3Player::Instance().onPlaySourceRemoved(source);
  PrintlnSourceAction(source, "removed");
}

void IRAM_ATTR Mp3Player::BusyStateISR()
{
  Mp3Player::Instance().busy = !Mp3Player::Instance().busy;
}
