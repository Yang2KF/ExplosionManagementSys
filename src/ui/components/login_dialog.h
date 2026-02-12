#pragma once

#include "m_button.h"
#include "m_input.h"
#include <QDialog>

class LoginDialog : public QDialog {
  Q_OBJECT

public:
  explicit LoginDialog(QWidget *parent = nullptr);

signals:
  void loginSuccess();

protected:
  // 覆盖事件以支持遮罩
  void showEvent(QShowEvent *event) override;
  void done(int r) override;

private slots:
  void onLoginClicked();

private:
  void init_ui();

  MaterialInput *user_edit_;
  MaterialInput *pass_edit_;
  MaterialButton *login_btn_;
  MaterialButton *cancel_btn_; // 新增一个取消/关闭按钮
};