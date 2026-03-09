#include "function_page.h"
#include "algo_edit_dialog.h"
#include "m_message_box.h"
#include "ui_system.h"
#include <QFrame>
#include <QHeaderView>

namespace {
QString current_category_id(const QTreeView *tree_view) {
  if (!tree_view || !tree_view->currentIndex().isValid()) {
    return QString();
  }
  return tree_view->currentIndex().data(Qt::UserRole + 1).toString().trimmed();
}
} // namespace

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
  table_model_->load_data(QString());
  algo_table_->setModel(table_model_);
  algo_table_->setColumnWidth(0, 70);
  algo_table_->setColumnWidth(1, 260);
  algo_table_->setColumnWidth(2, 110);
  algo_table_->setColumnWidth(3, 240);
  algo_table_->setColumnWidth(4, 360);

  init_connections();
}

void FunctionPage::setup_toolbar() {
  tool_layout_ = new QHBoxLayout();
  tool_layout_->setSpacing(10);

  search_input_ = new MaterialInput(MaterialInput::Search, this);
  search_input_->setPlaceholderText(
      QStringLiteral("搜索算法名称或入口函数..."));
  search_input_->setFixedWidth(320);

  add_btn_ =
      new MaterialButton(QStringLiteral("新增"), MaterialButton::Normal, this);
  add_btn_->set_theme_color(UISystem::instance().bg_primary());
  add_btn_->setFixedSize(100, 40);

  refresh_btn_ =
      new MaterialButton(QStringLiteral("刷新"), MaterialButton::Normal, this);
  refresh_btn_->setFixedSize(80, 40);
  refresh_btn_->set_theme_color(UISystem::instance().neutral());

  tool_layout_->addWidget(search_input_);
  tool_layout_->addStretch();
  tool_layout_->addWidget(refresh_btn_);
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
  category_tree_->setAnimated(true);
  category_tree_->setIndentation(16);
  category_tree_->setFocusPolicy(Qt::NoFocus);
  category_tree_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  category_tree_->setCursor(Qt::PointingHandCursor);

  right_panel_ = new QWidget(splitter_);
  right_layout_ = new QVBoxLayout(right_panel_);
  right_layout_->setContentsMargins(0, 0, 0, 0);
  right_layout_->setSpacing(0);

  algo_table_ = new QTableView(right_panel_);
  algo_table_->setObjectName("FunctionAlgoTable");
  algo_table_->setFrameShape(QFrame::NoFrame);
  algo_table_->setAlternatingRowColors(true);
  algo_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  algo_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  algo_table_->setContextMenuPolicy(Qt::CustomContextMenu);
  algo_table_->setFocusPolicy(Qt::NoFocus);
  algo_table_->setShowGrid(false);

  algo_table_->verticalHeader()->setVisible(false);
  algo_table_->verticalHeader()->setDefaultSectionSize(40);

  algo_table_->horizontalHeader()->setStretchLastSection(true);
  algo_table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft |
                                                       Qt::AlignVCenter);
  algo_table_->horizontalHeader()->setHighlightSections(false);
  algo_table_->horizontalHeader()->setMinimumSectionSize(70);

  right_layout_->addWidget(algo_table_);

  splitter_->addWidget(category_tree_);
  splitter_->addWidget(right_panel_);
  splitter_->setStretchFactor(0, 1);
  splitter_->setStretchFactor(1, 4);

  main_layout_->addWidget(splitter_);
}

void FunctionPage::init_connections() {
  connect(category_tree_, &QTreeView::clicked, this,
          [this](const QModelIndex &index) {
            const QString category_id =
                index.data(Qt::UserRole + 1).toString().trimmed();
            table_model_->load_data(category_id);
          });

  connect(refresh_btn_, &QPushButton::clicked, this, [this]() {
    tree_model_->reload();
    table_model_->load_data(current_category_id(category_tree_));
    category_tree_->expandAll();
  });

  connect(search_input_, &QLineEdit::returnPressed, this, [this]() {
    const QString keyword = search_input_->text().trimmed();
    if (keyword.isEmpty()) {
      table_model_->load_data(current_category_id(category_tree_));
      return;
    }
    table_model_->search_data(keyword);
  });

  connect(add_btn_, &QPushButton::clicked, this, [this]() {
    AlgoEditDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
      return;
    }

    const AlgorithmInfo info = dlg.get_data();
    QString error_message;
    if (algorithm_service_.create_algorithm(info, &error_message)) {
      table_model_->load_data(current_category_id(category_tree_));
      MaterialMessageBox::information(this, QStringLiteral("成功"),
                                      QStringLiteral("算法已新增。"));
    } else {
      MaterialMessageBox::error(this, QStringLiteral("失败"),
                                QStringLiteral("数据库错误：") + error_message);
    }
  });

  connect(algo_table_, &QTableView::doubleClicked, this,
          [this](const QModelIndex &index) {
            if (!index.isValid()) {
              return;
            }
            edit_algorithm_by_row(index.row());
          });

  connect(algo_table_, &QWidget::customContextMenuRequested, this,
          [this](const QPoint &pos) { show_table_context_menu(pos); });
}

void FunctionPage::show_table_context_menu(const QPoint &pos) {
  if (!algo_table_ || !table_model_) {
    return;
  }

  QPoint viewport_pos = pos;
  QModelIndex index = algo_table_->indexAt(viewport_pos);
  if (!index.isValid()) {
    viewport_pos = algo_table_->viewport()->mapFrom(algo_table_, pos);
    index = algo_table_->indexAt(viewport_pos);
  }

  if (!index.isValid()) {
    int row = algo_table_->rowAt(viewport_pos.y());
    if (row < 0) {
      const QPoint alt_viewport_pos =
          algo_table_->viewport()->mapFrom(algo_table_, pos);
      row = algo_table_->rowAt(alt_viewport_pos.y());
      if (row >= 0) {
        viewport_pos = alt_viewport_pos;
      }
    }
    if (row >= 0) {
      index = table_model_->index(row, 0);
    }
  }

  if (!index.isValid()) {
    return;
  }

  algo_table_->setCurrentIndex(index);

  MaterialMenu menu(this);
  menu.add_action("run", QStringLiteral("运行算法"));
  menu.add_action("edit", QStringLiteral("编辑算法"));
  menu.add_action("delete", QStringLiteral("删除算法"));
  const QString action_id =
      menu.exec_and_get_id(algo_table_->viewport()->mapToGlobal(viewport_pos));

  if (action_id == "run") {
    run_algorithm_by_row(index.row());
    return;
  }

  if (action_id == "edit") {
    edit_algorithm_by_row(index.row());
    return;
  }

  if (action_id == "delete") {
    delete_algorithm_by_row(index.row());
  }
}

void FunctionPage::run_algorithm_by_row(int row) {
  if (row < 0 || !table_model_) {
    return;
  }

  const AlgorithmInfo info = table_model_->get_item(row);
  emit requestRunTab(info);
}

void FunctionPage::edit_algorithm_by_row(int row) {
  if (row < 0 || !table_model_) {
    return;
  }

  const AlgorithmInfo info = table_model_->get_item(row);
  AlgoEditDialog dlg(this);
  dlg.set_data(info);
  if (dlg.exec() != QDialog::Accepted) {
    return;
  }

  AlgorithmInfo new_info = dlg.get_data();
  new_info.id = info.id;
  QString error_message;
  if (algorithm_service_.update_algorithm(new_info, &error_message)) {
    table_model_->load_data(current_category_id(category_tree_));
  } else {
    MaterialMessageBox::error(this, QStringLiteral("失败"),
                              QStringLiteral("更新失败：") + error_message);
  }
}

void FunctionPage::delete_algorithm_by_row(int row) {
  if (row < 0 || !table_model_) {
    return;
  }

  const AlgorithmInfo info = table_model_->get_item(row);
  if (info.id.isEmpty()) {
    MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                QStringLiteral("无效的算法数据。"));
    return;
  }

  const int reply = MaterialMessageBox::question(
      this, QStringLiteral("确认删除"),
      QStringLiteral("确认删除算法“%1”？此操作不可撤销。").arg(info.name));
  if (reply != QDialog::Accepted) {
    return;
  }

  QString error_message;
  if (algorithm_service_.delete_algorithm(info.id, &error_message)) {
    table_model_->load_data(current_category_id(category_tree_));
  } else {
    MaterialMessageBox::error(this, QStringLiteral("失败"),
                              QStringLiteral("删除失败：") + error_message);
  }
}
