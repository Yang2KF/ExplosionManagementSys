#pragma once

#include <QColor>
#include <QPropertyAnimation>
#include <QPushButton>

class MaterialButton : public QPushButton {
  Q_OBJECT
  Q_PROPERTY(
      QColor backgroundColor READ background_color WRITE set_background_color)
public:
  explicit MaterialButton(const QString &text, QWidget *parent = nullptr);
  explicit MaterialButton(QWidget *parent = nullptr);
  void set_theme_color(const QColor &color);

protected:
  // 绘制函数
  void paintEvent(QPaintEvent *event) override;

  // 鼠标事件
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  // getter/setter
  QColor background_color() const { return current_color_; }
  void set_background_color(const QColor &color);

  QColor base_color_;
  QColor hover_color_;
  QColor pressed_color_;
  QColor current_color_;

  QPropertyAnimation *color_animation_;
};