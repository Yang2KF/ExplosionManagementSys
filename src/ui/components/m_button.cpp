#include "m_button.h"
#include "ui_system.h"
#include <QPainter>
#include <QPainterPath>

MaterialButton::MaterialButton(const QString &text, Type type, QWidget *parent)
    : QPushButton(text, parent), type_(type) {
  init();
}

MaterialButton::MaterialButton(Type type, QWidget *parent)
    : QPushButton(parent), type_(type) {
  init();
}

void MaterialButton::init() {
  if (type_ == WithIcon) {
    setCheckable(true);
    setFixedSize(50, 50);
  } else {
    setMinimumSize(100, 40);
  }
  setCursor(Qt::PointingHandCursor);

  base_color_ = UISystem::instance().bg_primary();
  hover_color_ = base_color_.lighter(110);
  pressed_color_ = base_color_.darker(110);
  current_bg_color_ = Qt::transparent; // 初始背景透明

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

void MaterialButton::set_icons(const QIcon &normal, const QIcon &checked) {
  icon_normal_ = normal;
  // 如果没有提供 checked 图标，就复用 normal 图标
  icon_checked_ = checked.isNull() ? normal : checked;
  update();
}

void MaterialButton::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // 用于绘制图标的颜色，区别于动画颜色
  QColor bg_color = current_bg_color_;
  int radius = 8;

  if (type_ == WithIcon) {
    radius = height() / 2;

    // 侧边栏按钮的特殊背景逻辑
    if (isChecked()) {
      bg_color = UISystem::instance().bg_active();
    } else if (underMouse()) {
      bg_color = UISystem::instance().bg_hover();
    } else {
      bg_color = Qt::transparent;
    }
  } else {
    // Normal 按钮：直接使用动画计算出的 current_bg_color_
    radius = 8;
  }

  // 绘制背景
  painter.setPen(Qt::NoPen);
  painter.setBrush(bg_color);
  painter.drawRoundedRect(rect(), radius, radius);

  icon_color_normal_ = UISystem::instance().icon_normal();
  icon_color_checked_ = UISystem::instance().icon_active();

  QColor content_color;
  if (type_ == WithIcon) {
    content_color = isChecked() ? icon_color_checked_ : icon_color_normal_;
  } else {
    content_color = UISystem::instance().content_white();
  }

  // 根据状态选择对应的图标对象
  QIcon current_icon =
      (isChecked() && type_ == WithIcon) ? icon_checked_ : icon_normal_;

  // 如果还没设置 icon_normal_，尝试回退到 Qt 自带的 icon() 属性
  if (current_icon.isNull())
    current_icon = icon();

  if (!current_icon.isNull()) {
    // 获取图标对应的 pixmap
    QSize img_size = iconSize();
    QPixmap pixmap = current_icon.pixmap(img_size);

    // 利用 CompositionMode_SourceIn 颜色只会覆盖在图标上
    QPainter pixPainter(&pixmap);
    pixPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pixPainter.fillRect(pixmap.rect(), content_color);
    pixPainter.end();

    // 计算居中位置
    int x = (width() - img_size.width()) / 2;
    int y = (height() - img_size.height()) / 2;
    painter.drawPixmap(x, y, pixmap);
  }

  if (!text().isEmpty()) {
    painter.setPen(content_color);
    painter.setFont(font());
    painter.drawText(rect(), Qt::AlignCenter, text());
  }
}

void MaterialButton::set_background_color(const QColor &color) {
  current_bg_color_ = color;
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