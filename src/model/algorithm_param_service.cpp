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
      "DEFAULT_VALUE, MIN_VALUE, MAX_VALUE, VALIDATOR, TOOLTIP, REQUIRED, "
      "SHOWORDER "
      "FROM alg_inparams WHERE ALGID = :algid ORDER BY SHOWORDER ASC, "
      "P_IDENTIFIER ASC");
  query.bindValue(":algid", algo_id);

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
    param.minValue = query.value("MIN_VALUE").toString();
    param.maxValue = query.value("MAX_VALUE").toString();
    param.validator = query.value("VALIDATOR").toString();
    param.tooltip = query.value("TOOLTIP").toString();
    param.required = query.value("REQUIRED").toInt() != 0;
    param.order = query.value("SHOWORDER").toInt();
    result.append(param);
  }

  return result;
}

bool AlgorithmParamService::create_param(AlgorithmParam param,
                                         QString *error_message) const {
  if (param.id.isEmpty()) {
    param.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  }

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "INSERT INTO alg_inparams "
      "(UUID, ALGID, P_IDENTIFIER, P_ZHNAME, UNIT, DATATYPE, DEFAULT_VALUE, "
      "MIN_VALUE, MAX_VALUE, VALIDATOR, TOOLTIP, REQUIRED, SHOWORDER) "
      "VALUES (:id, :algid, :identifier, :name, :unit, :type, :default_value, "
      ":min_value, :max_value, :validator, :tooltip, :required, :showorder)");
  query.bindValue(":id", param.id);
  query.bindValue(":algid", param.algoId);
  query.bindValue(":identifier", param.identifier);
  query.bindValue(":name", param.name);
  query.bindValue(":unit", param.unit);
  query.bindValue(":type", param.type);
  query.bindValue(":default_value", param.defaultValue);
  query.bindValue(":min_value", param.minValue);
  query.bindValue(":max_value", param.maxValue);
  query.bindValue(":validator", param.validator);
  query.bindValue(":tooltip", param.tooltip);
  query.bindValue(":required", param.required ? 1 : 0);
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
      "UPDATE alg_inparams SET ALGID=:algid, P_IDENTIFIER=:identifier, "
      "P_ZHNAME=:name, UNIT=:unit, DATATYPE=:type, DEFAULT_VALUE=:default_value, "
      "MIN_VALUE=:min_value, MAX_VALUE=:max_value, VALIDATOR=:validator, "
      "TOOLTIP=:tooltip, REQUIRED=:required, SHOWORDER=:showorder "
      "WHERE UUID=:id");
  query.bindValue(":id", param.id);
  query.bindValue(":algid", param.algoId);
  query.bindValue(":identifier", param.identifier);
  query.bindValue(":name", param.name);
  query.bindValue(":unit", param.unit);
  query.bindValue(":type", param.type);
  query.bindValue(":default_value", param.defaultValue);
  query.bindValue(":min_value", param.minValue);
  query.bindValue(":max_value", param.maxValue);
  query.bindValue(":validator", param.validator);
  query.bindValue(":tooltip", param.tooltip);
  query.bindValue(":required", param.required ? 1 : 0);
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
  query.bindValue(":id", param_id);

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }
  return true;
}
