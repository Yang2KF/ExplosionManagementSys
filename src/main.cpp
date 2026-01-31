#include "ui/login_dialog.h"
#include <QApplication>
#include <QLabel>
#include <QWidget>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // 1. 先实例化并显示登录界面
  LoginDialog login;

  // exec() 会阻塞代码执行，直到登录窗口关闭
  // 如果返回 Accepted (我们在 onLoginClicked 里调用了 accept())，则继续
  if (login.exec() == QDialog::Accepted) {

    // 2. 登录成功，显示主界面 (这里先用简单窗口代替)
    QWidget mainWindow;
    mainWindow.setWindowTitle("Explosion Damage System - Dashboard");
    mainWindow.resize(800, 600);

    QLabel *label = new QLabel("欢迎进入系统主界面", &mainWindow);
    label->setAlignment(Qt::AlignCenter);

    mainWindow.show();

    // 进入主事件循环
    return app.exec();
  }

  // 如果用户直接关闭了登录框，程序直接退出
  return 0;
}