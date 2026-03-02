#include "algorithm_runner.h"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLibrary>
#include <QProcess>
#include <QSet>
#include <QStringList>

namespace {
enum class RuntimeType { Dll, Python };

struct PythonCommand {
  QString program;
  QStringList prefixArgs;
  QString displayName;
};

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
      (!file_name.isEmpty() &&
       !file_name.startsWith("lib", Qt::CaseInsensitive))
          ? QString("lib%1").arg(file_name)
          : QString();
  QString trimmed_relative = path;
  while (trimmed_relative.startsWith('/') ||
         trimmed_relative.startsWith('\\')) {
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
        add_candidate(
            QDir(root).filePath(QString("libs/%1").arg(alt_file_name)));
      }

      add_candidate(QDir(root).filePath(
          QString("examples/python_models/%1").arg(file_name)));

      add_candidate(QDir(root).filePath(
          QString("examples/blast_models_dll/build/%1").arg(file_name)));
      add_candidate(QDir(root).filePath(
          QString("examples/blast_models_dll/build/Debug/%1").arg(file_name)));
      add_candidate(QDir(root).filePath(
          QString("examples/blast_models_dll/build/Release/%1")
              .arg(file_name)));

      if (!alt_file_name.isEmpty()) {
        add_candidate(QDir(root).filePath(
            QString("examples/blast_models_dll/build/%1").arg(alt_file_name)));
        add_candidate(QDir(root).filePath(
            QString("examples/blast_models_dll/build/Debug/%1")
                .arg(alt_file_name)));
        add_candidate(QDir(root).filePath(
            QString("examples/blast_models_dll/build/Release/%1")
                .arg(alt_file_name)));
      }
    }
  }

  result.removeDuplicates();
  return result;
}

RuntimeType resolve_runtime_type(const AlgorithmInfo &algorithm) {
  const QString type = algorithm.sourceType.trimmed().toLower();
  if (type == "2" || type == "py" || type == "python") {
    return RuntimeType::Python;
  }
  if (type == "1" || type == "dll") {
    return RuntimeType::Dll;
  }

  const QString suffix =
      QFileInfo(algorithm.filePath.trimmed()).suffix().toLower();
  if (suffix == "py") {
    return RuntimeType::Python;
  }
  return RuntimeType::Dll;
}

QString resolve_existing_path(const QString &raw_path,
                              QStringList *tried_paths = nullptr) {
  const QStringList paths = candidate_paths(raw_path);
  for (const QString &path : paths) {
    if (tried_paths) {
      tried_paths->append(path);
    }
    if (QFileInfo::exists(path)) {
      return path;
    }
  }
  return QString{};
}

QList<PythonCommand> python_commands() {
  QList<PythonCommand> commands;
  QSet<QString> dedup_keys;

  auto add_command = [&](const QString &program, const QStringList &prefix_args,
                         const QString &display_name) {
    if (program.trimmed().isEmpty()) {
      return;
    }
    const QString key = program + "|" + prefix_args.join(" ");
    if (dedup_keys.contains(key)) {
      return;
    }
    dedup_keys.insert(key);
    commands.push_back(PythonCommand{program, prefix_args, display_name});
  };

  const QString env_python = qEnvironmentVariable("PYTHON_EXECUTABLE").trimmed();
  if (!env_python.isEmpty()) {
    add_command(env_python, {}, QString("PYTHON_EXECUTABLE=%1").arg(env_python));
  }

  add_command("python", {}, "python");
  add_command("python3", {}, "python3");
  add_command("py", {"-3"}, "py -3");
  add_command("py", {}, "py");

  return commands;
}
} // namespace

AlgorithmRunResult AlgorithmRunner::run(const AlgorithmInfo &algorithm,
                                        const QJsonObject &input_params) const {
  if (algorithm.filePath.trimmed().isEmpty()) {
    AlgorithmRunResult result;
    result.message = "算法文件路径为空。";
    return result;
  }
  if (algorithm.funcName.trimmed().isEmpty()) {
    AlgorithmRunResult result;
    result.message = "入口函数名为空。";
    return result;
  }

  if (resolve_runtime_type(algorithm) == RuntimeType::Python) {
    return run_python(algorithm, input_params);
  }
  return run_dll(algorithm, input_params);
}

AlgorithmRunResult AlgorithmRunner::run_dll(const AlgorithmInfo &algorithm,
                                            const QJsonObject &input_params) const {
  AlgorithmRunResult result;

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

  using EntryFn =
      int (*)(const char *input_json, char *output_json, int output_size);
  EntryFn entry = reinterpret_cast<EntryFn>(
      library.resolve(algorithm.funcName.toUtf8().constData()));
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
  const int ret =
      entry(input_json.constData(), output_buffer.data(), output_buffer.size());
  result.elapsedMs = timer.elapsed();

  const QByteArray output_json =
      QByteArray(output_buffer.constData()).trimmed();
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
    if (parse_error.error == QJsonParseError::NoError &&
        output_doc.isObject()) {
      result.outputJson = output_doc.object();
    }
  }

  result.success = true;
  result.message = "执行成功";
  library.unload();
  return result;
}

AlgorithmRunResult
AlgorithmRunner::run_python(const AlgorithmInfo &algorithm,
                            const QJsonObject &input_params) const {
  AlgorithmRunResult result;

  QStringList tried_script_paths;
  const QString script_path =
      resolve_existing_path(algorithm.filePath, &tried_script_paths);
  if (script_path.isEmpty()) {
    result.message = QString("未找到 Python 脚本。尝试路径如下：\n%1")
                         .arg(tried_script_paths.join("\n"));
    return result;
  }

  const QByteArray input_json =
      QJsonDocument(input_params).toJson(QJsonDocument::Compact);
  const QList<PythonCommand> commands = python_commands();

  QStringList start_failures;
  for (const PythonCommand &command : commands) {
    QProcess process;
    QStringList args = command.prefixArgs;
    args << script_path << algorithm.funcName << QString::fromUtf8(input_json);

    process.start(command.program, args);
    if (!process.waitForStarted(2500)) {
      start_failures << QString("[%1] %2")
                            .arg(command.displayName, process.errorString());
      continue;
    }

    QElapsedTimer timer;
    timer.start();
    const bool finished = process.waitForFinished(20000);
    result.elapsedMs = timer.elapsed();

    if (!finished) {
      process.kill();
      process.waitForFinished(1000);
      result.message = QString("Python 执行超时（20秒），解释器：%1。")
                           .arg(command.displayName);
      return result;
    }

    const QByteArray stdout_data = process.readAllStandardOutput().trimmed();
    const QByteArray stderr_data = process.readAllStandardError().trimmed();
    result.rawOutput =
        QString::fromUtf8(stdout_data.isEmpty() ? stderr_data : stdout_data);

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
      const QString details =
          !stderr_data.isEmpty()
              ? QString::fromUtf8(stderr_data)
              : (!stdout_data.isEmpty() ? QString::fromUtf8(stdout_data)
                                        : "未返回错误详情。");
      result.message = QString("Python 算法执行失败（%1，退出码=%2）：\n%3")
                           .arg(command.displayName)
                           .arg(process.exitCode())
                           .arg(details);
      return result;
    }

    if (!stdout_data.isEmpty()) {
      QJsonParseError parse_error{};
      const QJsonDocument output_doc =
          QJsonDocument::fromJson(stdout_data, &parse_error);
      if (parse_error.error == QJsonParseError::NoError && output_doc.isObject()) {
        result.outputJson = output_doc.object();
      }
    }

    result.success = true;
    result.message = QString("执行成功（Python：%1）").arg(command.displayName);
    return result;
  }

  result.message = QString("启动 Python 解释器失败。尝试如下：\n%1")
                       .arg(start_failures.join("\n"));
  return result;
}
