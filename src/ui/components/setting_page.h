#pragma once

#include <QWidget>

class SettingPage : public QWidget {
  Q_OBJECT
public:
  explicit SettingPage(QWidget *parent = nullptr);

private:
  void init_ui();
};