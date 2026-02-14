#include "auth_service.h"
#include "db/db_manager.h"
#include <QSqlError>
#include <QSqlQuery>

bool AuthService::login(const QString &username, const QString &password,
                        QString *role, QString *error_message) const {
  QSqlDatabase db = DBManager::instance().database();
  QSqlQuery query(db);
  query.prepare("SELECT role FROM users WHERE username = :u AND password = :p");
  query.bindValue(":u", username);
  query.bindValue(":p", password);

  if (!query.exec()) {
    if (error_message) {
      *error_message = query.lastError().text();
    }
    return false;
  }

  if (!query.next()) {
    return false;
  }

  if (role) {
    *role = query.value("role").toString();
  }
  return true;
}
