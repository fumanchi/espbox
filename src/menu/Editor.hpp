#ifndef EDITOR_HPP_
#define EDITOR_HPP_

#include "Menu.hpp"

class Editor
{
  public: /* types */
    enum class Result
    {
      None,
      Error = None,
      Ask,
      Accept,
      Cancel
    };

  public:
    Editor(Menu *menu, std::string label, Result result = Result::Ask);
    Editor(const Editor &other) = delete;
    virtual ~Editor() = default;

    virtual bool setup();
    virtual bool loop();

  protected:
    Menu *menu;
    Menu::SmartMenuIO menuIO;
    
    Result result;
    std::string label;
};

#endif /* EDITOR_HPP_ */
