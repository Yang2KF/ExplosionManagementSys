#pragma once

#include "m_button.h"
#include "m_input.h"
#include "model/algorithm_service.h"
#include "model/algorithm_table_model.h"
#include "model/category_tree_model.h"
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

private:
  void init_ui();
  void setup_toolbar();
  void setup_views();
  void init_connections();

  QVBoxLayout *main_layout_;
  QHBoxLayout *tool_layout_;
  MaterialInput *search_input_;
  MaterialButton *add_btn_;
  MaterialButton *delete_btn_;
  MaterialButton *refresh_btn_;

  QSplitter *splitter_;
  QTreeView *category_tree_; // 左侧分类树
  QTableView *algo_table_;   // 右侧算法表

  CategoryTreeModel *tree_model_;
  AlgorithmTableModel *table_model_;
  AlgorithmService algorithm_service_;
};
