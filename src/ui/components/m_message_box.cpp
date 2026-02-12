#include "m_message_box.h"
#include "m_button.h"
#include "mask_widget.h"
#include "ui_system.h"
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QStyle>
#include <QVBoxLayout>

MaterialMessageBox::MaterialMessageBox(QWidget *parent) : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
  resize(320, 180);
}

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

void MaterialMessageBox::setup_ui(Type type, const QString &title,
                                  const QString &msg) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(10, 10, 10, 10);

  QWidget *container = new QWidget(this);
  container->setObjectName("MessageBoxContainer");

  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 80));
  shadow->setOffset(0, 4);
  container->setGraphicsEffect(shadow);
  main_layout->addWidget(container);

  QVBoxLayout *content_layout = new QVBoxLayout(container);
  content_layout->setContentsMargins(20, 20, 20, 20);

  QString msg_type = "info";
  if (type == Error)
    msg_type = "error";
  else if (type == Warning)
    msg_type = "warning";
  else if (type == Success)
    msg_type = "success";

  QLabel *title_label = new QLabel(title, this);
  title_label->setObjectName("MessageBoxTitle");
  title_label->setProperty("msgType", msg_type);
  title_label->style()->unpolish(title_label);
  title_label->style()->polish(title_label);
  content_layout->addWidget(title_label);

  content_layout->addSpacing(10);

  QLabel *msg_label = new QLabel(msg, this);
  msg_label->setObjectName("MessageBoxText");
  msg_label->setWordWrap(true);
  content_layout->addWidget(msg_label);

  content_layout->addStretch();

  QHBoxLayout *btn_layout = new QHBoxLayout();
  btn_layout->addStretch();

  if (type == Question) {
    MaterialButton *cancel_btn =
        new MaterialButton("Cancel", MaterialButton::Normal, this);
    cancel_btn->setFixedSize(70, 32);
    cancel_btn->set_theme_color(UISystem::instance().neutral());
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    btn_layout->addWidget(cancel_btn);

    MaterialButton *ok_btn =
        new MaterialButton("Confirm", MaterialButton::Normal, this);
    ok_btn->setFixedSize(70, 32);
    ok_btn->set_theme_color(
        UISystem::instance().message_action_color(msg_type));
    connect(ok_btn, &QPushButton::clicked, this, &QDialog::accept);
    btn_layout->addWidget(ok_btn);
  } else {
    MaterialButton *ok_btn =
        new MaterialButton("OK", MaterialButton::Normal, this);
    ok_btn->setFixedSize(80, 32);
    ok_btn->set_theme_color(
        UISystem::instance().message_action_color(msg_type));
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
