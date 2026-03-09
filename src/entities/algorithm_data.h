#pragma once

#include <QList>
#include <QString>

struct AlgorithmParam {
  QString id;
  QString algoId;
  QString identifier;
  QString name;
  QString unit;
  QString type;
  QString defaultValue;
  QString minValue;
  QString maxValue;
  QString tooltip;
  QString comments;
  bool required = false;
  int order = 0;
};

struct AlgorithmInfo {
  QString id;
  QString categoryId;
  QString categoryName;
  QString name;
  QString description;
  QString filePath;
  QString sourceType; // "dll" or "python"
  QString funcName;
  QList<AlgorithmParam> params;
};

struct AlgoCategory {
  QString id;
  QString parentId;
  QString name;
  QString comments;
};
