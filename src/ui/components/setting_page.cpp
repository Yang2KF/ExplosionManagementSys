#include "setting_page.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

SettingPage::SettingPage(QWidget *parent) : QWidget(parent) { init_ui(); }

void SettingPage::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  this->setObjectName("SettingPage");

  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *label = new QLabel("setting", this);
  label->setAlignment(Qt::AlignCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
}