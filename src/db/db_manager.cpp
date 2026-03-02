#include "db_manager.h"
#include <QDateTime>
#include <QUuid>

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
    qDebug() << "错误：MySQL 连接失败" << db.lastError().text();
    return false;
  }

  QSqlQuery charset_query(db);
  if (!charset_query.exec("SET NAMES utf8mb4")) {
    qDebug() << "警告：设置数据库字符集为 utf8mb4 失败："
             << charset_query.lastError().text();
  }

  qDebug() << "成功：已连接到 MySQL。";

  if (!create_tables()) {
    qDebug() << "创建或升级数据表失败。";
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
      qDebug() << "数据库已关闭。";
    }
  }

  QSqlDatabase::removeDatabase(CONN_NAME);
  qDebug() << "数据库连接已移除。";
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
    qDebug() << "创建 alg_category 表失败：" << query.lastError().text();
    return false;
  }

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS algorithms ("
      "ID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '前端可读编号', "
      "ALGID VARCHAR(36) NOT NULL PRIMARY KEY COMMENT '算法ID', "
      "ALGNAME VARCHAR(255) DEFAULT NULL COMMENT '算法名称', "
      "COMMENTS TEXT COMMENT '备注说明', "
      "HELPURL TEXT COMMENT '帮助链接', "
      "CALLURL TEXT COMMENT '调用链接', "
      "CALLID VARCHAR(200) DEFAULT NULL COMMENT '调用函数的导出名', "
      "SRC VARCHAR(255) DEFAULT NULL COMMENT '算法文件路径（dll/py）', "
      "SRC_TYPE VARCHAR(3) DEFAULT NULL COMMENT '1:dll, 2:python', "
      "ALGIDENTIFIER VARCHAR(30) DEFAULT NULL COMMENT '算法标识符', "
      "CLSID VARCHAR(36) DEFAULT NULL COMMENT '类别ID(关联alg_category)', "
      "CREATED_AT DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间', "
      "UNIQUE KEY uk_algorithms_readable_id (ID) "
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;");
  if (!success) {
    qDebug() << "创建 algorithms 表失败：" << query.lastError().text();
    return false;
  }

  success = query.exec(
      "CREATE TABLE IF NOT EXISTS alg_inparams ("
      "ID BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '前端可读编号', "
      "UUID VARCHAR(36) NOT NULL PRIMARY KEY COMMENT '参数唯一ID', "
      "ALGID VARCHAR(36) NOT NULL COMMENT '所属算法ID', "
      "P_IDENTIFIER VARCHAR(50) DEFAULT NULL COMMENT '参数标识符', "
      "P_ZHNAME VARCHAR(100) DEFAULT NULL COMMENT '参数中文名', "
      "UNIT VARCHAR(25) DEFAULT NULL COMMENT '参数单位', "
      "DATATYPE VARCHAR(25) DEFAULT NULL COMMENT '数据类型', "
      "DEFAULT_VALUE VARCHAR(200) DEFAULT NULL COMMENT '默认值', "
      "REQUIRED TINYINT(1) DEFAULT 0 COMMENT '是否必填', "
      "MIN_VALUE VARCHAR(50) DEFAULT NULL COMMENT '最小值', "
      "MAX_VALUE VARCHAR(50) DEFAULT NULL COMMENT '最大值', "
      "INCONFIG VARCHAR(200) DEFAULT "
      "'{\"datatype\":\"number\",\"ui\":\"text\"}' COMMENT '前端UI配置', "
      "VALIDATOR VARCHAR(100) DEFAULT NULL COMMENT '验证规则', "
      "TOOLTIP VARCHAR(100) DEFAULT NULL COMMENT '提示信息', "
      "COMMENTS VARCHAR(100) DEFAULT NULL COMMENT '备注', "
      "SHOWORDER INT DEFAULT 0 COMMENT '显示顺序', "
      "UNIQUE KEY uk_alg_inparams_readable_id (ID) "
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;");
  if (!success) {
    qDebug() << "创建 alg_inparams 表失败：" << query.lastError().text();
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
    qDebug() << "创建 users 表失败：" << query.lastError().text();
    return false;
  }

  if (!ensure_column_exists(
          db, "alg_inparams", "DEFAULT_VALUE",
          "DEFAULT_VALUE VARCHAR(200) DEFAULT NULL COMMENT '默认值'")) {
    return false;
  }
  if (!ensure_column_exists(
          db, "alg_inparams", "REQUIRED",
          "REQUIRED TINYINT(1) DEFAULT 0 COMMENT '是否必填'")) {
    return false;
  }
  if (!ensure_column_exists(
          db, "alg_inparams", "MIN_VALUE",
          "MIN_VALUE VARCHAR(50) DEFAULT NULL COMMENT '最小值'")) {
    return false;
  }
  if (!ensure_column_exists(
          db, "alg_inparams", "MAX_VALUE",
          "MAX_VALUE VARCHAR(50) DEFAULT NULL COMMENT '最大值'")) {
    return false;
  }
  if (!ensure_column_exists(
          db, "algorithms", "SRC_TYPE",
          "SRC_TYPE VARCHAR(3) DEFAULT '1' COMMENT '1:dll, 2:python'")) {
    return false;
  }

  // 兼容历史数据：把 "/libs/xxx.dll" 规范成 "libs/xxx.dll"
  if (!query.exec("UPDATE algorithms SET SRC = TRIM(LEADING '/' FROM SRC) "
                  "WHERE SRC LIKE '/libs/%'")) {
    qDebug() << "规范化 SRC 路径失败：" << query.lastError().text();
  }

  if (!query.exec("UPDATE alg_category SET CATEGORY_NAME = CASE CATEGORY_ID "
                  "WHEN 'CAT_LOAD' THEN '爆炸载荷模型' "
                  "WHEN 'CAT_AIR' THEN '空气冲击波' "
                  "WHEN 'CAT_GROUND' THEN '地面冲击效应' "
                  "WHEN 'CAT_FRAG' THEN '破片毁伤模型' "
                  "WHEN 'CAT_VEL' THEN '破片初速' "
                  "WHEN 'CAT_DIST' THEN '破片分布' "
                  "WHEN 'CAT_HEAT' THEN '热辐射效应' "
                  "WHEN 'CAT_FIREBALL' THEN '火球模型' "
                  "ELSE CATEGORY_NAME END "
                  "WHERE CATEGORY_ID IN "
                  "('CAT_LOAD','CAT_AIR','CAT_GROUND','CAT_FRAG',"
                  "'CAT_VEL','CAT_DIST','CAT_HEAT','CAT_FIREBALL')")) {
    qDebug() << "更新算法分类中文名称失败：" << query.lastError().text();
  }

  if (!query.exec(
          "UPDATE algorithms SET "
          "ALGNAME = CASE CALLID "
          "WHEN 'calc_sadovsky' THEN '萨道夫斯基公式' "
          "WHEN 'calc_henrych' THEN '亨利希公式' "
          "WHEN 'calc_lampson' THEN '兰普森公式' "
          "WHEN 'calc_gurney' THEN '古尼公式' "
          "WHEN 'calc_mott' THEN '莫特分布' "
          "WHEN 'calc_point_source' THEN '点源模型' "
          "ELSE ALGNAME END, "
          "COMMENTS = CASE CALLID "
          "WHEN 'calc_sadovsky' THEN '估算中远场超压' "
          "WHEN 'calc_henrych' THEN '近场超压修正' "
          "WHEN 'calc_lampson' THEN '估算土体应力波传播' "
          "WHEN 'calc_gurney' THEN '估算装药-壳体比下的破片初速' "
          "WHEN 'calc_mott' THEN '估算破片数量与质量分布' "
          "WHEN 'calc_point_source' THEN '估算目标点热通量' "
          "ELSE COMMENTS END "
          "WHERE CALLID IN ('calc_sadovsky','calc_henrych','calc_lampson',"
          "'calc_gurney','calc_mott','calc_point_source')")) {
    qDebug() << "更新算法中文名称失败：" << query.lastError().text();
  }

  if (!query.exec(
          "UPDATE alg_inparams p "
          "JOIN algorithms a ON p.ALGID = a.ALGID "
          "SET p.P_ZHNAME = CASE CONCAT(a.CALLID, ':', p.P_IDENTIFIER) "
          "WHEN 'calc_sadovsky:charge_kg' THEN '装药量' "
          "WHEN 'calc_sadovsky:distance_m' THEN '距离' "
          "WHEN 'calc_henrych:charge_kg' THEN '装药量' "
          "WHEN 'calc_henrych:distance_m' THEN '距离' "
          "WHEN 'calc_lampson:charge_kg' THEN '装药量' "
          "WHEN 'calc_lampson:buried_depth_m' THEN '埋深' "
          "WHEN 'calc_gurney:charge_mass_kg' THEN '炸药质量' "
          "WHEN 'calc_gurney:shell_mass_kg' THEN '壳体质量' "
          "WHEN 'calc_mott:shell_mass_kg' THEN '壳体质量' "
          "WHEN 'calc_mott:frag_coeff' THEN '破片系数' "
          "WHEN 'calc_point_source:yield_kg' THEN '当量' "
          "WHEN 'calc_point_source:distance_m' THEN '距离' "
          "ELSE p.P_ZHNAME END "
          "WHERE a.CALLID IN ('calc_sadovsky','calc_henrych','calc_lampson',"
          "'calc_gurney','calc_mott','calc_point_source')")) {
    qDebug() << "更新算法参数中文名称失败：" << query.lastError().text();
  }

  return true;
}

bool DBManager::ensure_column_exists(QSqlDatabase &db,
                                     const QString &table_name,
                                     const QString &column_name,
                                     const QString &column_definition) {
  QSqlQuery query(db);
  query.prepare("SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS "
                "WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = :table_name "
                "AND COLUMN_NAME = :column_name");
  query.bindValue(":table_name", table_name);
  query.bindValue(":column_name", column_name);
  if (!query.exec()) {
    qDebug() << "检查列是否存在失败：" << table_name << column_name
             << query.lastError().text();
    return false;
  }

  if (!query.next() || query.value(0).toInt() > 0) {
    return true;
  }

  QSqlQuery alter(db);
  const QString sql = QString("ALTER TABLE `%1` ADD COLUMN %2")
                          .arg(table_name, column_definition);
  if (!alter.exec(sql)) {
    qDebug() << "新增列失败：" << sql << alter.lastError().text();
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

  qDebug() << "写入演示数据...";

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
                      const QString &desc, const QString &call_id,
                      const QString &src_path = "libs/blast_models.dll",
                      const QString &src_type = "1") {
    const QString alg_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery q(db);
    q.prepare("INSERT INTO algorithms "
              "(ALGID, ALGNAME, COMMENTS, HELPURL, CALLURL, CALLID, SRC, "
              "SRC_TYPE, ALGIDENTIFIER, CLSID, CREATED_AT) "
              "VALUES (:algid, :name, :comments, :help_url, :call_url, "
              ":call_id, :src, :src_type, :identifier, :clsid, :created_at)");
    q.bindValue(":algid", alg_id);
    q.bindValue(":name", name);
    q.bindValue(":comments", desc);
    q.bindValue(":help_url", "");
    q.bindValue(":call_url", "");
    q.bindValue(":call_id", call_id);
    q.bindValue(":src", src_path);
    q.bindValue(":src_type", src_type);
    q.bindValue(":identifier", call_id);
    q.bindValue(":clsid", clsid);
    q.bindValue(":created_at",
                QDateTime::currentDateTime().addDays(-rand() % 10));
    q.exec();
    return alg_id;
  };

  auto add_param = [&](const QString &alg_id, const QString &identifier,
                       const QString &zh_name, const QString &datatype,
                       const QString &default_value, bool required,
                       const QString &min_value, const QString &max_value,
                       int show_order, const QString &unit = QString()) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO alg_inparams "
              "(UUID, ALGID, P_IDENTIFIER, P_ZHNAME, UNIT, DATATYPE, "
              "DEFAULT_VALUE, "
              "REQUIRED, MIN_VALUE, MAX_VALUE, INCONFIG, SHOWORDER) "
              "VALUES (:uuid, :algid, :identifier, :name, :unit, :datatype, "
              ":default_value, :required, :min_value, :max_value, :inconfig, "
              ":showorder)");
    q.bindValue(":uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));
    q.bindValue(":algid", alg_id);
    q.bindValue(":identifier", identifier);
    q.bindValue(":name", zh_name);
    q.bindValue(":unit", unit);
    q.bindValue(":datatype", datatype);
    q.bindValue(":default_value", default_value);
    q.bindValue(":required", required ? 1 : 0);
    q.bindValue(":min_value", min_value);
    q.bindValue(":max_value", max_value);
    q.bindValue(":inconfig", "{\"datatype\":\"number\",\"ui\":\"text\"}");
    q.bindValue(":showorder", show_order);
    q.exec();
  };

  db.transaction();

  add_category("CAT_LOAD", "爆炸载荷模型", "0");
  add_category("CAT_AIR", "空气冲击波", "CAT_LOAD");
  add_category("CAT_GROUND", "地面冲击效应", "CAT_LOAD");
  add_category("CAT_FRAG", "破片毁伤模型", "0");
  add_category("CAT_VEL", "破片初速", "CAT_FRAG");
  add_category("CAT_DIST", "破片分布", "CAT_FRAG");
  add_category("CAT_HEAT", "热辐射效应", "0");
  add_category("CAT_FIREBALL", "火球模型", "CAT_HEAT");

  const QString sadovsky_id =
      add_algo("CAT_AIR", "萨道夫斯基公式", "估算中远场超压", "calc_sadovsky");
  add_param(sadovsky_id, "charge_kg", "装药量", "double", "10.0", true, "0", "",
            1, "kg");
  add_param(sadovsky_id, "distance_m", "距离", "double", "30.0", true, "0.1",
            "", 2, "m");

  const QString henrych_id =
      add_algo("CAT_AIR", "亨利希公式", "近场超压修正", "calc_henrych");
  add_param(henrych_id, "charge_kg", "装药量", "double", "10.0", true, "0", "",
            1, "kg");
  add_param(henrych_id, "distance_m", "距离", "double", "15.0", true, "0.1", "",
            2, "m");

  const QString lampson_id = add_algo("CAT_GROUND", "兰普森公式",
                                      "估算土体应力波传播", "calc_lampson");
  add_param(lampson_id, "charge_kg", "装药量", "double", "10.0", true, "0", "",
            1, "kg");
  add_param(lampson_id, "buried_depth_m", "埋深", "double", "1.0", true, "0",
            "", 2, "m");

  const QString gurney_id = add_algo(
      "CAT_VEL", "古尼公式", "估算装药-壳体比下的破片初速", "calc_gurney");
  add_param(gurney_id, "charge_mass_kg", "炸药质量", "double", "2.5", true, "0",
            "", 1, "kg");
  add_param(gurney_id, "shell_mass_kg", "壳体质量", "double", "8.0", true,
            "0.1", "", 2, "kg");

  const QString mott_id =
      add_algo("CAT_DIST", "莫特分布", "估算破片数量与质量分布", "calc_mott");
  add_param(mott_id, "shell_mass_kg", "壳体质量", "double", "8.0", true, "0.1",
            "", 1, "kg");
  add_param(mott_id, "frag_coeff", "破片系数", "double", "0.8", true, "0", "",
            2, "");

  const QString point_source_id = add_algo(
      "CAT_FIREBALL", "点源模型", "估算目标点热通量", "calc_point_source");
  add_param(point_source_id, "yield_kg", "当量", "double", "10.0", true, "0",
            "", 1, "kg");
  add_param(point_source_id, "distance_m", "距离", "double", "50.0", true,
            "0.1", "", 2, "m");

  const QString py_script = "examples/python_models/blast_models_py.py";

  const QString scaled_distance_py_id =
      add_algo("CAT_AIR", "当量距离（Python）", "按Hopkinson-Cranz计算当量距离",
               "calc_scaled_distance_py", py_script, "2");
  add_param(scaled_distance_py_id, "charge_kg", "装药量", "double", "8.0", true,
            "0.001", "", 1, "kg");
  add_param(scaled_distance_py_id, "distance_m", "距离", "double", "25.0", true,
            "0.1", "", 2, "m");

  const QString fragment_energy_py_id =
      add_algo("CAT_VEL", "破片动能（Python）", "按质量和速度估算破片动能",
               "calc_fragment_energy_py", py_script, "2");
  add_param(fragment_energy_py_id, "fragment_mass_kg", "破片质量", "double",
            "0.02", true, "0.0001", "", 1, "kg");
  add_param(fragment_energy_py_id, "velocity_m_s", "速度", "double", "1200.0",
            true, "0.1", "", 2, "m/s");

  const QString thermal_dose_py_id =
      add_algo("CAT_FIREBALL", "热剂量（Python）", "按热流和持续时间估算热剂量",
               "calc_thermal_dose_py", py_script, "2");
  add_param(thermal_dose_py_id, "heat_flux_kw_m2", "热流密度", "double", "35.0",
            true, "0.001", "", 1, "kW/m2");
  add_param(thermal_dose_py_id, "duration_s", "持续时间", "double", "4.0", true,
            "0.001", "", 2, "s");

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
  qDebug() << "演示数据写入完成。";
}
