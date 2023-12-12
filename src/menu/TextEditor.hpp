#ifndef TEXTEDITOR_HPP_
#define TEXTEDITOR_HPP_

#include "Menu.hpp"
#include "Editor.hpp"


class TextEditor : Editor
{
  public:
    enum Result
    {
      None,
      Error = None,
      Rejected,
      Accepted 
    };
    
  public:
    TextEditor(Menu *menu, Menu::StringValue *stringValue);
    ~TextEditor();

    virtual bool setup();
    virtual bool loop();

    static void Spawn(Menu *menu, Menu::Value *value);

  private:
    void click();
    void backspace();
    void exit();
    
    void drawTitle();
    void drawSelector(bool clear = false);
    void drawString(bool clear = false);
    void draw();
    
  protected:
    static void Click(Button2&);
    static void Backspace(Button2&);
    static void Exit(Button2&);
    
    static TextEditor *CurrentTextEditor;

  private:
    Menu::StringValue *stringValue;

    String string;

    volatile bool running;
    enum Dirty
    {
      Title       = 1 << 0,
      Selector    = 1 << 1,
      Value       = 1 << 2,
      All         = Title | Selector | Value
    };
    volatile uint8_t dirty;
    Result result;
    
    int16_t index;
    int16_t locationCurrent;
    unsigned long lastChange;

    int questionResult;
};

#endif // TEXTEDITOR_HPP_
