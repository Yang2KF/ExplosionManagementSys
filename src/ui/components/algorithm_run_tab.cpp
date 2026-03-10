#include "algorithm_run_tab.h"
#include "m_message_box.h"
#include "ui_system.h"
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLayoutItem>
#include <QScrollArea>
#include <QSplitter>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

AlgorithmRunTab::AlgorithmRunTab(const AlgorithmInfo &algorithm,
                                 QWidget *parent)
    : QWidget(parent), algorithm_(algorithm) {
  setAttribute(Qt::WA_StyledBackground, true);
  init_ui();
  load_params();
}

QString AlgorithmRunTab::tabKey() const {
  if (!algorithm_.id.trimmed().isEmpty()) {
    return algorithm_.id.trimmed();
  }
  return algorithm_.name.trimmed() + "|" + algorithm_.filePath.trimmed() + "|" +
         algorithm_.funcName.trimmed();
}

void AlgorithmRunTab::init_ui() {
  auto *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(0, 0, 0, 0);
  main_layout->setSpacing(0);

  auto *splitter = new QSplitter(Qt::Horizontal, this);
  splitter->setChildrenCollapsible(false);
  splitter->setHandleWidth(1);
  main_layout->addWidget(splitter);

  auto *left_panel = new QWidget(splitter);
  left_panel->setObjectName("FunctionRunnerPanel");
  auto *left_layout = new QVBoxLayout(left_panel);
  left_layout->setContentsMargins(24, 24, 24, 24);
  left_layout->setSpacing(12);

  auto *title_label = new QLabel(QStringLiteral("算法运行"), left_panel);
  title_label->setObjectName("FunctionRunnerTitle");

  const QString runtime_type =
      (algorithm_.sourceType.trimmed().compare("python", Qt::CaseInsensitive) ==
           0 ||
       algorithm_.sourceType.trimmed() == "2" ||
       QFileInfo(algorithm_.filePath.trimmed()).suffix().toLower() == "py")
          ? QStringLiteral("Python")
          : QStringLiteral("DLL");

  algo_label_ = new QLabel(algorithm_.name, left_panel);
  algo_label_->setObjectName("FunctionRunnerAlgoName");
  algo_label_->setWordWrap(true);

  auto *algo_meta_label =
      new QLabel(QStringLiteral("运行类型: %1    |    入口函数: %2")
                     .arg(runtime_type, algorithm_.funcName),
                 left_panel);
  algo_meta_label->setObjectName("FunctionRunnerAlgoMeta");

  param_scroll_area_ = new QScrollArea(left_panel);
  param_scroll_area_->setObjectName("FunctionRunnerParamScroll");
  param_scroll_area_->setWidgetResizable(true);
  param_scroll_area_->setFrameShape(QFrame::NoFrame);

  param_form_widget_ = new QWidget(param_scroll_area_);
  param_form_widget_->setObjectName("FunctionRunnerParamForm");
  param_form_layout_ = new QFormLayout(param_form_widget_);
  param_form_layout_->setContentsMargins(0, 10, 0, 0);
  param_form_layout_->setHorizontalSpacing(16);
  param_form_layout_->setVerticalSpacing(16);
  param_scroll_area_->setWidget(param_form_widget_);

  auto *button_layout = new QHBoxLayout();
  button_layout->addStretch();

  reset_btn_ = new MaterialButton(QStringLiteral("重置"),
                                  MaterialButton::Normal, left_panel);
  reset_btn_->set_theme_color(UISystem::instance().neutral());
  reset_btn_->setFixedSize(90, 34);

  run_btn_ = new MaterialButton(QStringLiteral("运行"), MaterialButton::Normal,
                                left_panel);
  run_btn_->set_theme_color(UISystem::instance().bg_primary());
  run_btn_->setFixedSize(90, 34);

  button_layout->addWidget(reset_btn_);
  button_layout->addWidget(run_btn_);

  left_layout->addWidget(title_label);
  left_layout->addWidget(algo_label_);
  left_layout->addWidget(algo_meta_label);
  left_layout->addSpacing(10);
  left_layout->addWidget(param_scroll_area_, 1);
  left_layout->addLayout(button_layout);

  auto *right_panel = new QWidget(splitter);
  right_panel->setObjectName("FunctionRunnerPanel");
  auto *right_layout = new QVBoxLayout(right_panel);
  right_layout->setContentsMargins(24, 24, 24, 24);
  right_layout->setSpacing(12);

  auto *result_title = new QLabel(QStringLiteral("运行结果"), right_panel);
  result_title->setObjectName("FunctionRunnerTitle");

  result_table_ = new QTableWidget(right_panel);
  result_table_->setObjectName("FunctionRunnerResultTable");
  result_table_->setColumnCount(2);
  result_table_->setHorizontalHeaderLabels(
      {QStringLiteral("字段"), QStringLiteral("值")});
  result_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  result_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  result_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  result_table_->setAlternatingRowColors(true);
  result_table_->setWordWrap(true);
  result_table_->verticalHeader()->setVisible(false);
  result_table_->horizontalHeader()->setStretchLastSection(true);
  result_table_->horizontalHeader()->setMinimumSectionSize(80);
  result_table_->horizontalHeader()->resizeSection(0, 220);
  result_table_->setShowGrid(false);
  result_table_->setFrameShape(QFrame::NoFrame);

  right_layout->addWidget(result_title);
  right_layout->addWidget(result_table_, 1);

  AlgorithmRunResult initial_result;
  initial_result.message = QStringLiteral("等待运行。");
  render_result(initial_result);

  splitter->addWidget(left_panel);
  splitter->addWidget(right_panel);
  splitter->setStretchFactor(0, 2);
  splitter->setStretchFactor(1, 3);

  connect(reset_btn_, &QPushButton::clicked, this,
          [this]() { reset_param_inputs(); });
  connect(run_btn_, &QPushButton::clicked, this, [this]() { run_algorithm(); });
}

void AlgorithmRunTab::load_params() {
  QString error_message;
  params_ = param_service_.fetch_params(algorithm_.id, &error_message);
  rebuild_param_form();
  reset_param_inputs();

  if (!error_message.isEmpty()) {
    AlgorithmRunResult run_result;
    run_result.message = QStringLiteral("加载参数定义失败：") + error_message;
    render_result(run_result);
  }
}

void AlgorithmRunTab::rebuild_param_form() {
  while (param_form_layout_->count() > 0) {
    QLayoutItem *item = param_form_layout_->takeAt(0);
    if (item) {
      if (QWidget *widget = item->widget()) {
        widget->deleteLater();
      }
      delete item;
    }
  }
  param_inputs_.clear();

  if (params_.isEmpty()) {
    auto *empty_label =
        new QLabel(QStringLiteral("该算法尚未配置参数。"), param_form_widget_);
    empty_label->setWordWrap(true);
    param_form_layout_->addRow(empty_label);
    return;
  }

  for (const AlgorithmParam &param : params_) {
    auto *name_label = new QLabel(display_name(param), param_form_widget_);
    name_label->setObjectName("FunctionParamLabel");

    auto *value_input =
        new MaterialInput(MaterialInput::Edit, param_form_widget_);
    value_input->setPlaceholderText(param.identifier.isEmpty()
                                        ? QStringLiteral("请输入参数值")
                                        : param.identifier);
    value_input->setText(param.defaultValue);

    if (!param.tooltip.trimmed().isEmpty()) {
      name_label->setToolTip(param.tooltip);
      value_input->setToolTip(param.tooltip);
    }

    param_form_layout_->addRow(name_label, value_input);
    param_inputs_.insert(cache_key(param), value_input);
  }
}

void AlgorithmRunTab::reset_param_inputs() {
  for (const AlgorithmParam &param : params_) {
    MaterialInput *input = param_inputs_.value(cache_key(param), nullptr);
    if (input) {
      input->setText(param.defaultValue);
    }
  }
}

bool AlgorithmRunTab::collect_input_json(QJsonObject *input_json,
                                         QString *error_message) const {
  if (!input_json) {
    return false;
  }
  *input_json = QJsonObject{};

  for (const AlgorithmParam &param : params_) {
    MaterialInput *input = param_inputs_.value(cache_key(param), nullptr);
    if (!input) {
      continue;
    }

    const QString display = param.name.isEmpty() ? param.identifier.trimmed()
                                                 : param.name.trimmed();
    const QString param_key = param.identifier.isEmpty()
                                  ? param.name.trimmed()
                                  : param.identifier.trimmed();
    if (param_key.isEmpty()) {
      continue;
    }

    QString value_text = input->text().trimmed();
    if (value_text.isEmpty()) {
      value_text = param.defaultValue.trimmed();
    }
    if (value_text.isEmpty()) {
      if (param.required) {
        if (error_message) {
          *error_message = QStringLiteral("参数“%1”不能为空。").arg(display);
        }
        return false;
      }
      continue;
    }

    const QString type = param.type.trimmed().toLower();
    if (type == "int" || type == "integer") {
      bool ok = false;
      const int value = value_text.toInt(&ok);
      if (!ok) {
        if (error_message) {
          *error_message = QStringLiteral("参数“%1”必须是整数。").arg(display);
        }
        return false;
      }

      if (!param.minValue.trimmed().isEmpty()) {
        bool min_ok = false;
        const double min_value = param.minValue.toDouble(&min_ok);
        if (min_ok && value < min_value) {
          if (error_message) {
            *error_message = QStringLiteral("参数“%1”应大于等于 %2。")
                                 .arg(display, param.minValue.trimmed());
          }
          return false;
        }
      }
      if (!param.maxValue.trimmed().isEmpty()) {
        bool max_ok = false;
        const double max_value = param.maxValue.toDouble(&max_ok);
        if (max_ok && value > max_value) {
          if (error_message) {
            *error_message = QStringLiteral("参数“%1”应小于等于 %2。")
                                 .arg(display, param.maxValue.trimmed());
          }
          return false;
        }
      }

      input_json->insert(param_key, value);
      continue;
    }

    if (type == "double" || type == "float" || type == "number") {
      bool ok = false;
      const double value = value_text.toDouble(&ok);
      if (!ok) {
        if (error_message) {
          *error_message = QStringLiteral("参数“%1”必须是数字。").arg(display);
        }
        return false;
      }

      if (!param.minValue.trimmed().isEmpty()) {
        bool min_ok = false;
        const double min_value = param.minValue.toDouble(&min_ok);
        if (min_ok && value < min_value) {
          if (error_message) {
            *error_message = QStringLiteral("参数“%1”应大于等于 %2。")
                                 .arg(display, param.minValue.trimmed());
          }
          return false;
        }
      }
      if (!param.maxValue.trimmed().isEmpty()) {
        bool max_ok = false;
        const double max_value = param.maxValue.toDouble(&max_ok);
        if (max_ok && value > max_value) {
          if (error_message) {
            *error_message = QStringLiteral("参数“%1”应小于等于 %2。")
                                 .arg(display, param.maxValue.trimmed());
          }
          return false;
        }
      }

      input_json->insert(param_key, value);
      continue;
    }

    if (type == "bool" || type == "boolean") {
      bool ok = false;
      const bool value = parse_bool(value_text, &ok);
      if (!ok) {
        if (error_message) {
          *error_message =
              QStringLiteral("参数“%1”必须是布尔值。").arg(display);
        }
        return false;
      }
      input_json->insert(param_key, value);
      continue;
    }

    input_json->insert(param_key, value_text);
  }

  return true;
}

void AlgorithmRunTab::run_algorithm() {
  QJsonObject input_json;
  QString error_message;
  if (!collect_input_json(&input_json, &error_message)) {
    MaterialMessageBox::warning(this, QStringLiteral("参数不合法"),
                                error_message);
    return;
  }

  const AlgorithmRunResult run_result =
      algorithm_runner_.run(algorithm_, input_json);
  render_result(run_result);
}

void AlgorithmRunTab::render_result(const AlgorithmRunResult &run_result) {
  if (!result_table_) {
    return;
  }

  QList<QPair<QString, QString>> rows;
  const bool is_pending =
      !run_result.success && run_result.elapsedMs == 0 &&
      run_result.outputJson.isEmpty() && run_result.rawOutput.trimmed().isEmpty() &&
      run_result.message.trimmed() == QStringLiteral("等待运行。");
  rows.append({QStringLiteral("状态"),
               is_pending ? QStringLiteral("未运行")
                          : (run_result.success ? QStringLiteral("成功")
                                                : QStringLiteral("失败"))});
  rows.append(
      {QStringLiteral("耗时(ms)"), QString::number(run_result.elapsedMs)});
  rows.append({QStringLiteral("消息"),
               run_result.message.trimmed().isEmpty()
                   ? QStringLiteral("-")
                   : run_result.message.trimmed()});

  if (!run_result.outputJson.isEmpty()) {
    const auto keys = run_result.outputJson.keys();
    for (const QString &key : keys) {
      append_result_value_rows(key, run_result.outputJson.value(key), &rows);
    }
  } else if (!run_result.rawOutput.trimmed().isEmpty()) {
    const QByteArray raw_bytes = run_result.rawOutput.trimmed().toUtf8();
    const QJsonDocument raw_doc = QJsonDocument::fromJson(raw_bytes);
    if (raw_doc.isObject()) {
      const auto keys = raw_doc.object().keys();
      for (const QString &key : keys) {
        append_result_value_rows(key, raw_doc.object().value(key), &rows);
      }
    } else if (raw_doc.isArray()) {
      append_result_value_rows(QStringLiteral("结果"), QJsonValue(raw_doc.array()),
                               &rows);
    } else {
      rows.append({QStringLiteral("输出"), run_result.rawOutput.trimmed()});
    }
  } else {
    rows.append({QStringLiteral("输出"), QStringLiteral("无输出内容。")});
  }

  result_table_->clearContents();
  result_table_->setRowCount(rows.size());

  for (int i = 0; i < rows.size(); ++i) {
    auto *key_item = new QTableWidgetItem(rows.at(i).first);
    auto *value_item = new QTableWidgetItem(rows.at(i).second);
    key_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    value_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    key_item->setFlags(key_item->flags() & ~Qt::ItemIsEditable);
    value_item->setFlags(value_item->flags() & ~Qt::ItemIsEditable);
    key_item->setToolTip(rows.at(i).first);
    value_item->setToolTip(rows.at(i).second);
    result_table_->setItem(i, 0, key_item);
    result_table_->setItem(i, 1, value_item);
  }

  result_table_->resizeRowsToContents();
}

void AlgorithmRunTab::append_result_value_rows(
    const QString &key, const QJsonValue &value,
    QList<QPair<QString, QString>> *rows) const {
  if (!rows) {
    return;
  }

  if (value.isObject()) {
    const QJsonObject object = value.toObject();
    const auto keys = object.keys();
    for (const QString &child_key : keys) {
      append_result_value_rows(key + "." + child_key, object.value(child_key),
                               rows);
    }
    if (keys.isEmpty()) {
      rows->append({key, QStringLiteral("{}")});
    }
    return;
  }

  if (value.isArray()) {
    const QJsonArray array = value.toArray();
    if (array.isEmpty()) {
      rows->append({key, QStringLiteral("[]")});
      return;
    }

    bool all_scalar = true;
    for (const QJsonValue &item : array) {
      if (item.isObject() || item.isArray()) {
        all_scalar = false;
        break;
      }
    }

    if (all_scalar) {
      QStringList parts;
      for (const QJsonValue &item : array) {
        parts.append(result_cell_text(item));
      }
      rows->append({key, parts.join(QStringLiteral(", "))});
      return;
    }

    for (int i = 0; i < array.size(); ++i) {
      append_result_value_rows(QStringLiteral("%1[%2]").arg(key).arg(i),
                               array.at(i), rows);
    }
    return;
  }

  rows->append({key, result_cell_text(value)});
}

QString AlgorithmRunTab::result_cell_text(const QJsonValue &value) const {
  if (value.isString()) {
    return value.toString();
  }
  if (value.isBool()) {
    return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
  }
  if (value.isDouble()) {
    return QString::number(value.toDouble(), 'g', 12);
  }
  if (value.isNull()) {
    return QStringLiteral("null");
  }
  if (value.isUndefined()) {
    return QStringLiteral("undefined");
  }
  return QString::fromUtf8(
      QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact));
}

QString AlgorithmRunTab::cache_key(const AlgorithmParam &param) const {
  if (!param.id.trimmed().isEmpty()) {
    return param.id.trimmed();
  }
  if (!param.identifier.trimmed().isEmpty()) {
    return param.identifier.trimmed();
  }
  return param.name.trimmed();
}

QString AlgorithmRunTab::display_name(const AlgorithmParam &param) const {
  QString text = param.name.trimmed();
  if (text.isEmpty()) {
    text = param.identifier.trimmed();
  }
  if (!param.unit.trimmed().isEmpty()) {
    text += " (" + param.unit.trimmed() + ")";
  }
  if (param.required) {
    text += " *";
  }
  return text;
}

bool AlgorithmRunTab::parse_bool(const QString &text, bool *ok) const {
  const QString lower = text.trimmed().toLower();
  if (lower == "1" || lower == "true" || lower == "yes" || lower == "y" ||
      lower == QStringLiteral("是") || lower == QStringLiteral("真")) {
    *ok = true;
    return true;
  }
  if (lower == "0" || lower == "false" || lower == "no" || lower == "n" ||
      lower == QStringLiteral("否") || lower == QStringLiteral("假")) {
    *ok = true;
    return false;
  }
  *ok = false;
  return false;
}
