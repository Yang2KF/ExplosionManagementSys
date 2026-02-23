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
  QSqlDatabase db;
  if (QSqlDatabase::contains(CONN_NAME)) {
    db = database();
  } else {
    db = QSqlDatabase::addDatabase("QMYSQL", CONN_NAME);

    db.setHostName("127.0.0.1");

    db.setPort(3306);

    db.setDatabaseName("explosion");
    db.setUserName("root");
    db.setPassword("200463");
  }

  if (!db.open()) {
    qDebug() << "Error: MySQL Connection failed" << db.lastError().text();
    return false;
  }

  qDebug() << "Success: Connected to MySQL!";

  create_tables();
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
  bool success;

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS alg_category ("
      "CATEGORY_ID VARCHAR(36) NOT NULL PRIMARY KEY COMMENT '类别ID', "
      "CATEGORY_NAME VARCHAR(255) DEFAULT NULL COMMENT '类别名称', "
      "PARENT_ID VARCHAR(36) DEFAULT '0' COMMENT '父级ID', "
      "COMMENTS VARCHAR(255) DEFAULT NULL COMMENT '备注', "
      "ALG_URL VARCHAR(255) DEFAULT NULL COMMENT '浏览或调用算法的URL' "
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;");
  if (!success) {
    qDebug() << "Create alg_category failed:" << query.lastError().text();
    return false;
  }

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS algorithms ("
      "ALGID VARCHAR(36) NOT NULL PRIMARY KEY COMMENT '算法ID', "
      "ALGNAME VARCHAR(255) DEFAULT NULL COMMENT '算法名称', "
      "COMMENTS TEXT COMMENT '备注说明', "
      "HELPURL TEXT COMMENT '帮助链接', "
      "CALLURL TEXT COMMENT '调用链接', "
      "CALLID VARCHAR(200) DEFAULT NULL COMMENT '调用函数的导出名', "
      "SRC VARCHAR(255) DEFAULT NULL COMMENT '算法文件(dll/jar)路径', "
      "SRC_TYPE VARCHAR(3) DEFAULT NULL COMMENT '1:dll, 2:jar/py', "
      "ALGIDENTIFIER VARCHAR(30) DEFAULT NULL COMMENT '算法标识符', "
      "CLSID VARCHAR(36) DEFAULT NULL COMMENT '类别ID(关联alg_category)', "
      "CREATED_AT DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间' "
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;");
  if (!success) {
    qDebug() << "Create algorithms failed:" << query.lastError().text();
    return false;
  }

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS alg_inparams ("
      "UUID VARCHAR(36) NOT NULL PRIMARY KEY COMMENT '参数唯一ID', "
      "ALGID VARCHAR(36) NOT NULL COMMENT '所属算法ID', "
      "P_IDENTIFIER VARCHAR(50) DEFAULT NULL COMMENT '参数标识符', "
      "P_ZHNAME VARCHAR(100) DEFAULT NULL COMMENT '参数中文名', "
      "UNIT VARCHAR(25) DEFAULT NULL COMMENT '参数单位', "
      "DATATYPE VARCHAR(25) DEFAULT NULL COMMENT '数据类型', "
      "INCONFIG VARCHAR(200) DEFAULT "
      "'{\"datatype\":\"number\",\"ui\":\"text\"}' COMMENT '前端UI配置', "
      "VALIDATOR VARCHAR(100) DEFAULT NULL COMMENT '验证规则', "
      "TOOLTIP VARCHAR(100) DEFAULT NULL COMMENT '提示信息', "
      "COMMENTS VARCHAR(100) DEFAULT NULL COMMENT '备注', "
      "SHOWORDER INT DEFAULT 0 COMMENT '显示顺序' "
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;");
  if (!success) {
    qDebug() << "Create alg_inparams failed:" << query.lastError().text();
    return false;
  }

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS users ("
      "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, "
      "username VARCHAR(50) NOT NULL UNIQUE, "
      "password VARCHAR(255) NOT NULL, "
      "role VARCHAR(20) DEFAULT 'user' "
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;");
  if (!success) {
    qDebug() << "Create users failed:" << query.lastError().text();
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
    q.bindValue(":created_at",
                QDateTime::currentDateTime().addDays(-rand() % 10));
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
