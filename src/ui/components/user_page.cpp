#include "user_page.h"
#include <QLabel>
#include <QVBoxLayout>

UserPage::UserPage(QWidget *parent) : QWidget(parent) { init_ui(); }

void UserPage::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  this->setObjectName("UserPage");

  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *label = new QLabel("user", this);
  label->setAlignment(Qt::AlignCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
}