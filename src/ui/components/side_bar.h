#pragma once

#include <QWidget>

class SideBar : public QWidget {
  Q_OBJECT
public:
  explicit SideBar(QWidget *parent = nullptr);

signals:
  void onSiderBtnRequest(int id);

private:
  void init_ui();
};