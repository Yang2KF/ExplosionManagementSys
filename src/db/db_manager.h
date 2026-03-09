#pragma once

#include <QSqlDatabase>
#include <QString>

class DBManager {
public:
  DBManager(const DBManager &) = delete;
  DBManager &operator=(const DBManager &) = delete;

  static DBManager &instance();

  bool init();
  void close();
  QSqlDatabase database();

private:
  DBManager();
  ~DBManager() = default;

  bool create_tables();
  void seed_data();

  const QString CONN_NAME = "ExplosionConnection";
};
