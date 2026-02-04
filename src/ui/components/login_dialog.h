#pragma once

#include "m_button.h"
#include "m_input.h"
#include <QDialog>

class LoginDialog : public QDialog {
  Q_OBJECT

public:
  explicit LoginDialog(QWidget *parent = nullptr);

signals:
  // 这是一个业务信号：告诉主程序“用户通过验证了”
  void loginSuccess();

private slots:
  void onLoginClicked();

private:
  MaterialInput *user_edit_;
  MaterialInput *pass_edit_;
  MaterialButton *login_btn_;
};