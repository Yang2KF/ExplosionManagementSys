#include "m_message_box.h"
#include "m_button.h"
#include "mask_widget.h"
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QVBoxLayout>

MaterialMessageBox::MaterialMessageBox(QWidget *parent) : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
  resize(320, 180); // 默认尺寸
}

// --- 静态方法实现 ---

void MaterialMessageBox::information(QWidget *parent, const QString &title,
                                     const QString &msg) {
  MaterialMessageBox box(parent);
  box.setup_ui(Information, title, msg);
  box.exec();
}

void MaterialMessageBox::warning(QWidget *parent, const QString &title,
                                 const QString &msg) {
  MaterialMessageBox box(parent);
  box.setup_ui(Warning, title, msg);
  box.exec();
}

void MaterialMessageBox::error(QWidget *parent, const QString &title,
                               const QString &msg) {
  MaterialMessageBox box(parent);
  box.setup_ui(Error, title, msg);
  box.exec();
}

int MaterialMessageBox::question(QWidget *parent, const QString &title,
                                 const QString &msg) {
  MaterialMessageBox box(parent);
  box.setup_ui(Question, title, msg);
  return box.exec();
}

// --- 核心 UI 逻辑 ---

void MaterialMessageBox::setup_ui(Type type, const QString &title,
                                  const QString &msg) {
  // 1. 布局结构 (Container + Shadow)
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(10, 10, 10, 10);

  QWidget *container = new QWidget(this);
  container->setStyleSheet("background-color: white; border-radius: 8px;");

  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 80));
  shadow->setOffset(0, 4);
  container->setGraphicsEffect(shadow);

  main_layout->addWidget(container);

  QVBoxLayout *content_layout = new QVBoxLayout(container);
  content_layout->setContentsMargins(20, 20, 20, 20);

  // 2. 标题颜色根据类型变化
  QLabel *title_label = new QLabel(title, this);
  QString title_color = "#333333";
  if (type == Error)
    title_color = "#B3261E"; // 红色
  if (type == Warning)
    title_color = "#F57F17"; // 橙色
  if (type == Success)
    title_color = "#198754"; // 绿色

  title_label->setStyleSheet(
      QString("font-size: 16px; font-weight: bold; color: %1;")
          .arg(title_color));
  content_layout->addWidget(title_label);

  content_layout->addSpacing(10);

  // 3. 内容
  QLabel *msg_label = new QLabel(msg, this);
  msg_label->setWordWrap(true); // 自动换行
  msg_label->setStyleSheet("font-size: 14px; color: #5F6368;");
  content_layout->addWidget(msg_label);

  content_layout->addStretch();

  // 4. 按钮
  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->addStretch();

  if (type == Question) {
    MaterialButton *cancel_btn =
        new MaterialButton("取消", MaterialButton::Normal, this);
    cancel_btn->setFixedSize(70, 32);
    cancel_btn->set_theme_color(QColor("#9E9E9E"));
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    btn_layout->addWidget(cancel_btn);

    MaterialButton *ok_btn =
        new MaterialButton("确定", MaterialButton::Normal, this);
    ok_btn->setFixedSize(70, 32);
    ok_btn->set_theme_color(QColor("#0B57D0"));
    connect(ok_btn, &QPushButton::clicked, this, &QDialog::accept);
    btn_layout->addWidget(ok_btn);
  } else {
    // Info, Warning, Error 只有一个“知道了”按钮
    MaterialButton *ok_btn =
        new MaterialButton("知道了", MaterialButton::Normal, this);
    ok_btn->setFixedSize(80, 32);
    if (type == Error)
      ok_btn->set_theme_color(QColor("#B3261E"));
    else
      ok_btn->set_theme_color(QColor("#0B57D0"));

    connect(ok_btn, &QPushButton::clicked, this, &QDialog::accept);
    btn_layout->addWidget(ok_btn);
  }

  content_layout->addLayout(btn_layout);
}

void MaterialMessageBox::showEvent(QShowEvent *event) {
  if (auto mask = MaskWidget::instance()) {
    mask->show_mask();
  }
  QDialog::showEvent(event);
}

void MaterialMessageBox::done(int r) {
  if (auto mask = MaskWidget::instance()) {
    mask->hide_mask();
  }
  QDialog::done(r);
}