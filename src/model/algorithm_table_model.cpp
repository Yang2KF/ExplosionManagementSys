#include "algorithm_table_model.h"
#include "db/db_manager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

AlgorithmTableModel::AlgorithmTableModel(QObject *parent)
    : QAbstractTableModel(parent) {
  headers_ << QStringLiteral("编号") << QStringLiteral("算法名称")
           << QStringLiteral("运行类型") << QStringLiteral("调用标识")
           << QStringLiteral("说明");
}

int AlgorithmTableModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : data_list_.size();
}

int AlgorithmTableModel::columnCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : headers_.size();
}

QVariant AlgorithmTableModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal &&
      section >= 0 && section < headers_.size()) {
    return headers_.at(section);
  }
  return QVariant{};
}

QVariant AlgorithmTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= data_list_.size()) {
    return QVariant{};
  }

  const AlgorithmInfo &item = data_list_.at(index.row());

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case 0:
      return index.row() + 1;
    case 1:
      return item.name;
    case 2:
      return item.sourceType.compare("python", Qt::CaseInsensitive) == 0
                 ? QStringLiteral("Python")
                 : QStringLiteral("DLL");
    case 3:
      return item.funcName;
    case 4:
      return item.description;
    default:
      return QVariant{};
    }
  }

  if (role == Qt::ToolTipRole) {
    switch (index.column()) {
    case 1:
      return item.name;
    case 3:
      return item.funcName;
    case 4:
      return item.description;
    default:
      return QVariant{};
    }
  }

  if (role == Qt::TextAlignmentRole) {
    if (index.column() == 0 || index.column() == 2) {
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
      "SELECT a.ALGID, a.CATEGORY_ID, c.CATEGORY_NAME, a.ALGNAME, a.COMMENTS, "
      "a.ENTRY_FUNC, a.SRC, a.SRC_TYPE "
      "FROM algorithms a "
      "LEFT JOIN alg_category c ON a.CATEGORY_ID = c.CATEGORY_ID";

  const QString trimmed_category_id = category_id.trimmed();
  if (!trimmed_category_id.isEmpty()) {
    QString error_message;
    const QStringList category_ids =
        category_service_.fetch_subtree_ids(trimmed_category_id, &error_message);
    if (!error_message.isEmpty()) {
      qDebug() << "Fetch category subtree failed:" << error_message;
    }

    if (category_ids.isEmpty()) {
      endResetModel();
      return;
    }

    QStringList placeholders;
    placeholders.reserve(category_ids.size());
    for (int i = 0; i < category_ids.size(); ++i) {
      placeholders.append("?");
    }

    sql += " WHERE a.CATEGORY_ID IN (" + placeholders.join(", ") + ")";
    sql += " ORDER BY a.ALGNAME ASC";
    query.prepare(sql);
    for (const QString &id : category_ids) {
      query.addBindValue(id);
    }
  } else {
    sql += " ORDER BY a.ALGNAME ASC";
    query.prepare(sql);
  }

  if (query.exec()) {
    load_from_query(&query);
  } else {
    qDebug() << "Load algorithms failed:" << query.lastError().text();
  }

  endResetModel();
}

void AlgorithmTableModel::search_data(const QString &keyword) {
  beginResetModel();
  data_list_.clear();

  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare(
      "SELECT a.ALGID, a.CATEGORY_ID, c.CATEGORY_NAME, a.ALGNAME, a.COMMENTS, "
      "a.ENTRY_FUNC, a.SRC, a.SRC_TYPE "
      "FROM algorithms a "
      "LEFT JOIN alg_category c ON a.CATEGORY_ID = c.CATEGORY_ID "
      "WHERE a.ALGNAME LIKE :keyword OR a.ENTRY_FUNC LIKE :keyword "
      "OR a.SRC LIKE :keyword OR a.COMMENTS LIKE :keyword "
      "OR c.CATEGORY_NAME LIKE :keyword "
      "ORDER BY a.ALGNAME ASC");
  query.bindValue(":keyword", "%" + keyword.trimmed() + "%");

  if (query.exec()) {
    load_from_query(&query);
  } else {
    qDebug() << "Search algorithms failed:" << query.lastError().text();
  }

  endResetModel();
}

AlgorithmInfo AlgorithmTableModel::get_item(int row) const {
  if (row >= 0 && row < data_list_.size()) {
    return data_list_.at(row);
  }
  return AlgorithmInfo{};
}

void AlgorithmTableModel::load_from_query(QSqlQuery *query) {
  if (!query) {
    return;
  }

  while (query->next()) {
    AlgorithmInfo info;
    info.id = query->value("ALGID").toString();
    info.categoryId = query->value("CATEGORY_ID").toString();
    info.categoryName = query->value("CATEGORY_NAME").toString();
    info.name = query->value("ALGNAME").toString();
    info.description = query->value("COMMENTS").toString();
    info.funcName = query->value("ENTRY_FUNC").toString();
    info.filePath = query->value("SRC").toString();
    info.sourceType = query->value("SRC_TYPE").toString();
    data_list_.append(info);
  }
}
