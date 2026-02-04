#include "sider_bar.h"
#include "m_button.h"
#include "ui_system.h"
#include <QButtonGroup>
#include <QVBoxLayout>

SiderBar::SiderBar(QWidget *parent) : QWidget(parent) { init_ui(); }

void SiderBar::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 20, 10, 20); // 上下留白
  layout->setSpacing(15);

  QButtonGroup *nav_group = new QButtonGroup(this);
  nav_group->setExclusive(true);

  struct SiderInfo {
    int id;
    QIcon icon;
    QString tool_tip;
  };

  QList<SiderInfo> sider_lists{
      {0, UISystem::instance()->user_icon(), "user"},
      {1, UISystem::instance()->home_icon(), "home"},
      {2, UISystem::instance()->function_icon(), "function"},
      {3, UISystem::instance()->settings_icon(), "settings"},
      {4, UISystem::instance()->information_icon(), "information"}};

  for (const auto &sider_btn : sider_lists) {
    MaterialButton *btn = new MaterialButton(this);
    btn->setFixedSize(50, 50);
    btn->setIcon(sider_btn.icon);
    btn->setIconSize(QSize{35, 35});
    btn->setCheckable(true);

    nav_group->addButton(btn, sider_btn.id);
    layout->addWidget(btn);

    if (sider_btn.id == 0)
      layout->addSpacing(20); // 用户下稍微加点距离
  }

  layout->addStretch();

  connect(nav_group, &QButtonGroup::idClicked, this,
          &SiderBar::onSiderBtnRequest);
}