#pragma once

#include <QColor>
#include <QLineEdit>
#include <QPropertyAnimation>

class MaterialInput : public QLineEdit {
  Q_OBJECT
  Q_PROPERTY(float lineProgress READ line_progress WRITE set_line_progress)

public:
  explicit MaterialInput(QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *event) override;
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;

private:
  float line_progress() const { return line_progress_; }
  void set_line_progress(float p);

  float line_progress_;
  QColor accent_color_; // 激活时颜色
  QPropertyAnimation *line_animation_;
};