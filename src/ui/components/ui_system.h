#pragma once

#include <QColor>
#include <QIcon>
#include <QObject>

class UISystem : public QObject {
  Q_OBJECT
public:
  UISystem(const UISystem &) = delete;            // 禁用拷贝构造
  UISystem &operator=(const UISystem &) = delete; // 禁用赋值操作

  static UISystem &instance() {
    static UISystem instance_;
    return instance_;
  }

  QColor theme_color() const { return QColor{"#D3E3F3"}; }

  QIcon home_icon() const { return QIcon{":/icons/home_icon_line"}; }
  QIcon min_icon() const { return QIcon{":/icons/min_icon_line"}; }
  QIcon max_icon() const { return QIcon{":/icons/fullscreen_icon_line"}; }
  QIcon information_icon() const {
    return QIcon{":/icons/information_icon_line"};
  }
  QIcon settings_icon() const { return QIcon{":/icons/settings_icon_line"}; }
  QIcon close_icon() const { return QIcon{":/icons/close_icon_line"}; }
  QIcon user_icon() const { return QIcon{":/icons/user_icon_line"}; }
  QIcon function_icon() const { return QIcon{":/icons/function_icon_line"}; }

  QIcon home_icon_checked() const { return QIcon{":/icons/home_icon_fill"}; }

  QIcon function_icon_checked() const {
    return QIcon{":/icons/function_icon_fill"};
  }
  QIcon information_icon_checked() const {
    return QIcon{":/icons/information_icon_fill"};
  }
  QIcon settings_icon_checked() const {
    return QIcon{":/icons/settings_icon_fill"};
  }

private:
  UISystem() = default;
};