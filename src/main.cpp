#include "ui/main_window.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QFile file(":/styles/style"); // 对应 qrc 中的 alias
  if (file.open(QFile::ReadOnly)) {
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);
    file.close();
  }

  MainWindow window;
  window.show();

  return app.exec();
}