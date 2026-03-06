#include "title_bar.h"
#include "ui_system.h"
#include <QToolButton>

TitleBar::TitleBar(QWidget *parent) : QWidget(parent) { init_ui(); }

void TitleBar::init_ui() {
  setAttribute(Qt::WA_StyledBackground, true);
  setObjectName("TitleBar");
  setFixedHeight(48);

  layout_ = new QHBoxLayout(this);
  layout_->setContentsMargins(8, 6, 8, 4);
  layout_->setSpacing(6);

  title_label_ =
      new QLabel(QStringLiteral("爆炸毁伤算法管理系统"), this);
  title_label_->setObjectName("TitleLabel");

  center_host_ = new QWidget(this);
  center_host_->setObjectName("TitleCenterHost");
  center_host_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  center_layout_ = new QHBoxLayout(center_host_);
  center_layout_->setContentsMargins(0, 0, 0, 0);
  center_layout_->setSpacing(0);

  auto *min_btn = create_btn(UISystem::instance().min_icon());
  auto *max_btn = create_btn(UISystem::instance().max_icon());
  auto *close_btn = create_btn(UISystem::instance().close_icon());

  layout_->addWidget(title_label_, 0, Qt::AlignVCenter);
  layout_->addWidget(center_host_, 1);
  layout_->addWidget(min_btn, 0, Qt::AlignVCenter);
  layout_->addWidget(max_btn, 0, Qt::AlignVCenter);
  layout_->addWidget(close_btn, 0, Qt::AlignVCenter);

  connect(min_btn, &QToolButton::clicked, this, &TitleBar::minClicked);
  connect(max_btn, &QToolButton::clicked, this, &TitleBar::maxClicked);
  connect(close_btn, &QToolButton::clicked, this, &TitleBar::closeClicked);
}

void TitleBar::setCenterWidget(QWidget *widget) {
  if (!center_layout_ || !widget) {
    return;
  }

  while (QLayoutItem *item = center_layout_->takeAt(0)) {
    if (QWidget *old_widget = item->widget()) {
      old_widget->setParent(nullptr);
    }
    delete item;
  }

  widget->setParent(center_host_);
  center_layout_->addWidget(widget);
}

QToolButton *TitleBar::create_btn(const QIcon &icon) {
  auto *btn = new QToolButton(this);
  btn->setFixedSize(30, 30);
  btn->setIcon(icon);
  btn->setIconSize(QSize{20, 20});

  return btn;
}
