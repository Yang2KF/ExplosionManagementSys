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
  void onNavBtnClicked(int id);

private:
  void init_ui();
  void setup_sidebar();
  void setup_header();
  void setup_content();

  // UI 组件
  QHBoxLayout *main_layout_;    // 全局水平布局
  QVBoxLayout *sidebar_layout_; // 左侧垂直布局
  QVBoxLayout *content_layout_; // 右侧垂直布局 (包含 Header + Stack)

  QStackedWidget *pages_stack_; // 扑克牌堆栈，用于切换页面
  QButtonGroup *nav_group_;     // 按钮组，实现“单选”效果

  // 状态
  bool is_logged_;
};