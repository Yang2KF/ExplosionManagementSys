#pragma once

#include "components/frameless_widget.h"
#include "components/slide_stacked_widget.h"
#include "components/tab_bar.h"
#include "components/title_bar.h"
#include "entities/algorithm_data.h"
#include <QHash>
#include <QVBoxLayout>

class FunctionPage;

class MainWindow : public FramelessWidget {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void onTabRequested(int id);
  void onTabCloseRequested(int id);
  void open_run_tab(const AlgorithmInfo &info);
  void open_edit_tab(const AlgorithmInfo &info);
  void on_edit_tab_saved(const QString &old_key, const QString &new_key,
                         const QString &title);

private:
  void init_ui();
  void setup_header();
  void setup_tabs();
  void setup_content();

  QString run_tab_key(const AlgorithmInfo &info) const;
  QString run_tab_title(const AlgorithmInfo &info) const;
  QString edit_tab_key(const AlgorithmInfo &info) const;
  int page_index(QWidget *page) const;
  void adjust_tab_indexes_after_remove(QHash<QString, int> *indexes, int removed);

  QVBoxLayout *main_layout_ = nullptr;
  QVBoxLayout *content_layout_ = nullptr;
  TitleBar *title_bar_ = nullptr;
  TabBar *tab_bar_ = nullptr;
  SlideStackedWidget *pages_stack_ = nullptr;

  FunctionPage *function_page_ = nullptr;
  QHash<QString, int> run_tab_indexes_;
  QHash<QString, int> edit_tab_indexes_;
  int current_page_index_ = 0;
};
