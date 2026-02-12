#include "db_manager.h"

DBManager::DBManager() {}

DBManager &DBManager::instance() {
  static DBManager instance;
  return instance;
}

QSqlDatabase DBManager::database() {
  // 通过连接名获取连接，如果未打开会自动打开
  return QSqlDatabase::database(CONN_NAME);
}

bool DBManager::init() {
  // 1. 确定路径
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  QDir dir(path);
  if (!dir.exists())
    dir.mkpath("."); // 确保目录存在

  QString dbPath = dir.filePath(DB_NAME);
  qDebug() << "DB Path:" << dbPath;

  // 2. 添加数据库连接（指定连接名）
  // 检查连接是否已存在，防止重复添加
  QSqlDatabase db;
  if (QSqlDatabase::contains(CONN_NAME)) {
    db = database();
  } else {
    db = QSqlDatabase::addDatabase("QSQLITE", CONN_NAME);
    db.setDatabaseName(dbPath);
  }

  // 3. 打开连接
  if (!db.open()) {
    qDebug() << "Error: Connection failed" << db.lastError();
    return false;
  }

  if (!create_tables())
    return false;

  seed_data(); // 装填数据

  return true;
}

void DBManager::close() {
  // 显式关闭逻辑
  {
    // 作用域块：确保 db 对象在 removeDatabase 前销毁
    QSqlDatabase db = database();
    if (db.isOpen()) {
      db.close();
      qDebug() << "Database closed.";
    }
  } // db 在这里析构，引用计数减1

  QSqlDatabase::removeDatabase(CONN_NAME);
  qDebug() << "Database connection removed.";
}

bool DBManager::create_tables() {
  QSqlDatabase db = database();
  QSqlQuery query(db);

  // ... (建表语句保持不变) ...
  // 1. Categories
  bool success = query.exec("CREATE TABLE IF NOT EXISTS categories ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "name TEXT NOT NULL, "
                            "parent_id INTEGER DEFAULT 0)");
  if (!success)
    qDebug() << "Create categories failed:" << query.lastError();

  // 2. Algorithms
  success = query.exec("CREATE TABLE IF NOT EXISTS algorithms ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "category_id INTEGER, "
                       "name TEXT NOT NULL, "
                       "description TEXT, "
                       "file_path TEXT, "
                       "func_name TEXT, "
                       "created_at DATETIME, "
                       "FOREIGN KEY(category_id) REFERENCES categories(id))");
  if (!success)
    qDebug() << "Create algorithms failed:" << query.lastError();

  // 3. Parameters
  success = query.exec(
      "CREATE TABLE IF NOT EXISTS parameters ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "algo_id INTEGER, "
      "name TEXT NOT NULL, "
      "param_type TEXT, "
      "default_value TEXT, "
      "FOREIGN KEY(algo_id) REFERENCES algorithms(id) ON DELETE CASCADE)");
  if (!success)
    qDebug() << "Create parameters failed:" << query.lastError();

  // 4.user
  success = query.exec("CREATE TABLE IF NOT EXISTS users ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "username TEXT UNIQUE NOT NULL, "
                       "password TEXT NOT NULL, "
                       "role TEXT DEFAULT 'user')");
  if (!success)
    qDebug() << "Create users failed:" << query.lastError();

  return true;
}

void DBManager::seed_data() {
  QSqlDatabase db = database();
  QSqlQuery query(db);

  // 1. 检查是否已经有数据，避免重复插入
  query.exec("SELECT COUNT(*) FROM categories");
  if (query.next() && query.value(0).toInt() > 0) {
    return; // 已经有数据了，不做任何事
  }

  qDebug() << "Seeding demo data...";

  // 工具 Lambda：插入分类并返回 ID
  auto add_category = [&](const QString &name, int parent_id) -> int {
    QSqlQuery q(db);
    q.prepare("INSERT INTO categories (name, parent_id) VALUES (:name, :pid)");
    q.bindValue(":name", name);
    q.bindValue(":pid", parent_id);
    if (q.exec())
      return q.lastInsertId().toInt();
    return -1;
  };

  // 工具 Lambda：插入算法
  auto add_algo = [&](int cid, const QString &name, const QString &desc,
                      const QString &func) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO algorithms (category_id, name, description, "
              "file_path, func_name, created_at) "
              "VALUES (:cid, :name, :desc, :path, :func, :time)");
    q.bindValue(":cid", cid);
    q.bindValue(":name", name);
    q.bindValue(":desc", desc);
    q.bindValue(":path", "/libs/blast_models.dll"); // 模拟路径
    q.bindValue(":func", func);
    q.bindValue(":time", QDateTime::currentDateTime().addDays(
                             -rand() % 10)); // 随机一点过去的时间
    q.exec();
  };

  db.transaction(); // 开启事务，加速插入

  // --- 插入数据 ---

  // 1. 根分类：爆炸载荷
  int cat_load = add_category("爆炸载荷模型", 0);
  // 子分类：空气冲击波
  int cat_air = add_category("空气冲击波", cat_load);
  add_algo(cat_air, "萨多夫斯基公式 (Sadovsky)",
           "经典的空气冲击波超压计算公式，适用于中远场爆炸评估。",
           "calc_sadovsky");
  add_algo(cat_air, "亨利奇公式 (Henrych)",
           "修正后的超压计算模型，涵盖了近场爆炸效应。", "calc_henrych");
  add_algo(cat_air, "布罗德公式 (Brode)",
           "高精度数值拟合公式，适用于高达100MPa的超压计算。", "calc_brode");

  // 子分类：地冲击
  int cat_ground = add_category("地冲击效应", cat_load);
  add_algo(cat_ground, "兰普森公式 (Lampson)",
           "计算地下爆炸引起的土壤应力波传播。", "calc_lampson");

  // 2. 根分类：破片毁伤
  int cat_frag = add_category("破片毁伤模型", 0);
  // 子分类：初速
  int cat_vel = add_category("破片初速", cat_frag);
  add_algo(cat_vel, "格尼公式 (Gurney)",
           "根据炸药与金属壳体质量比计算破片初速的经典公式。", "calc_gurney");
  // 子分类：分布
  int cat_dist = add_category("破片分布", cat_frag);
  add_algo(cat_dist, "莫特分布定律 (Mott)",
           "预测战斗部破碎后破片尺寸与数量的统计分布模型。", "calc_mott");
  add_algo(cat_dist, "赫尔德公式 (Held)", "基于毁伤概率的破片密度分布计算。",
           "calc_held");

  // 3. 根分类：热辐射
  int cat_heat = add_category("热辐射效应", 0);
  int cat_fireball = add_category("火球模型", cat_heat);
  add_algo(cat_fireball, "点源模型 (Point Source)",
           "假设火球为点热源，计算距离 R 处的热通量。", "calc_point_source");
  add_algo(cat_fireball, "固体火焰模型 (Solid Flame)",
           "考虑火球形状与视线因子的复杂热辐射计算。", "calc_solid_flame");

  // 4. admin user
  query.exec("SELECT COUNT(*) FROM users");
  if (query.next() && query.value(0).toInt() == 0) {
    qDebug() << "Seeding admin user...";
    query.prepare(
        "INSERT INTO users (username, password, role) VALUES (:u, :p, :r)");
    query.bindValue(":u", "admin");
    query.bindValue(":p", "123456"); // 默认密码
    query.bindValue(":r", "admin");
    query.exec();
  }

  db.commit(); // 提交事务
  qDebug() << "Seeding completed.";
}