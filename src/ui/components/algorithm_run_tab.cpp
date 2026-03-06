#include "algorithm_run_tab.h"
#include "m_message_box.h"
#include "ui_system.h"
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLayoutItem>
#include <QScrollArea>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

AlgorithmRunTab::AlgorithmRunTab(const AlgorithmInfo &algorithm, QWidget *parent)
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
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(0, 0, 0, 0);
  main_layout->setSpacing(0);

  QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
  splitter->setChildrenCollapsible(false);
  splitter->setHandleWidth(1);
  main_layout->addWidget(splitter);

  QWidget *left_panel = new QWidget(splitter);
  left_panel->setObjectName("FunctionRunnerPanel");
  QVBoxLayout *left_layout = new QVBoxLayout(left_panel);
  left_layout->setContentsMargins(14, 14, 14, 14);
  left_layout->setSpacing(10);

  QLabel *title_label = new QLabel(QStringLiteral("\u7b97\u6cd5\u8fd0\u884c"), left_panel);
  title_label->setObjectName("FunctionRunnerTitle");

  algo_label_ = new QLabel(left_panel);
  algo_label_->setObjectName("FunctionRunnerAlgo");
  algo_label_->setWordWrap(true);

  const QString runtime_type =
      (algorithm_.sourceType.trimmed() == "2" ||
       QFileInfo(algorithm_.filePath.trimmed()).suffix().toLower() == "py")
          ? QStringLiteral("Python")
          : QStringLiteral("DLL");
  algo_label_->setText(
      QStringLiteral("%1\n%2: %3\n%4: %5")
          .arg(algorithm_.name, QStringLiteral("\u8fd0\u884c\u7c7b\u578b"),
               runtime_type, QStringLiteral("\u5165\u53e3\u51fd\u6570"),
               algorithm_.funcName));

  param_scroll_area_ = new QScrollArea(left_panel);
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

  reset_btn_ = new MaterialButton(QStringLiteral("\u91cd\u7f6e"),
                                  MaterialButton::Normal, left_panel);
  reset_btn_->set_theme_color(UISystem::instance().neutral());
  reset_btn_->setFixedSize(90, 34);

  run_btn_ = new MaterialButton(QStringLiteral("\u8fd0\u884c"),
                                MaterialButton::Normal, left_panel);
  run_btn_->set_theme_color(UISystem::instance().bg_primary());
  run_btn_->setFixedSize(90, 34);

  button_layout->addWidget(reset_btn_);
  button_layout->addWidget(run_btn_);

  left_layout->addWidget(title_label);
  left_layout->addWidget(algo_label_);
  left_layout->addWidget(param_scroll_area_, 1);
  left_layout->addLayout(button_layout);

  QWidget *right_panel = new QWidget(splitter);
  right_panel->setObjectName("FunctionRunnerPanel");
  QVBoxLayout *right_layout = new QVBoxLayout(right_panel);
  right_layout->setContentsMargins(14, 14, 14, 14);
  right_layout->setSpacing(10);

  QLabel *result_title =
      new QLabel(QStringLiteral("\u8fd0\u884c\u7ed3\u679c"), right_panel);
  result_title->setObjectName("FunctionRunnerTitle");

  result_output_ = new QTextEdit(right_panel);
  result_output_->setObjectName("FunctionRunnerResult");
  result_output_->setReadOnly(true);
  result_output_->setPlaceholderText(
      QStringLiteral("\u8fd0\u884c\u7ed3\u679c\u5c06\u5728\u6b64\u663e\u793a\u3002"));

  right_layout->addWidget(result_title);
  right_layout->addWidget(result_output_, 1);

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
    result_output_->setPlainText(QStringLiteral("\u52a0\u8f7d\u53c2\u6570\u5b9a\u4e49\u5931\u8d25\uff1a") +
                                 error_message);
  }
}

void AlgorithmRunTab::rebuild_param_form() {
  while (param_form_layout_->count() > 0) {
    QLayoutItem *item = param_form_layout_->takeAt(0);
    if (item) {
      if (QWidget *w = item->widget()) {
        w->deleteLater();
      }
      delete item;
    }
  }
  param_inputs_.clear();

  if (params_.isEmpty()) {
    QLabel *empty_label = new QLabel(
        QStringLiteral("\u8be5\u7b97\u6cd5\u5c1a\u672a\u914d\u7f6e\u53c2\u6570\u3002"),
        param_form_widget_);
    empty_label->setWordWrap(true);
    param_form_layout_->addRow(empty_label);
    return;
  }

  for (const AlgorithmParam &param : params_) {
    QLabel *name_label = new QLabel(display_name(param), param_form_widget_);
    name_label->setObjectName("FunctionParamLabel");

    MaterialInput *value_input = new MaterialInput(param_form_widget_);
    value_input->setPlaceholderText(
        param.identifier.isEmpty()
            ? QStringLiteral("\u8bf7\u8f93\u5165\u53c2\u6570\u503c")
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
          *error_message =
              QStringLiteral("\u53c2\u6570\u300c%1\u300d\u4e0d\u80fd\u4e3a\u7a7a\u3002")
                  .arg(display);
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
          *error_message =
              QStringLiteral("\u53c2\u6570\u300c%1\u300d\u5fc5\u987b\u662f\u6574\u6570\u3002")
                  .arg(display);
        }
        return false;
      }

      if (!param.minValue.trimmed().isEmpty()) {
        bool min_ok = false;
        const double min_value = param.minValue.toDouble(&min_ok);
        if (min_ok && value < min_value) {
          if (error_message) {
            *error_message = QStringLiteral(
                                 "\u53c2\u6570\u300c%1\u300d\u5e94\u5927\u4e8e\u7b49\u4e8e %2\u3002")
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
            *error_message = QStringLiteral(
                                 "\u53c2\u6570\u300c%1\u300d\u5e94\u5c0f\u4e8e\u7b49\u4e8e %2\u3002")
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
          *error_message =
              QStringLiteral("\u53c2\u6570\u300c%1\u300d\u5fc5\u987b\u662f\u6570\u503c\u3002")
                  .arg(display);
        }
        return false;
      }

      if (!param.minValue.trimmed().isEmpty()) {
        bool min_ok = false;
        const double min_value = param.minValue.toDouble(&min_ok);
        if (min_ok && value < min_value) {
          if (error_message) {
            *error_message = QStringLiteral(
                                 "\u53c2\u6570\u300c%1\u300d\u5e94\u5927\u4e8e\u7b49\u4e8e %2\u3002")
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
            *error_message = QStringLiteral(
                                 "\u53c2\u6570\u300c%1\u300d\u5e94\u5c0f\u4e8e\u7b49\u4e8e %2\u3002")
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
          *error_message = QStringLiteral(
                               "\u53c2\u6570\u300c%1\u300d\u5fc5\u987b\u662f\u5e03\u5c14\u503c\u3002")
                               .arg(display);
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
    MaterialMessageBox::warning(
        this, QStringLiteral("\u53c2\u6570\u4e0d\u5408\u6cd5"), error_message);
    return;
  }

  const AlgorithmRunResult run_result = algorithm_runner_.run(algorithm_, input_json);

  QString output_text;
  output_text +=
      QStringLiteral("\u72b6\u6001\uff1a%1\n")
          .arg(run_result.success ? QStringLiteral("\u6210\u529f")
                                  : QStringLiteral("\u5931\u8d25"));
  output_text +=
      QStringLiteral("\u8017\u65f6\uff1a%1 ms\n").arg(run_result.elapsedMs);
  output_text += QStringLiteral("\u6d88\u606f\uff1a%1\n").arg(run_result.message);
  output_text += QStringLiteral("------------------------------\n");

  if (!run_result.outputJson.isEmpty()) {
    output_text += QString::fromUtf8(
        QJsonDocument(run_result.outputJson).toJson(QJsonDocument::Indented));
  } else if (!run_result.rawOutput.trimmed().isEmpty()) {
    output_text += run_result.rawOutput;
  } else {
    output_text += QStringLiteral("\u65e0\u8f93\u51fa\u5185\u5bb9\u3002");
  }

  result_output_->setPlainText(output_text);
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
      lower == QStringLiteral("\u662f") || lower == QStringLiteral("\u771f")) {
    *ok = true;
    return true;
  }
  if (lower == "0" || lower == "false" || lower == "no" || lower == "n" ||
      lower == QStringLiteral("\u5426") || lower == QStringLiteral("\u5047")) {
    *ok = true;
    return false;
  }
  *ok = false;
  return false;
}

