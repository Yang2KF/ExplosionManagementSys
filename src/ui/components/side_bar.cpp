#include "side_bar.h"
#include "m_button.h"
#include "ui_system.h"
#include <QButtonGroup>
#include <QVBoxLayout>

SideBar::SideBar(QWidget *parent) : QWidget(parent) { init_ui(); }

void SideBar::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 20, 10, 20); // 上下留白
  layout->setSpacing(15);

  QButtonGroup *nav_group = new QButtonGroup(this);
  nav_group->setExclusive(true);

  struct SideInfo {
    int id;
    QIcon icon;
    QIcon icon_checked;
    QString tool_tip;
  };

  UISystem *ins = UISystem::instance();
  QList<SideInfo> sider_lists{
      {0, ins->user_icon(), ins->user_icon(), "user"},
      {1, ins->home_icon(), ins->home_icon_checked(), "home"},
      {2, ins->function_icon(), ins->function_icon_checked(), "function"},
      {3, ins->settings_icon(), ins->settings_icon_checked(), "settings"},
      {4, ins->information_icon(), ins->information_icon_checked(),
       "information"}};

  for (const auto &side_btn : sider_lists) {
    MaterialButton *btn = new MaterialButton(MaterialButton::SideBar, this);
    btn->setFixedSize(50, 50);
    btn->setIcon(side_btn.icon);
    btn->setIconSize(QSize{35, 35});
    btn->set_icons(side_btn.icon, side_btn.icon_checked);
    nav_group->addButton(btn, side_btn.id);
    layout->addWidget(btn);

    if (side_btn.id == 0)
      layout->addSpacing(20); // 用户下稍微加点距离
  }

  layout->addStretch();

  connect(nav_group, &QButtonGroup::idClicked, this,
          &SideBar::onSiderBtnRequest);
}