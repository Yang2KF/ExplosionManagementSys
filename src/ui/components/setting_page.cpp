#include "setting_page.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

SettingPage::SettingPage(QWidget *parent) : QWidget(parent) { init_ui(); }

void SettingPage::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  this->setStyleSheet("background-color: white; border-radius: 8px;");

  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *label = new QLabel("setting", this);
  label->setAlignment(Qt::AlignCenter);
  label->setStyleSheet(
      "background: blue; border-radius: 8px; font-size: 24px;");
  layout->addWidget(label);
}