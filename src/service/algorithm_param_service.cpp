#include "algorithm_param_service.h"
#include "db/db_manager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

QList<AlgorithmParam>
AlgorithmParamService::fetch_params(const QString &algo_id,
                                    QString *error_message) const {
  QList<AlgorithmParam> result;

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "SELECT UUID, ALGID, P_IDENTIFIER, P_ZHNAME, UNIT, DATATYPE, "
      "DEFAULT_VALUE, REQUIRED, MIN_VALUE, MAX_VALUE, TOOLTIP, COMMENTS, "
      "SHOWORDER "
      "FROM alg_inparams "
      "WHERE ALGID = :algid "
      "ORDER BY SHOWORDER ASC, P_IDENTIFIER ASC");
  query.bindValue(":algid", algo_id.trimmed());

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return result;
  }

  while (query.next()) {
    AlgorithmParam param;
    param.id = query.value("UUID").toString();
    param.algoId = query.value("ALGID").toString();
    param.identifier = query.value("P_IDENTIFIER").toString();
    param.name = query.value("P_ZHNAME").toString();
    param.unit = query.value("UNIT").toString();
    param.type = query.value("DATATYPE").toString();
    param.defaultValue = query.value("DEFAULT_VALUE").toString();
    param.required = query.value("REQUIRED").toInt() != 0;
    param.minValue = query.value("MIN_VALUE").toString();
    param.maxValue = query.value("MAX_VALUE").toString();
    param.tooltip = query.value("TOOLTIP").toString();
    param.comments = query.value("COMMENTS").toString();
    param.order = query.value("SHOWORDER").toInt();
    result.append(param);
  }

  return result;
}

bool AlgorithmParamService::create_param(AlgorithmParam param,
                                         QString *error_message) const {
  if (param.id.trimmed().isEmpty()) {
    param.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  }

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "INSERT INTO alg_inparams "
      "(UUID, ALGID, P_IDENTIFIER, P_ZHNAME, UNIT, DATATYPE, DEFAULT_VALUE, "
      "REQUIRED, MIN_VALUE, MAX_VALUE, TOOLTIP, COMMENTS, SHOWORDER) "
      "VALUES (:id, :algid, :identifier, :name, :unit, :type, :default_value, "
      ":required, :min_value, :max_value, :tooltip, :comments, :showorder)");
  query.bindValue(":id", param.id.trimmed());
  query.bindValue(":algid", param.algoId.trimmed());
  query.bindValue(":identifier", param.identifier.trimmed());
  query.bindValue(":name", param.name.trimmed());
  query.bindValue(":unit", param.unit.trimmed());
  query.bindValue(":type", param.type.trimmed());
  query.bindValue(":default_value", param.defaultValue.trimmed());
  query.bindValue(":required", param.required ? 1 : 0);
  query.bindValue(":min_value", param.minValue.trimmed());
  query.bindValue(":max_value", param.maxValue.trimmed());
  query.bindValue(":tooltip", param.tooltip.trimmed());
  query.bindValue(":comments", param.comments.trimmed());
  query.bindValue(":showorder", param.order);

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}

bool AlgorithmParamService::update_param(const AlgorithmParam &param,
                                         QString *error_message) const {
  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "UPDATE alg_inparams SET "
      "ALGID=:algid, P_IDENTIFIER=:identifier, P_ZHNAME=:name, UNIT=:unit, "
      "DATATYPE=:type, DEFAULT_VALUE=:default_value, REQUIRED=:required, "
      "MIN_VALUE=:min_value, MAX_VALUE=:max_value, TOOLTIP=:tooltip, "
      "COMMENTS=:comments, SHOWORDER=:showorder "
      "WHERE UUID=:id");
  query.bindValue(":id", param.id.trimmed());
  query.bindValue(":algid", param.algoId.trimmed());
  query.bindValue(":identifier", param.identifier.trimmed());
  query.bindValue(":name", param.name.trimmed());
  query.bindValue(":unit", param.unit.trimmed());
  query.bindValue(":type", param.type.trimmed());
  query.bindValue(":default_value", param.defaultValue.trimmed());
  query.bindValue(":required", param.required ? 1 : 0);
  query.bindValue(":min_value", param.minValue.trimmed());
  query.bindValue(":max_value", param.maxValue.trimmed());
  query.bindValue(":tooltip", param.tooltip.trimmed());
  query.bindValue(":comments", param.comments.trimmed());
  query.bindValue(":showorder", param.order);

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}

bool AlgorithmParamService::delete_param(const QString &param_id,
                                         QString *error_message) const {
  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare("DELETE FROM alg_inparams WHERE UUID = :id");
  query.bindValue(":id", param_id.trimmed());

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}
