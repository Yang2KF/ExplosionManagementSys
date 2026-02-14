#include "db_manager.h"
#include <QDateTime>
#include <QUuid>
#include <cstdlib>

DBManager::DBManager() {}

DBManager &DBManager::instance() {
  static DBManager instance;
  return instance;
}

QSqlDatabase DBManager::database() { return QSqlDatabase::database(CONN_NAME); }

bool DBManager::init() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  QDir dir(path);
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  QString dbPath = dir.filePath(DB_NAME);
  qDebug() << "DB Path:" << dbPath;

  QSqlDatabase db;
  if (QSqlDatabase::contains(CONN_NAME)) {
    db = database();
  } else {
    db = QSqlDatabase::addDatabase("QSQLITE", CONN_NAME);
    db.setDatabaseName(dbPath);
  }

  if (!db.open()) {
    qDebug() << "Error: Connection failed" << db.lastError();
    return false;
  }

  if (!create_tables()) {
    return false;
  }

  seed_data();
  return true;
}

void DBManager::close() {
  {
    QSqlDatabase db = database();
    if (db.isOpen()) {
      db.close();
      qDebug() << "Database closed.";
    }
  }

  QSqlDatabase::removeDatabase(CONN_NAME);
  qDebug() << "Database connection removed.";
}

bool DBManager::create_tables() {
  QSqlDatabase db = database();
  QSqlQuery query(db);

  bool success = query.exec(
      "CREATE TABLE IF NOT EXISTS alg_category ("
      "CATEGORY_NAME TEXT, "
      "CATEGORY_ID TEXT PRIMARY KEY, "
      "PARENT_ID TEXT DEFAULT '0', "
      "COMMENTS TEXT, "
      "ALG_URL TEXT)");
  if (!success) {
    qDebug() << "Create alg_category failed:" << query.lastError();
    return false;
  }

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS algorithms ("
      "ALGID TEXT PRIMARY KEY, "
      "ALGNAME TEXT, "
      "COMMENTS TEXT, "
      "HELPURL TEXT, "
      "CALLURL TEXT, "
      "CALLID TEXT, "
      "SRC TEXT, "
      "SRC_TYPE TEXT, "
      "ALGIDENTIFIER TEXT, "
      "CLSID TEXT, "
      "CREATED_AT DATETIME)");
  if (!success) {
    qDebug() << "Create algorithms failed:" << query.lastError();
    return false;
  }

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS alg_inparams ("
      "UUID TEXT, "
      "ALGID TEXT, "
      "P_IDENTIFIER TEXT, "
      "P_ZHNAME TEXT, "
      "UNIT TEXT, "
      "DATATYPE TEXT, "
      "INCONFIG TEXT DEFAULT '{\"datatype\":\"number\",\"ui\":\"text\"}', "
      "VALIDATOR TEXT, "
      "TOOLTIP TEXT, "
      "COMMENTS TEXT, "
      "SHOWORDER TEXT)");
  if (!success) {
    qDebug() << "Create alg_inparams failed:" << query.lastError();
    return false;
  }

  success = query.exec("CREATE TABLE IF NOT EXISTS users ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "username TEXT UNIQUE NOT NULL, "
                       "password TEXT NOT NULL, "
                       "role TEXT DEFAULT 'user')");
  if (!success) {
    qDebug() << "Create users failed:" << query.lastError();
    return false;
  }

  return true;
}

void DBManager::seed_data() {
  QSqlDatabase db = database();
  QSqlQuery query(db);

  query.exec("SELECT COUNT(*) FROM alg_category");
  if (query.next() && query.value(0).toInt() > 0) {
    return;
  }

  qDebug() << "Seeding demo data...";

  auto add_category = [&](const QString &id, const QString &name,
                          const QString &parent_id,
                          const QString &comments = QString()) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO alg_category "
              "(CATEGORY_ID, CATEGORY_NAME, PARENT_ID, COMMENTS, ALG_URL) "
              "VALUES (:id, :name, :pid, :comments, :url)");
    q.bindValue(":id", id);
    q.bindValue(":name", name);
    q.bindValue(":pid", parent_id);
    q.bindValue(":comments", comments);
    q.bindValue(":url", "");
    q.exec();
  };

  auto add_algo = [&](const QString &clsid, const QString &name,
                      const QString &desc, const QString &call_id) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO algorithms "
              "(ALGID, ALGNAME, COMMENTS, HELPURL, CALLURL, CALLID, SRC, "
              "SRC_TYPE, ALGIDENTIFIER, CLSID, CREATED_AT) "
              "VALUES (:algid, :name, :comments, :help_url, :call_url, "
              ":call_id, :src, :src_type, :identifier, :clsid, :created_at)");
    q.bindValue(":algid", QUuid::createUuid().toString(QUuid::WithoutBraces));
    q.bindValue(":name", name);
    q.bindValue(":comments", desc);
    q.bindValue(":help_url", "");
    q.bindValue(":call_url", "");
    q.bindValue(":call_id", call_id);
    q.bindValue(":src", "/libs/blast_models.dll");
    q.bindValue(":src_type", "1");
    q.bindValue(":identifier", call_id);
    q.bindValue(":clsid", clsid);
    q.bindValue(":created_at", QDateTime::currentDateTime().addDays(-rand() % 10));
    q.exec();
  };

  db.transaction();

  add_category("CAT_LOAD", "Blast Load Models", "0");
  add_category("CAT_AIR", "Air Shock Wave", "CAT_LOAD");
  add_category("CAT_GROUND", "Ground Shock Effect", "CAT_LOAD");
  add_category("CAT_FRAG", "Fragment Damage Models", "0");
  add_category("CAT_VEL", "Fragment Initial Velocity", "CAT_FRAG");
  add_category("CAT_DIST", "Fragment Distribution", "CAT_FRAG");
  add_category("CAT_HEAT", "Thermal Radiation Effect", "0");
  add_category("CAT_FIREBALL", "Fireball Models", "CAT_HEAT");

  add_algo("CAT_AIR", "Sadovsky Formula",
           "Estimate overpressure in medium and far field", "calc_sadovsky");
  add_algo("CAT_AIR", "Henrych Formula", "Near-field overpressure correction",
           "calc_henrych");
  add_algo("CAT_GROUND", "Lampson Formula",
           "Estimate stress wave propagation in soil", "calc_lampson");
  add_algo("CAT_VEL", "Gurney Formula",
           "Estimate fragment initial velocity by charge-mass ratio",
           "calc_gurney");
  add_algo("CAT_DIST", "Mott Distribution",
           "Statistical distribution of fragment count and size", "calc_mott");
  add_algo("CAT_FIREBALL", "Point Source Model",
           "Estimate target-point thermal flux", "calc_point_source");

  query.exec("SELECT COUNT(*) FROM users");
  if (query.next() && query.value(0).toInt() == 0) {
    query.prepare(
        "INSERT INTO users (username, password, role) VALUES (:u, :p, :r)");
    query.bindValue(":u", "admin");
    query.bindValue(":p", "123456");
    query.bindValue(":r", "admin");
    query.exec();
  }

  db.commit();
  qDebug() << "Seeding completed.";
}
