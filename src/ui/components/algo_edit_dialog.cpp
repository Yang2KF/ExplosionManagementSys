#include "algo_edit_dialog.h"
#include "m_message_box.h"
#include "mask_widget.h"
#include "ui_system.h"
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

AlgoEditDialog::AlgoEditDialog(QWidget *parent) : QDialog(parent) {
  setObjectName("AlgoEditDialog");
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
  resize(480, 550);

  init_ui();
  load_categories();
}

void AlgoEditDialog::paintEvent(QPaintEvent *event) { Q_UNUSED(event); }

void AlgoEditDialog::showEvent(QShowEvent *event) {
  if (auto mask = MaskWidget::instance()) {
    mask->show_mask();
  }
  QDialog::showEvent(event);
  raise();
}

void AlgoEditDialog::done(int r) {
  if (auto mask = MaskWidget::instance()) {
    mask->hide_mask();
  }
  QDialog::done(r);
}

void AlgoEditDialog::init_ui() {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(10, 10, 10, 10);

  QWidget *container = new QWidget(this);
  container->setObjectName("AlgoEditDialogContainer");

  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 60));
  shadow->setOffset(0, 5);
  container->setGraphicsEffect(shadow);

  main_layout->addWidget(container);

  QVBoxLayout *content_layout = new QVBoxLayout(container);
  content_layout->setContentsMargins(30, 30, 30, 30);
  content_layout->setSpacing(20);

  QLabel *title = new QLabel("算法信息", this);
  title->setObjectName("AlgoEditDialogTitle");
  content_layout->addWidget(title);

  name_input_ = new MaterialInput(this);
  name_input_->setPlaceholderText("算法名称");
  content_layout->addWidget(name_input_);

  category_combo_ = new QComboBox(this);
  category_combo_->setObjectName("AlgoEditCategoryCombo");
  category_combo_->setFixedHeight(40);
  content_layout->addWidget(category_combo_);

  source_type_combo_ = new QComboBox(this);
  source_type_combo_->setObjectName("AlgoEditSourceTypeCombo");
  source_type_combo_->setFixedHeight(40);
  source_type_combo_->addItem("动态库 DLL", "1");
  source_type_combo_->addItem("Python 脚本", "2");
  content_layout->addWidget(source_type_combo_);

  QHBoxLayout *path_layout = new QHBoxLayout();
  path_input_ = new MaterialInput(this);

  path_btn_ = new MaterialButton("...", MaterialButton::Normal, this);
  path_btn_->setFixedSize(40, 40);
  path_btn_->set_theme_color(UISystem::instance().neutral());

  path_layout->addWidget(path_input_);
  path_layout->addWidget(path_btn_);
  content_layout->addLayout(path_layout);

  func_input_ = new MaterialInput(this);
  func_input_->setPlaceholderText("导出函数名");
  content_layout->addWidget(func_input_);

  desc_input_ = new MaterialInput(this);
  desc_input_->setPlaceholderText("算法说明");
  content_layout->addWidget(desc_input_);

  content_layout->addStretch();

  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->addStretch();

  cancel_btn_ = new MaterialButton("取消", MaterialButton::Normal, this);
  cancel_btn_->set_theme_color(UISystem::instance().neutral());
  cancel_btn_->setFixedSize(80, 35);

  confirm_btn_ = new MaterialButton("保存", MaterialButton::Normal, this);
  confirm_btn_->set_theme_color(UISystem::instance().bg_primary());
  confirm_btn_->setFixedSize(80, 35);

  btn_layout->addWidget(cancel_btn_);
  btn_layout->addWidget(confirm_btn_);
  content_layout->addLayout(btn_layout);

  connect(cancel_btn_, &QPushButton::clicked, this, &QDialog::reject);

  connect(confirm_btn_, &QPushButton::clicked, this, [this]() {
    if (name_input_->text().isEmpty() || category_combo_->currentData().isNull()) {
      MaterialMessageBox::warning(this, "提示", "请输入算法名称并选择分类。");
      return;
    }
    if (path_input_->text().trimmed().isEmpty()) {
      MaterialMessageBox::warning(this, "提示", "请输入算法文件路径。");
      return;
    }
    if (func_input_->text().trimmed().isEmpty()) {
      MaterialMessageBox::warning(this, "提示", "请输入入口函数名。");
      return;
    }
    accept();
  });

  connect(path_btn_, &QPushButton::clicked, this, [this]() {
    const bool is_python = source_type_combo_->currentData().toString() == "2";
    const QString title = is_python ? "选择Python脚本" : "选择动态库文件";
    const QString filter =
        is_python ? "Python 脚本 (*.py)" : "动态库文件 (*.dll *.so *.dylib)";
    QString path = QFileDialog::getOpenFileName(this, title, QString(), filter);
    if (!path.isEmpty()) {
      path_input_->setText(path);
    }
  });

  connect(source_type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int) { update_source_type_ui(); });

  update_source_type_ui();
}

void AlgoEditDialog::load_categories() {
  category_combo_->clear();
  category_combo_->addItem("请选择算法分类...", QVariant());

  QList<AlgoCategory> categories = category_service_.fetch_all_categories();
  for (const AlgoCategory &category : categories) {
    category_combo_->addItem(category.name, category.id);
  }
}

AlgorithmInfo AlgoEditDialog::get_data() const {
  AlgorithmInfo info;
  info.id = current_algo_id_;
  info.name = name_input_->text();
  info.categoryId = category_combo_->currentData().toString();
  info.sourceType = source_type_combo_->currentData().toString();
  info.filePath = path_input_->text();
  info.funcName = func_input_->text();
  info.description = desc_input_->text();
  return info;
}

void AlgoEditDialog::set_data(const AlgorithmInfo &info) {
  current_algo_id_ = info.id;
  name_input_->setText(info.name);
  path_input_->setText(info.filePath);
  func_input_->setText(info.funcName);
  desc_input_->setText(info.description);

  int source_index = source_type_combo_->findData(info.sourceType);
  if (source_index == -1) {
    source_index = 0;
  }
  source_type_combo_->setCurrentIndex(source_index);
  update_source_type_ui();

  int index = category_combo_->findData(info.categoryId);
  if (index != -1) {
    category_combo_->setCurrentIndex(index);
  }
}

void AlgoEditDialog::update_source_type_ui() {
  const bool is_python = source_type_combo_->currentData().toString() == "2";
  path_input_->setPlaceholderText(
      is_python ? "Python脚本路径（.py）" : "算法库路径（.dll）");
  func_input_->setPlaceholderText(is_python ? "Python函数名" : "导出函数名");
}
