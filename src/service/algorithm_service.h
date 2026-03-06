#pragma once

#include "entities/algorithm_data.h"
#include <QString>

class AlgorithmService {
public:
  bool create_algorithm(AlgorithmInfo info, QString *error_message = nullptr);
  bool update_algorithm(const AlgorithmInfo &info,
                        QString *error_message = nullptr);
  bool delete_algorithm(const QString &alg_id,
                        QString *error_message = nullptr);
};
