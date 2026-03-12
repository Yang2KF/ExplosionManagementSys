#pragma once

#include "entities/algorithm_data.h"
#include "m_button.h"
#include "m_input.h"
#include "service/algorithm_param_service.h"
#include "service/algorithm_service.h"
#include "service/category_service.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QSpinBox;
class QTableWidget;

class AlgorithmEditTab : public QWidget {
  Q_OBJECT
public:
  explicit AlgorithmEditTab(const AlgorithmInfo &info = AlgorithmInfo{},
                            QWidget *parent = nullptr);

  QString tabKey() const;
  QString tabTitle() const;

signals:
  void saved(const QString &old_key, const QString &new_key,
             const QString &title);

private:
  void init_ui();
  void load_categories();
  void load_data(const AlgorithmInfo &info);
  void load_params();
  void reload_param_table();
  void reset_param_form();
  void fill_param_form(const AlgorithmParam &param, int row);
  void sync_param_group_state();
  int current_param_row() const;
  void update_source_type_ui();
  void save_algorithm();
  void save_param();
  void edit_selected_param();
  void delete_selected_param();
  AlgorithmInfo get_data() const;
  AlgorithmParam get_param_data() const;

  AlgorithmInfo current_info_;
  CategoryService category_service_;
  AlgorithmService algorithm_service_;
  AlgorithmParamService algorithm_param_service_;
  QList<AlgorithmParam> params_;
  int editing_param_row_ = -1;
  QGroupBox *param_group_ = nullptr;

  MaterialInput *name_input_ = nullptr;
  QComboBox *category_combo_ = nullptr;
  QComboBox *source_type_combo_ = nullptr;
  MaterialInput *path_input_ = nullptr;
  MaterialButton *path_btn_ = nullptr;
  MaterialInput *func_input_ = nullptr;
  MaterialInput *desc_input_ = nullptr;

  QTableWidget *param_table_ = nullptr;
  MaterialButton *add_param_btn_ = nullptr;
  MaterialButton *edit_param_btn_ = nullptr;
  MaterialButton *delete_param_btn_ = nullptr;
  MaterialInput *param_identifier_input_ = nullptr;
  MaterialInput *param_name_input_ = nullptr;
  MaterialInput *param_unit_input_ = nullptr;
  QComboBox *param_type_combo_ = nullptr;
  MaterialInput *param_default_input_ = nullptr;
  MaterialInput *param_min_input_ = nullptr;
  MaterialInput *param_max_input_ = nullptr;
  MaterialInput *param_tooltip_input_ = nullptr;
  MaterialInput *param_comments_input_ = nullptr;
  QCheckBox *param_required_check_ = nullptr;
  QSpinBox *param_order_spin_ = nullptr;
  MaterialButton *save_param_btn_ = nullptr;
  MaterialButton *cancel_param_btn_ = nullptr;

  MaterialButton *save_algo_btn_ = nullptr;
};
