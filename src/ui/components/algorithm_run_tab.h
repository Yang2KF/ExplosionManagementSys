#pragma once

#include "entities/algorithm_data.h"
#include "m_button.h"
#include "m_input.h"
#include "model/algorithm_runner.h"
#include "service/algorithm_param_service.h"
#include <QMap>
#include <QJsonObject>
#include <QJsonValue>
#include <QWidget>

class QFormLayout;
class QLabel;
class QScrollArea;
class QTableWidget;

class AlgorithmRunTab : public QWidget {
  Q_OBJECT

public:
  explicit AlgorithmRunTab(const AlgorithmInfo &algorithm,
                           QWidget *parent = nullptr);

  QString tabKey() const;

private:
  void init_ui();
  void load_params();
  void rebuild_param_form();
  void reset_param_inputs();
  bool collect_input_json(QJsonObject *input_json,
                          QString *error_message) const;
  void run_algorithm();
  void render_result(const AlgorithmRunResult &run_result);
  void append_result_value_rows(const QString &key, const QJsonValue &value,
                                QList<QPair<QString, QString>> *rows) const;
  QString result_cell_text(const QJsonValue &value) const;

  QString cache_key(const AlgorithmParam &param) const;
  QString display_name(const AlgorithmParam &param) const;
  bool parse_bool(const QString &text, bool *ok) const;

  AlgorithmInfo algorithm_;
  QList<AlgorithmParam> params_;
  QMap<QString, MaterialInput *> param_inputs_;

  AlgorithmParamService param_service_;
  AlgorithmRunner algorithm_runner_;

  QLabel *algo_label_ = nullptr;
  QScrollArea *param_scroll_area_ = nullptr;
  QWidget *param_form_widget_ = nullptr;
  QFormLayout *param_form_layout_ = nullptr;
  MaterialButton *reset_btn_ = nullptr;
  MaterialButton *run_btn_ = nullptr;
  QTableWidget *result_table_ = nullptr;
};
