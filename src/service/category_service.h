#pragma once

#include "entities/algorithm_data.h"
#include <QList>
#include <QString>

class CategoryService {
public:
  QList<AlgoCategory> fetch_all_categories(QString *error_message = nullptr) const;
};
