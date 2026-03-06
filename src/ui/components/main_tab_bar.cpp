#include "main_tab_bar.h"
#include "ui_system.h"
#include <QEnterEvent>
#include <QFontMetrics>
#include <QPainter>

namespace {

QPixmap tint_icon(const QIcon &icon, const QSize &size, const QColor &color) {
  QPixmap pixmap = icon.pixmap(size);
  if (pixmap.isNull()) {
    return pixmap;
  }

  QPainter painter(&pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(pixmap.rect(), color);
  painter.end();
  return pixmap;
}

} // namespace

MainTabButton::MainTabButton(const QString &title, const QIcon &normal_icon,
                             const QIcon &active_icon, QWidget *parent)
    : QAbstractButton(parent), title_(title), normal_icon_(normal_icon),
      active_icon_(active_icon.isNull() ? normal_icon : active_icon) {
  setCheckable(true);
  setCursor(Qt::PointingHandCursor);
  setFixedHeight(34);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

QSize MainTabButton::sizeHint() const {
  const QFontMetrics metrics(font());
  const int min_width = 100;
  const int text_width = metrics.horizontalAdvance(title_);
  const int width = qMax(min_width, 24 + 14 + text_width + 16);
  return QSize(width, 34);
}

void MainTabButton::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  const QColor bg_color =
      isChecked() ? QColor("#EAF1FF")
                  : (hovered_ ? UISystem::instance().bg_hover()
                              : Qt::transparent);
  const QColor icon_color =
      isChecked() ? UISystem::instance().icon_active()
                  : UISystem::instance().icon_normal();
  const QColor text_color =
      isChecked() ? UISystem::instance().text_primary()
                  : UISystem::instance().text_secondary();

  const QRect body_rect = rect().adjusted(2, 2, -2, -2);
  painter.setPen(Qt::NoPen);
  painter.setBrush(bg_color);
  painter.drawRoundedRect(body_rect, 9, 9);

  const QIcon icon = isChecked() ? active_icon_ : normal_icon_;
  const QSize icon_size(16, 16);
  const QRect icon_rect(body_rect.left() + 10,
                        body_rect.top() + (body_rect.height() - icon_size.height()) / 2,
                        icon_size.width(), icon_size.height());
  const QPixmap icon_pixmap = tint_icon(icon, icon_size, icon_color);
  if (!icon_pixmap.isNull()) {
    painter.drawPixmap(icon_rect, icon_pixmap);
  }

  QRect text_rect = body_rect.adjusted(icon_rect.right() + 7, 0, -11, 0);
  painter.setPen(text_color);
  painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, title_);

  if (isChecked()) {
    painter.setBrush(UISystem::instance().bg_primary());
    painter.drawRoundedRect(QRect(body_rect.left() + 16, body_rect.bottom() - 2,
                                  body_rect.width() - 32, 2),
                            1.0, 1.0);
  }
}

void MainTabButton::enterEvent(QEnterEvent *event) {
  hovered_ = true;
  QAbstractButton::enterEvent(event);
  update();
}

void MainTabButton::leaveEvent(QEvent *event) {
  hovered_ = false;
  QAbstractButton::leaveEvent(event);
  update();
}

MainTabBar::MainTabBar(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_StyledBackground, true);
  setObjectName("MainTabBar");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  layout_ = new QHBoxLayout(this);
  layout_->setContentsMargins(6, 0, 6, 0);
  layout_->setSpacing(6);
  layout_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  button_group_ = new QButtonGroup(this);
  button_group_->setExclusive(true);
  connect(button_group_, &QButtonGroup::idClicked, this,
          &MainTabBar::tabRequested);
}

int MainTabBar::addTab(const QString &title, const QIcon &normal_icon,
                       const QIcon &active_icon) {
  MainTabButton *button =
      new MainTabButton(title, normal_icon, active_icon, this);
  const int index = buttons_.size();
  buttons_.append(button);
  layout_->addWidget(button);
  button_group_->addButton(button, index);

  if (current_index_ < 0) {
    setCurrentIndex(index);
  }

  return index;
}

void MainTabBar::setCurrentIndex(int index) {
  if (index < 0 || index >= buttons_.size()) {
    return;
  }

  current_index_ = index;
  for (int i = 0; i < buttons_.size(); ++i) {
    buttons_[i]->setChecked(i == index);
    buttons_[i]->update();
  }
}
