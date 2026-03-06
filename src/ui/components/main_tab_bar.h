#pragma once

#include <QAbstractButton>
#include <QButtonGroup>
#include <QEnterEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QList>
#include <QMouseEvent>
#include <QWidget>

class MainTabButton : public QAbstractButton {
  Q_OBJECT

public:
  explicit MainTabButton(const QString &title, const QIcon &normal_icon,
                         const QIcon &active_icon = QIcon(),
                         bool closable = false,
                         QWidget *parent = nullptr);

  QSize sizeHint() const override;
  bool isClosable() const { return closable_; }

protected:
  void paintEvent(QPaintEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

signals:
  void closeRequested();

private:
  QRect close_rect() const;
  QString elided_title(int width) const;

  QString title_;
  QIcon normal_icon_;
  QIcon active_icon_;
  bool hovered_ = false;
  bool closable_ = false;
  bool close_hovered_ = false;
};

class MainTabBar : public QWidget {
  Q_OBJECT

public:
  explicit MainTabBar(QWidget *parent = nullptr);

  int addTab(const QString &title, const QIcon &normal_icon,
             const QIcon &active_icon = QIcon(), bool closable = false);
  void removeTab(int index);
  int currentIndex() const { return current_index_; }
  int count() const { return buttons_.size(); }
  void setCurrentIndex(int index);

signals:
  void tabRequested(int index);
  void tabCloseRequested(int index);

private:
  QHBoxLayout *layout_ = nullptr;
  QButtonGroup *button_group_ = nullptr;
  QList<MainTabButton *> buttons_;
  int current_index_ = -1;
};
