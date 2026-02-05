#include "login_dialog.h"
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
  this->setObjectName("LoginDialog");
  // 设置窗口属性
  setWindowTitle("系统登录");
  resize(400, 500);

  // 布局管理器
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(40, 40, 40, 40); // 设置四周留白
  layout->setSpacing(20);                     // 组件间距

  //  标题
  QLabel *titleLabel = new QLabel("爆炸毁伤\n算法管理系统", this);
  titleLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(titleLabel);

  layout->addSpacing(30); // 增加一点距离

  //  输入框区域
  user_edit_ = new MaterialInput(this);
  user_edit_->setPlaceholderText("用户名");
  layout->addWidget(user_edit_);

  pass_edit_ = new MaterialInput(this);
  pass_edit_->setPlaceholderText("密码");
  pass_edit_->setEchoMode(QLineEdit::Password); // 密码模式
  layout->addWidget(pass_edit_);

  layout->addSpacing(30);

  //  登录按钮
  login_btn_ = new MaterialButton("登 录", MaterialButton::Normal, this);
  login_btn_->set_theme_color(QColor("#0B57D0"));
  layout->addWidget(login_btn_);

  layout->addStretch(); // 底部弹簧，把内容顶上去

  // 4. 连接信号
  connect(login_btn_, &QPushButton::clicked, this,
          &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked() {
  // 模拟验证逻辑 (后续我们会连接数据库)
  QString user = user_edit_->text();
  QString pass = user_edit_->text();

  if (user == "admin" && pass == "123456") {
    emit loginSuccess(); // 发射成功信号
    accept();            // 关闭对话框并返回 QDialog::Accepted
  } else {
    // 原生MessageBox演示，后期需修改弹窗
    QMessageBox::warning(this, "登录失败",
                         "用户名或密码错误！\n(admin/123456)");
    pass_edit_->clear();
    pass_edit_->setFocus();
  }
}