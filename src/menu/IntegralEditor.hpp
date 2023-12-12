#ifndef INTEGRALEDITOR_HPP_
#define INTEGRALEDITOR_HPP_

#include "Menu.hpp"
#include "Editor.hpp"


class IntegralEditor : public Editor
{
  public:
    
  public:
    IntegralEditor(Menu *menu, Menu::Value *value, long min, long max);
    IntegralEditor(Menu *menu, std::string label, long min, long max);
    IntegralEditor(Menu *menu, std::string label, long min, long max, long current);
    ~IntegralEditor();

    virtual bool setup();
    virtual bool loop();

    static void Spawn(Menu *menu, Menu::Value *value, long min, long max);

  private:
    void exit(Result result);
    
    void drawTitle();
    void draw();
    
  private:

    static void Exit(Button2&);
    static void Click(Button2&);
    
    static IntegralEditor *CurrentIntegralEditor;

  protected:
    Menu::Value *value;
    const long min; 
    const long max;
    long current;
   
    volatile bool running;    
};

#endif // INTEGRALEDITOR_HPP_
