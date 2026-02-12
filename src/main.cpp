#include "db/db_manager.h"
#include "ui/main_window.h"
#include "ui/components/ui_system.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  if (!DBManager::instance().init()) {
    qWarning() << "Database Init Failed!";
  }

  QFile file(":/styles/style");
  if (file.open(QFile::ReadOnly)) {
    QString styleSheet =
        UISystem::instance().render_qss(QLatin1String(file.readAll()));
    app.setStyleSheet(styleSheet);
    file.close();
  }

  MainWindow window;
  window.show();

  int ret = app.exec();

  // 此时 QApplication 尚未完全销毁，QSqlDatabase 仍可工作
  DBManager::instance().close();

  return ret;
}
