#pragma once

#include <QParallelAnimationGroup>
#include <QStackedWidget>

class SlideStackedWidget : public QStackedWidget {
  Q_OBJECT

public:
  enum class Orientation { Horizontal, Vertical };

  explicit SlideStackedWidget(QWidget *parent = nullptr);

  void setSlideDuration(int duration_ms) { duration_ms_ = duration_ms; }
  void setOrientation(Orientation orientation) { orientation_ = orientation; }
  bool isAnimating() const { return is_animating_; }

public slots:
  void slideToIndex(int target_index);

private:
  QParallelAnimationGroup *anim_group_ = nullptr;
  int duration_ms_ = 240;
  Orientation orientation_ = Orientation::Vertical;
  bool is_animating_ = false;
};

