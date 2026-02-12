#include "login_dialog.h"
#include "db/db_manager.h"
#include "m_message_box.h"
#include "mask_widget.h"
#include "ui_system.h"
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
  setObjectName("LoginDialog");
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
  resize(380, 450);

  init_ui();
}

void LoginDialog::showEvent(QShowEvent *event) {
  if (auto mask = MaskWidget::instance()) {
    mask->show_mask();
  }
  QDialog::showEvent(event);
  raise();
}

void LoginDialog::done(int r) {
  if (auto mask = MaskWidget::instance()) {
    mask->hide_mask();
  }
  QDialog::done(r);
}

void LoginDialog::init_ui() {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(10, 10, 10, 10);

  QWidget *container = new QWidget(this);
  container->setObjectName("LoginDialogContainer");

  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 60));
  shadow->setOffset(0, 5);
  container->setGraphicsEffect(shadow);

  main_layout->addWidget(container);

  QVBoxLayout *content_layout = new QVBoxLayout(container);
  content_layout->setContentsMargins(40, 40, 40, 40);
  content_layout->setSpacing(25);

  QLabel *title_label = new QLabel("System Login", this);
  title_label->setObjectName("LoginDialogTitle");
  title_label->setAlignment(Qt::AlignCenter);
  content_layout->addWidget(title_label);

  QLabel *sub_title = new QLabel("Explosion Damage Algorithm Management", this);
  sub_title->setObjectName("LoginDialogSubtitle");
  sub_title->setAlignment(Qt::AlignCenter);
  content_layout->addWidget(sub_title);

  user_edit_ = new MaterialInput(this);
  user_edit_->setPlaceholderText("Username");
  user_edit_->setText("admin");
  content_layout->addWidget(user_edit_);

  pass_edit_ = new MaterialInput(this);
  pass_edit_->setPlaceholderText("Password");
  pass_edit_->setEchoMode(QLineEdit::Password);
  pass_edit_->setText("123456");
  content_layout->addWidget(pass_edit_);

  connect(pass_edit_, &QLineEdit::returnPressed, this,
          &LoginDialog::onLoginClicked);

  content_layout->addStretch();

  login_btn_ = new MaterialButton("Login", MaterialButton::Normal, this);
  login_btn_->set_theme_color(UISystem::instance().bg_primary());
  login_btn_->setFixedHeight(40);
  content_layout->addWidget(login_btn_);

  cancel_btn_ = new MaterialButton("Cancel", MaterialButton::Normal, this);
  cancel_btn_->set_theme_color(UISystem::instance().neutral());
  cancel_btn_->setFixedHeight(40);
  content_layout->addWidget(cancel_btn_);

  connect(login_btn_, &QPushButton::clicked, this,
          &LoginDialog::onLoginClicked);
  connect(cancel_btn_, &QPushButton::clicked, this, &QDialog::reject);
}

void LoginDialog::onLoginClicked() {
  QString user = user_edit_->text().trimmed();
  QString pass = pass_edit_->text().trimmed();

  if (user.isEmpty() || pass.isEmpty()) {
    MaterialMessageBox::warning(this, "Hint",
                                "Please enter username and password.");
    return;
  }

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare("SELECT role FROM users WHERE username = :u AND password = :p");
  query.bindValue(":u", user);
  query.bindValue(":p", pass);

  if (query.exec()) {
    if (query.next()) {
      emit loginSuccess();
      accept();
    } else {
      MaterialMessageBox::error(this, "Login Failed",
                                "Invalid username or password.");
      pass_edit_->clear();
      pass_edit_->setFocus();
    }
  } else {
    MaterialMessageBox::error(
        this, "Error", "Database query failed: " + query.lastError().text());
  }
}
