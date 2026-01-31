#include "m_button.h"
#include <QPainter>
#include <QPainterPath>

MaterialButton::MaterialButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent) {
  base_color_ = QColor{"#4285F4"};
  hover_color_ = base_color_.lighter(110);
  pressed_color_ = base_color_.darker(110);
  current_color_ = base_color_;

  setMinimumSize(100, 40);

  // 所监测的color
  color_animation_ = new QPropertyAnimation(this, "backgroundColor");
  color_animation_->setDuration(200);
  color_animation_->setEasingCurve(QEasingCurve::OutQuad);
}

void MaterialButton::set_theme_color(const QColor &color) {
  base_color_ = color;
  hover_color_ = color.lighter(110);
  pressed_color_ = color.darker(110);
  set_background_color(color);
}

void MaterialButton::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event); // 消除未使用参数event的警告

  QPainter painter{this};
  painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

  QPainterPath path;
  path.addRoundedRect(rect(), 6, 6);
  painter.fillPath(path, current_color_);

  painter.setPen(Qt::white);
  painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
  painter.drawText(rect(), Qt::AlignCenter, text());
}

void MaterialButton::set_background_color(const QColor &color) {
  current_color_ = color;
  update();
}

void MaterialButton::enterEvent(QEnterEvent *event) {
  Q_UNUSED(event);
  color_animation_->stop();
  color_animation_->setEndValue(hover_color_);
  color_animation_->start();
}

void MaterialButton::leaveEvent(QEvent *event) {
  Q_UNUSED(event);
  color_animation_->stop();
  color_animation_->setEndValue(base_color_);
  color_animation_->start();
}

void MaterialButton::mousePressEvent(QMouseEvent *event) {
  QPushButton::mousePressEvent(event);
  color_animation_->stop();
  color_animation_->setEndValue(pressed_color_);
  color_animation_->start();
}

void MaterialButton::mouseReleaseEvent(QMouseEvent *event) {
  QPushButton::mouseReleaseEvent(event);
  color_animation_->stop();
  color_animation_->setEndValue(hover_color_);
  color_animation_->start();
}