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

MaterialButton::MaterialButton(QWidget *parent) : QPushButton(parent) {
  base_color_ = QColor{"#4285F4"};
  hover_color_ = base_color_.lighter(110);
  pressed_color_ = base_color_.darker(110);
  current_color_ = base_color_;
  setCursor(Qt::PointingHandCursor);

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
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // 确定背景颜色
  QColor bgColor = current_color_;
  if (isCheckable()) {
    if (isChecked())
      bgColor = QColor("#D3E3FD");
    else if (underMouse())
      bgColor = QColor(240, 240, 240);
    else
      bgColor = Qt::transparent;
  } else {
    // 如果是普通按钮（比如登录按钮），悬停时变深一点
    if (underMouse())
      bgColor = bgColor.darker(110);
  }

  //  绘制背景
  painter.setPen(Qt::NoPen);
  painter.setBrush(bgColor);
  int radius = 8; // 默认值
  if (isCheckable()) {
    radius = height() / 2; // 侧边栏按钮：完全半圆
  } else {
    radius = 12; // 登录按钮：大圆角
  }
  painter.drawRoundedRect(rect(), radius, radius);

  QColor contentColor =
      (isCheckable() && isChecked()) ? QColor("#0B57D0") : Qt::white;

  // 绘制文字
  if (!text().isEmpty()) {
    painter.setPen(contentColor);
    painter.setFont(this->font());
    painter.drawText(rect(), Qt::AlignCenter, text());
  }

  // 绘制图标
  if (!icon().isNull()) {
    QPixmap pixmap = icon().pixmap(iconSize());

    // 如果被选中，把图标强制涂成蓝色；否则保持灰色
    QPainter pixPainter(&pixmap);
    pixPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pixPainter.fillRect(pixmap.rect(),
                        isChecked() ? QColor("#0B57D0") : QColor("#747775"));
    pixPainter.end();

    // 居中绘制
    int x = (width() - iconSize().width()) / 2;
    int y = (height() - iconSize().height()) / 2;
    painter.drawPixmap(x, y, pixmap);
  }
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