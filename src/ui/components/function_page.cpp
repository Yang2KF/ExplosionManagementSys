#include "function_page.h"
#include <QLabel>
#include <QVBoxLayout>

FunctionPage::FunctionPage(QWidget *parent) : QWidget(parent) { init_ui(); }

void FunctionPage::init_ui() {
  this->setAttribute(Qt::WA_StyledBackground, true);
  this->setObjectName("FunctionPage");

  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *label = new QLabel("算法配置", this);
  label->setAlignment(Qt::AlignCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
}