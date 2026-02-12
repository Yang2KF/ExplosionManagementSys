#include "category_tree_model.h"
#include "db/db_manager.h"
#include <QIcon>
#include <QSqlQuery>
#include <QSqlRecord>

CategoryTreeModel::CategoryTreeModel(QObject *parent)
    : QStandardItemModel(parent) {
  // 设置表头
  setHorizontalHeaderLabels(QStringList() << "分类名称");
}

void CategoryTreeModel::reload() {
  this->removeRows(0, this->rowCount()); // 清空现有数据

  // 从根节点开始加载
  load_children(this->invisibleRootItem(), 0);
}

void CategoryTreeModel::load_children(QStandardItem *parent_item,
                                      int parent_id) {
  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);

  query.prepare("SELECT id, name FROM categories WHERE parent_id = :pid");
  query.bindValue(":pid", parent_id);

  if (query.exec()) {
    while (query.next()) {
      int id = query.value("id").toInt();
      QString name = query.value("name").toString();

      // 创建树节点
      QStandardItem *item = new QStandardItem(name);

      // 重要：将数据库 ID 藏在 Item 的 Data 里，方便后续点击时获取
      item->setData(id, Qt::UserRole + 1);

      item->setEditable(false); // 禁止直接双击编辑文本

      // 递归查找该节点的子节点
      load_children(item, id);

      // 添加到父节点下
      parent_item->appendRow(item);
    }
  }
}