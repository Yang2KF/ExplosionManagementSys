#pragma once

#include <QPropertyAnimation>
#include <QWidget>

class MaskWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(float opacity READ opacity WRITE setOpacity)

public:
  static MaskWidget *instance(QWidget *parent = nullptr);

  void show_mask();

  void hide_mask();

protected:
  void paintEvent(QPaintEvent *event) override;
  // 事件过滤器：用于监听父窗口大小变化
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  explicit MaskWidget(QWidget *parent = nullptr);
  ~MaskWidget();

  float opacity() const { return opacity_; }
  void setOpacity(float opacity);

  static MaskWidget *self_;  // 单例指针
  float opacity_;            // 当前透明度 (0.0 - 1.0)
  QPropertyAnimation *anim_; // 动画对象

  int ref_count_ = 0;
};