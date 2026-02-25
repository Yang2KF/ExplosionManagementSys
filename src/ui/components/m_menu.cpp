#include "m_menu.h"
#include <QAction>
#include <QPainterPath>
#include <QRegion>
#include <QResizeEvent>
#include <QShowEvent>

MaterialMenu::MaterialMenu(QWidget *parent) : QMenu(parent) {
  setObjectName("MaterialMenu");
  setWindowFlag(Qt::FramelessWindowHint, true);
  setWindowFlag(Qt::NoDropShadowWindowHint, true);
  setAttribute(Qt::WA_TranslucentBackground, true);
  setAttribute(Qt::WA_StyledBackground, true);
  setSeparatorsCollapsible(true);
}

QAction *MaterialMenu::add_action(const QString &action_id, const QString &text) {
  QAction *action = addAction(text);
  action_map_.insert(action, action_id);
  return action;
}

QString MaterialMenu::exec_and_get_id(const QPoint &global_pos) {
  QAction *chosen = exec(global_pos);
  if (!chosen) {
    return QString();
  }
  return action_map_.value(chosen, QString());
}

void MaterialMenu::showEvent(QShowEvent *event) {
  update_round_mask();
  QMenu::showEvent(event);
}

void MaterialMenu::resizeEvent(QResizeEvent *event) {
  QMenu::resizeEvent(event);
  update_round_mask();
}

void MaterialMenu::update_round_mask() {
  if (width() <= 0 || height() <= 0) {
    return;
  }

  constexpr qreal kRadius = 8.0;
  QPainterPath path;
  path.addRoundedRect(rect().adjusted(0, 0, -1, -1), kRadius, kRadius);
  setMask(QRegion(path.toFillPolygon().toPolygon()));
}
