#include "main_window.h"
#include "components/function_page.h"
#include "components/home_page.h"
#include "components/information_page.h"
#include "components/m_button.h"
#include "components/setting_page.h"
#include "components/title_bar.h"
#include "components/ui_system.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QToolButton>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : FramelessWidget(parent), is_logged_(false) {

  resize(1024, 768);

  init_ui();
  // 默认显示 "Home" 页
  pages_stack_->setCurrentIndex(0);

  this->dumpObjectTree(); // debug
}

void MainWindow::init_ui() {
  // 1. 基础布局结构
  this->setStyleSheet("background-color: #c00303;"); // 纯白背景

  // 全局左右分栏
  main_layout_ = new QHBoxLayout(this);
  main_layout_->setContentsMargins(0, 0, 0, 0); // 无边距，贴边
  main_layout_->setSpacing(0);

  // 左侧 Sidebar 容器
  QWidget *sidebar_widget = new QWidget(this);
  sidebar_widget->setFixedWidth(80); // 侧边栏固定宽度
  sidebar_widget->setStyleSheet(
      "background-color: #F5F6F7; border-right: 1px solid #E0E0E0;");
  sidebar_layout_ = new QVBoxLayout(sidebar_widget);
  sidebar_layout_->setContentsMargins(10, 20, 10, 20); // 上下留白
  sidebar_layout_->setSpacing(15);

  // 右侧 Content 容器
  QWidget *content_widget = new QWidget(this);
  content_layout_ = new QVBoxLayout(content_widget);
  content_layout_->setContentsMargins(0, 0, 0, 0); // 内容区留白
  content_layout_->setSpacing(0);

  // 将左右容器加入主布局
  main_layout_->addWidget(sidebar_widget);
  main_layout_->addWidget(content_widget);

  // 2. 填充内容
  setup_sidebar();
  setup_header();
  setup_content();
}

void MainWindow::setup_sidebar() {
  nav_group_ = new QButtonGroup(this);
  nav_group_->setExclusive(true); // 确保只有一个按钮被选中

  struct NavInfo {
    int id;
    QIcon icon;
    QString tool_tip;
  };

  QList<NavInfo> nav_lists{
      {0, UISystem::instance()->user_icon(), "user"},
      {1, UISystem::instance()->home_icon(), "home"},
      {2, UISystem::instance()->function_icon(), "function"},
      {3, UISystem::instance()->settings_icon(), "settings"},
      {4, UISystem::instance()->information_icon(), "information"}};

  for (const auto &nav_btn : nav_lists) {
    MaterialButton *btn = new MaterialButton(this);
    btn->setFixedSize(50, 50);
    btn->setIcon(nav_btn.icon);
    btn->setIconSize(QSize{35, 35});
    btn->setCheckable(true);

    nav_group_->addButton(btn, nav_btn.id);
    sidebar_layout_->addWidget(btn);

    if (nav_btn.id == 0)
      sidebar_layout_->addSpacing(20); // 用户下稍微加点距离
  }

  // 连接导航组信号
  // 注意：Qt6中 QButtonGroup::buttonClicked(int) 已过时，用 idClicked
  connect(nav_group_, &QButtonGroup::idClicked, this,
          &MainWindow::onNavBtnClicked);

  sidebar_layout_->addStretch();
}

void MainWindow::setup_header() {
  TitleBar *title_bar = new TitleBar(this);
  // title_bar->setStyleSheet("background-color: #ead5d5"); // debug
  set_drag_bar(title_bar);
  content_layout_->addWidget(title_bar);

  connect(title_bar, &TitleBar::minClicked, this, &MainWindow::showMinimized);
  connect(title_bar, &TitleBar::maxClicked, this, [this]() {
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());

    // 如果当前最大化，点击恢复；否则最大化
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwnd, &wp);

    if (wp.showCmd == SW_MAXIMIZE) {
      ShowWindow(hwnd, SW_RESTORE);
    } else {
      ShowWindow(hwnd, SW_MAXIMIZE);
    }
#endif
  });
  connect(title_bar, &TitleBar::closeClicked, this, &MainWindow::close);
}

void MainWindow::setup_content() {
  pages_stack_ = new QStackedWidget(this);

  pages_stack_->setContentsMargins(10, 10, 10, 10);

  pages_stack_->addWidget(new HomePage(this));        // Index 0
  pages_stack_->addWidget(new FunctionPage(this));    // Index 1
  pages_stack_->addWidget(new SettingPage(this));     // Index 2
  pages_stack_->addWidget(new InformationPage(this)); // Index 3

  content_layout_->addWidget(pages_stack_);
}

// --- 逻辑处理 ---

void MainWindow::onNavBtnClicked(int id) {
  // 切换右侧堆栈页面
  pages_stack_->setCurrentIndex(id);
  // 改变按钮颜色
}