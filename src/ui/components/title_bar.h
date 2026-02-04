#pragma once

#include <QToolButton>
#include <QWidget>

class TitleBar : public QWidget {
  Q_OBJECT
public:
  explicit TitleBar(QWidget *parent = nullptr);

signals:
  void minClicked();
  void maxClicked();
  void closeClicked();

private:
  void init_ui();
  QToolButton *create_btn(const QIcon &icon);
};