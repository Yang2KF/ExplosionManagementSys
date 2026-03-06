#include "main_window.h"
#include "components/function_page.h"
#include "components/home_page.h"
#include "components/information_page.h"
#include "components/login_dialog.h"
#include "components/mask_widget.h"
#include "components/setting_page.h"
#include "components/ui_system.h"
#include "components/user_page.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) : FramelessWidget(parent) {
  resize(1024, 768);

  init_ui();
  pages_stack_->setCurrentIndex(Page_Home);
  tab_bar_->setCurrentIndex(Page_Home);
  current_page_index_ = Page_Home;
}

void MainWindow::init_ui() {
  main_layout_ = new QVBoxLayout(this);
  main_layout_->setContentsMargins(0, 0, 0, 0);
  main_layout_->setSpacing(0);

  setup_header();

  QWidget *content_container = new QWidget(this);
  content_container->setObjectName("ContentContainer");
  content_layout_ = new QVBoxLayout(content_container);
  content_layout_->setContentsMargins(0, 0, 0, 0);
  content_layout_->setSpacing(0);
  main_layout_->addWidget(content_container, 1);

  MaskWidget::instance(this);

  setup_content();
}

void MainWindow::setup_header() {
  title_bar_ = new TitleBar(this);
  set_drag_bar(title_bar_);
  main_layout_->addWidget(title_bar_);

  setup_tabs();

  connect(title_bar_, &TitleBar::minClicked, this, &MainWindow::showMinimized);
  connect(title_bar_, &TitleBar::maxClicked, this, [this]() {
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());

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
  connect(title_bar_, &TitleBar::closeClicked, this, &MainWindow::close);
}

void MainWindow::setup_tabs() {
  tab_bar_ = new MainTabBar(title_bar_);
  tab_bar_->addTab(QStringLiteral("\u7528\u6237"), UISystem::instance().user_icon(),
                   UISystem::instance().user_icon());
  tab_bar_->addTab(QStringLiteral("\u4e3b\u9875"), UISystem::instance().home_icon(),
                   UISystem::instance().home_icon_checked());
  tab_bar_->addTab(QStringLiteral("\u7b97\u6cd5"),
                   UISystem::instance().function_icon(),
                   UISystem::instance().function_icon_checked());
  tab_bar_->addTab(QStringLiteral("\u8bbe\u7f6e"),
                   UISystem::instance().settings_icon(),
                   UISystem::instance().settings_icon_checked());
  tab_bar_->addTab(QStringLiteral("\u5e2e\u52a9"),
                   UISystem::instance().information_icon(),
                   UISystem::instance().information_icon_checked());

  connect(tab_bar_, &MainTabBar::tabRequested, this, &MainWindow::onTabRequested);
  title_bar_->setCenterWidget(tab_bar_);
}

void MainWindow::setup_content() {
  pages_stack_ = new SlideStackedWidget(this);
  pages_stack_->setContentsMargins(10, 10, 10, 10);
  pages_stack_->setOrientation(SlideStackedWidget::Orientation::Horizontal);
  pages_stack_->setSlideDuration(220);

  pages_stack_->insertWidget(Page_User, new UserPage(this));
  pages_stack_->insertWidget(Page_Home, new HomePage(this));
  pages_stack_->insertWidget(Page_Function, new FunctionPage(this));
  pages_stack_->insertWidget(Page_Setting, new SettingPage(this));
  pages_stack_->insertWidget(Page_Info, new InformationPage(this));

  content_layout_->addWidget(pages_stack_, 1);
}

void MainWindow::onTabRequested(int id) {
  if (!pages_stack_ || !tab_bar_) {
    return;
  }

  if (id < 0 || id >= pages_stack_->count()) {
    tab_bar_->setCurrentIndex(current_page_index_);
    return;
  }

  if (id == current_page_index_) {
    tab_bar_->setCurrentIndex(current_page_index_);
    return;
  }

  if (pages_stack_->isAnimating()) {
    tab_bar_->setCurrentIndex(current_page_index_);
    return;
  }

  if (id == Page_User && !is_logged_) {
    LoginDialog loginDlg(this);
    if (loginDlg.exec() != QDialog::Accepted) {
      tab_bar_->setCurrentIndex(current_page_index_);
      return;
    }
    is_logged_ = true;
  }

  tab_bar_->setCurrentIndex(id);
  pages_stack_->slideToIndex(id);
  current_page_index_ = id;
}
