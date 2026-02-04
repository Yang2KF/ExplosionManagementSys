#include "m_button.h"
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
  // 1. 基础属性设置
  // 侧边栏按钮通常是 Checkable 的
  if (type_ == SideBar) {
    setCheckable(true);
    setFixedSize(50, 50); // 默认大小，外部可以覆盖
  } else {
    setMinimumSize(100, 40);
    setCursor(Qt::PointingHandCursor);
  }

  // 2. 颜色初始化 (默认蓝色系，外部可通过 set_theme_color 修改)
  base_color_ = QColor("#4285F4");
  hover_color_ = base_color_.lighter(110);
  pressed_color_ = base_color_.darker(110);
  current_bg_color_ = Qt::transparent; // 初始背景透明

  // 3. 动画系统
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

  // =================================================
  // 1. 计算背景颜色与形状
  // =================================================
  QColor bg_color = current_bg_color_;
  int radius = 8;

  if (type_ == SideBar) {
    radius = height() / 2; // 圆形或胶囊形

    // 侧边栏按钮的特殊背景逻辑
    if (isChecked()) {
      bg_color = QColor("#D3E3FD"); // 选中时的浅蓝色背景
    } else if (underMouse()) {
      // 悬停时稍微灰一点，不使用动画的 current_bg_color_ 以避免冲突，
      // 或者你可以在 enterEvent 里针对 SideBar 做特殊动画处理。
      // 这里简单处理：
      bg_color = QColor(240, 240, 240);
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

  // =================================================
  // 2. 决定前景色（图标和文字的颜色）
  // =================================================
  QColor content_color;
  if (type_ == SideBar) {
    content_color = isChecked() ? icon_color_checked_ : icon_color_normal_;
  } else {
    content_color = Qt::white; // 普通按钮通常是深底白字
  }

  // =================================================
  // 3. 绘制图标 (关键切换逻辑)
  // =================================================
  // 根据状态选择对应的图标对象
  QIcon current_icon =
      (isChecked() && type_ == SideBar) ? icon_checked_ : icon_normal_;

  // 如果还没设置 icon_normal_，尝试回退到 Qt 自带的 icon() 属性
  if (current_icon.isNull())
    current_icon = icon();

  if (!current_icon.isNull()) {
    // 获取图标对应的 pixmap
    QSize img_size = iconSize();
    QPixmap pixmap = current_icon.pixmap(img_size);

    // --- 核心：图标染色 ---
    // 我们利用 CompositionMode_SourceIn 保持图标形状，只改变颜色
    QPainter pixPainter(&pixmap);
    pixPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pixPainter.fillRect(pixmap.rect(), content_color);
    pixPainter.end();
    // -------------------

    // 计算居中位置
    int x = (width() - img_size.width()) / 2;
    int y = (height() - img_size.height()) / 2;
    painter.drawPixmap(x, y, pixmap);
  }

  // =================================================
  // 4. 绘制文字
  // =================================================
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