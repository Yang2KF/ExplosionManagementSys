#include "algorithm_service.h"
#include "db/db_manager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

bool AlgorithmService::create_algorithm(AlgorithmInfo info,
                                        QString *error_message) {
  if (info.id.trimmed().isEmpty()) {
    info.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  }
  if (info.sourceType.trimmed().isEmpty()) {
    info.sourceType = "dll";
  }

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "INSERT INTO algorithms "
      "(ALGID, CATEGORY_ID, ALGNAME, ENTRY_FUNC, SRC, SRC_TYPE, COMMENTS) "
      "VALUES (:id, :category_id, :name, :entry_func, :src, :src_type, "
      ":comments)");
  query.bindValue(":id", info.id.trimmed());
  query.bindValue(":category_id", info.categoryId.trimmed());
  query.bindValue(":name", info.name.trimmed());
  query.bindValue(":entry_func", info.funcName.trimmed());
  query.bindValue(":src", info.filePath.trimmed());
  query.bindValue(":src_type", info.sourceType.trimmed());
  query.bindValue(":comments", info.description.trimmed());

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
  query.prepare("UPDATE algorithms SET "
                "CATEGORY_ID=:category_id, ALGNAME=:name, ENTRY_FUNC=:entry_func, "
                "SRC=:src, SRC_TYPE=:src_type, COMMENTS=:comments "
                "WHERE ALGID=:id");
  query.bindValue(":category_id", info.categoryId.trimmed());
  query.bindValue(":name", info.name.trimmed());
  query.bindValue(":entry_func", info.funcName.trimmed());
  query.bindValue(":src", info.filePath.trimmed());
  query.bindValue(":src_type",
                  info.sourceType.trimmed().isEmpty() ? "dll"
                                                       : info.sourceType.trimmed());
  query.bindValue(":comments", info.description.trimmed());
  query.bindValue(":id", info.id.trimmed());

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
  query.bindValue(":id", alg_id.trimmed());

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}
