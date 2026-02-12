#pragma once

#include <QColor>
#include <QIcon>
#include <QMap>
#include <QObject>
#include <QString>

class UISystem : public QObject {
  Q_OBJECT
public:
  UISystem(const UISystem &) = delete;
  UISystem &operator=(const UISystem &) = delete;

  static UISystem &instance() {
    static UISystem instance;
    return instance;
  }

  // background
  QColor bg_primary() const { return QColor{"#4285F4"}; }
  QColor bg_active() const { return QColor{"#D3E3FD"}; }
  QColor bg_hover() const { return QColor{"#F0F0F0"}; }

  // content
  QColor content_white() const { return Qt::white; }
  QColor icon_normal() const { return QColor{"#5F6368"}; }
  QColor icon_active() const { return QColor{"#1A73E8"}; }
  QColor line() const { return QColor{"#E0E0E0"}; }
  QColor text_primary() const { return QColor{"#333333"}; }
  QColor text_secondary() const { return QColor{"#5F6368"}; }
  QColor surface() const { return QColor{"#FFFFFF"}; }
  QColor status_error() const { return QColor{"#B3261E"}; }
  QColor status_warning() const { return QColor{"#F57F17"}; }
  QColor status_success() const { return QColor{"#198754"}; }
  QColor neutral() const { return QColor{"#9E9E9E"}; }

  QMap<QString, QString> qss_tokens() const {
    return {
        {"PRIMARY", bg_primary().name()},
        {"TEXT_PRIMARY", text_primary().name()},
        {"TEXT_SECONDARY", text_secondary().name()},
        {"SURFACE", surface().name()},
        {"LINE", line().name()},
        {"ERROR", status_error().name()},
        {"WARNING", status_warning().name()},
        {"SUCCESS", status_success().name()},
        {"NEUTRAL", neutral().name()},
    };
  }

  QString render_qss(QString qss) const {
    const auto tokens = qss_tokens();
    for (auto it = tokens.cbegin(); it != tokens.cend(); ++it) {
      qss.replace(QString("${%1}").arg(it.key()), it.value());
    }
    return qss;
  }

  QColor message_action_color(const QString &msg_type) const {
    if (msg_type == "error")
      return status_error();
    if (msg_type == "warning")
      return status_warning();
    if (msg_type == "success")
      return status_success();
    return bg_primary();
  }

  // icon
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
