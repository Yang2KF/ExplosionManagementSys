#include "title_bar.h"
#include "ui_system.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

TitleBar::TitleBar(QWidget *parent) : QWidget(parent) { init_ui(); }

void TitleBar::init_ui() {
  this->setObjectName("TitleBar");
  auto *layout = new QHBoxLayout(this);
  QMargins margins = layout->contentsMargins();
  qDebug() << "Left:" << margins.left() << "Top:" << margins.top()
           << "Right:" << margins.right() << "Bottom:" << margins.bottom();

  // 标题
  QLabel *title = new QLabel("爆炸毁伤算法管理系统", this);
  title->setObjectName("TitleLabel");

  // 按钮
  auto *min_btn = create_btn(UISystem::instance().min_icon());
  auto *max_btn = create_btn(UISystem::instance().max_icon());
  auto *close_btn = create_btn(UISystem::instance().close_icon());

  layout->addWidget(title);
  layout->addStretch();
  layout->addWidget(min_btn);
  layout->addWidget(max_btn);
  layout->addWidget(close_btn);

  // 需转发给 main_window
  connect(min_btn, &QToolButton::clicked, this, &TitleBar::minClicked);
  connect(max_btn, &QToolButton::clicked, this, &TitleBar::maxClicked);
  connect(close_btn, &QToolButton::clicked, this, &TitleBar::closeClicked);
}

QToolButton *TitleBar::create_btn(const QIcon &icon) {
  auto *btn = new QToolButton(this);
  btn->setFixedSize(30, 30);
  btn->setIcon(icon);
  btn->setIconSize(QSize{25, 25});

  return btn;
}