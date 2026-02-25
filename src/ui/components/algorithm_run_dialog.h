#pragma once

#include "entities/algorithm_data.h"
#include "m_button.h"
#include "m_input.h"
#include "model/algorithm_param_service.h"
#include "model/algorithm_runner.h"
#include <QDialog>
#include <QFormLayout>
#include <QJsonObject>
#include <QMap>

class QLabel;
class QScrollArea;
class QTextEdit;

class AlgorithmRunDialog : public QDialog {
  Q_OBJECT
public:
  explicit AlgorithmRunDialog(const AlgorithmInfo &algorithm,
                              QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *event) override;
  void done(int r) override;
  void showEvent(QShowEvent *event) override;

private:
  void init_ui();
  void load_params();
  void rebuild_param_form();
  void reset_param_inputs();
  bool collect_input_json(QJsonObject *input_json, QString *error_message) const;
  void run_algorithm();

  QString cache_key(const AlgorithmParam &param) const;
  QString display_name(const AlgorithmParam &param) const;
  bool parse_bool(const QString &text, bool *ok) const;

  AlgorithmInfo algorithm_;
  QList<AlgorithmParam> params_;
  QMap<QString, MaterialInput *> param_inputs_;

  AlgorithmParamService param_service_;
  AlgorithmRunner algorithm_runner_;

  QLabel *algo_label_;
  QScrollArea *param_scroll_area_;
  QWidget *param_form_widget_;
  QFormLayout *param_form_layout_;
  MaterialButton *reset_btn_;
  MaterialButton *run_btn_;
  MaterialButton *close_btn_;
  QTextEdit *result_output_;
};
