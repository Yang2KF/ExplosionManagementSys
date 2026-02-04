#pragma once

#include "components/frameless_widget.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public FramelessWidget {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  // 处理导航跳转
  void onSiderBtnClicked(int id);

private:
  void init_ui();
  void setup_sidebar();
  void setup_header();
  void setup_content();

  // UI 组件
  QHBoxLayout *main_layout_;    // 全局水平布局
  QVBoxLayout *content_layout_; // 右侧垂直布局 (包含 Header + Stack)

  QStackedWidget *pages_stack_;

  // 状态
  bool is_logged_;
};