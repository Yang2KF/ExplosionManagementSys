#include "main_tab_bar.h"
#include "ui_system.h"
#include <QEnterEvent>
#include <QFontMetrics>
#include <QPalette>
#include <QPainter>

namespace {
constexpr auto kContentSurface = "#F2F3F5";
constexpr auto kTitleSurface = "#E5E7EA";
constexpr auto kTabHover = "#ECEEF1";
constexpr int kTabWidth = 180;
constexpr int kTabHeight = 34;
constexpr int kIconSize = 16;
constexpr int kCloseSize = 14;

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
                             const QIcon &active_icon, bool closable,
                             QWidget *parent)
    : QAbstractButton(parent), title_(title), normal_icon_(normal_icon),
      active_icon_(active_icon.isNull() ? normal_icon : active_icon),
      closable_(closable) {
  setCheckable(true);
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::NoFocus);
  setMouseTracking(true);
  setFixedSize(kTabWidth, kTabHeight);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize MainTabButton::sizeHint() const {
  return QSize(kTabWidth, kTabHeight);
}

void MainTabButton::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  const QColor bg_color =
      isChecked() ? QColor(kContentSurface)
                  : (hovered_ ? QColor(kTabHover) : Qt::transparent);
  const QColor icon_color =
      isChecked() ? UISystem::instance().icon_active()
                  : UISystem::instance().icon_normal();
  const QColor text_color =
      isChecked() ? UISystem::instance().text_primary()
                  : UISystem::instance().text_secondary();

  const QRect body_rect = rect().adjusted(2, 1, -2, 0);
  painter.setPen(Qt::NoPen);
  painter.setBrush(bg_color);
  painter.drawRoundedRect(body_rect, 8, 8);

  const QIcon icon = isChecked() ? active_icon_ : normal_icon_;
  const QSize icon_size(kIconSize, kIconSize);
  const QRect icon_rect(body_rect.left() + 10,
                        body_rect.top() + (body_rect.height() - icon_size.height()) / 2,
                        icon_size.width(), icon_size.height());
  const QPixmap icon_pixmap = tint_icon(icon, icon_size, icon_color);
  if (!icon_pixmap.isNull()) {
    painter.drawPixmap(icon_rect, icon_pixmap);
  }

  const int right_padding = closable_ ? (kCloseSize + 12) : 11;
  QRect text_rect = body_rect.adjusted(icon_rect.right() + 7, 0, -right_padding, 0);
  painter.setPen(text_color);
  const QString shown_title = elided_title(text_rect.width());
  setToolTip(shown_title == title_ ? QString() : title_);
  painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, shown_title);

  if (closable_) {
    const QRect close_rect = this->close_rect();
    if (close_hovered_) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(UISystem::instance().line());
      painter.drawRoundedRect(close_rect.adjusted(-2, -2, 2, 2), 4, 4);
    }
    const QPixmap close_icon = tint_icon(UISystem::instance().close_icon(),
                                         QSize(12, 12), text_color);
    if (!close_icon.isNull()) {
      const QRect icon_rect(close_rect.center().x() - 6, close_rect.center().y() - 6,
                            12, 12);
      painter.drawPixmap(icon_rect, close_icon);
    }
  }

  if (isChecked()) {
    // Make selected tab blend into content region.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(kContentSurface));
    painter.drawRect(QRect(body_rect.left() + 1, body_rect.bottom() - 1,
                           body_rect.width() - 2, 2));
  }
}

void MainTabButton::enterEvent(QEnterEvent *event) {
  hovered_ = true;
  close_hovered_ = closable_ && close_rect().contains(event->position().toPoint());
  QAbstractButton::enterEvent(event);
  update();
}

void MainTabButton::leaveEvent(QEvent *event) {
  hovered_ = false;
  close_hovered_ = false;
  QAbstractButton::leaveEvent(event);
  update();
}

void MainTabButton::mouseMoveEvent(QMouseEvent *event) {
  const bool hovered = closable_ && close_rect().contains(event->pos());
  if (hovered != close_hovered_) {
    close_hovered_ = hovered;
    update();
  }
  QAbstractButton::mouseMoveEvent(event);
}

void MainTabButton::mouseReleaseEvent(QMouseEvent *event) {
  if (closable_ && event->button() == Qt::LeftButton &&
      close_rect().contains(event->pos())) {
    setDown(false);
    update();
    emit closeRequested();
    event->accept();
    return;
  }
  QAbstractButton::mouseReleaseEvent(event);
}

QRect MainTabButton::close_rect() const {
  const QRect body_rect = rect().adjusted(2, 1, -2, 0);
  const int x = body_rect.right() - kCloseSize - 8;
  const int y = body_rect.top() + (body_rect.height() - kCloseSize) / 2;
  return QRect(x, y, kCloseSize, kCloseSize);
}

QString MainTabButton::elided_title(int width) const {
  const QFontMetrics metrics(font());
  return metrics.elidedText(title_, Qt::ElideRight, qMax(10, width));
}

MainTabBar::MainTabBar(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_StyledBackground, true);
  setObjectName("MainTabBar");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  setAutoFillBackground(true);
  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, QColor(kTitleSurface));
  setPalette(palette);

  layout_ = new QHBoxLayout(this);
  layout_->setContentsMargins(6, 1, 6, 0);
  layout_->setSpacing(6);
  layout_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  button_group_ = new QButtonGroup(this);
  button_group_->setExclusive(true);
  connect(button_group_, &QButtonGroup::idClicked, this,
          &MainTabBar::tabRequested);
}

int MainTabBar::addTab(const QString &title, const QIcon &normal_icon,
                       const QIcon &active_icon, bool closable) {
  MainTabButton *button =
      new MainTabButton(title, normal_icon, active_icon, closable, this);
  const int index = buttons_.size();
  buttons_.append(button);
  layout_->addWidget(button);
  button_group_->addButton(button, index);
  connect(button, &MainTabButton::closeRequested, this, [this, button]() {
    const int idx = buttons_.indexOf(button);
    if (idx >= 0) {
      emit tabCloseRequested(idx);
    }
  });

  if (current_index_ < 0) {
    setCurrentIndex(index);
  }

  return index;
}

void MainTabBar::removeTab(int index) {
  if (index < 0 || index >= buttons_.size()) {
    return;
  }

  MainTabButton *button = buttons_.takeAt(index);
  button_group_->removeButton(button);
  layout_->removeWidget(button);
  button->deleteLater();

  for (int i = 0; i < buttons_.size(); ++i) {
    button_group_->setId(buttons_[i], i);
  }

  if (buttons_.isEmpty()) {
    current_index_ = -1;
    return;
  }

  if (current_index_ == index) {
    current_index_ = qMax(0, index - 1);
  } else if (current_index_ > index) {
    --current_index_;
  }
  setCurrentIndex(current_index_);
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
