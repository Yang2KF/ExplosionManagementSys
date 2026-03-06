#pragma once

#include <QAbstractButton>
#include <QButtonGroup>
#include <QEnterEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QList>
#include <QWidget>

class MainTabButton : public QAbstractButton {
  Q_OBJECT

public:
  explicit MainTabButton(const QString &title, const QIcon &normal_icon,
                         const QIcon &active_icon = QIcon(),
                         QWidget *parent = nullptr);

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;

private:
  QString title_;
  QIcon normal_icon_;
  QIcon active_icon_;
  bool hovered_ = false;
};

class MainTabBar : public QWidget {
  Q_OBJECT

public:
  explicit MainTabBar(QWidget *parent = nullptr);

  int addTab(const QString &title, const QIcon &normal_icon,
             const QIcon &active_icon = QIcon());
  int currentIndex() const { return current_index_; }
  int count() const { return buttons_.size(); }
  void setCurrentIndex(int index);

signals:
  void tabRequested(int index);

private:
  QHBoxLayout *layout_ = nullptr;
  QButtonGroup *button_group_ = nullptr;
  QList<MainTabButton *> buttons_;
  int current_index_ = -1;
};
