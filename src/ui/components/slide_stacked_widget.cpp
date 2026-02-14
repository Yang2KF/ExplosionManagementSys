#include "slide_stacked_widget.h"
#include <QAbstractAnimation>
#include <QPropertyAnimation>

SlideStackedWidget::SlideStackedWidget(QWidget *parent) : QStackedWidget(parent) {}

void SlideStackedWidget::slideToIndex(int target_index) {
  if (is_animating_) {
    return;
  }

  const int current_index = currentIndex();
  if (target_index == current_index) {
    return;
  }
  if (target_index < 0 || target_index >= count()) {
    return;
  }

  QWidget *current_page = widget(current_index);
  QWidget *next_page = widget(target_index);
  if (!current_page || !next_page) {
    return;
  }

  const QRect base_rect = current_page->geometry();
  const int direction = (target_index > current_index) ? 1 : -1;

  QPoint offset;
  if (orientation_ == Orientation::Vertical) {
    offset = QPoint(0, direction * base_rect.height());
  } else {
    offset = QPoint(direction * base_rect.width(), 0);
  }

  next_page->setGeometry(base_rect.translated(offset));
  next_page->show();
  next_page->raise();

  QPropertyAnimation *current_anim =
      new QPropertyAnimation(current_page, "geometry");
  current_anim->setDuration(duration_ms_);
  current_anim->setStartValue(base_rect);
  current_anim->setEndValue(base_rect.translated(-offset));
  current_anim->setEasingCurve(QEasingCurve::OutCubic);

  QPropertyAnimation *next_anim = new QPropertyAnimation(next_page, "geometry");
  next_anim->setDuration(duration_ms_);
  next_anim->setStartValue(base_rect.translated(offset));
  next_anim->setEndValue(base_rect);
  next_anim->setEasingCurve(QEasingCurve::OutCubic);

  if (anim_group_) {
    if (anim_group_->state() == QAbstractAnimation::Running) {
      anim_group_->stop();
    }
    anim_group_->deleteLater();
    anim_group_ = nullptr;
  }

  anim_group_ = new QParallelAnimationGroup(this);
  anim_group_->addAnimation(current_anim);
  anim_group_->addAnimation(next_anim);

  is_animating_ = true;
  connect(anim_group_, &QParallelAnimationGroup::finished, this,
          [this, target_index, current_page, next_page, base_rect]() {
            setCurrentIndex(target_index);
            current_page->setGeometry(base_rect);
            next_page->setGeometry(base_rect);
            is_animating_ = false;

            anim_group_->deleteLater();
            anim_group_ = nullptr;
          });

  anim_group_->start();
}

