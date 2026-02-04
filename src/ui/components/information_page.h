#pragma once

#include <QWidget>

class InformationPage : public QWidget {
  Q_OBJECT
public:
  explicit InformationPage(QWidget *parent = nullptr);

private:
  void init_ui();
};