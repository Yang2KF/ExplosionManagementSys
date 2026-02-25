#include "algorithm_run_dialog.h"
#include "m_message_box.h"
#include "mask_widget.h"
#include "ui_system.h"
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLayoutItem>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>

AlgorithmRunDialog::AlgorithmRunDialog(const AlgorithmInfo &algorithm,
                                       QWidget *parent)
    : QDialog(parent), algorithm_(algorithm) {
  setObjectName("AlgorithmRunDialog");
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
  resize(760, 620);
  setModal(true);

  init_ui();
  load_params();
}

void AlgorithmRunDialog::paintEvent(QPaintEvent *event) { Q_UNUSED(event); }

void AlgorithmRunDialog::showEvent(QShowEvent *event) {
  if (auto mask = MaskWidget::instance()) {
    mask->show_mask();
  }
  QDialog::showEvent(event);
  raise();
}

void AlgorithmRunDialog::done(int r) {
  if (auto mask = MaskWidget::instance()) {
    mask->hide_mask();
  }
  QDialog::done(r);
}

void AlgorithmRunDialog::init_ui() {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(10, 10, 10, 10);

  QWidget *container = new QWidget(this);
  container->setObjectName("AlgoRunDialogContainer");
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 60));
  shadow->setOffset(0, 5);
  container->setGraphicsEffect(shadow);
  main_layout->addWidget(container);

  QVBoxLayout *container_layout = new QVBoxLayout(container);
  container_layout->setContentsMargins(0, 0, 0, 0);
  container_layout->setSpacing(0);

  QWidget *panel = new QWidget(container);
  panel->setObjectName("FunctionRunnerPanel");
  container_layout->addWidget(panel);

  QVBoxLayout *runner_layout = new QVBoxLayout(panel);
  runner_layout->setContentsMargins(16, 14, 16, 14);
  runner_layout->setSpacing(12);

  QLabel *title_label = new QLabel("算法运行面板", panel);
  title_label->setObjectName("FunctionRunnerTitle");

  algo_label_ = new QLabel(panel);
  algo_label_->setObjectName("FunctionRunnerAlgo");
  algo_label_->setWordWrap(true);
  algo_label_->setText(QString("%1\n入口函数：%2")
                           .arg(algorithm_.name, algorithm_.funcName));

  param_scroll_area_ = new QScrollArea(panel);
  param_scroll_area_->setObjectName("FunctionRunnerParamScroll");
  param_scroll_area_->setWidgetResizable(true);
  param_scroll_area_->setFrameShape(QFrame::NoFrame);

  param_form_widget_ = new QWidget(param_scroll_area_);
  param_form_widget_->setObjectName("FunctionRunnerParamForm");
  param_form_layout_ = new QFormLayout(param_form_widget_);
  param_form_layout_->setContentsMargins(0, 0, 0, 0);
  param_form_layout_->setHorizontalSpacing(16);
  param_form_layout_->setVerticalSpacing(10);
  param_scroll_area_->setWidget(param_form_widget_);

  QHBoxLayout *button_layout = new QHBoxLayout();
  button_layout->addStretch();

  reset_btn_ = new MaterialButton("重置", MaterialButton::Normal, panel);
  reset_btn_->set_theme_color(UISystem::instance().neutral());
  reset_btn_->setFixedSize(90, 34);

  run_btn_ = new MaterialButton("运行", MaterialButton::Normal, panel);
  run_btn_->set_theme_color(UISystem::instance().bg_primary());
  run_btn_->setFixedSize(90, 34);

  close_btn_ = new MaterialButton("关闭", MaterialButton::Normal, panel);
  close_btn_->set_theme_color(UISystem::instance().neutral());
  close_btn_->setFixedSize(90, 34);

  button_layout->addWidget(reset_btn_);
  button_layout->addWidget(run_btn_);
  button_layout->addWidget(close_btn_);

  result_output_ = new QTextEdit(panel);
  result_output_->setObjectName("FunctionRunnerResult");
  result_output_->setReadOnly(true);
  result_output_->setPlaceholderText("运行结果将在此显示。");
  result_output_->setMinimumHeight(170);

  runner_layout->addWidget(title_label);
  runner_layout->addWidget(algo_label_);
  runner_layout->addWidget(param_scroll_area_, 1);
  runner_layout->addLayout(button_layout);
  runner_layout->addWidget(result_output_);

  connect(close_btn_, &QPushButton::clicked, this, &QDialog::reject);
  connect(reset_btn_, &QPushButton::clicked, this,
          [this]() { reset_param_inputs(); });
  connect(run_btn_, &QPushButton::clicked, this,
          [this]() { run_algorithm(); });
}

void AlgorithmRunDialog::load_params() {
  QString error_message;
  params_ = param_service_.fetch_params(algorithm_.id, &error_message);
  rebuild_param_form();
  reset_param_inputs();

  if (!error_message.isEmpty()) {
    result_output_->setPlainText("加载参数定义失败：" + error_message);
  }
}

void AlgorithmRunDialog::rebuild_param_form() {
  while (QLayoutItem *item = param_form_layout_->takeAt(0)) {
    if (item->widget()) {
      item->widget()->deleteLater();
    }
    delete item;
  }
  param_inputs_.clear();

  if (params_.isEmpty()) {
    QLabel *empty_label = new QLabel("该算法尚未配置参数。", param_form_widget_);
    empty_label->setWordWrap(true);
    param_form_layout_->addRow(empty_label);
    return;
  }

  for (const AlgorithmParam &param : params_) {
    QLabel *name_label = new QLabel(display_name(param), param_form_widget_);
    name_label->setObjectName("FunctionParamLabel");

    MaterialInput *value_input = new MaterialInput(param_form_widget_);
    value_input->setPlaceholderText(
        param.identifier.isEmpty() ? "请输入参数值" : param.identifier);
    value_input->setText(param.defaultValue);

    if (!param.tooltip.trimmed().isEmpty()) {
      name_label->setToolTip(param.tooltip);
      value_input->setToolTip(param.tooltip);
    }

    param_form_layout_->addRow(name_label, value_input);
    param_inputs_.insert(cache_key(param), value_input);
  }
}

void AlgorithmRunDialog::reset_param_inputs() {
  for (const AlgorithmParam &param : params_) {
    MaterialInput *input = param_inputs_.value(cache_key(param), nullptr);
    if (input) {
      input->setText(param.defaultValue);
    }
  }
}

bool AlgorithmRunDialog::collect_input_json(QJsonObject *input_json,
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

    const QString display =
        param.name.isEmpty() ? param.identifier.trimmed() : param.name.trimmed();
    const QString param_key =
        param.identifier.isEmpty() ? param.name.trimmed() : param.identifier.trimmed();
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
          *error_message = QString("参数“%1”不能为空。").arg(display);
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
          *error_message = QString("参数“%1”必须是整数。").arg(display);
        }
        return false;
      }

      if (!param.minValue.trimmed().isEmpty()) {
        bool min_ok = false;
        const double min_value = param.minValue.toDouble(&min_ok);
        if (min_ok && value < min_value) {
          if (error_message) {
            *error_message =
                QString("参数“%1”应大于等于 %2。").arg(display, param.minValue.trimmed());
          }
          return false;
        }
      }
      if (!param.maxValue.trimmed().isEmpty()) {
        bool max_ok = false;
        const double max_value = param.maxValue.toDouble(&max_ok);
        if (max_ok && value > max_value) {
          if (error_message) {
            *error_message =
                QString("参数“%1”应小于等于 %2。").arg(display, param.maxValue.trimmed());
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
          *error_message = QString("参数“%1”必须是数值。").arg(display);
        }
        return false;
      }

      if (!param.minValue.trimmed().isEmpty()) {
        bool min_ok = false;
        const double min_value = param.minValue.toDouble(&min_ok);
        if (min_ok && value < min_value) {
          if (error_message) {
            *error_message =
                QString("参数“%1”应大于等于 %2。").arg(display, param.minValue.trimmed());
          }
          return false;
        }
      }
      if (!param.maxValue.trimmed().isEmpty()) {
        bool max_ok = false;
        const double max_value = param.maxValue.toDouble(&max_ok);
        if (max_ok && value > max_value) {
          if (error_message) {
            *error_message =
                QString("参数“%1”应小于等于 %2。").arg(display, param.maxValue.trimmed());
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
              QString("参数“%1”必须是布尔值（true/false/1/0/是/否）。").arg(display);
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

void AlgorithmRunDialog::run_algorithm() {
  QJsonObject input_json;
  QString error_message;
  if (!collect_input_json(&input_json, &error_message)) {
    MaterialMessageBox::warning(this, "参数不合法", error_message);
    return;
  }

  const AlgorithmRunResult run_result = algorithm_runner_.run(algorithm_, input_json);

  QString output_text;
  output_text += QString("状态：%1\n").arg(run_result.success ? "成功" : "失败");
  output_text += QString("耗时：%1 ms\n").arg(run_result.elapsedMs);
  output_text += QString("消息：%1\n").arg(run_result.message);
  output_text += "------------------------------\n";

  if (!run_result.outputJson.isEmpty()) {
    output_text += QString::fromUtf8(
        QJsonDocument(run_result.outputJson).toJson(QJsonDocument::Indented));
  } else if (!run_result.rawOutput.trimmed().isEmpty()) {
    output_text += run_result.rawOutput;
  } else {
    output_text += "无输出内容。";
  }

  result_output_->setPlainText(output_text);
}

QString AlgorithmRunDialog::cache_key(const AlgorithmParam &param) const {
  if (!param.id.trimmed().isEmpty()) {
    return param.id.trimmed();
  }
  if (!param.identifier.trimmed().isEmpty()) {
    return param.identifier.trimmed();
  }
  return param.name.trimmed();
}

QString AlgorithmRunDialog::display_name(const AlgorithmParam &param) const {
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

bool AlgorithmRunDialog::parse_bool(const QString &text, bool *ok) const {
  const QString lower = text.trimmed().toLower();
  if (lower == "1" || lower == "true" || lower == "yes" || lower == "y" ||
      lower == QString::fromUtf8("是") || lower == QString::fromUtf8("真")) {
    *ok = true;
    return true;
  }
  if (lower == "0" || lower == "false" || lower == "no" || lower == "n" ||
      lower == QString::fromUtf8("否") || lower == QString::fromUtf8("假")) {
    *ok = true;
    return false;
  }
  *ok = false;
  return false;
}
