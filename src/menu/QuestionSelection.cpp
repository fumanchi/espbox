
#include "QuestionSelection.hpp"

QuestionSelection::QuestionSelection(Menu *menu, std::initializer_list<Entry> entries, const char *title, int *value)
  : ListEditor {menu},
    title {title && title[0] != 0 ? title : "Question"},
    value {value ? *value : localValue},
    action {nullptr}
{
  Serial.println(__PRETTY_FUNCTION__);
  this->entries.reserve(entries.size());
  for (Entry entry : entries)
    this->entries.emplace_back(std::move(entry));
}

QuestionSelection::QuestionSelection(Menu *menu, std::initializer_list<Entry> entries, const char *title, Action action)
  : ListEditor {menu},
    title {title && title[0] != 0 ? title : "Question"},
    value {localValue},
    action {action}
{
  Serial.println(__PRETTY_FUNCTION__);
  this->entries.reserve(entries.size());
  for (Entry entry : entries)
    this->entries.emplace_back(std::move(entry));
}


void QuestionSelection::drawTitle()
{
  this->menu->drawTitle(this->title.c_str());
//   // title
//  this->menuIO.u8g2.drawBox(0, 0, 128, 10);
//  this->menuIO.u8g2.setDrawColor(0);
////  const char *label = this->value->label.c_str();
////  int strWidth = menuIO.u8g2.getStrWidth(label);
////  if (this->value->glyph)
////  {
////    this->menuIO.u8g2.drawStr(64 - ((strWidth + 16)/ 2) + 16, 9, label);
////    this->menuIO.u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
////    this->menuIO.u8g2.drawGlyph(64 - ((strWidth + 16)/ 2), 9, this->value->glyph);
////  }
////  else
////      this->menuIO.u8g2.drawStr(64 - (strWidth / 2), 9, label);
//  this->menuIO.u8g2.setFont(u8g2_font_5x7_tf);
//  this->menuIO.u8g2.drawStr(4, 9, this->title.c_str());
//
//  this->menuIO.u8g2.setDrawColor(1);
//  
//  // frame & clip...
//  this->menuIO.u8g2.drawFrame(0, 10, 128, 53);
}

int QuestionSelection::getNumberOfItems() const
{
  return this->entries.size();
}

const char *QuestionSelection::getItem(int index, Role role) const
{
  if (index >= 0 && index < this->entries.size())
  switch (role)
  {
    case Role::Label:
      return this->entries[index].second.c_str();
      break;
//    case Role::Value:
//      return this->entries[index].first.c_str();
//      break;
    default:
      break;
  }
  return nullptr;
}

void QuestionSelection::select()
{
  Serial.printf("%s: current entry index=%d\n", __PRETTY_FUNCTION__, this->currentEntryIndex);
  this->ListEditor::select();    
  this->value = this->entries[this->currentEntryIndex].first;
  this->exit(Result::Accept);
}

void QuestionSelection::exit(Result result)
{
  if (result == Result::Accept)
    if (this->action && this->action(this->value))
      return;
  this->ListEditor::exit(result);
}
