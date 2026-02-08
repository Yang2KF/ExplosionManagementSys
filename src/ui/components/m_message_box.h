#pragma once

#include <QDialog>
#include <QLabel>

class MaterialMessageBox : public QDialog {
  Q_OBJECT
public:
  enum Type { Information, Warning, Question, Success, Error };

  explicit MaterialMessageBox(QWidget *parent = nullptr);

  // 静态便捷函数 (模仿 QMessageBox)
  static void information(QWidget *parent, const QString &title,
                          const QString &msg);
  static void warning(QWidget *parent, const QString &title,
                      const QString &msg);
  static void error(QWidget *parent, const QString &title, const QString &msg);
  static int question(QWidget *parent, const QString &title,
                      const QString &msg); // 返回 QDialog::Accepted / Rejected

protected:
  void done(int r) override;
  void showEvent(QShowEvent *event) override;

private:
  void setup_ui(Type type, const QString &title, const QString &msg);
};