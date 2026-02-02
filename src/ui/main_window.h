#pragma once

#include "components/m_button.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

protected:
  bool nativeEvent(const QByteArray &eventType, void *message,
                   qintptr *result) override;
  void showEvent(QShowEvent *event) override;

private slots:
  // 处理用户头像点击
  void onUserBtnClicked();
  // 处理导航跳转
  void onNavBtnClicked(int id);

private:
  void init_ui();
  void setup_sidebar();
  void setup_header();
  void setup_content();

  // 辅助函数：创建一个侧边栏按钮
  MaterialButton *create_navbtn(const QIcon &path, int id);

  // UI 组件
  QWidget *central_widget_;
  QHBoxLayout *main_layout_;    // 全局水平布局
  QVBoxLayout *sidebar_layout_; // 左侧垂直布局
  QVBoxLayout *content_layout_; // 右侧垂直布局 (包含 Header + Stack)

  QStackedWidget *pages_stack_; // 扑克牌堆栈，用于切换页面
  QButtonGroup *nav_group_;     // 按钮组，实现“单选”效果

  // 状态
  bool is_logged_;
  MaterialButton *user_btn_; // 单独引用用户按钮，因为要变色/变图标
};