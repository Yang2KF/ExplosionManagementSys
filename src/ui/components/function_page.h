#pragma once

#include "m_button.h"
#include "m_input.h"
#include "m_menu.h"
#include "model/algorithm_table_model.h"
#include "model/category_tree_model.h"
#include "service/algorithm_service.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QTableView>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

class FunctionPage : public QWidget {
  Q_OBJECT
public:
  explicit FunctionPage(QWidget *parent = nullptr);

signals:
  void requestRunTab(const AlgorithmInfo &info);

private:
  void init_ui();
  void setup_toolbar();
  void setup_views();
  void init_connections();
  void show_table_context_menu(const QPoint &pos);
  void run_algorithm_by_row(int row);
  void edit_algorithm_by_row(int row);
  void delete_algorithm_by_row(int row);

  QVBoxLayout *main_layout_ = nullptr;
  QHBoxLayout *tool_layout_ = nullptr;
  MaterialInput *search_input_ = nullptr;
  MaterialButton *add_btn_ = nullptr;
  MaterialButton *refresh_btn_ = nullptr;

  QSplitter *splitter_ = nullptr;
  QTreeView *category_tree_ = nullptr;

  QWidget *right_panel_ = nullptr;
  QVBoxLayout *right_layout_ = nullptr;
  QTableView *algo_table_ = nullptr;

  CategoryTreeModel *tree_model_ = nullptr;
  AlgorithmTableModel *table_model_ = nullptr;
  AlgorithmService algorithm_service_;
};
