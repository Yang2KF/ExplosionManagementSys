#include "main_window.h"
#include "components/function_page.h"
#include "components/home_page.h"
#include "components/information_page.h"
#include "components/login_dialog.h"
#include "components/setting_page.h"
#include "components/side_bar.h"
#include "components/title_bar.h"
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
  this->setStyleSheet("background-color: #c00303;"); // 纯白背景

  // 全局左右分栏
  main_layout_ = new QHBoxLayout(this);
  main_layout_->setContentsMargins(0, 0, 0, 0); // 无边距，贴边
  main_layout_->setSpacing(0);

  setup_sidebar();

  // Content 容器
  QWidget *content_widget = new QWidget(this);
  content_layout_ = new QVBoxLayout(content_widget);
  content_layout_->setContentsMargins(0, 0, 0, 0); // 内容区留白
  content_layout_->setSpacing(0);

  main_layout_->addWidget(content_widget);

  setup_header();
  setup_content();
}

void MainWindow::setup_sidebar() {
  SideBar *sider_bar = new SideBar();
  sider_bar->setFixedWidth(80);
  sider_bar->setStyleSheet(
      "background-color: #F5F6F7; border-right: 1px solid #E0E0E0;");
  main_layout_->addWidget(sider_bar);

  connect(sider_bar, &SideBar::onSiderBtnRequest, this,
          &MainWindow::onSiderBtnClicked);
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

void MainWindow::onSiderBtnClicked(int id) {
  // 切换右侧堆栈页面
  if (id == 0) {
    if (is_logged_) {
      // 如果已登录，显示个人信息
      qDebug() << "Show User Profile";
    } else {
      // 如果未登录，弹出登录框
      LoginDialog loginDlg(this);
      if (loginDlg.exec() == QDialog::Accepted) {
        is_logged_ = true;
      }
    }
  } else
    pages_stack_->setCurrentIndex(id - 1);
}