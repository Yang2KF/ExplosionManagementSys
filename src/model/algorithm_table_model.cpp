#include "algorithm_table_model.h"
#include "db/db_manager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

AlgorithmTableModel::AlgorithmTableModel(QObject *parent)
    : QAbstractTableModel(parent) {
  headers_ << "编号" << "算法名称" << "调用标识" << "创建时间";
}

int AlgorithmTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return data_list_.size();
}

int AlgorithmTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
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
  if (!index.isValid() || index.row() >= data_list_.size()) {
    return QVariant{};
  }

  const AlgorithmInfo &item = data_list_.at(index.row());

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case 0:
      return item.displayId;
    case 1:
      return item.name;
    case 2:
      return item.funcName;
    case 3:
      return item.createdAt.toString("yyyy-MM-dd HH:mm");
    default:
      return QVariant{};
    }
  }

  if (role == Qt::TextAlignmentRole) {
    if (index.column() == 0) {
      return Qt::AlignCenter;
    }
    return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QVariant{};
}

void AlgorithmTableModel::load_data(const QString &category_id) {
  beginResetModel();
  data_list_.clear();

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);

  QString sql =
      "SELECT ID, ALGID, ALGNAME, COMMENTS, CALLID, CREATED_AT, CLSID, SRC, "
      "SRC_TYPE "
      "FROM algorithms";
  if (!category_id.isEmpty()) {
    sql += " WHERE CLSID = :cid";
  }
  sql += " ORDER BY ID ASC";
  query.prepare(sql);

  if (!category_id.isEmpty()) {
    query.bindValue(":cid", category_id);
  }

  if (query.exec()) {
    while (query.next()) {
      AlgorithmInfo info;
      info.displayId = query.value("ID").toLongLong();
      info.id = query.value("ALGID").toString();
      info.name = query.value("ALGNAME").toString();
      info.description = query.value("COMMENTS").toString();
      info.funcName = query.value("CALLID").toString();
      info.createdAt = query.value("CREATED_AT").toDateTime();
      info.categoryId = query.value("CLSID").toString();
      info.filePath = query.value("SRC").toString();
      info.sourceType = query.value("SRC_TYPE").toString();
      data_list_.append(info);
    }
  } else {
    qDebug() << "加载算法数据失败：" << query.lastError();
  }

  endResetModel();
}

void AlgorithmTableModel::search_data(const QString &keyword) {
  beginResetModel();
  data_list_.clear();

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "SELECT ID, ALGID, ALGNAME, COMMENTS, CALLID, CREATED_AT, CLSID, SRC, "
      "SRC_TYPE FROM algorithms WHERE ALGNAME LIKE :key ORDER BY ID ASC");
  query.bindValue(":key", "%" + keyword + "%");

  if (query.exec()) {
    while (query.next()) {
      AlgorithmInfo info;
      info.displayId = query.value("ID").toLongLong();
      info.id = query.value("ALGID").toString();
      info.name = query.value("ALGNAME").toString();
      info.description = query.value("COMMENTS").toString();
      info.funcName = query.value("CALLID").toString();
      info.createdAt = query.value("CREATED_AT").toDateTime();
      info.categoryId = query.value("CLSID").toString();
      info.filePath = query.value("SRC").toString();
      info.sourceType = query.value("SRC_TYPE").toString();
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
