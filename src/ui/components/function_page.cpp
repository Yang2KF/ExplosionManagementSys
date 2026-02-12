#include "function_page.h"
#include "algo_edit_dialog.h"
#include "db/db_manager.h"
#include "m_message_box.h"
#include "ui_system.h"
#include <QDateTime>
#include <QHeaderView>
#include <QLabel>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>

FunctionPage::FunctionPage(QWidget *parent) : QWidget(parent) { init_ui(); }

void FunctionPage::init_ui() {
  setAttribute(Qt::WA_StyledBackground, true);
  setObjectName("FunctionPage");

  main_layout_ = new QVBoxLayout(this);
  main_layout_->setContentsMargins(20, 20, 20, 20);
  main_layout_->setSpacing(15);

  setup_toolbar();
  setup_views();

  tree_model_ = new CategoryTreeModel(this);
  tree_model_->reload();
  category_tree_->setModel(tree_model_);
  category_tree_->expandAll();

  table_model_ = new AlgorithmTableModel(this);
  table_model_->load_data(-1);
  algo_table_->setModel(table_model_);

  algo_table_->setColumnWidth(0, 50);
  algo_table_->setColumnWidth(1, 150);
  algo_table_->setColumnWidth(2, 150);

  init_connections();
}

void FunctionPage::setup_toolbar() {
  tool_layout_ = new QHBoxLayout();
  tool_layout_->setSpacing(10);

  search_input_ = new MaterialInput(this);
  search_input_->setPlaceholderText("Search algorithm name...");
  search_input_->setFixedWidth(300);

  add_btn_ = new MaterialButton("New", MaterialButton::Normal, this);
  add_btn_->set_theme_color(UISystem::instance().bg_primary());
  add_btn_->setFixedSize(100, 40);

  delete_btn_ = new MaterialButton("Delete", MaterialButton::Normal, this);
  delete_btn_->set_theme_color(UISystem::instance().status_error());
  delete_btn_->setFixedSize(100, 40);

  refresh_btn_ = new MaterialButton("Refresh", MaterialButton::Normal, this);
  refresh_btn_->setFixedSize(80, 40);
  refresh_btn_->set_theme_color(UISystem::instance().neutral());

  tool_layout_->addWidget(search_input_);
  tool_layout_->addStretch();
  tool_layout_->addWidget(refresh_btn_);
  tool_layout_->addWidget(delete_btn_);
  tool_layout_->addWidget(add_btn_);

  main_layout_->addLayout(tool_layout_);
}

void FunctionPage::setup_views() {
  splitter_ = new QSplitter(Qt::Horizontal, this);
  splitter_->setHandleWidth(1);
  splitter_->setChildrenCollapsible(false);

  category_tree_ = new QTreeView(splitter_);
  category_tree_->setObjectName("FunctionCategoryTree");
  category_tree_->setHeaderHidden(true);
  category_tree_->setFrameShape(QFrame::NoFrame);

  algo_table_ = new QTableView(splitter_);
  algo_table_->setObjectName("FunctionAlgoTable");
  algo_table_->setFrameShape(QFrame::NoFrame);
  algo_table_->setAlternatingRowColors(true);
  algo_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  algo_table_->verticalHeader()->setVisible(false);
  algo_table_->horizontalHeader()->setStretchLastSection(true);

  splitter_->addWidget(category_tree_);
  splitter_->addWidget(algo_table_);

  splitter_->setStretchFactor(0, 1);
  splitter_->setStretchFactor(1, 4);

  main_layout_->addWidget(splitter_);
}

void FunctionPage::init_connections() {
  connect(category_tree_, &QTreeView::clicked, this,
          [this](const QModelIndex &index) {
            QVariant id_data = index.data(Qt::UserRole + 1);
            if (id_data.isValid()) {
              int category_id = id_data.toInt();
              table_model_->load_data(category_id);
            }
          });

  connect(refresh_btn_, &QPushButton::clicked, this, [this]() {
    tree_model_->reload();
    table_model_->load_data(-1);
    category_tree_->expandAll();
  });

  connect(search_input_, &QLineEdit::returnPressed, this, [this]() {
    QString key = search_input_->text();
    table_model_->search_data(key);
  });

  connect(add_btn_, &QPushButton::clicked, this, [this]() {
    AlgoEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
      AlgorithmInfo info = dlg.get_data();

      QSqlDatabase db = DBManager::instance().database();
      QSqlQuery query(db);
      query.prepare("INSERT INTO algorithms (category_id, name, description, "
                    "file_path, func_name, created_at) "
                    "VALUES (:cid, :name, :desc, :path, :func, :time)");

      query.bindValue(":cid", info.categoryId);
      query.bindValue(":name", info.name);
      query.bindValue(":desc", info.description);
      query.bindValue(":path", info.filePath);
      query.bindValue(":func", info.funcName);
      query.bindValue(":time", QDateTime::currentDateTime());

      if (query.exec()) {
        table_model_->load_data(-1);
        MaterialMessageBox::information(this, "Success",
                                        "Algorithm has been added.");
      } else {
        MaterialMessageBox::error(this, "Failed",
                                  "Database error: " + query.lastError().text());
      }
    }
  });

  connect(delete_btn_, &QPushButton::clicked, this, [this]() {
    QModelIndex index = algo_table_->currentIndex();
    if (!index.isValid()) {
      MaterialMessageBox::warning(this, "Hint",
                                  "Please select an algorithm to delete.");
      return;
    }

    int algo_id = index.siblingAtColumn(0).data().toInt();
    QString algo_name = index.siblingAtColumn(1).data().toString();

    int reply = MaterialMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete '%1'? This cannot be undone.")
            .arg(algo_name));

    if (reply == QDialog::Accepted) {
      QSqlDatabase db = DBManager::instance().database();
      QSqlQuery query(db);
      query.prepare("DELETE FROM algorithms WHERE id = :id");
      query.bindValue(":id", algo_id);

      if (query.exec()) {
        table_model_->load_data(-1);
      } else {
        MaterialMessageBox::error(this, "Failed",
                                  "Delete failed: " + query.lastError().text());
      }
    }
  });

  connect(algo_table_, &QTableView::doubleClicked, this,
          [this](const QModelIndex &index) {
            if (!index.isValid())
              return;

            AlgorithmInfo info = table_model_->get_item(index.row());

            AlgoEditDialog dlg(this);
            dlg.set_data(info);

            if (dlg.exec() == QDialog::Accepted) {
              AlgorithmInfo new_info = dlg.get_data();

              QSqlDatabase db = DBManager::instance().database();
              QSqlQuery query(db);
              query.prepare("UPDATE algorithms SET category_id=:cid, "
                            "name=:name, description=:desc, "
                            "file_path=:path, func_name=:func WHERE id=:id");

              query.bindValue(":cid", new_info.categoryId);
              query.bindValue(":name", new_info.name);
              query.bindValue(":desc", new_info.description);
              query.bindValue(":path", new_info.filePath);
              query.bindValue(":func", new_info.funcName);
              query.bindValue(":id", info.id);

              if (query.exec()) {
                table_model_->load_data(-1);
              } else {
                MaterialMessageBox::error(this, "Failed",
                                          "Update failed: " +
                                              query.lastError().text());
              }
            }
          });
}
