#include "algorithm_runner.h"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLibrary>
#include <QStringList>

namespace {
QStringList candidate_paths(const QString &raw_path) {
  const QString path = QDir::fromNativeSeparators(raw_path.trimmed());
  QStringList result;
  if (path.isEmpty()) {
    return result;
  }

  const QDir app_dir(QCoreApplication::applicationDirPath());
  const QFileInfo input_info(path);
  const QString file_name = input_info.fileName();
  const QString alt_file_name =
      (!file_name.isEmpty() && !file_name.startsWith("lib", Qt::CaseInsensitive))
          ? QString("lib%1").arg(file_name)
          : QString();
  QString trimmed_relative = path;
  while (trimmed_relative.startsWith('/') || trimmed_relative.startsWith('\\')) {
    trimmed_relative.remove(0, 1);
  }

  auto add_candidate = [&](const QString &value) {
    if (!value.trimmed().isEmpty()) {
      result << QDir::cleanPath(value);
    }
  };

  QStringList search_roots;
  search_roots << app_dir.absolutePath() << QDir::currentPath();
  QDir cursor = app_dir;
  for (int i = 0; i < 4; ++i) {
    if (!cursor.cdUp()) {
      break;
    }
    search_roots << cursor.absolutePath();
  }
  search_roots.removeDuplicates();

  add_candidate(path);

  if (input_info.isRelative()) {
    for (const QString &root : search_roots) {
      add_candidate(QDir(root).filePath(path));
    }
  } else {
    add_candidate(input_info.absoluteFilePath());
  }

  if (!trimmed_relative.isEmpty()) {
    for (const QString &root : search_roots) {
      add_candidate(QDir(root).filePath(trimmed_relative));
    }
  }

  if (!file_name.isEmpty()) {
    for (const QString &root : search_roots) {
      add_candidate(QDir(root).filePath(QString("libs/%1").arg(file_name)));
      if (!alt_file_name.isEmpty()) {
        add_candidate(QDir(root).filePath(QString("libs/%1").arg(alt_file_name)));
      }

      add_candidate(
          QDir(root).filePath(QString("examples/blast_models_dll/build/%1").arg(file_name)));
      add_candidate(QDir(root).filePath(
          QString("examples/blast_models_dll/build/Debug/%1").arg(file_name)));
      add_candidate(QDir(root).filePath(
          QString("examples/blast_models_dll/build/Release/%1").arg(file_name)));

      if (!alt_file_name.isEmpty()) {
        add_candidate(QDir(root).filePath(
            QString("examples/blast_models_dll/build/%1").arg(alt_file_name)));
        add_candidate(QDir(root).filePath(
            QString("examples/blast_models_dll/build/Debug/%1").arg(alt_file_name)));
        add_candidate(QDir(root).filePath(
            QString("examples/blast_models_dll/build/Release/%1").arg(alt_file_name)));
      }
    }
  }

  result.removeDuplicates();
  return result;
}
} // namespace

AlgorithmRunResult AlgorithmRunner::run(const AlgorithmInfo &algorithm,
                                        const QJsonObject &input_params) const {
  AlgorithmRunResult result;

  if (algorithm.filePath.trimmed().isEmpty()) {
    result.message = "算法库路径为空。";
    return result;
  }
  if (algorithm.funcName.trimmed().isEmpty()) {
    result.message = "导出函数名为空。";
    return result;
  }

  QLibrary library;
  QString loaded_path;
  QStringList tried_errors;
  const QStringList paths = candidate_paths(algorithm.filePath);
  for (const QString &path : paths) {
    library.setFileName(path);
    if (library.load()) {
      loaded_path = path;
      break;
    }
    tried_errors << QString("[%1] %2").arg(path, library.errorString());
  }

  if (loaded_path.isEmpty()) {
    result.message = QString("加载算法库失败。尝试路径如下：\n%1")
                         .arg(tried_errors.join("\n"));
    return result;
  }

  using EntryFn = int (*)(const char *input_json, char *output_json,
                          int output_size);
  EntryFn entry =
      reinterpret_cast<EntryFn>(library.resolve(algorithm.funcName.toUtf8().constData()));
  if (!entry) {
    result.message =
        QString("在“%2”中解析函数“%1”失败：%3")
            .arg(algorithm.funcName, loaded_path, library.errorString());
    library.unload();
    return result;
  }

  const QByteArray input_json =
      QJsonDocument(input_params).toJson(QJsonDocument::Compact);
  QByteArray output_buffer(64 * 1024, '\0');

  QElapsedTimer timer;
  timer.start();
  const int ret = entry(input_json.constData(), output_buffer.data(),
                        output_buffer.size());
  result.elapsedMs = timer.elapsed();

  const QByteArray output_json = QByteArray(output_buffer.constData()).trimmed();
  result.rawOutput = QString::fromUtf8(output_json);

  if (ret != 0) {
    result.message = QString("算法函数返回错误码：%1。").arg(ret);
    library.unload();
    return result;
  }

  if (!output_json.isEmpty()) {
    QJsonParseError parse_error{};
    const QJsonDocument output_doc =
        QJsonDocument::fromJson(output_json, &parse_error);
    if (parse_error.error == QJsonParseError::NoError && output_doc.isObject()) {
      result.outputJson = output_doc.object();
    }
  }

  result.success = true;
  result.message = "执行成功";
  library.unload();
  return result;
}
