#pragma once

#include "entities/algorithm_data.h"
#include <QList>
#include <QString>

class AlgorithmParamService {
public:
  QList<AlgorithmParam> fetch_params(const QString &algo_id,
                                     QString *error_message = nullptr) const;

  bool create_param(AlgorithmParam param, QString *error_message = nullptr) const;
  bool update_param(const AlgorithmParam &param,
                    QString *error_message = nullptr) const;
  bool delete_param(const QString &param_id,
                    QString *error_message = nullptr) const;
};
