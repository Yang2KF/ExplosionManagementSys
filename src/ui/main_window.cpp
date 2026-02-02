#include "main_window.h"
#include "login_dialog.h"
#include <QApplication>
#include <QDebug>
#include <QToolButton>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), is_logged_(false) {

  setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

  setAttribute(Qt::WA_TranslucentBackground, false);
  setAttribute(Qt::WA_NoSystemBackground);

  resize(1024, 768);

  init_ui();

  // 默认显示 "Home" 页
  pages_stack_->setCurrentIndex(0);
}

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);

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
  return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::init_ui() {
  // 1. 基础布局结构
  central_widget_ = new QWidget(this);
  setCentralWidget(central_widget_);
  central_widget_->setStyleSheet("background-color: #FFFFFF;"); // 纯白背景

  // 全局左右分栏
  main_layout_ = new QHBoxLayout(central_widget_);
  main_layout_->setContentsMargins(0, 0, 0, 0); // 无边距，贴边
  main_layout_->setSpacing(0);

  // 左侧 Sidebar 容器
  QWidget *sidebarContainer = new QWidget(this);
  sidebarContainer->setFixedWidth(80); // 侧边栏固定宽度
  sidebarContainer->setStyleSheet(
      "background-color: #F5F6F7; border-right: 1px solid #E0E0E0;");
  sidebar_layout_ = new QVBoxLayout(sidebarContainer);
  sidebar_layout_->setContentsMargins(10, 20, 10, 20); // 上下留白
  sidebar_layout_->setSpacing(15);

  // 右侧 Content 容器
  QWidget *contentContainer = new QWidget(this);
  content_layout_ = new QVBoxLayout(contentContainer);
  content_layout_->setContentsMargins(20, 20, 20, 20); // 内容区留白
  content_layout_->setSpacing(20);

  // 将左右容器加入主布局
  main_layout_->addWidget(sidebarContainer);
  main_layout_->addWidget(contentContainer);

  // 2. 填充内容
  setup_sidebar();
  setup_header();
  setup_content();
}

void MainWindow::setup_sidebar() {
  nav_group_ = new QButtonGroup(this);
  nav_group_->setExclusive(true); // 确保只有一个按钮被选中

  // --- 用户按钮 (最上面) ---
  // 这个按钮不属于导航组，点击它有特殊逻辑
  user_btn_ = new MaterialButton(this);
  user_btn_->setFixedSize(50, 50);
  user_btn_->set_theme_color(QColor("#D3E3FD")); // 未登录是灰色
  user_btn_->setIcon(QIcon{":/icons/user_icon"});
  user_btn_->setIconSize(QSize{35, 35});
  connect(user_btn_, &QPushButton::clicked, this,
          &MainWindow::onUserBtnClicked);

  sidebar_layout_->addWidget(user_btn_);
  sidebar_layout_->addSpacing(20); // 分隔线距离

  // home
  // ID对应 m_pagesStack 的索引：0=Home, 1=Grid, 2=Settings
  MaterialButton *home_btn = create_navbtn(QIcon{":/icons/home_icon"}, 0);
  MaterialButton *btn_func = create_navbtn(QIcon{":/icons/function_icon"}, 1);

  // --- 底部按钮 ---
  MaterialButton *settings_btn =
      create_navbtn(QIcon{":/icons/settings_icon"}, 2); // Setting
  MaterialButton *help_btn =
      create_navbtn(QIcon{":/icons/information_icon"}, 3); // Help

  // 连接导航组信号
  // 注意：Qt6中 QButtonGroup::buttonClicked(int) 已过时，用 idClicked
  connect(nav_group_, &QButtonGroup::idClicked, this,
          &MainWindow::onNavBtnClicked);

  // 弹簧撑开，把下面的按钮顶到底部
  sidebar_layout_->addStretch();
}

MaterialButton *MainWindow::create_navbtn(const QIcon &path, int id) {
  MaterialButton *btn = new MaterialButton(this);
  btn->setFixedSize(50, 50);
  btn->setIcon(path);
  btn->setIconSize(QSize{35, 35});
  btn->setCheckable(true); // 允许选中状态
  btn->set_theme_color(QColor("#D3E3FD"));

  nav_group_->addButton(btn, id);
  sidebar_layout_->addWidget(btn);
  return btn;
}

void MainWindow::setup_header() {
  // 对应设计图右上角的 "TITLE    Search   - [] X"
  QHBoxLayout *headerLayout = new QHBoxLayout();
  // headerLayout->setContentsMargins(0, 0, 10, 0); // 右侧留点边距
  headerLayout->setSpacing(7);

  QLabel *title = new QLabel("爆炸毁伤算法管理系统", this);
  title->setFont(QFont("Microsoft YaHei", 15, QFont::Bold));
  title->setStyleSheet("color: #333;");

  // 简易搜索框 (复用我们的 MaterialInput)
  // 实际需要引入 MaterialInput 头文件，这里先用 QWidget 占位演示布局
  QLineEdit *searchBar = new QLineEdit(this);
  searchBar->setPlaceholderText("搜索内容...");
  searchBar->setStyleSheet("border: 1px solid #DDD; border-radius: 4px; "
                           "padding: 4px; background: #F0F2F5;");
  searchBar->setFixedWidth(200);

  headerLayout->addWidget(title);
  headerLayout->addStretch(); // 弹簧
  headerLayout->addWidget(searchBar);

  // 最小化
  QToolButton *min_btn = new QToolButton(this);
  min_btn->setFixedSize(30, 30);
  min_btn->setIcon(QIcon{":/icons/min_icon"});
  min_btn->setIconSize(QSize{25, 25});
  min_btn->setStyleSheet("QToolButton { border: none; background: transparent;"
                         "} QToolButton:hover { background: #E0E0E0; }");

  connect(min_btn, &QToolButton::clicked, this, &MainWindow::showMinimized);

  // 最大化
  QToolButton *max_btn = new QToolButton(this);
  max_btn->setFixedSize(30, 30);
  max_btn->setIcon(QIcon{":/icons/fullscreen_icon"});

  max_btn->setIconSize(QSize{25, 25});

  max_btn->setStyleSheet("QToolButton { border: none; background: transparent;"
                         "} QToolButton:hover { background: #E0E0E0; }");
  connect(max_btn, &QToolButton::clicked, this, [this]() {
    if (this->isMaximized()) {
      this->showNormal();
    } else {
      this->showMaximized();
    }
  });

  // 关闭
  QToolButton *close_btn = new QToolButton(this);
  close_btn->setFixedSize(30, 30);
  close_btn->setIcon(QIcon{":/icons/close_icon"});

  close_btn->setIconSize(QSize{25, 25});
  close_btn->setStyleSheet(
      "QToolButton { border: none; background: transparent; }"
      "QToolButton:hover { background: #FF4D4F; }");

  connect(close_btn, &QToolButton::clicked, this, &MainWindow::close);

  // 加一点间距
  headerLayout->addSpacing(20);
  headerLayout->addWidget(min_btn);
  headerLayout->addWidget(max_btn);
  headerLayout->addWidget(close_btn);

  content_layout_->addLayout(headerLayout);
  content_layout_->addLayout(headerLayout);
}

void MainWindow::setup_content() {
  pages_stack_ = new QStackedWidget(this);

  // Page 0: Home / Dashboard
  QLabel *pageHome = new QLabel("主界面", this);
  pageHome->setAlignment(Qt::AlignCenter);
  pageHome->setStyleSheet(
      "background: white; border-radius: 8px; font-size: 24px;");

  // Page 1: Algorithm Grid
  QLabel *pageGrid = new QLabel("算法配置", this);
  pageGrid->setAlignment(Qt::AlignCenter);
  pageGrid->setStyleSheet(
      "background: white; border-radius: 8px; font-size: 24px;");

  // Page 2: Settings
  QLabel *pageSettings = new QLabel("系统设置", this);
  pageSettings->setAlignment(Qt::AlignCenter);
  pageSettings->setStyleSheet(
      "background: white; border-radius: 8px; font-size: 24px;");

  // Page 3: Help
  QLabel *pageHelp = new QLabel("帮助", this);
  pageHelp->setAlignment(Qt::AlignCenter);
  pageHelp->setStyleSheet(
      "background: white; border-radius: 8px; font-size: 24px;");

  pages_stack_->addWidget(pageHome);     // Index 0
  pages_stack_->addWidget(pageGrid);     // Index 1
  pages_stack_->addWidget(pageSettings); // Index 2
  pages_stack_->addWidget(pageHelp);     // Index 3

  content_layout_->addWidget(pages_stack_);
}

// --- 逻辑处理 ---

void MainWindow::onUserBtnClicked() {
  if (is_logged_) {
    // 如果已登录，显示个人信息
    qDebug() << "Show User Profile";
    // 这里可以弹出一个 Tip 或者跳转到个人中心页
  } else {
    // 如果未登录，弹出登录框
    LoginDialog loginDlg(this);
    if (loginDlg.exec() == QDialog::Accepted) {
      is_logged_ = true;
      user_btn_->set_theme_color(QColor("#34A853")); // 登录成功变绿
      user_btn_->setText("V");                       // 变成 Vip 或用户首字母

      // 可以在这里获取 LoginDialog 里的用户名
      // m_userName = loginDlg.getUserName();
    }
  }
}

void MainWindow::onNavBtnClicked(int id) {
  // 切换右侧堆栈页面
  pages_stack_->setCurrentIndex(id);
  // 改变按钮颜色
}