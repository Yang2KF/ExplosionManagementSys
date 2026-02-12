#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

class DBManager {
public:
  DBManager(const DBManager &) = delete;            // 禁用拷贝构造
  DBManager &operator=(const DBManager &) = delete; // 禁用赋值操作

  static DBManager &instance();

  bool init();

  void close();

  QSqlDatabase database();

private:
  DBManager();
  ~DBManager() = default; // 析构不再做任何数据库操作

  bool create_tables();
  void seed_data();

  const QString DB_NAME = "explosion_algo.db";
  const QString CONN_NAME = "ExplosionConnection";
};