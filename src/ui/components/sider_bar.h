#pragma once

#include <QWidget>

class SiderBar : public QWidget {
  Q_OBJECT
public:
  explicit SiderBar(QWidget *parent = nullptr);

signals:
  void onSiderBtnRequest(int id);

private:
  void init_ui();
};