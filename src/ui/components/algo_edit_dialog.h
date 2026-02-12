#pragma once

#include "entities/algorithm_data.h"
#include "m_button.h"
#include "m_input.h"
#include <QComboBox>
#include <QDialog>

class AlgoEditDialog : public QDialog {
  Q_OBJECT
public:
  // 构造函数：is_edit=true 时为编辑模式，否则为新增模式
  explicit AlgoEditDialog(QWidget *parent = nullptr);

  // 设置回显数据（用于编辑模式）
  void set_data(const AlgorithmInfo &info);

  // 获取用户输入的数据
  AlgorithmInfo get_data() const;

protected:
  // 重写绘图事件以支持样式表背景
  void paintEvent(QPaintEvent *event) override;
  void done(int r) override;
  void showEvent(QShowEvent *event) override;

private:
  void init_ui();
  void load_categories(); // 加载分类到下拉框

  // UI 组件
  MaterialInput *name_input_;
  QComboBox *category_combo_;
  MaterialInput *path_input_;
  MaterialButton *path_btn_; // 选择文件按钮
  MaterialInput *func_input_;
  MaterialInput *desc_input_;

  MaterialButton *confirm_btn_;
  MaterialButton *cancel_btn_;

  // 拖拽相关
  QPoint drag_pos_;
  int current_algo_id_ = -1; // -1 表示新增
};