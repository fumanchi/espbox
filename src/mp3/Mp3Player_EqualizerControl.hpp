#ifndef MP3PLAYER_EQUALIZERCONTROL_HPP_
#define MP3PLAYER_EQUALIZERCONTROL_HPP_

#include "Mp3Player.hpp"

#include "menu/ListEditor.hpp"


class Mp3Player::EqualizerControl : public ListEditor
{
  public: /* types */

  public:
    EqualizerControl(Menu *menu, std::string label, Mp3Player *client);
    virtual ~EqualizerControl() = default;

    virtual int getNumberOfItems() const override;
    virtual const char *getItem(int index, Role role) const override;

    virtual bool setup() override;
    virtual bool loop() override;

    virtual void select() override;
    // virtual bool update(int index = -1) override;
    // virtual void exit(Result result) override;

    static void Spawn(Menu *menu, std::string label, Mp3Player *client);

  private:
    Mp3Player *client;

    int currentValue;

};

#endif /* MP3PLAYER_EQUALIZERCONTROL_HPP_ */
