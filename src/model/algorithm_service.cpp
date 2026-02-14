#include "algorithm_service.h"
#include "db/db_manager.h"
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

bool AlgorithmService::create_algorithm(AlgorithmInfo info,
                                        QString *error_message) {
  if (info.id.isEmpty()) {
    info.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  }

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "INSERT INTO algorithms "
      "(ALGID, ALGNAME, COMMENTS, CALLID, SRC, CLSID, SRC_TYPE, CREATED_AT) "
      "VALUES (:id, :name, :comments, :call_id, :src, :clsid, :src_type, "
      ":created_at)");
  query.bindValue(":id", info.id);
  query.bindValue(":name", info.name);
  query.bindValue(":comments", info.description);
  query.bindValue(":call_id", info.funcName);
  query.bindValue(":src", info.filePath);
  query.bindValue(":clsid", info.categoryId);
  query.bindValue(":src_type", "1");
  query.bindValue(":created_at", QDateTime::currentDateTime());

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}

bool AlgorithmService::update_algorithm(const AlgorithmInfo &info,
                                        QString *error_message) {
  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare("UPDATE algorithms SET ALGNAME=:name, COMMENTS=:comments, "
                "CALLID=:call_id, SRC=:src, CLSID=:clsid WHERE ALGID=:id");
  query.bindValue(":name", info.name);
  query.bindValue(":comments", info.description);
  query.bindValue(":call_id", info.funcName);
  query.bindValue(":src", info.filePath);
  query.bindValue(":clsid", info.categoryId);
  query.bindValue(":id", info.id);

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}

bool AlgorithmService::delete_algorithm(const QString &alg_id,
                                        QString *error_message) {
  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare("DELETE FROM algorithms WHERE ALGID = :id");
  query.bindValue(":id", alg_id);

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}
