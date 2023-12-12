
#include "Editor.hpp"


Editor::Editor(Menu *menu, std::string label, Result result)
  : menu {menu},
    menuIO {menu->getIO()},
    result {result},
    label {std::move(label)}
{

}

bool Editor::setup()
{
  return true;  
}

bool Editor::loop()
{
  return false;
}
