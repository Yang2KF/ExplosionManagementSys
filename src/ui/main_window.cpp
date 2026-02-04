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
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>
#endif

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), is_logged_(false) {

  setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

  setAttribute(Qt::WA_TranslucentBackground, false);
  setAttribute(Qt::WA_NoSystemBackground);

  resize(1024, 768);

  init_ui();

  // 默认显示 "Home" 页
  pages_stack_->setCurrentIndex(0);

  this->dumpObjectTree(); // debug
}

void MainWindow::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);

#ifdef Q_OS_WIN
  HWND hwnd = (HWND)winId();
  DWORD style = GetWindowLong(hwnd, GWL_STYLE);
  // WS_THICKFRAME: 开启缩放边框和原生阴影
  // WS_CAPTION: 允许系统识别标题栏，支持 Snap Layout 分屏
  SetWindowLong(hwnd, GWL_STYLE,
                style | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX |
                    WS_MINIMIZEBOX);

  // 开启 Win11 标准圆角
  DWORD corner = 2;
  DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner,
                        sizeof(corner));
#endif
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message,
                             qintptr *result) {
#ifdef Q_OS_WIN
  MSG *msg = static_cast<MSG *>(message);
  switch (msg->message) {
  case WM_NCCALCSIZE: {
    // 告诉 Windows：客户区覆盖整个窗口，不需要画默认标题栏
    *result = 0;
    return true;
  }
  case WM_NCHITTEST: {
    const LONG border_width = 8; // 边缘缩放感应区
    RECT winrect;
    GetWindowRect(msg->hwnd, &winrect);
    long x = GET_X_LPARAM(msg->lParam);
    long y = GET_Y_LPARAM(msg->lParam);

    // DPI 适配：将鼠标物理坐标转换为逻辑判断
    bool left = x < winrect.left + border_width;
    bool right = x > winrect.right - border_width;
    bool top = y < winrect.top + border_width;
    bool bottom = y > winrect.bottom - border_width;

    if (top && left)
      *result = HTTOPLEFT;
    else if (top && right)
      *result = HTTOPRIGHT;
    else if (bottom && left)
      *result = HTBOTTOMLEFT;
    else if (bottom && right)
      *result = HTBOTTOMRIGHT;
    else if (left)
      *result = HTLEFT;
    else if (right)
      *result = HTRIGHT;
    else if (top)
      *result = HTTOP;
    else if (bottom)
      *result = HTBOTTOM;
    // 标题栏判断：假设顶部 60 像素为拖拽区
    else if (y < winrect.top + 60) {
      // 检查是否点到了搜索框或按钮（这些需要响应点击，不能识别为拖拽）
      // 简单处理：如果是在右侧按钮区，则返回 HTCLIENT 让 Qt 处理点击
      if (x > winrect.right - 120) {
        *result = HTCLIENT;
        return false;
      }
      *result = HTCAPTION; // 识别为标题栏，支持拖动和双击最大化
    } else {
      *result = HTCLIENT;
    }
    return true;
  }
  }
#endif
  return QWidget::nativeEvent(eventType, message, result);
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
  content_layout_->addWidget(title_bar);

  connect(title_bar, &TitleBar::minClicked, this, &MainWindow::showMinimized);
  connect(title_bar, &TitleBar::maxClicked, this,
          [this]() { isMaximized() ? showNormal() : showMaximized(); });
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