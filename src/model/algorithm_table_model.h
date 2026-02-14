#pragma once

#include "entities/algorithm_data.h"
#include <QAbstractTableModel>
#include <QList>

class AlgorithmTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  explicit AlgorithmTableModel(QObject *parent = nullptr);

  // --- 必须实现的虚函数 ---
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  // --- 自定义功能 ---
  // 根据分类 ID 加载数据
  void load_data(const QString &category_id);
  // 根据搜索关键词加载数据
  void search_data(const QString &keyword);

  // 获取某一行的数据实体（供删除/编辑使用）
  AlgorithmInfo get_item(int row) const;

private:
  QStringList headers_;
  QList<AlgorithmInfo> data_list_; // 内存中的数据缓存
};
