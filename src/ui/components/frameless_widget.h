#pragma once

#include <QWidget>

class FramelessWidget : public QWidget {
  Q_OBJECT
public:
  explicit FramelessWidget(QWidget *parent = nullptr);

protected:
  void set_drag_bar(QWidget *bar);

  bool nativeEvent(const QByteArray &eventType, void *message,
                   qintptr *result) override;
  void showEvent(QShowEvent *event) override;

private:
  QWidget *drag_bar_;
};