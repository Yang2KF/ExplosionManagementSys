#include "home_page.h"
#include <QLabel>
#include <QVBoxLayout>

HomePage::HomePage(QWidget *parent) : QWidget(parent) { init_ui(); }

void HomePage::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  this->setObjectName("HomePage");

  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *label = new QLabel("主界面", this);
  label->setAlignment(Qt::AlignCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
}