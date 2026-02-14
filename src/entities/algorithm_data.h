#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

// 算法参数实体
struct AlgorithmParam {
  QString id;
  QString algoId;
  QString name;
  QString type; // "double", "int", "string"
  QString defaultValue;
};

// 算法信息实体
struct AlgorithmInfo {
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
