#pragma once

#include "entities/algorithm_data.h"
#include "service/category_service.h"
#include <QAbstractTableModel>
#include <QList>
#include <QStringList>

class QSqlQuery;

class AlgorithmTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit AlgorithmTableModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  void load_data(const QString &category_id);
  void search_data(const QString &keyword);
  AlgorithmInfo get_item(int row) const;

private:
  void load_from_query(QSqlQuery *query);

  QStringList headers_;
  QList<AlgorithmInfo> data_list_;
  CategoryService category_service_;
};
