#include "category_service.h"
#include "db/db_manager.h"
#include <QHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

QList<AlgoCategory>
CategoryService::fetch_all_categories(QString *error_message) const {
  QList<AlgoCategory> result;

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  if (!query.exec("SELECT CATEGORY_ID, CATEGORY_NAME, PARENT_ID, COMMENTS "
                  "FROM alg_category "
                  "ORDER BY PARENT_ID ASC, CATEGORY_NAME ASC")) {
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
    item.comments = query.value("COMMENTS").toString();
    result.append(item);
  }
  return result;
}

QStringList CategoryService::fetch_subtree_ids(const QString &root_id,
                                               QString *error_message) const {
  if (root_id.trimmed().isEmpty()) {
    return {};
  }

  const QList<AlgoCategory> categories = fetch_all_categories(error_message);
  if (categories.isEmpty()) {
    return QStringList{root_id};
  }

  QHash<QString, QStringList> children_by_parent;
  for (const AlgoCategory &category : categories) {
    children_by_parent[category.parentId.trimmed()].append(category.id.trimmed());
  }

  QStringList pending{root_id.trimmed()};
  QStringList result;
  while (!pending.isEmpty()) {
    const QString current_id = pending.takeFirst();
    if (current_id.isEmpty() || result.contains(current_id)) {
      continue;
    }

    result.append(current_id);
    pending.append(children_by_parent.value(current_id));
  }

  return result;
}
