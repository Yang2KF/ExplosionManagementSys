#pragma once

#include "entities/algorithm_data.h"
#include <QJsonObject>
#include <QString>

struct AlgorithmRunResult {
  bool success = false;
  QString message;
  QString rawOutput;
  QJsonObject outputJson;
  qint64 elapsedMs = 0;
};

class AlgorithmRunner {
public:
  AlgorithmRunResult run(const AlgorithmInfo &algorithm,
                         const QJsonObject &input_params) const;

private:
  AlgorithmRunResult run_dll(const AlgorithmInfo &algorithm,
                             const QJsonObject &input_params) const;
  AlgorithmRunResult run_python(const AlgorithmInfo &algorithm,
                                const QJsonObject &input_params) const;
};
