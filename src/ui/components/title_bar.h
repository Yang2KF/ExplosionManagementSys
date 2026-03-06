#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QWidget>

class TitleBar : public QWidget {
  Q_OBJECT
public:
  explicit TitleBar(QWidget *parent = nullptr);
  void setCenterWidget(QWidget *widget);

signals:
  void minClicked();
  void maxClicked();
  void closeClicked();

private:
  void init_ui();
  QToolButton *create_btn(const QIcon &icon);
  QHBoxLayout *layout_ = nullptr;
  QHBoxLayout *center_layout_ = nullptr;
  QLabel *title_label_ = nullptr;
  QWidget *center_host_ = nullptr;
};
