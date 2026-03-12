#include "algorithm_edit_tab.h"

#include "m_message_box.h"
#include "ui_system.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QSet>
#include <QSpinBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QUuid>

AlgorithmEditTab::AlgorithmEditTab(const AlgorithmInfo &info, QWidget *parent)
    : QWidget(parent), current_info_(info) {
  setAttribute(Qt::WA_StyledBackground, true);
  setObjectName("AlgorithmEditTab");
  init_ui();
  load_categories();
  load_data(current_info_);
}

QString AlgorithmEditTab::tabKey() const {
  if (!current_info_.id.trimmed().isEmpty()) {
    return QStringLiteral("edit_algo_%1").arg(current_info_.id.trimmed());
  }
  return QStringLiteral("new_algo_tab");
}

QString AlgorithmEditTab::tabTitle() const {
  const QString name = name_input_ ? name_input_->text().trimmed()
                                   : current_info_.name.trimmed();
  if (!current_info_.id.trimmed().isEmpty()) {
    return name.isEmpty() ? QStringLiteral("编辑算法")
                          : QStringLiteral("编辑: %1").arg(name);
  }
  return name.isEmpty() ? QStringLiteral("新增算法")
                        : QStringLiteral("新增: %1").arg(name);
}

void AlgorithmEditTab::init_ui() {
  auto *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(10, 10, 10, 10);

  auto *splitter = new QSplitter(Qt::Horizontal, this);
  splitter->setChildrenCollapsible(false);
  main_layout->addWidget(splitter);

  auto *left_panel = new QWidget(splitter);
  auto *left_layout = new QVBoxLayout(left_panel);
  left_layout->setContentsMargins(16, 16, 16, 16);
  left_layout->setSpacing(16);

  auto *info_group = new QGroupBox(QStringLiteral("算法基础信息"), left_panel);
  auto *form_layout = new QFormLayout(info_group);
  form_layout->setContentsMargins(16, 20, 16, 16);
  form_layout->setVerticalSpacing(16);

  name_input_ = new MaterialInput(MaterialInput::Edit, this);

  category_combo_ = new QComboBox(this);
  category_combo_->setObjectName("AlgorithmEditCombo");
  category_combo_->setFixedHeight(40);

  source_type_combo_ = new QComboBox(this);
  source_type_combo_->setObjectName("AlgorithmEditCombo");
  source_type_combo_->setFixedHeight(40);
  source_type_combo_->addItem(QStringLiteral("动态库 DLL"), QStringLiteral("dll"));
  source_type_combo_->addItem(QStringLiteral("Python 脚本"),
                              QStringLiteral("python"));

  auto *path_layout = new QHBoxLayout();
  path_layout->setSpacing(8);
  path_input_ = new MaterialInput(MaterialInput::Edit, this);
  path_btn_ = new MaterialButton("...", MaterialButton::Normal, this);
  path_btn_->setFixedSize(40, 40);
  path_btn_->set_theme_color(UISystem::instance().neutral());
  path_layout->addWidget(path_input_);
  path_layout->addWidget(path_btn_);

  func_input_ = new MaterialInput(MaterialInput::Edit, this);
  desc_input_ = new MaterialInput(MaterialInput::Edit, this);

  form_layout->addRow(QStringLiteral("算法名称 * :"), name_input_);
  form_layout->addRow(QStringLiteral("所属分类 * :"), category_combo_);
  form_layout->addRow(QStringLiteral("实现方式 :"), source_type_combo_);
  form_layout->addRow(QStringLiteral("文件路径 * :"), path_layout);
  form_layout->addRow(QStringLiteral("入口函数 * :"), func_input_);
  form_layout->addRow(QStringLiteral("算法说明 :"), desc_input_);

  auto *left_btn_layout = new QHBoxLayout();
  left_btn_layout->addStretch();
  save_algo_btn_ = new MaterialButton(QStringLiteral("保存基础信息"),
                                      MaterialButton::Normal, this);
  save_algo_btn_->set_theme_color(UISystem::instance().bg_primary());
  left_btn_layout->addWidget(save_algo_btn_);

  left_layout->addWidget(info_group);
  left_layout->addStretch();
  left_layout->addLayout(left_btn_layout);

  auto *right_panel = new QWidget(splitter);
  auto *right_layout = new QVBoxLayout(right_panel);
  right_layout->setContentsMargins(16, 16, 16, 16);
  right_layout->setSpacing(16);

  param_group_ = new QGroupBox(QStringLiteral("输入参数配置"), right_panel);
  auto *param_layout = new QVBoxLayout(param_group_);
  param_layout->setContentsMargins(16, 20, 16, 16);
  param_layout->setSpacing(12);

  auto *toolbar_layout = new QHBoxLayout();
  add_param_btn_ = new MaterialButton(QStringLiteral("+ 添加参数"),
                                      MaterialButton::Normal, this);
  add_param_btn_->set_theme_color(UISystem::instance().bg_primary());

  edit_param_btn_ = new MaterialButton(QStringLiteral("编辑选中"),
                                       MaterialButton::Normal, this);
  delete_param_btn_ = new MaterialButton(QStringLiteral("删除选中"),
                                         MaterialButton::Normal, this);
  edit_param_btn_->setEnabled(false);
  delete_param_btn_->setEnabled(false);

  toolbar_layout->addWidget(add_param_btn_);
  toolbar_layout->addWidget(edit_param_btn_);
  toolbar_layout->addWidget(delete_param_btn_);
  toolbar_layout->addStretch();
  param_layout->addLayout(toolbar_layout);

  param_table_ = new QTableWidget(this);
  param_table_->setObjectName("AlgorithmEditParamTable");
  param_table_->setColumnCount(7);
  param_table_->setHorizontalHeaderLabels(
      {QStringLiteral("排序"), QStringLiteral("参数标识"),
       QStringLiteral("参数名称"), QStringLiteral("类型"),
       QStringLiteral("默认值"), QStringLiteral("必填"),
       QStringLiteral("说明")});
  param_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  param_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  param_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  param_table_->setAlternatingRowColors(true);
  param_table_->setShowGrid(false);
  param_table_->setFrameShape(QFrame::NoFrame);
  param_table_->verticalHeader()->setVisible(false);
  param_table_->verticalHeader()->setDefaultSectionSize(40);
  param_table_->horizontalHeader()->setStretchLastSection(true);
  param_table_->horizontalHeader()->setMinimumSectionSize(70);
  param_table_->horizontalHeader()->resizeSection(0, 70);
  param_table_->horizontalHeader()->resizeSection(1, 140);
  param_table_->horizontalHeader()->resizeSection(2, 150);
  param_table_->horizontalHeader()->resizeSection(3, 90);
  param_table_->horizontalHeader()->resizeSection(4, 90);
  param_layout->addWidget(param_table_, 1);

  auto *editor_section = new QWidget(param_group_);
  editor_section->setObjectName("AlgorithmEditSection");
  auto *editor_layout = new QVBoxLayout(editor_section);
  editor_layout->setContentsMargins(16, 16, 16, 16);
  editor_layout->setSpacing(12);

  auto *editor_title = new QLabel(QStringLiteral("参数编辑"), editor_section);
  editor_title->setObjectName("AlgorithmEditSectionTitle");
  editor_layout->addWidget(editor_title);

  auto *editor_grid = new QGridLayout();
  editor_grid->setHorizontalSpacing(12);
  editor_grid->setVerticalSpacing(10);

  param_identifier_input_ = new MaterialInput(MaterialInput::Edit, this);
  param_name_input_ = new MaterialInput(MaterialInput::Edit, this);
  param_unit_input_ = new MaterialInput(MaterialInput::Edit, this);
  param_default_input_ = new MaterialInput(MaterialInput::Edit, this);
  param_min_input_ = new MaterialInput(MaterialInput::Edit, this);
  param_max_input_ = new MaterialInput(MaterialInput::Edit, this);
  param_tooltip_input_ = new MaterialInput(MaterialInput::Edit, this);
  param_comments_input_ = new MaterialInput(MaterialInput::Edit, this);

  param_type_combo_ = new QComboBox(this);
  param_type_combo_->setObjectName("AlgorithmEditCombo");
  param_type_combo_->setFixedHeight(40);
  param_type_combo_->addItem(QStringLiteral("浮点数"), QStringLiteral("double"));
  param_type_combo_->addItem(QStringLiteral("整数"), QStringLiteral("int"));
  param_type_combo_->addItem(QStringLiteral("布尔值"), QStringLiteral("bool"));
  param_type_combo_->addItem(QStringLiteral("字符串"), QStringLiteral("string"));

  param_required_check_ = new QCheckBox(QStringLiteral("必填参数"), this);
  param_order_spin_ = new QSpinBox(this);
  param_order_spin_->setMinimum(0);
  param_order_spin_->setMaximum(9999);
  param_order_spin_->setFixedHeight(36);

  editor_grid->addWidget(new QLabel(QStringLiteral("参数标识 *"), this), 0, 0);
  editor_grid->addWidget(param_identifier_input_, 0, 1);
  editor_grid->addWidget(new QLabel(QStringLiteral("参数名称 *"), this), 0, 2);
  editor_grid->addWidget(param_name_input_, 0, 3);

  editor_grid->addWidget(new QLabel(QStringLiteral("数据类型"), this), 1, 0);
  editor_grid->addWidget(param_type_combo_, 1, 1);
  editor_grid->addWidget(new QLabel(QStringLiteral("单位"), this), 1, 2);
  editor_grid->addWidget(param_unit_input_, 1, 3);

  editor_grid->addWidget(new QLabel(QStringLiteral("默认值"), this), 2, 0);
  editor_grid->addWidget(param_default_input_, 2, 1);
  editor_grid->addWidget(new QLabel(QStringLiteral("显示顺序"), this), 2, 2);
  editor_grid->addWidget(param_order_spin_, 2, 3);

  editor_grid->addWidget(new QLabel(QStringLiteral("最小值"), this), 3, 0);
  editor_grid->addWidget(param_min_input_, 3, 1);
  editor_grid->addWidget(new QLabel(QStringLiteral("最大值"), this), 3, 2);
  editor_grid->addWidget(param_max_input_, 3, 3);

  editor_grid->addWidget(new QLabel(QStringLiteral("提示信息"), this), 4, 0);
  editor_grid->addWidget(param_tooltip_input_, 4, 1);
  editor_grid->addWidget(new QLabel(QStringLiteral("备注"), this), 4, 2);
  editor_grid->addWidget(param_comments_input_, 4, 3);

  editor_layout->addLayout(editor_grid);

  auto *state_layout = new QHBoxLayout();
  state_layout->addWidget(param_required_check_);
  state_layout->addStretch();
  editor_layout->addLayout(state_layout);

  auto *param_btn_layout = new QHBoxLayout();
  param_btn_layout->addStretch();
  cancel_param_btn_ = new MaterialButton(QStringLiteral("清空"),
                                         MaterialButton::Normal, this);
  cancel_param_btn_->set_theme_color(UISystem::instance().neutral());
  save_param_btn_ = new MaterialButton(QStringLiteral("保存参数"),
                                       MaterialButton::Normal, this);
  save_param_btn_->set_theme_color(UISystem::instance().bg_primary());
  param_btn_layout->addWidget(cancel_param_btn_);
  param_btn_layout->addWidget(save_param_btn_);
  editor_layout->addLayout(param_btn_layout);

  param_layout->addWidget(editor_section);
  right_layout->addWidget(param_group_);

  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 2);

  connect(path_btn_, &QPushButton::clicked, this, [this]() {
    const bool is_python =
        source_type_combo_->currentData().toString() == "python";
    const QString title = is_python ? QStringLiteral("选择 Python 脚本")
                                    : QStringLiteral("选择动态库文件");
    const QString filter =
        is_python ? QStringLiteral("Python 脚本 (*.py)")
                  : QStringLiteral("动态库文件 (*.dll *.so *.dylib)");
    const QString path =
        QFileDialog::getOpenFileName(this, title, QString(), filter);
    if (!path.isEmpty()) {
      path_input_->setText(path);
    }
  });

  connect(source_type_combo_,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this](int) { update_source_type_ui(); });
  connect(save_algo_btn_, &QPushButton::clicked, this,
          [this]() { save_algorithm(); });
  connect(add_param_btn_, &QPushButton::clicked, this,
          [this]() { reset_param_form(); });
  connect(edit_param_btn_, &QPushButton::clicked, this,
          [this]() { edit_selected_param(); });
  connect(delete_param_btn_, &QPushButton::clicked, this,
          [this]() { delete_selected_param(); });
  connect(save_param_btn_, &QPushButton::clicked, this,
          [this]() { save_param(); });
  connect(cancel_param_btn_, &QPushButton::clicked, this,
          [this]() { reset_param_form(); });
  connect(param_table_, &QTableWidget::itemSelectionChanged, this,
          [this]() {
            const bool has_selection = current_param_row() >= 0;
            edit_param_btn_->setEnabled(has_selection);
            delete_param_btn_->setEnabled(has_selection);
          });
}

void AlgorithmEditTab::load_categories() {
  category_combo_->clear();
  category_combo_->addItem(QStringLiteral("请选择算法分类..."), QVariant());

  const QList<AlgoCategory> categories =
      category_service_.fetch_all_categories();

  QSet<QString> parent_ids;
  for (const AlgoCategory &category : categories) {
    parent_ids.insert(category.parentId.trimmed());
  }

  for (const AlgoCategory &category : categories) {
    const QString id = category.id.trimmed();
    if (!parent_ids.contains(id) && category.parentId.trimmed() != "0") {
      category_combo_->addItem(category.name, id);
    }
  }
}

void AlgorithmEditTab::load_data(const AlgorithmInfo &info) {
  name_input_->setText(info.name);
  path_input_->setText(info.filePath);
  func_input_->setText(info.funcName);
  desc_input_->setText(info.description);

  int source_index = source_type_combo_->findData(info.sourceType.trimmed());
  if (source_index < 0) {
    source_index = (info.sourceType.trimmed() == "2") ? 1 : 0;
  }
  source_type_combo_->setCurrentIndex(source_index);

  const int category_index = category_combo_->findData(info.categoryId);
  category_combo_->setCurrentIndex(category_index >= 0 ? category_index : 0);

  update_source_type_ui();
  load_params();
  sync_param_group_state();
}

void AlgorithmEditTab::load_params() {
  params_.clear();
  if (current_info_.id.trimmed().isEmpty()) {
    reload_param_table();
    reset_param_form();
    return;
  }

  QString error_message;
  params_ = algorithm_param_service_.fetch_params(current_info_.id, &error_message);
  reload_param_table();
  reset_param_form();

  if (!error_message.trimmed().isEmpty()) {
    MaterialMessageBox::error(this, QStringLiteral("失败"),
                              QStringLiteral("加载参数失败：") + error_message);
  }
}

void AlgorithmEditTab::reload_param_table() {
  param_table_->clearContents();
  param_table_->setRowCount(params_.size());

  for (int row = 0; row < params_.size(); ++row) {
    const AlgorithmParam &param = params_.at(row);
    const QString required_text =
        param.required ? QStringLiteral("是") : QStringLiteral("否");
    const QString comment_text = param.comments.trimmed().isEmpty()
                                     ? param.tooltip.trimmed()
                                     : param.comments.trimmed();

    const QStringList cells = {
        QString::number(param.order), param.identifier.trimmed(),
        param.name.trimmed(),         param.type.trimmed(),
        param.defaultValue.trimmed(), required_text,
        comment_text};

    for (int col = 0; col < cells.size(); ++col) {
      auto *item = new QTableWidgetItem(cells.at(col));
      item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      item->setToolTip(cells.at(col));
      if (col == 0) {
        item->setData(Qt::UserRole, param.id);
      }
      param_table_->setItem(row, col, item);
    }
  }

  if (params_.isEmpty()) {
    param_table_->setRowCount(0);
  }
}

void AlgorithmEditTab::reset_param_form() {
  editing_param_row_ = -1;
  param_table_->clearSelection();
  param_identifier_input_->clear();
  param_name_input_->clear();
  param_unit_input_->clear();
  param_default_input_->clear();
  param_min_input_->clear();
  param_max_input_->clear();
  param_tooltip_input_->clear();
  param_comments_input_->clear();
  param_required_check_->setChecked(false);
  param_type_combo_->setCurrentIndex(0);
  param_order_spin_->setValue(params_.size() + 1);
  edit_param_btn_->setEnabled(false);
  delete_param_btn_->setEnabled(false);
}

void AlgorithmEditTab::fill_param_form(const AlgorithmParam &param, int row) {
  editing_param_row_ = row;
  param_identifier_input_->setText(param.identifier);
  param_name_input_->setText(param.name);
  param_unit_input_->setText(param.unit);
  param_default_input_->setText(param.defaultValue);
  param_min_input_->setText(param.minValue);
  param_max_input_->setText(param.maxValue);
  param_tooltip_input_->setText(param.tooltip);
  param_comments_input_->setText(param.comments);
  param_required_check_->setChecked(param.required);
  param_order_spin_->setValue(param.order);

  int type_index = param_type_combo_->findData(param.type.trimmed().toLower());
  if (type_index < 0 && param.type.trimmed().compare("integer", Qt::CaseInsensitive) == 0) {
    type_index = param_type_combo_->findData(QStringLiteral("int"));
  }
  if (type_index < 0 && param.type.trimmed().compare("float", Qt::CaseInsensitive) == 0) {
    type_index = param_type_combo_->findData(QStringLiteral("double"));
  }
  param_type_combo_->setCurrentIndex(type_index >= 0 ? type_index : 0);
}

void AlgorithmEditTab::sync_param_group_state() {
  const bool is_edit_mode = !current_info_.id.trimmed().isEmpty();
  param_group_->setEnabled(is_edit_mode);
  param_group_->setTitle(is_edit_mode
                             ? QStringLiteral("输入参数配置")
                             : QStringLiteral("输入参数配置 (请先保存左侧基础信息)"));
}

int AlgorithmEditTab::current_param_row() const {
  if (!param_table_) {
    return -1;
  }
  const QModelIndexList rows =
      param_table_->selectionModel() ? param_table_->selectionModel()->selectedRows()
                                     : QModelIndexList{};
  if (rows.isEmpty()) {
    return -1;
  }
  return rows.first().row();
}

void AlgorithmEditTab::update_source_type_ui() {
  const bool is_python =
      source_type_combo_->currentData().toString() == "python";
  path_input_->setPlaceholderText(
      is_python ? QStringLiteral("Python 脚本路径（.py）")
                : QStringLiteral("算法库路径（.dll/.so/.dylib）"));
  func_input_->setPlaceholderText(is_python ? QStringLiteral("Python 函数名")
                                            : QStringLiteral("导出函数名"));
}

AlgorithmInfo AlgorithmEditTab::get_data() const {
  AlgorithmInfo info = current_info_;
  info.name = name_input_->text().trimmed();
  info.categoryId = category_combo_->currentData().toString().trimmed();
  info.sourceType = source_type_combo_->currentData().toString().trimmed();
  info.filePath = path_input_->text().trimmed();
  info.funcName = func_input_->text().trimmed();
  info.description = desc_input_->text().trimmed();
  info.params = params_;
  return info;
}

AlgorithmParam AlgorithmEditTab::get_param_data() const {
  AlgorithmParam param;
  if (editing_param_row_ >= 0 && editing_param_row_ < params_.size()) {
    param = params_.at(editing_param_row_);
  }

  param.algoId = current_info_.id.trimmed();
  param.identifier = param_identifier_input_->text().trimmed();
  param.name = param_name_input_->text().trimmed();
  param.unit = param_unit_input_->text().trimmed();
  param.type = param_type_combo_->currentData().toString().trimmed();
  param.defaultValue = param_default_input_->text().trimmed();
  param.minValue = param_min_input_->text().trimmed();
  param.maxValue = param_max_input_->text().trimmed();
  param.tooltip = param_tooltip_input_->text().trimmed();
  param.comments = param_comments_input_->text().trimmed();
  param.required = param_required_check_->isChecked();
  param.order = param_order_spin_->value();
  return param;
}

void AlgorithmEditTab::save_algorithm() {
  AlgorithmInfo info = get_data();
  if (info.name.isEmpty() || info.categoryId.isEmpty()) {
    MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                QStringLiteral("请输入算法名称并选择分类。"));
    return;
  }
  if (info.filePath.isEmpty()) {
    MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                QStringLiteral("请输入算法文件路径。"));
    return;
  }
  if (info.funcName.isEmpty()) {
    MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                QStringLiteral("请输入入口函数名。"));
    return;
  }

  const QString old_key = tabKey();
  if (info.id.trimmed().isEmpty()) {
    info.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  }

  QString error_message;
  const bool ok = current_info_.id.trimmed().isEmpty()
                      ? algorithm_service_.create_algorithm(info, &error_message)
                      : algorithm_service_.update_algorithm(info, &error_message);
  if (!ok) {
    MaterialMessageBox::error(this, QStringLiteral("失败"),
                              QStringLiteral("保存失败：") + error_message);
    return;
  }

  current_info_ = info;
  sync_param_group_state();
  load_params();
  MaterialMessageBox::information(this, QStringLiteral("成功"),
                                  QStringLiteral("算法信息已保存。"));
  emit saved(old_key, tabKey(), tabTitle());
}

void AlgorithmEditTab::save_param() {
  if (current_info_.id.trimmed().isEmpty()) {
    MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                QStringLiteral("请先保存左侧算法基础信息。"));
    return;
  }

  AlgorithmParam param = get_param_data();
  if (param.identifier.isEmpty() || param.name.isEmpty()) {
    MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                QStringLiteral("请填写参数标识和参数名称。"));
    return;
  }
  if (param.type.isEmpty()) {
    MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                QStringLiteral("请选择参数类型。"));
    return;
  }
  if (editing_param_row_ < 0) {
    for (const AlgorithmParam &item : params_) {
      if (item.identifier.trimmed().compare(param.identifier, Qt::CaseInsensitive) ==
          0) {
        MaterialMessageBox::warning(
            this, QStringLiteral("提示"),
            QStringLiteral("参数标识不能重复，请更换后再保存。"));
        return;
      }
    }
  } else {
    for (int i = 0; i < params_.size(); ++i) {
      if (i == editing_param_row_) {
        continue;
      }
      if (params_.at(i).identifier.trimmed().compare(param.identifier,
                                                     Qt::CaseInsensitive) == 0) {
        MaterialMessageBox::warning(
            this, QStringLiteral("提示"),
            QStringLiteral("参数标识不能重复，请更换后再保存。"));
        return;
      }
    }
  }

  if (param.id.trimmed().isEmpty()) {
    param.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  }

  QString error_message;
  const bool ok = (editing_param_row_ >= 0 && editing_param_row_ < params_.size())
                       ? algorithm_param_service_.update_param(param, &error_message)
                       : algorithm_param_service_.create_param(param, &error_message);
  if (!ok) {
    MaterialMessageBox::error(this, QStringLiteral("失败"),
                              QStringLiteral("保存参数失败：") + error_message);
    return;
  }

  load_params();
  MaterialMessageBox::information(this, QStringLiteral("成功"),
                                  QStringLiteral("参数已保存。"));
}

void AlgorithmEditTab::edit_selected_param() {
  const int row = current_param_row();
  if (row < 0 || row >= params_.size()) {
    return;
  }
  fill_param_form(params_.at(row), row);
}

void AlgorithmEditTab::delete_selected_param() {
  const int row = current_param_row();
  if (row < 0 || row >= params_.size()) {
    return;
  }

  const AlgorithmParam &param = params_.at(row);
  const int reply = MaterialMessageBox::question(
      this, QStringLiteral("确认删除"),
      QStringLiteral("确认删除参数“%1”？此操作不可撤销。")
          .arg(param.name.trimmed().isEmpty() ? param.identifier : param.name));
  if (reply != QDialog::Accepted) {
    return;
  }

  QString error_message;
  if (!algorithm_param_service_.delete_param(param.id, &error_message)) {
    MaterialMessageBox::error(this, QStringLiteral("失败"),
                              QStringLiteral("删除参数失败：") + error_message);
    return;
  }

  load_params();
}
