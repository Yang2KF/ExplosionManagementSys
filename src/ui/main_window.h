#pragma once

#include "components/frameless_widget.h"
#include "components/main_tab_bar.h"
#include "components/slide_stacked_widget.h"
#include "components/title_bar.h"
#include <QVBoxLayout>

class MainWindow : public FramelessWidget {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void onTabRequested(int id);

private:
  enum PageIndex {
    Page_User = 0,
    Page_Home,
    Page_Function,
    Page_Setting,
    Page_Info
  };

  void init_ui();
  void setup_header();
  void setup_tabs();
  void setup_content();

  QVBoxLayout *main_layout_ = nullptr;
  QVBoxLayout *content_layout_ = nullptr;
  TitleBar *title_bar_ = nullptr;
  MainTabBar *tab_bar_ = nullptr;
  SlideStackedWidget *pages_stack_ = nullptr;
  bool is_logged_ = false;
  int current_page_index_ = Page_Home;
};
