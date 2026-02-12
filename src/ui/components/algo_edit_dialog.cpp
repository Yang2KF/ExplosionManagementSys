#include "algo_edit_dialog.h"
#include "db/db_manager.h"
#include "m_message_box.h"
#include "mask_widget.h"
#include "ui_system.h"
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QSqlQuery>
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

  QLabel *title = new QLabel("Algorithm Information", this);
  title->setObjectName("AlgoEditDialogTitle");
  content_layout->addWidget(title);

  name_input_ = new MaterialInput(this);
  name_input_->setPlaceholderText("Algorithm Name");
  content_layout->addWidget(name_input_);

  category_combo_ = new QComboBox(this);
  category_combo_->setObjectName("AlgoEditCategoryCombo");
  category_combo_->setFixedHeight(40);
  content_layout->addWidget(category_combo_);

  QHBoxLayout *path_layout = new QHBoxLayout();
  path_input_ = new MaterialInput(this);
  path_input_->setPlaceholderText("Library Path (.dll)");

  path_btn_ = new MaterialButton("...", MaterialButton::Normal, this);
  path_btn_->setFixedSize(40, 40);
  path_btn_->set_theme_color(UISystem::instance().neutral());

  path_layout->addWidget(path_input_);
  path_layout->addWidget(path_btn_);
  content_layout->addLayout(path_layout);

  func_input_ = new MaterialInput(this);
  func_input_->setPlaceholderText("Export Function Name");
  content_layout->addWidget(func_input_);

  desc_input_ = new MaterialInput(this);
  desc_input_->setPlaceholderText("Short Description");
  content_layout->addWidget(desc_input_);

  content_layout->addStretch();

  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->addStretch();

  cancel_btn_ = new MaterialButton("Cancel", MaterialButton::Normal, this);
  cancel_btn_->set_theme_color(UISystem::instance().neutral());
  cancel_btn_->setFixedSize(80, 35);

  confirm_btn_ = new MaterialButton("Save", MaterialButton::Normal, this);
  confirm_btn_->set_theme_color(UISystem::instance().bg_primary());
  confirm_btn_->setFixedSize(80, 35);

  btn_layout->addWidget(cancel_btn_);
  btn_layout->addWidget(confirm_btn_);
  content_layout->addLayout(btn_layout);

  connect(cancel_btn_, &QPushButton::clicked, this, &QDialog::reject);

  connect(confirm_btn_, &QPushButton::clicked, this, [this]() {
    if (name_input_->text().isEmpty() || category_combo_->currentData().isNull()) {
      MaterialMessageBox::warning(this, "Hint",
                                  "Please enter algorithm name and select a category.");
      return;
    }
    accept();
  });

  connect(path_btn_, &QPushButton::clicked, this, [this]() {
    QString path = QFileDialog::getOpenFileName(
        this, "Select Algorithm Library", QString(),
        "Library Files (*.dll *.so *.dylib)");
    if (!path.isEmpty()) {
      path_input_->setText(path);
    }
  });
}

void AlgoEditDialog::load_categories() {
  category_combo_->clear();
  category_combo_->addItem("Please select category...", QVariant());

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  if (query.exec("SELECT id, name FROM categories")) {
    while (query.next()) {
      category_combo_->addItem(query.value("name").toString(), query.value("id"));
    }
  }
}

AlgorithmInfo AlgoEditDialog::get_data() const {
  AlgorithmInfo info;
  info.id = current_algo_id_;
  info.name = name_input_->text();
  info.categoryId = category_combo_->currentData().toInt();
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

  int index = category_combo_->findData(info.categoryId);
  if (index != -1) {
    category_combo_->setCurrentIndex(index);
  }
}
