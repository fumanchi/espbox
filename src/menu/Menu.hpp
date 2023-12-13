#ifndef MENU_HPP_
#define MENU_HPP_

#include <type_traits>
#include <list>
#include <vector>

#include "TFT_eSPI.h"

#include "TFT.hpp"
#include "RotaryEncoder.hpp"

class Editor;

struct MenuIO
{
  MenuIO() = delete;
  MenuIO(TFT &tft, RotaryEncoder &rotary);
  MenuIO(const MenuIO &other);
  virtual ~MenuIO() = default;

  // void loop();

  TFT &tft;  
  RotaryEncoder &rotary;
};


class Menu : public MenuIO
{
public:

  class Entry;
  typedef void (*callback_t)(Menu*, Entry*);
  
  class SmartMenuIO : public MenuIO
  {
    private:
      friend class Menu;
      SmartMenuIO(Menu *menu);
    public:
      SmartMenuIO(SmartMenuIO &&other);
      SmartMenuIO(const SmartMenuIO&) = delete;
      virtual ~SmartMenuIO();

      bool release();
      
    private:
      Menu *menu;
  };

  struct SubMenu;
  
  struct Entry
  {
    enum class Type
    {
      Leaf,
      Value,
      SubMenu,
      Root  
    };
    
    Entry() = default;
    virtual ~Entry() = default;

    virtual Type getType() const = 0;
    virtual uint16_t getGlyph() const = 0;
    virtual const std::string &getLabel() const = 0;

    virtual SubMenu *getParent() const = 0;
    virtual void setParent(SubMenu *parent) = 0;

    inline virtual bool hasCallback() const { return this->getCallback() != nullptr; }
    virtual callback_t getCallback() const = 0;
    inline virtual void call(Menu *menu) { if (this->hasCallback()) this->getCallback()(menu, this); };

    virtual bool isSubMenu() = 0;

  };

  struct EntryImpl : Entry
  {       
    EntryImpl(Type type, uint16_t glyph, std::string label, callback_t callback, SubMenu *parent = nullptr);
    EntryImpl(const EntryImpl &other) = delete;
    virtual ~EntryImpl() = default;

    inline virtual Type getType() const override { return this->type; }
    inline virtual uint16_t getGlyph() const override { return this->glyph; }
    inline virtual const std::string &getLabel() const override { return this->label; }

    inline virtual SubMenu *getParent() const override { return this->parent; }
    inline virtual void setParent(SubMenu *parent) override { this->parent = parent; }

    inline virtual bool hasCallback() const override { return this->callback != nullptr; }
    inline virtual callback_t getCallback() const override { return this->callback; }
    inline virtual void call(Menu *menu) { if (this->callback) this->callback(menu, this); };

    SubMenu *parent;

    const Type type;
    uint16_t glyph;
    std::string label;

    callback_t callback; 
  };

  struct Leaf : EntryImpl
  {
    Leaf(uint16_t glyph, std::string label, callback_t callback = nullptr, SubMenu *parent = nullptr);
    Leaf(const Leaf &other) = delete;
    virtual ~Leaf() = default;

    virtual bool isSubMenu();
  };

  struct Toggle : Leaf
  {
    typedef std::pair<uint16_t, std::string> StateEntry;

    Toggle(StateEntry stateEntryEnabled, StateEntry stateEntryDisabled, bool state);
    Toggle(StateEntry stateEntryEnabled, StateEntry stateEntryDisabled, callback_t callback = nullptr, bool state = true);
    Toggle(const Toggle &other) = delete;
    virtual ~Toggle() = default;

    void toggle();
    void setState(bool state);
    bool getState() const;

    StateEntry stateEntries[2];
    callback_t callback;

    bool state;
  };

  template<int NSTATES>
  struct Triggle : Leaf
  {
    typedef std::pair<uint16_t, std::string> StateEntry;

    Triggle(std::initializer_list<StateEntry> entries, callback_t callback, int state = 0)
      : Leaf {
            font, 
            entries.begin()->first,
            entries.begin()->second, 
            [](Menu *menu, Entry *entry)
              {
                if (Menu::Triggle<NSTATES> *triggle = static_cast<Menu::Triggle<NSTATES>*>(entry))
                {
                  if (triggle->Triggle::callback)
                    triggle->Triggle::callback(menu, entry);
                }
              } 
            },
            stateEntries {},
            callback {callback},

            state {state}
    {
      this->Leaf::glyph = this->stateEntries[state].first;
      this->Leaf::label = this->stateEntries[state].second;
      assert(entries.size() == NSTATES);
      int index = 0;
      for (const StateEntry &entry : entries)
        this->stateEntries[index++] = entry;
    }  
    Triggle(const Toggle &other) = delete;
    virtual ~Triggle() = default;
    void setState(int state, bool force = false)
    {
      state = std::max(0, state);
      state = std::min(NSTATES, state);
      if (state != this->state || force)
      {
        this->state = state;
        this->Leaf::glyph = this->stateEntries[state].first;
        this->Leaf::label = this->stateEntries[state].second;
        if (CurrentInstance && CurrentInstance->current && CurrentInstance->current == this->Leaf::parent)
          CurrentInstance->dirty = true;
      }
    }
    int getState() const
    {
      return this->state;
    }

    std::array<StateEntry, NSTATES> stateEntries;
    callback_t callback;

    int state;
  };

  struct Value : EntryImpl
  {
    Value(std::string label, callback_t callback = nullptr);
    Value(uint16_t glyph, std::string label, callback_t callback = nullptr);
    Value(const Value &other) = delete;
    virtual ~Value() = default;

    virtual bool isSubMenu();

    virtual const uint8_t *getValueFont() = 0;
    virtual const char *getValue(Menu *menu, Value *value) = 0;
  };
  
  struct CheckValue : Value
  {
    typedef bool (*getState_t)(Menu*, CheckValue*);
    typedef void (*setState_t)(Menu*, CheckValue*, bool);
    
    CheckValue(std::string label, getState_t getState, setState_t setState);
    CheckValue(const CheckValue &other) = delete;
    virtual ~CheckValue() = default;

    virtual const uint8_t *getValueFont();
    virtual const char *getValue(Menu *menu, Value *value);

    getState_t getState;
    setState_t setState;
  };

  struct StringValue : Value
  {
    typedef const char *(*getStringValue_t)(Menu*, StringValue*);
    typedef void (*setStringValue_t)(Menu*, StringValue*, const char *data);
    typedef void (*editCallback_t)(Menu*, Value*);
    
    StringValue(std::string label, getStringValue_t getStringValue, setStringValue_t setStringValue = nullptr, editCallback_t editCallback = nullptr, callback_t callback = nullptr);
    StringValue(uint16_t glyph, std::string label, getStringValue_t getStringValue, setStringValue_t setStringValue = nullptr, editCallback_t editCallback = nullptr, callback_t callback = nullptr);
    StringValue(const StringValue &other) = delete;
    virtual ~StringValue() = default;

    virtual const uint8_t *getValueFont();
    virtual const char *getValue(Menu *menu, Value *value);
    virtual void setValue(Menu *menu, Value *value, const char *data);
    
    getStringValue_t getStringValue;
    setStringValue_t setStringValue;
    editCallback_t editCallback;
  };

  template<typename T>
  struct BufferedValue : StringValue
  {
    BufferedValue(std::string label, T &value, editCallback_t editCallback = nullptr, callback_t callback = nullptr)
      : BufferedValue {0, std::move(label), value, editCallback, callback}
    {
    }
    
    BufferedValue(uint16_t glyph, std::string label, T &value, editCallback_t editCallback = nullptr, callback_t callback = nullptr)
      : StringValue {glyph, std::move(label), &BufferedValue<T>::ReadValue, &BufferedValue<T>::WriteValue, editCallback, callback},
        value {value},
        string {}
    {
      this->update();
    }
    
    BufferedValue(const Value &other) = delete;
    virtual ~BufferedValue() = default;

    template<typename T_>
    typename std::enable_if<std::is_arithmetic<T_>::value>::type update_()
    {
      this->string = String(this->value, DEC);
    }

    template<typename T_>
    typename std::enable_if<std::is_same<T_, String>::value || std::is_same<T_, std::string>::value>::type update_()
    {
      this->string = this->value.c_str();
    }

    template<typename T_>
    typename std::enable_if<std::is_array<T_>::value && std::is_same<typename std::decay<T_>::type, char*>::value>::type update_()
    {
      this->string = this->value;
    }

    void update()
    {
      this->update_<T>();
    }

    template<typename T_>
    typename std::enable_if<std::is_integral<T_>::value>::type sync_()
    {
      this->value = this->string.toInt();
    }

    template<typename T_>
    typename std::enable_if<std::is_floating_point<T_>::value>::type sync_()
    {
      this->value = this->string.toDouble();
    }

    template<typename T_>
    typename std::enable_if<std::is_same<T_, String>::value || std::is_same<T_, std::string>::value>::type sync_()
    {
      this->value = this->string.c_str();
    }

    template<typename T_>
    typename std::enable_if<std::is_array<T_>::value && std::is_same<typename std::decay<T_>::type, char*>::value>::type sync_()
    {
      int len = std::min(this->string.length(), std::extent<T_>::value - 1);
      memcpy(this->value, this->string.c_str(), len);
      this->value[len] = 0;
    }

    void sync(Menu *menu)
    {
      this->sync_<T>();
      if (this->callback)
        this->callback(menu, this);
    }

    static const char *ReadValue(Menu *, StringValue *stringValue)
    {
      BufferedValue<T> *bufferedValue = (BufferedValue<T>*)stringValue;
      bufferedValue->update();
      return bufferedValue->string.c_str();
    }

    static void WriteValue(Menu *menu, StringValue *stringValue, const char *data)
    {
      BufferedValue<T> *bufferedValue = (BufferedValue<T>*)stringValue;
      bufferedValue->string = data;
      bufferedValue->sync(menu);
    }

    T &value;
    String string;
  };

  
  struct SubMenu : EntryImpl
  {
    typedef bool(*ExitCallback)(Menu*);
    
    enum class Direction
    {
      HORIZONTAL,
      VERTICAL
    };

    SubMenu(uint16_t glyph, std::string label, std::initializer_list<Entry*> entries, int8_t index = 0, Direction direction = Direction::HORIZONTAL, ExitCallback exitCallback = nullptr);
    SubMenu(const SubMenu &other) = delete;
    virtual ~SubMenu() = default;

    virtual bool isSubMenu();

    std::vector<Entry*> entries;
    const int8_t index;
    Direction direction;

    ExitCallback exitCallback;
  };

  struct Root : SubMenu
  {
    Root(Menu *menu, std::string label, std::initializer_list<Entry*> entries, int8_t index = 0, ExitCallback exitCallback = nullptr);
    Root(const Root &other) = delete;
    virtual ~Root() = default;
    
    Menu *menu;  
  };

public:
  Menu(TFT &tft, RotaryEncoder &rotary, std::string name, std::initializer_list<Entry*> entries, int8_t index = 0, SubMenu::ExitCallback exitCallback = nullptr);
  Menu(Menu *menu, std::string name, std::initializer_list<Entry*> entries, int8_t index = 0, SubMenu::ExitCallback exitCallback = nullptr);
  virtual ~Menu();

  void setCurrent(SubMenu *subMenu = nullptr, int8_t index = -1, bool draw = true);

  void begin();
  void exit();

  SmartMenuIO getIO();
  
  void setDirty();
  void setBackground(uint16_t color);

  void selected();
  void back();

  void drawTitle(const Entry *entry);
  void drawTitle(const char *label, uint16_t glyph = 0);
  void draw();
  
  void loop();

  static void UpdateBlackout(int seconds = -1);

  static unsigned long Blackout;
  static unsigned long BlackoutTimeout;
  static unsigned long BlackoutTimeoutDefault;
  static Menu *CurrentInstance; 
   
  static void Selected(Button2&);
  static void Back(Button2&);
  static void Screenshot(Button2&);

  virtual std::pair<int, uint16_t*> getTitleGlyphs();

  bool registerIO();
  bool unregisterIO();

  

public:
  TFT_eSprite sprite;

  Root root;

  std::list<Editor*> editors;
  SubMenu *current;

  uint16_t backgroundColor;

  volatile bool dirty;
  uint8_t index;
  size_t location;
  
  size_t posCurrent;
  size_t locationCurrent;

  bool registered;
  SmartMenuIO *smartMenuIOGiven;
};

#endif // MENU_HPP_
