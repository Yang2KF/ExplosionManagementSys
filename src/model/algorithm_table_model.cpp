#include "algorithm_table_model.h"
#include "db/db_manager.h"
#include <QSqlError>
#include <QSqlQuery>

AlgorithmTableModel::AlgorithmTableModel(QObject *parent)
    : QAbstractTableModel(parent) {
  headers_ << "ID" << "算法名称" << "函数入口" << "创建时间";
}

int AlgorithmTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;
  return data_list_.size();
}

int AlgorithmTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;
  return headers_.size();
}

QVariant AlgorithmTableModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    return headers_.at(section);
  }
  return QVariant{};
}

QVariant AlgorithmTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= data_list_.size())
    return QVariant{};

  const AlgorithmInfo &item = data_list_.at(index.row());

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case 0:
      return item.id;
    case 1:
      return item.name;
    case 2:
      return item.funcName;
    case 3:
      return item.createdAt.toString("yyyy-MM-dd HH:mm");
    default:
      break;
    }
  }
  // 文本对齐方式
  else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 0)
      return Qt::AlignCenter;
    return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QVariant{};
}

void AlgorithmTableModel::load_data(int category_id) {
  beginResetModel(); // 通知 View：数据要大换血了，准备刷新
  data_list_.clear();

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);

  // 准备 SQL
  QString sql =
      "SELECT id, name, description, func_name, created_at FROM algorithms";
  if (category_id != -1) {
    sql += " WHERE category_id = :cid";
  }
  query.prepare(sql);

  if (category_id != -1) {
    query.bindValue(":cid", category_id);
  }

  if (query.exec()) {
    while (query.next()) {
      AlgorithmInfo info;
      info.id = query.value("id").toInt();
      info.name = query.value("name").toString();
      info.description = query.value("description").toString();
      info.funcName = query.value("func_name").toString();
      info.createdAt = query.value("created_at").toDateTime();

      data_list_.append(info);
    }
  } else {
    qDebug() << "Load algorithms failed:" << query.lastError();
  }

  endResetModel(); // 刷新完成
}

void AlgorithmTableModel::search_data(const QString &keyword) {
  beginResetModel();
  data_list_.clear();

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare("SELECT id, name, description, func_name, created_at FROM "
                "algorithms WHERE name LIKE :key");
  query.bindValue(":key", "%" + keyword + "%");

  if (query.exec()) {
    while (query.next()) {
      AlgorithmInfo info;
      info.id = query.value("id").toInt();
      info.name = query.value("name").toString();
      info.description = query.value("description").toString();
      info.funcName = query.value("func_name").toString();
      info.createdAt = query.value("created_at").toDateTime();
      data_list_.append(info);
    }
  }
  endResetModel();
}

AlgorithmInfo AlgorithmTableModel::get_item(int row) const {
  if (row >= 0 && row < data_list_.size()) {
    return data_list_.at(row);
  }
  return AlgorithmInfo{};
}