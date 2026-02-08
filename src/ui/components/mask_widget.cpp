#include "mask_widget.h"
#include <QDebug>
#include <QEvent>
#include <QPainter>

MaskWidget *MaskWidget::self_ = nullptr;

MaskWidget *MaskWidget::instance(QWidget *parent) {
  if (!self_) {
    // 确保必须传入 parent (MainWindow) 才能创建
    if (!parent)
      return nullptr;
    self_ = new MaskWidget(parent);
  }
  return self_;
}

MaskWidget::MaskWidget(QWidget *parent) : QWidget(parent), opacity_(0.0) {
  this->hide(); // 默认隐藏
  this->setAttribute(Qt::WA_StyledBackground);
  this->setAttribute(Qt::WA_TransparentForMouseEvents, false);

  if (parent) {
    this->setGeometry(parent->rect());
    // 安装事件过滤器，监听父窗口的 Resize 事件
    parent->installEventFilter(this);
  }

  anim_ = new QPropertyAnimation(this, "opacity");
  anim_->setDuration(250);                       // 动画时长 250ms
  anim_->setEasingCurve(QEasingCurve::OutCubic); // 缓动曲线

  connect(anim_, &QPropertyAnimation::finished, this, [this]() {
    // 只有当目标是“隐藏”（即透明度趋于0）时，才执行 hide
    if (this->opacity_ < 0.01f) {
      this->hide();
      qDebug() << "Mask hidden safely.";
    }
  });
}

MaskWidget::~MaskWidget() { self_ = nullptr; }

void MaskWidget::setOpacity(float opacity) {
  if (opacity_ == opacity)
    return;
  opacity_ = opacity;
  update(); // 触发重绘
}

void MaskWidget::show_mask() {
  if (this->isVisible() && opacity_ == 0.4f)
    return;

  // 提到最上层，覆盖所有其他控件
  this->raise();
  this->show();

  // 开始渐显动画 (0.0 -> 0.4)
  anim_->stop();
  anim_->setStartValue(opacity_); // 从当前值开始，防止动画跳跃
  anim_->setEndValue(0.4);        // 最终透明度
  anim_->start();
}

void MaskWidget::hide_mask() {
  // 开始渐隐动画 (Current -> 0.0)
  anim_->stop();
  anim_->setStartValue(opacity_);
  anim_->setEndValue(0.0);
  anim_->start();
}

void MaskWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  // 绘制半透明黑色背景
  // opacity_ * 255 计算 alpha 值
  painter.fillRect(rect(), QColor(0, 0, 0, static_cast<int>(opacity_ * 255)));
}

bool MaskWidget::eventFilter(QObject *obj, QEvent *event) {
  // 如果父窗口大小改变，遮罩也要跟着改变
  if (obj == parent() && event->type() == QEvent::Resize) {
    this->resize(static_cast<QWidget *>(obj)->size());
  }
  return QWidget::eventFilter(obj, event);
}