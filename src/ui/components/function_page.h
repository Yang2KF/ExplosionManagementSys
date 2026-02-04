#pragma once

#include <QWidget>

class FunctionPage : public QWidget {
  Q_OBJECT
public:
  explicit FunctionPage(QWidget *parent = nullptr);

private:
  void init_ui();
};