#pragma once

#include <QColor>
#include <QIcon>
#include <QObject>

class UISystem : public QObject {
  Q_OBJECT
public:
  static UISystem *instance() {
    static UISystem instance_;
    return &instance_;
  }

  QColor theme_color() const { return QColor{"#D3E3F3"}; }

  QIcon home_icon() const { return QIcon{":/icons/home_icon"}; }
  QIcon min_icon() const { return QIcon{":/icons/min_icon"}; }
  QIcon max_icon() const { return QIcon{":/icons/fullscreen_icon"}; }
  QIcon information_icon() const { return QIcon{":/icons/information_icon"}; }
  QIcon settings_icon() const { return QIcon{":/icons/settings_icon"}; }
  QIcon close_icon() const { return QIcon{":/icons/close_icon"}; }
  QIcon user_icon() const { return QIcon{":/icons/user_icon"}; }
  QIcon function_icon() const { return QIcon{":/icons/function_icon"}; }

private:
  UISystem() = default;
};