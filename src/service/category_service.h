#pragma once

#include "entities/algorithm_data.h"
#include <QList>
#include <QString>
#include <QStringList>

class CategoryService {
public:
  QList<AlgoCategory> fetch_all_categories(QString *error_message = nullptr) const;
  QStringList fetch_subtree_ids(const QString &root_id,
                                QString *error_message = nullptr) const;
};
