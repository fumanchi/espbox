#ifndef MP3PLAYER_TRACKSELECTION_HPP_
#define MP3PLAYER_TRACKSELECTION_HPP_

#include <bitset>

#include "Mp3Player.hpp"

#include "menu/ListEditor.hpp"


class Mp3Player::TrackSelection : public ListEditor
{
  public: /* types */
    using OnExitCallback = bool(*)(TrackSelection*, Result);

    enum class Mode : uint8_t
    {
      Back,
      Folder,
      Random,
      SingleTrack
    };

  public:
    TrackSelection(Menu *menu, std::string label, Mp3Player *client, uint16_t folder, OnExitCallback onExitCallback);
    virtual ~TrackSelection() = default;

    uint16_t getFolder() const;

    Mode getCurrentMode() const;
    uint16_t getCurrentTrack() const;

    virtual int getNumberOfItems() const override;
    virtual const char *getItem(int index, Role role) const override;

    virtual bool setup() override;
    virtual bool loop() override;

    virtual void select() override;
    virtual void exit(Result result) override;

    static void Spawn(Menu *menu, std::string label, Mp3Player *client, uint16_t folder, OnExitCallback onExitCallback);

  private:
    Mp3Player *client;
    uint16_t folder;
    OnExitCallback onExitCallback;

    uint16_t folderTrackCount;
    uint8_t currentTrack;       
    size_t currentTrackMillis; // 0 => current track playing; >0 => currenTrack updated at

};

#endif /* MP3PLAYER_TRACKSELECTION_HPP_ */
