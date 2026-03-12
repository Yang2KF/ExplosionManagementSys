#include "tab_bar.h"
#include "ui_system.h"
#include <QEnterEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>

namespace {
constexpr int kTabWidth = 180;
constexpr int kTabHeight = 34;
constexpr int kIconSize = 16;
constexpr int kCloseSize = 14;
constexpr int kVisualOffsetY = -3;

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

TabButton::TabButton(const QString &title, const QIcon &normal_icon,
                     const QIcon &active_icon, bool closable, QWidget *parent)
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

QSize TabButton::sizeHint() const { return QSize(kTabWidth, kTabHeight); }

void TabButton::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  QColor content_surface = UISystem::instance().surface();
  QColor tab_hover = QColor{"#d6dae1"};

  const QColor bg_color =
      isChecked() ? content_surface : (hovered_ ? tab_hover : Qt::transparent);
  const QColor icon_color = isChecked() ? UISystem::instance().icon_active()
                                        : UISystem::instance().icon_normal();
  const QColor text_color = isChecked() ? UISystem::instance().bg_primary()
                                        : UISystem::instance().text_secondary();

  const QRect body_rect = rect();
  painter.setPen(Qt::NoPen);

  if (isChecked()) {
    painter.setBrush(content_surface);
    QPainterPath path;
    int radius = 8; // 顶部圆角大小

    path.moveTo(body_rect.bottomLeft());
    path.lineTo(body_rect.left(), body_rect.top() + radius);
    path.arcTo(body_rect.left(), body_rect.top(), radius * 2, radius * 2, 180,
               -90);
    path.lineTo(body_rect.right() - radius, body_rect.top());
    path.arcTo(body_rect.right() - radius * 2, body_rect.top(), radius * 2,
               radius * 2, 90, -90);
    path.lineTo(body_rect.bottomRight());
    path.closeSubpath();
    painter.drawPath(path);

    painter.drawRect(
        QRect(body_rect.left(), body_rect.bottom() - 1, body_rect.width(), 2));

  } else if (hovered_) {
    painter.setBrush(tab_hover);
    QRect hover_rect = body_rect.adjusted(2, 2, -2, -6);
    painter.drawRoundedRect(hover_rect, 6, 6); // 药丸圆角
  }

  const QIcon icon = isChecked() ? active_icon_ : normal_icon_;
  const QSize icon_size(kIconSize, kIconSize);

  QRect icon_rect(body_rect.left() + 12, // 左侧留白稍微大点
                  body_rect.top() +
                      (body_rect.height() - icon_size.height()) / 2 +
                      kVisualOffsetY,
                  icon_size.width(), icon_size.height());

  const QPixmap icon_pixmap = tint_icon(icon, icon_size, icon_color);
  if (!icon_pixmap.isNull()) {
    painter.drawPixmap(icon_rect, icon_pixmap);
  }

  const int right_padding = closable_ ? (kCloseSize + 16) : 12;
  QRect text_rect =
      body_rect.adjusted(icon_rect.right() + 8, 0, -right_padding, 0);
  text_rect.translate(0, kVisualOffsetY); // 统一上移

  painter.setPen(text_color);
  QFont font = painter.font();
  font.setBold(isChecked());
  painter.setFont(font);

  const QString shown_title = elided_title(text_rect.width());
  setToolTip(shown_title == title_ ? QString() : title_);
  painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, shown_title);

  if (closable_) {
    QRect close_rect = this->close_rect();
    close_rect.translate(0, kVisualOffsetY + 2);

    if (close_hovered_) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(UISystem::instance().line());
      painter.drawRoundedRect(close_rect.adjusted(-2, -2, 2, 2), 4, 4);
    }
    const QPixmap close_icon =
        tint_icon(UISystem::instance().close_icon(), QSize(12, 12), text_color);
    if (!close_icon.isNull()) {
      QRect c_icon_rect(close_rect.center().x() - 5,
                        close_rect.center().y() - 5, 10, 10);
      painter.drawPixmap(c_icon_rect, close_icon);
    }
  }
}

void TabButton::enterEvent(QEnterEvent *event) {
  hovered_ = true;
  close_hovered_ =
      closable_ && close_rect().contains(event->position().toPoint());
  QAbstractButton::enterEvent(event);
  update();
}

void TabButton::leaveEvent(QEvent *event) {
  hovered_ = false;
  close_hovered_ = false;
  QAbstractButton::leaveEvent(event);
  update();
}

void TabButton::mouseMoveEvent(QMouseEvent *event) {
  const bool hovered =
      closable_ &&
      close_rect().translated(0, kVisualOffsetY).contains(event->pos());
  if (hovered != close_hovered_) {
    close_hovered_ = hovered;
    update();
  }
  QAbstractButton::mouseMoveEvent(event);
}

void TabButton::mouseReleaseEvent(QMouseEvent *event) {
  if (closable_ && event->button() == Qt::LeftButton &&
      close_rect().translated(0, kVisualOffsetY).contains(event->pos())) {
    setDown(false);
    update();
    emit closeRequested();
    event->accept();
    return;
  }
  QAbstractButton::mouseReleaseEvent(event);
}

QRect TabButton::close_rect() const {
  const QRect body_rect = rect();
  const int x = body_rect.right() - kCloseSize - 8;
  const int y = body_rect.top() + (body_rect.height() - kCloseSize) / 2;
  return QRect(x, y, kCloseSize, kCloseSize);
}

QString TabButton::elided_title(int width) const {
  const QFontMetrics metrics(font());
  return metrics.elidedText(title_, Qt::ElideRight, qMax(10, width));
}

void TabButton::setTitle(const QString &title) {
  if (title_ == title) {
    return;
  }
  title_ = title;
  update();
}

TabBar::TabBar(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_StyledBackground, true);
  setObjectName("TabBar");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  setAutoFillBackground(true);
  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, QColor("#E5E7EA")); // 标题栏底色
  setPalette(palette);

  layout_ = new QHBoxLayout(this);
  layout_->setContentsMargins(6, 1, 6, 0);
  layout_->setSpacing(2); // Tab 之间的间距
  layout_->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

  button_group_ = new QButtonGroup(this);
  button_group_->setExclusive(true);
  connect(button_group_, &QButtonGroup::idClicked, this, &TabBar::tabRequested);
}

int TabBar::addTab(const QString &title, const QIcon &normal_icon,
                   const QIcon &active_icon, bool closable) {
  TabButton *button =
      new TabButton(title, normal_icon, active_icon, closable, this);
  const int index = buttons_.size();
  buttons_.append(button);
  layout_->addWidget(button);
  button_group_->addButton(button, index);
  connect(button, &TabButton::closeRequested, this, [this, button]() {
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

void TabBar::removeTab(int index) {
  if (index < 0 || index >= buttons_.size()) {
    return;
  }

  TabButton *button = buttons_.takeAt(index);
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

void TabBar::setTabTitle(int index, const QString &title) {
  if (index < 0 || index >= buttons_.size()) {
    return;
  }
  buttons_[index]->setTitle(title);
}

void TabBar::setCurrentIndex(int index) {
  if (index < 0 || index >= buttons_.size()) {
    return;
  }

  current_index_ = index;
  for (int i = 0; i < buttons_.size(); ++i) {
    buttons_[i]->setChecked(i == index);
    buttons_[i]->update();
  }
}
