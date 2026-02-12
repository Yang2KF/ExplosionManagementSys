#pragma once

#include <QWidget>

class UserPage : public QWidget {
  Q_OBJECT
public:
  explicit UserPage(QWidget *parent = nullptr);

private:
  void init_ui();
};