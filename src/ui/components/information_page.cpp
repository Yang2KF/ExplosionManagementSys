#include "information_page.h"
#include <QLabel>
#include <QVBoxLayout>

InformationPage::InformationPage(QWidget *parent) : QWidget(parent) {
  init_ui();
}

void InformationPage::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  this->setObjectName("InformationPage");

  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *label = new QLabel("help", this);
  label->setAlignment(Qt::AlignCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
}