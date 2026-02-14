#include "category_tree_model.h"
#include "db/db_manager.h"
#include <QSqlQuery>

CategoryTreeModel::CategoryTreeModel(QObject *parent)
    : QStandardItemModel(parent) {
  setHorizontalHeaderLabels(QStringList() << "Category");
}

void CategoryTreeModel::reload() {
  removeRows(0, rowCount());
  load_children(invisibleRootItem(), "0");
}

void CategoryTreeModel::load_children(QStandardItem *parent_item,
                                      const QString &parent_id) {
  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare("SELECT CATEGORY_ID, CATEGORY_NAME FROM alg_category "
                "WHERE PARENT_ID = :pid");
  query.bindValue(":pid", parent_id);

  if (!query.exec()) {
    return;
  }

  while (query.next()) {
    const QString id = query.value("CATEGORY_ID").toString();
    const QString name = query.value("CATEGORY_NAME").toString();

    QStandardItem *item = new QStandardItem(name);
    item->setData(id, Qt::UserRole + 1);
    item->setEditable(false);

    load_children(item, id);
    parent_item->appendRow(item);
  }
}
