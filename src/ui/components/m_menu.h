#pragma once

#include <QMap>
#include <QMenu>

class MaterialMenu : public QMenu {
  Q_OBJECT
public:
  explicit MaterialMenu(QWidget *parent = nullptr);

  QAction *add_action(const QString &action_id, const QString &text);
  QString exec_and_get_id(const QPoint &global_pos);

protected:
  void showEvent(QShowEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  void update_round_mask();

  QMap<QAction *, QString> action_map_;
};
