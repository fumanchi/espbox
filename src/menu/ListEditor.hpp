#ifndef LISTEDITOR_HPP_
#define LISTEDITOR_HPP_

#include "Menu.hpp"
#include "Editor.hpp"


class ListEditor : public Editor
{
  public:
    enum class Role
    {
      Glyph,
      Label,
      Value  
    };
    
  public:
    ListEditor(Menu *menu, Menu::Value *value = nullptr, bool refresh = false);
    ListEditor(Menu *menu, std::string label, bool refresh = false);
    ~ListEditor();

    virtual int getNumberOfItems() const = 0;
    virtual const char *getItem(int index, Role role) const = 0;

    virtual bool setup();
    virtual bool update(int index = -1);
    virtual bool loop();

    template<typename T, typename...ARGS>
    static void Spawn(Menu *menu, Menu::Value *value, ARGS&&...args);

    template<typename T, typename...ARGS>
    static void Spawn(Menu *menu, ARGS&&...args);

  protected:
    virtual void select();
    virtual void exit(Result result = Result::Cancel);
    
    virtual void drawTitle();
    virtual void draw();
    
  protected:
    static void Select(Button2&);
    static void Exit(Button2&);
    
    static ListEditor *CurrentListEditor;

  protected:
    Menu::Value *value;
    bool refresh;
    
    int currentItem;
    
    int currentEntryIndex;
    int currentLocation;
    
    volatile bool running;    
};

template<typename T, typename...ARGS>
void ListEditor::Spawn(Menu *menu, Menu::Value *value, ARGS&&...args)
{
  Serial.println(__PRETTY_FUNCTION__);
  menu->editors.push_back(new T(menu, value, std::forward<ARGS>(args)...));
  menu->editors.back()->setup();
  Serial.printf("%s: done...\n", __PRETTY_FUNCTION__);
}

template<typename T, typename...ARGS>
void ListEditor::Spawn(Menu *menu, ARGS&&...args)
{
  Serial.println(__PRETTY_FUNCTION__);
  menu->editors.push_back(new T(menu, std::forward<ARGS>(args)...));
  menu->editors.back()->setup();
  Serial.printf("%s: done...\n", __PRETTY_FUNCTION__);
}

#endif /* LISTEDITOR_HPP_ */
