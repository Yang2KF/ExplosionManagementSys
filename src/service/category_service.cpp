#include "category_service.h"
#include "db/db_manager.h"
#include <QSqlError>
#include <QSqlQuery>

QList<AlgoCategory>
CategoryService::fetch_all_categories(QString *error_message) const {
  QList<AlgoCategory> result;

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  if (!query.exec("SELECT CATEGORY_ID, CATEGORY_NAME, PARENT_ID FROM "
                  "alg_category")) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return result;
  }

  while (query.next()) {
    AlgoCategory item;
    item.id = query.value("CATEGORY_ID").toString();
    item.name = query.value("CATEGORY_NAME").toString();
    item.parentId = query.value("PARENT_ID").toString();
    result.append(item);
  }
  return result;
}
