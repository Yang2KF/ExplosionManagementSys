#pragma once

#include <QColor>
#include <QLineEdit>
#include <QPropertyAnimation>

class QAction;

class MaterialInput : public QLineEdit {
  Q_OBJECT
  Q_PROPERTY(float lineProgress READ line_progress WRITE set_line_progress)

public:
  enum InputType { Edit, Search };
  Q_ENUM(InputType)
  explicit MaterialInput(InputType type = Edit, QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *event) override;
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;

private:
  void update_search_icon();

  float line_progress() const { return line_progress_; }
  void set_line_progress(float p);

  InputType input_type_;
  float line_progress_;
  QColor active_color_; // 激活时颜色
  QAction *search_action_{nullptr};
  QPropertyAnimation *line_animation_{nullptr};
};
