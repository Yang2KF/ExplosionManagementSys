#include "m_input.h"
#include "ui_system.h"
#include <QPainter>

namespace {
// 改造后的染色函数，增加尺寸对齐
QIcon tint_icon(const QIcon &icon, const QSize &target_size,
                const QColor &color) {
  // 强制获取目标尺寸的 pixmap
  QPixmap pixmap = icon.pixmap(target_size);
  if (pixmap.isNull()) {
    return QIcon();
  }

  QPainter painter(&pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(pixmap.rect(), color);
  painter.end();
  return QIcon(pixmap); // 返回一个包含染色后 Pixmap 的新 QIcon
}
} // namespace

MaterialInput::MaterialInput(InputType type, QWidget *parent)
    : QLineEdit(parent), input_type_(type), line_progress_(0.0) {

  if (input_type_ == Edit) {
    setObjectName("MaterialEditInput");
    setFixedHeight(40);
    active_color_ = UISystem::instance().bg_primary();
    line_animation_ = new QPropertyAnimation(this, "lineProgress");
    line_animation_->setDuration(300);
    line_animation_->setEasingCurve(QEasingCurve::OutCurve);
  } else {
    setObjectName("MaterialSearchInput");
    setFixedHeight(38);
    setClearButtonEnabled(true);

    search_action_ = new QAction(this);
    this->addAction(search_action_, QLineEdit::LeadingPosition);
    setTextMargins(4, 0, 0, 0);

    connect(this, &QLineEdit::textChanged, this,
            [this](const QString &) { update_search_icon(); });
    update_search_icon();
  }
}

void MaterialInput::paintEvent(QPaintEvent *event) {
  QLineEdit::paintEvent(event);

  if (input_type_ == Search)
    return;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, false);

  painter.fillRect(0, height() - 1, width(), 1, UISystem::instance().line());
  if (line_progress_ > 0.0) {
    int line_width = static_cast<int>(width() * line_progress_);
    int start_x = (width() - line_width) / 2;

    painter.fillRect(start_x, height() - 2, line_width, 2, active_color_);
  }
}

void MaterialInput::focusInEvent(QFocusEvent *event) {
  QLineEdit::focusInEvent(event);

  if (input_type_ == Search && search_action_) {
    update_search_icon();
  }
  if (input_type_ == Edit && line_animation_) {
    line_animation_->stop();
    line_animation_->setStartValue(0.0);
    line_animation_->setEndValue(1.0);
    line_animation_->start();
  }
}

void MaterialInput::focusOutEvent(QFocusEvent *event) {
  QLineEdit::focusOutEvent(event);

  if (input_type_ == Search && search_action_) {
    update_search_icon();
  }
  if (input_type_ == Edit && line_animation_) {
    line_animation_->stop();
    line_animation_->setStartValue(1.0);
    line_animation_->setEndValue(0.0);
    line_animation_->start();
  }
}

void MaterialInput::set_line_progress(float p) {
  line_progress_ = p;
  update(); // 触发重绘
}

void MaterialInput::update_search_icon() {
  if (input_type_ != Search || !search_action_) {
    return;
  }

  const bool focused = hasFocus();
  const bool has_text = !text().trimmed().isEmpty();
  const QColor icon_color =
      (focused || has_text) ? UISystem::instance().icon_active()
                            : UISystem::instance().text_secondary();
  const QIcon base_icon = UISystem::instance().search_icon();
  search_action_->setIcon(tint_icon(base_icon, QSize(16, 16), icon_color));
}
