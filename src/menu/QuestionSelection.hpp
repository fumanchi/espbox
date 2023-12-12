#ifndef QUESTIONSELECTION_HPP_
#define QUESTIONSELECTION_HPP_

#include <initializer_list>
#include <functional>

#include "ListEditor.hpp"


class QuestionSelection : public ListEditor
{
public: /* types */
  using Action = std::function<bool(int)>;
  using Entry = std::pair<int, String>;
  
public:
  QuestionSelection(Menu *menu, std::initializer_list<Entry> entries, const char *title = nullptr, int *value = nullptr);
  QuestionSelection(Menu *menu, std::initializer_list<Entry> entries, const char *title, Action action);
  virtual ~QuestionSelection() = default;

  virtual void drawTitle() override;

  virtual int getNumberOfItems() const override;
  virtual const char *getItem(int index, Role role) const override;

  virtual void select() override;
  virtual void exit(Result result) override;
 
protected:
  std::vector<Entry> entries;

  String title;

  int localValue;
  int &value;

  Action action;
};

#endif /* QUESTIONSELECTION_HPP_ */
