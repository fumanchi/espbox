#ifndef MP3PLAYER_VOLUMECONTROL_HPP_
#define MP3PLAYER_VOLUMECONTROL_HPP_

#include "Mp3Player.hpp"

#include "menu/IntegralEditor.hpp"


class Mp3Player::VolumeControl : public IntegralEditor
{
  public: /* types */

  public:
    VolumeControl(Menu *menu, std::string label, Mp3Player *client, long max, long min = 0);
    virtual ~VolumeControl() = default;

    // virtual bool setup();
    virtual bool loop();

    static void Spawn(Menu *menu, std::string label, Mp3Player *client, long max, long min = 0);

  private:
    Mp3Player *client;

};

#endif /* MP3PLAYER_VOLUMECONTROL_HPP_ */
