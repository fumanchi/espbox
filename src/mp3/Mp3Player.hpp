#pragma once

#ifndef MP3PLAYER_HPP_
#define MP3PLAYER_HPP_

#include <HardwareSerial.h>

#include "DFMiniMp3.h"

#include "Settings.hpp"


class Mp3Player : public DFMiniMp3<HardwareSerial, Mp3Player> //, Mp3ChipMH2024K16SS Mp3ChipIncongruousNoAck>
{
public: /* types */
  class State
  {
    public:
      enum Flag : uint8_t
      {
        LOOP,
        RANDOM,

        // aliases    
        REPEAT = LOOP
      };

      class FlagReference
      {
        public:
          FlagReference(Flag flag, uint8_t &storage);
          FlagReference(const FlagReference &other);
          // FlagReference(FlagReference &&other) = delete;
          ~FlagReference() = default;

          FlagReference &operator=(const FlagReference &other);
          // FlagReference &operator=(FlagReference &&other) = delete;

          operator bool() const;
          FlagReference &operator=(bool value);

        private:
          Flag flag;
          uint8_t &storage;
      };

      enum Mode
      {
        HasFolder = 1 << 0,
        HasTrack  = 1 << 1,

        AllGlobalTracks = 0,
        SingleFolder = HasFolder,
        SingleGlobalTrack = HasTrack,
        SingleFolderTrack = HasFolder + HasTrack
      };

    protected:
      State(uint16_t folder, uint16_t track, uint8_t flags);
      
    public: 
      State();
      State(uint16_t folder, uint16_t track, std::initializer_list<Flag> flags);
      State(const State &other);
      ~State() = default;

      State &operator=(const State &other);

      bool operator[](Flag flag) const;
      FlagReference operator[](Flag flag);

      Mode getMode() const;

      bool hasFolder() const;
      uint16_t getFolder() const;

      bool hasTrack() const;
      uint16_t getTrack() const;

    protected:
      uint16_t folder;
      uint16_t track;
      uint8_t flags;    
  };

public:
  using DfMp3 = DFMiniMp3<HardwareSerial, Mp3Player>; //, Mp3ChipIncongruousNoAck>;
  using PlaySources = DfMp3_PlaySources;
 
  class VolumeControl;
  class EqualizerControl;
  
  class FolderSelection;
  class TrackSelection;

private:
  Mp3Player();

public:
  ~Mp3Player() = default;

  // DfMp3_Eq getCurrentEqualizer();
  // void setCurrentEqualizer(DfMp3_Eq equalizer);
  
  void setup(const Settings &settings, uint8_t volume = 1, uint8_t maxVolume = 0);
  void loop();

  void updateNumberOfFolderCount();
  uint16_t getNumberOfFolders() const;

  void updateTotalTrackCount();
  uint16_t getTotalTrackCount() const;

  using DfMp3::getFolderTrackCount;
  uint16_t getFolderTrackCount();

  uint8_t getVolume() const;
  uint8_t setVolume(uint8_t currentVolume); // returns actual current volume
  
  uint8_t getMaxVolume() const;
  uint8_t setMaxVolume(uint8_t maxVolume); // returns current volume

  bool waitUntilIdle(size_t millis = 500);
  bool waitUntilBusy(size_t millis = 500);

  void runState(const State &state);

  void playRandomTrackFromAll();
  void playNextRandomTrackFromAll();

  void playRandomTrackFromFolder();
  void playNextRandomTrackFromFolder();

  using DfMp3::playFolderTrack;
  void playFolderTrack();
  void playNextFolderTrack();

  // notofications
  void onError(uint16_t errorCode);
  void onPlayFinished(PlaySources source, uint16_t track);
  void onPlaySourceOnline(PlaySources source);
  void onPlaySourceInserted(PlaySources source);
  void onPlaySourceRemoved(PlaySources source);

  static Mp3Player &Instance();

public: // static interface to obtain DFMiniMp3 notifications...
  static void PrintlnSourceAction(PlaySources source, const char* action);
  static void OnError([[maybe_unused]] DfMp3& mp3, uint16_t errorCode);
  static void OnPlayFinished([[maybe_unused]] DfMp3& mp3, [[maybe_unused]] PlaySources source, uint16_t track);
  static void OnPlaySourceOnline([[maybe_unused]] DfMp3& mp3, PlaySources source);
  static void OnPlaySourceInserted([[maybe_unused]] DfMp3& mp3, PlaySources source);
  static void OnPlaySourceRemoved([[maybe_unused]] DfMp3& mp3, PlaySources source);

private:
  static void IRAM_ATTR BusyStateISR();
  volatile bool busy;
  
protected:
  int32_t numberOfFolders; 
  int32_t totalTrackCount;
  int32_t folderTrackCount;

  uint8_t currentVolume;
  uint8_t maxVolume;

  State currentState;
  uint16_t currentTrackIndex;
  uint16_t randomSeed;
};

#endif /* MP3PLAYER_HPP_ */
