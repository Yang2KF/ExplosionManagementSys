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
  auto *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(10, 10, 10, 10);

  auto *container = new QWidget(this);
  container->setObjectName("AlgoEditDialogContainer");

  auto *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 60));
  shadow->setOffset(0, 5);
  container->setGraphicsEffect(shadow);

  main_layout->addWidget(container);

  auto *content_layout = new QVBoxLayout(container);
  content_layout->setContentsMargins(30, 30, 30, 30);
  content_layout->setSpacing(20);

  auto *title = new QLabel(QStringLiteral("算法信息"), this);
  title->setObjectName("AlgoEditDialogTitle");
  content_layout->addWidget(title);

  name_input_ = new MaterialInput(MaterialInput::Edit, this);
  name_input_->setPlaceholderText(QStringLiteral("算法名称"));
  content_layout->addWidget(name_input_);

  category_combo_ = new QComboBox(this);
  category_combo_->setObjectName("AlgoEditCategoryCombo");
  category_combo_->setFixedHeight(40);
  content_layout->addWidget(category_combo_);

  source_type_combo_ = new QComboBox(this);
  source_type_combo_->setObjectName("AlgoEditSourceTypeCombo");
  source_type_combo_->setFixedHeight(40);
  source_type_combo_->addItem(QStringLiteral("动态库 DLL"), "dll");
  source_type_combo_->addItem(QStringLiteral("Python 脚本"), "python");
  content_layout->addWidget(source_type_combo_);

  auto *path_layout = new QHBoxLayout();
  path_input_ = new MaterialInput(MaterialInput::Edit, this);

  path_btn_ = new MaterialButton("...", MaterialButton::Normal, this);
  path_btn_->setFixedSize(40, 40);
  path_btn_->set_theme_color(UISystem::instance().neutral());

  path_layout->addWidget(path_input_);
  path_layout->addWidget(path_btn_);
  content_layout->addLayout(path_layout);

  func_input_ = new MaterialInput(MaterialInput::Edit, this);
  func_input_->setPlaceholderText(QStringLiteral("导出函数名"));
  content_layout->addWidget(func_input_);

  desc_input_ = new MaterialInput(MaterialInput::Edit, this);
  desc_input_->setPlaceholderText(QStringLiteral("算法说明"));
  content_layout->addWidget(desc_input_);

  content_layout->addStretch();

  auto *btn_layout = new QHBoxLayout();
  btn_layout->addStretch();

  cancel_btn_ =
      new MaterialButton(QStringLiteral("取消"), MaterialButton::Normal, this);
  cancel_btn_->set_theme_color(UISystem::instance().neutral());
  cancel_btn_->setFixedSize(80, 35);

  confirm_btn_ =
      new MaterialButton(QStringLiteral("保存"), MaterialButton::Normal, this);
  confirm_btn_->set_theme_color(UISystem::instance().bg_primary());
  confirm_btn_->setFixedSize(80, 35);

  btn_layout->addWidget(cancel_btn_);
  btn_layout->addWidget(confirm_btn_);
  content_layout->addLayout(btn_layout);

  connect(cancel_btn_, &QPushButton::clicked, this, &QDialog::reject);

  connect(confirm_btn_, &QPushButton::clicked, this, [this]() {
    if (name_input_->text().trimmed().isEmpty() ||
        category_combo_->currentData().toString().trimmed().isEmpty()) {
      MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                  QStringLiteral("请输入算法名称并选择分类。"));
      return;
    }
    if (path_input_->text().trimmed().isEmpty()) {
      MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                  QStringLiteral("请输入算法文件路径。"));
      return;
    }
    if (func_input_->text().trimmed().isEmpty()) {
      MaterialMessageBox::warning(this, QStringLiteral("提示"),
                                  QStringLiteral("请输入入口函数名。"));
      return;
    }
    accept();
  });

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

  update_source_type_ui();
}

void AlgoEditDialog::load_categories() {
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

AlgorithmInfo AlgoEditDialog::get_data() const {
  AlgorithmInfo info;
  info.id = current_algo_id_;
  info.name = name_input_->text().trimmed();
  info.categoryId = category_combo_->currentData().toString().trimmed();
  info.sourceType = source_type_combo_->currentData().toString().trimmed();
  info.filePath = path_input_->text().trimmed();
  info.funcName = func_input_->text().trimmed();
  info.description = desc_input_->text().trimmed();
  return info;
}

void AlgoEditDialog::set_data(const AlgorithmInfo &info) {
  current_algo_id_ = info.id;
  name_input_->setText(info.name);
  path_input_->setText(info.filePath);
  func_input_->setText(info.funcName);
  desc_input_->setText(info.description);

  int source_index = source_type_combo_->findData(info.sourceType.trimmed());
  if (source_index < 0) {
    source_index = (info.sourceType.trimmed() == "2") ? 1 : 0;
  }
  source_type_combo_->setCurrentIndex(source_index);
  update_source_type_ui();

  const int category_index = category_combo_->findData(info.categoryId);
  if (category_index >= 0) {
    category_combo_->setCurrentIndex(category_index);
  }
}

void AlgoEditDialog::update_source_type_ui() {
  const bool is_python =
      source_type_combo_->currentData().toString() == "python";
  path_input_->setPlaceholderText(
      is_python ? QStringLiteral("Python 脚本路径（.py）")
                : QStringLiteral("算法库路径（.dll/.so/.dylib）"));
  func_input_->setPlaceholderText(is_python ? QStringLiteral("Python 函数名")
                                            : QStringLiteral("导出函数名"));
}
