#include "db/db_manager.h"
#include "ui/components/ui_system.h"
#include "ui/main_window.h"
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  const int font_id =
      QFontDatabase::addApplicationFont(":/fonts/inter_variable");
  if (font_id >= 0) {
    const QStringList families = QFontDatabase::applicationFontFamilies(font_id);
    if (!families.isEmpty()) {
      QFont app_font(families.first());
      app_font.setStyleStrategy(QFont::PreferAntialias);
      app.setFont(app_font);
    }
  }

  if (!DBManager::instance().init()) {
    qWarning() << "数据库初始化失败";
  }

  QFile file(":/styles/style");
  if (file.open(QFile::ReadOnly)) {
    const QString style_sheet =
        UISystem::instance().render_qss(QLatin1String(file.readAll()));
    app.setStyleSheet(style_sheet);
    file.close();
  }

  MainWindow window;
  window.show();

  const int ret = app.exec();

  // QApplication 结束前关闭 DB 连接，避免静态析构顺序问题。
  DBManager::instance().close();

  return ret;
}
