#include "m_input.h"
#include <QPainter>

MaterialInput::MaterialInput(QWidget *parent)
    : QLineEdit(parent), line_progress_(0.0) {

  setFixedHeight(40); // 留出足够的高度画底部线条

  accent_color_ = QColor{"#4285F4"};
  line_animation_ = new QPropertyAnimation(this, "lineProgress");
  line_animation_->setDuration(300);
  line_animation_->setEasingCurve(QEasingCurve::OutCurve);
}

void MaterialInput::paintEvent(QPaintEvent *event) {
  // 先绘制原生输入框
  QLineEdit::paintEvent(event);

  QPainter painter(this);
  // 绘制底部的灰色背景线
  painter.setPen(QPen{QColor{"#E0E0E0"}, 1});
  painter.drawLine(0, height() - 1, width(), height() - 1);

  // 绘制中间向两边扩散的蓝色激活线
  if (line_progress_ > 0.0) {
    painter.setPen(QPen{accent_color_, 2}); // 激活时线宽为2

    int center = width() / 2;
    int halfWidth = (width() * line_progress_) / 2;

    // 从中心向两边画线
    painter.drawLine(center - halfWidth, height() - 1, center + halfWidth,
                     height() - 1);
  }
}

void MaterialInput::focusInEvent(QFocusEvent *event) {
  QLineEdit::focusInEvent(event); // 保持光标显示
  // 启动动画：从中心展开
  line_animation_->stop();
  line_animation_->setStartValue(0.0);
  line_animation_->setEndValue(1.0);
  line_animation_->start();
}

void MaterialInput::focusOutEvent(QFocusEvent *event) {
  QLineEdit::focusOutEvent(event);
  // 启动动画：收缩回中心
  line_animation_->stop();
  line_animation_->setStartValue(1.0);
  line_animation_->setEndValue(0.0);
  line_animation_->start();
}

void MaterialInput::set_line_progress(float p) {
  line_progress_ = p;
  update(); // 触发重绘
}