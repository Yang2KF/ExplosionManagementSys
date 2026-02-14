#pragma once

#include <QStandardItemModel>
#include <QString>

class CategoryTreeModel : public QStandardItemModel {
  Q_OBJECT
public:
  explicit CategoryTreeModel(QObject *parent = nullptr);

  // 核心功能：从数据库重新加载所有分类
  void reload();

private:
  // 递归辅助函数：加载子分类
  void load_children(QStandardItem *parent_item, const QString &parent_id);
};
