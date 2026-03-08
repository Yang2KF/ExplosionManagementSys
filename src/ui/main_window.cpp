#include "main_window.h"
#include "components/algorithm_run_tab.h"
#include "components/function_page.h"
#include "components/mask_widget.h"
#include "components/ui_system.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) : FramelessWidget(parent) {
  resize(1024, 768);

  init_ui();
  pages_stack_->setCurrentIndex(0);
  tab_bar_->setCurrentIndex(0);
  current_page_index_ = 0;
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
  tab_bar_ = new TabBar(title_bar_);
  tab_bar_->addTab(QStringLiteral("算法列表"),
                   UISystem::instance().function_icon(),
                   UISystem::instance().function_icon_checked(), false);

  connect(tab_bar_, &TabBar::tabRequested, this, &MainWindow::onTabRequested);
  connect(tab_bar_, &TabBar::tabCloseRequested, this,
          &MainWindow::onTabCloseRequested);
  title_bar_->setCenterWidget(tab_bar_);
}

void MainWindow::setup_content() {
  pages_stack_ = new SlideStackedWidget(this);
  pages_stack_->setContentsMargins(10, 10, 10, 10);
  pages_stack_->setOrientation(SlideStackedWidget::Orientation::Horizontal);
  pages_stack_->setSlideDuration(220);

  function_page_ = new FunctionPage(this);
  connect(function_page_, &FunctionPage::requestRunTab, this,
          &MainWindow::open_run_tab);

  pages_stack_->addWidget(function_page_);
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

  tab_bar_->setCurrentIndex(id);
  pages_stack_->slideToIndex(id);
  current_page_index_ = id;
}

void MainWindow::onTabCloseRequested(int id) {
  if (!tab_bar_ || !pages_stack_) {
    return;
  }
  if (id <= 0 || id >= pages_stack_->count()) {
    return;
  }

  QWidget *page = pages_stack_->widget(id);
  pages_stack_->removeWidget(page);
  if (page) {
    page->deleteLater();
  }
  tab_bar_->removeTab(id);

  QString removed_key;
  for (auto it = run_tab_indexes_.begin(); it != run_tab_indexes_.end(); ++it) {
    if (it.value() == id) {
      removed_key = it.key();
      break;
    }
  }
  if (!removed_key.isEmpty()) {
    run_tab_indexes_.remove(removed_key);
  }
  for (auto it = run_tab_indexes_.begin(); it != run_tab_indexes_.end(); ++it) {
    if (it.value() > id) {
      it.value() = it.value() - 1;
    }
  }

  int target_index = current_page_index_;
  if (current_page_index_ == id) {
    target_index = qMax(0, id - 1);
  } else if (current_page_index_ > id) {
    target_index = current_page_index_ - 1;
  }

  if (target_index >= pages_stack_->count()) {
    target_index = pages_stack_->count() - 1;
  }

  current_page_index_ = qMax(0, target_index);
  pages_stack_->setCurrentIndex(current_page_index_);
  tab_bar_->setCurrentIndex(current_page_index_);
}

void MainWindow::open_run_tab(const AlgorithmInfo &info) {
  if (!tab_bar_ || !pages_stack_) {
    return;
  }

  const QString key = run_tab_key(info);
  if (key.isEmpty()) {
    return;
  }

  int index = 0;
  if (run_tab_indexes_.contains(key)) {
    index = run_tab_indexes_.value(key);
  } else {
    AlgorithmRunTab *run_tab = new AlgorithmRunTab(info, pages_stack_);
    const int stack_index = pages_stack_->addWidget(run_tab);
    tab_bar_->addTab(run_tab_title(info), UISystem::instance().code_icon(),
                     UISystem::instance().code_icon_checked(), true);
    index = stack_index;
    run_tab_indexes_.insert(key, index);
  }

  onTabRequested(index);
}

QString MainWindow::run_tab_key(const AlgorithmInfo &info) const {
  if (!info.id.trimmed().isEmpty()) {
    return info.id.trimmed();
  }
  return info.name.trimmed() + "|" + info.filePath.trimmed() + "|" +
         info.funcName.trimmed();
}

QString MainWindow::run_tab_title(const AlgorithmInfo &info) const {
  QString name = info.name.trimmed();
  if (name.isEmpty()) {
    name = info.funcName.trimmed();
  }
  if (name.size() > 16) {
    name = name.left(16) + "...";
  }
  return QStringLiteral("运行: ") + name;
}
