#pragma once

#include <QDateTime>
#include <QList>
#include <QString>
#include <QtGlobal>

// 算法参数实体
struct AlgorithmParam {
  qint64 displayId = 0; // 前端可读编号
  QString id;
  QString algoId;
  QString identifier; // 参数键名(调用时使用)
  QString name;       // 参数显示名
  QString unit;
  QString type; // "double", "int", "string"
  QString defaultValue;
  QString minValue;
  QString maxValue;
  QString validator;
  QString tooltip;
  bool required = false;
  int order = 0;
};

// 算法信息实体
struct AlgorithmInfo {
  qint64 displayId = 0; // 前端可读编号
  QString id;
  QString categoryId;
  QString name;
  QString description;

  // 核心调用字段
  QString filePath; // DLL路径
  QString funcName; // 导出函数名

  QDateTime createdAt;

  // 运行时可能需要加载参数列表
  QList<AlgorithmParam> params;
};

// 分类实体
struct AlgoCategory {
  QString id;
  QString parentId;
  QString name;
};
