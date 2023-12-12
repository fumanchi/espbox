#ifndef MP3PLAYER_FOLDERSELECTION_HPP_
#define MP3PLAYER_FOLDERSELECTION_HPP_

#include <bitset>

#include "Mp3Player.hpp"

#include "menu/ListEditor.hpp"


class Mp3Player::FolderSelection : public ListEditor
{
  public: /* types */
    using OnExitCallback = bool(*)(FolderSelection*, Result);

    enum class Mode
    {
      Folder,
      Track,
      RandomGlobalTrack
    };

    enum Extra : uint8_t
    {
      RandomGlobalTrack   = 1 << 0,
      LoopGlobalTrack     = 1 << 1,

      AllExtras           = RandomGlobalTrack | LoopGlobalTrack
    };

  public:
    FolderSelection(Menu *menu, std::string label, Mp3Player *client, OnExitCallback onExitCallback, uint8_t extras = 0);
    virtual ~FolderSelection() = default;

    Mode getCurrentMode() const;
    uint16_t getCurrentFolder() const;
    uint16_t getCurrentTrack() const;

    virtual int getNumberOfItems() const override;
    virtual const char *getItem(int index, Role role) const override;

    virtual bool setup() override;
    virtual bool loop() override;

    virtual void select() override;
    virtual void exit(Result result) override;

    static void Spawn(Menu *menu, std::string label, Mp3Player *client, OnExitCallback onExitCallback, uint8_t extras = 0);

  private:
    Mp3Player *client;
    OnExitCallback onExitCallback;
    uint8_t extras;

    uint8_t currentFolder;  
    size_t currentFolderMillis; // 0 => current folder playing; >0 => currenFolder updated at

    uint16_t currentGlobalTrack;  
    size_t currentGlobalTrackMillis; // 0 => current global playing; >0 => currenFolder updated at

};

#endif /* MP3PLAYER_FOLDERSELECTION_HPP_ */
