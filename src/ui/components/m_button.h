#pragma once

#include <QColor>
#include <QPropertyAnimation>
#include <QPushButton>

class MaterialButton : public QPushButton {
  Q_OBJECT
  Q_PROPERTY(
      QColor backgroundColor READ background_color WRITE set_background_color)
public:
  enum Type { Normal, SideBar };
  Q_ENUM(Type)

  explicit MaterialButton(const QString &text, Type type,
                          QWidget *parent = nullptr);
  explicit MaterialButton(Type type, QWidget *parent = nullptr);
  void set_theme_color(const QColor &color);
  void set_icons(const QIcon &normal, const QIcon &checked);

protected:
  // 绘制函数
  void paintEvent(QPaintEvent *event) override;

  // 鼠标事件
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  void init();
  // getter/setter
  QColor background_color() const { return current_bg_color_; }
  void set_background_color(const QColor &color);
  Type type_;

  // 颜色配置 (待改进)
  QColor base_color_;       // 基础色
  QColor hover_color_;      // 悬停色
  QColor pressed_color_;    // 按下色
  QColor current_bg_color_; // 当前背景色（动画插值用）

  QColor text_color_normal_ = QColor("#1f1f1f");  // 普通文字颜色
  QColor text_color_checked_ = QColor("#0B57D0"); // 选中文字颜色 (蓝色)

  QColor icon_color_normal_ = QColor("#444746");  // 未选中图标灰
  QColor icon_color_checked_ = QColor("#0B57D0"); // 选中图标蓝

  QPropertyAnimation *color_animation_ = nullptr;

  QIcon icon_normal_;
  QIcon icon_checked_;
};