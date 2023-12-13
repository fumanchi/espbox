#ifndef MP3PLAYER_CONTROLENTRY_HPP_
#define MP3PLAYER_CONTROLENTRY_HPP_

#include "Mp3Player.hpp"

#include "menu/Menu.hpp"

class Mp3Player::ControlEntry : public Menu::Entry
{
public: /* types */
    enum class Action : uint16_t
    {
        PlayAll = (uint16_t)0x0094,
        Play = (uint16_t)0x0099,
        Pause = (uint16_t)0x0093,
        Stop = (uint16_t)0x009A
    };

public:
    ControlEntry(Menu *menu, Mp3Player *client);
    virtual ~ControlEntry() = default;

    inline virtual Type getType() const override { return Type::Leaf; }
    virtual uint16_t getGlyph() const override;
    virtual const std::string &getLabel() const override;

    virtual Menu::SubMenu *getParent() const { return this->parent; };
    virtual void setParent(Menu::SubMenu *parent) { this->parent = parent; };

    inline virtual bool hasCallback() const override { return true; }
    inline virtual Menu::callback_t getCallback() const override { return &ControlEntry::Callback; };
    virtual void call(Menu *menu) override;

    inline virtual bool isSubMenu() override { return false; }

private:
    Action action() const;

    static inline void Callback(Menu *menu, Entry *entry)
    {
        ControlEntry *controlEntry = static_cast<ControlEntry*>(entry);
        assert(controlEntry);
        controlEntry->call(menu);
    }

private:
    Menu::SubMenu *parent;

    Mp3Player *client;
};

#endif /* MP3PLAYER_CONTROLENTRY_HPP_ */
