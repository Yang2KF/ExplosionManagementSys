#include "db_manager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
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
    qDebug() << "MySQL connect failed:" << db.lastError().text();
    return false;
  }

  QSqlQuery charset_query(db);
  if (!charset_query.exec("SET NAMES utf8mb4")) {
    qDebug() << "SET NAMES utf8mb4 failed:" << charset_query.lastError().text();
  }

  if (!create_tables()) {
    qDebug() << "Create tables failed.";
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
    }
  }
  QSqlDatabase::removeDatabase(CONN_NAME);
}

bool DBManager::create_tables() {
  QSqlDatabase db = database();
  QSqlQuery query(db);

  const QStringList statements = {
      "CREATE TABLE IF NOT EXISTS alg_category ("
      "  CATEGORY_ID VARCHAR(64) NOT NULL,"
      "  CATEGORY_NAME VARCHAR(255) NOT NULL,"
      "  PARENT_ID VARCHAR(64) NOT NULL DEFAULT '0',"
      "  COMMENTS VARCHAR(255) DEFAULT NULL,"
      "  PRIMARY KEY (CATEGORY_ID),"
      "  KEY idx_alg_category_parent (PARENT_ID)"
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci",

      "CREATE TABLE IF NOT EXISTS algorithms ("
      "  ALGID VARCHAR(36) NOT NULL,"
      "  CATEGORY_ID VARCHAR(64) NOT NULL,"
      "  ALGNAME VARCHAR(255) NOT NULL,"
      "  ENTRY_FUNC VARCHAR(128) NOT NULL,"
      "  SRC VARCHAR(255) NOT NULL,"
      "  SRC_TYPE VARCHAR(16) NOT NULL DEFAULT 'dll',"
      "  COMMENTS TEXT DEFAULT NULL,"
      "  PRIMARY KEY (ALGID),"
      "  KEY idx_algorithms_category (CATEGORY_ID),"
      "  KEY idx_algorithms_name (ALGNAME),"
      "  CONSTRAINT fk_algorithms_category FOREIGN KEY (CATEGORY_ID)"
      "    REFERENCES alg_category (CATEGORY_ID)"
      "    ON UPDATE CASCADE ON DELETE RESTRICT"
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci",

      "CREATE TABLE IF NOT EXISTS alg_inparams ("
      "  UUID VARCHAR(36) NOT NULL,"
      "  ALGID VARCHAR(36) NOT NULL,"
      "  P_IDENTIFIER VARCHAR(64) NOT NULL,"
      "  P_ZHNAME VARCHAR(100) DEFAULT NULL,"
      "  UNIT VARCHAR(25) DEFAULT NULL,"
      "  DATATYPE VARCHAR(25) NOT NULL DEFAULT 'double',"
      "  DEFAULT_VALUE VARCHAR(200) DEFAULT NULL,"
      "  REQUIRED TINYINT(1) NOT NULL DEFAULT 0,"
      "  MIN_VALUE VARCHAR(50) DEFAULT NULL,"
      "  MAX_VALUE VARCHAR(50) DEFAULT NULL,"
      "  TOOLTIP VARCHAR(100) DEFAULT NULL,"
      "  COMMENTS VARCHAR(100) DEFAULT NULL,"
      "  SHOWORDER INT NOT NULL DEFAULT 0,"
      "  PRIMARY KEY (UUID),"
      "  KEY idx_alg_inparams_algid_order (ALGID, SHOWORDER, P_IDENTIFIER),"
      "  CONSTRAINT fk_alg_inparams_algorithm FOREIGN KEY (ALGID)"
      "    REFERENCES algorithms (ALGID)"
      "    ON UPDATE CASCADE ON DELETE CASCADE"
      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci"};

  for (const QString &sql : statements) {
    if (!query.exec(sql)) {
      qDebug() << "Schema SQL failed:" << query.lastError().text() << sql;
      return false;
    }
  }

  return true;
}

void DBManager::seed_data() {
  QSqlDatabase db = database();
  QSqlQuery query(db);

  if (!query.exec("SELECT COUNT(*) FROM alg_category")) {
    qDebug() << "Seed check failed:" << query.lastError().text();
    return;
  }
  if (query.next() && query.value(0).toInt() > 0) {
    return;
  }

  db.transaction();

  auto add_category = [&](const QString &id, const QString &name,
                          const QString &parent_id,
                          const QString &comments = QString()) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO alg_category "
              "(CATEGORY_ID, CATEGORY_NAME, PARENT_ID, COMMENTS) "
              "VALUES (:id, :name, :parent_id, :comments)");
    q.bindValue(":id", id);
    q.bindValue(":name", name);
    q.bindValue(":parent_id", parent_id);
    q.bindValue(":comments", comments);
    if (!q.exec()) {
      qDebug() << "Insert category failed:" << q.lastError().text() << id;
    }
  };

  auto add_algo = [&](const QString &category_id, const QString &name,
                      const QString &description, const QString &entry_func,
                      const QString &src_path,
                      const QString &src_type = "dll") -> QString {
    const QString alg_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlQuery q(db);
    q.prepare("INSERT INTO algorithms "
              "(ALGID, CATEGORY_ID, ALGNAME, ENTRY_FUNC, SRC, SRC_TYPE, COMMENTS) "
              "VALUES (:algid, :category_id, :name, :entry_func, :src, "
              ":src_type, :comments)");
    q.bindValue(":algid", alg_id);
    q.bindValue(":category_id", category_id);
    q.bindValue(":name", name);
    q.bindValue(":entry_func", entry_func);
    q.bindValue(":src", src_path);
    q.bindValue(":src_type", src_type);
    q.bindValue(":comments", description);
    if (!q.exec()) {
      qDebug() << "Insert algorithm failed:" << q.lastError().text() << name;
    }
    return alg_id;
  };

  auto add_param = [&](const QString &alg_id, const QString &identifier,
                       const QString &display_name, const QString &type,
                       const QString &default_value, bool required,
                       const QString &min_value, const QString &max_value,
                       int order, const QString &unit = QString(),
                       const QString &tooltip = QString()) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO alg_inparams "
              "(UUID, ALGID, P_IDENTIFIER, P_ZHNAME, UNIT, DATATYPE, "
              "DEFAULT_VALUE, REQUIRED, MIN_VALUE, MAX_VALUE, TOOLTIP, "
              "SHOWORDER) "
              "VALUES (:id, :algid, :identifier, :name, :unit, :type, "
              ":default_value, :required, :min_value, :max_value, :tooltip, "
              ":showorder)");
    q.bindValue(":id", QUuid::createUuid().toString(QUuid::WithoutBraces));
    q.bindValue(":algid", alg_id);
    q.bindValue(":identifier", identifier);
    q.bindValue(":name", display_name);
    q.bindValue(":unit", unit);
    q.bindValue(":type", type);
    q.bindValue(":default_value", default_value);
    q.bindValue(":required", required ? 1 : 0);
    q.bindValue(":min_value", min_value);
    q.bindValue(":max_value", max_value);
    q.bindValue(":tooltip", tooltip);
    q.bindValue(":showorder", order);
    if (!q.exec()) {
      qDebug() << "Insert param failed:" << q.lastError().text() << identifier;
    }
  };

  add_category("CAT_LOAD", "爆炸载荷模型", "0");
  add_category("CAT_AIR", "空气冲击波", "CAT_LOAD");
  add_category("CAT_GROUND", "地面冲击效应", "CAT_LOAD");
  add_category("CAT_FRAG", "破片毁伤模型", "0");
  add_category("CAT_VEL", "破片初速", "CAT_FRAG");
  add_category("CAT_DIST", "破片分布", "CAT_FRAG");
  add_category("CAT_HEAT", "热辐射效应", "0");
  add_category("CAT_FIREBALL", "火球模型", "CAT_HEAT");

  const QString blast_dll = "libs/blast_models.dll";
  const QString python_script = "examples/python_models/blast_models_py.py";

  const QString sadovsky_id =
      add_algo("CAT_AIR", "萨道夫斯基公式", "估算中远场超压", "calc_sadovsky",
               blast_dll);
  add_param(sadovsky_id, "charge_kg", "装药量", "double", "10.0", true, "0",
            "", 1, "kg");
  add_param(sadovsky_id, "distance_m", "距离", "double", "30.0", true, "0.1",
            "", 2, "m");

  const QString henrych_id =
      add_algo("CAT_AIR", "亨利希公式", "近场超压修正", "calc_henrych",
               blast_dll);
  add_param(henrych_id, "charge_kg", "装药量", "double", "10.0", true, "0",
            "", 1, "kg");
  add_param(henrych_id, "distance_m", "距离", "double", "15.0", true, "0.1",
            "", 2, "m");

  const QString lampson_id =
      add_algo("CAT_GROUND", "兰普森公式", "估算土体应力波传播", "calc_lampson",
               blast_dll);
  add_param(lampson_id, "charge_kg", "装药量", "double", "10.0", true, "0",
            "", 1, "kg");
  add_param(lampson_id, "buried_depth_m", "埋深", "double", "1.0", true, "0",
            "", 2, "m");

  const QString gurney_id =
      add_algo("CAT_VEL", "古尼公式", "估算装药壳体比下的破片初速", "calc_gurney",
               blast_dll);
  add_param(gurney_id, "charge_mass_kg", "炸药质量", "double", "2.5", true, "0",
            "", 1, "kg");
  add_param(gurney_id, "shell_mass_kg", "壳体质量", "double", "8.0", true,
            "0.1", "", 2, "kg");

  const QString mott_id =
      add_algo("CAT_DIST", "莫特分布", "估算破片数量与质量分布", "calc_mott",
               blast_dll);
  add_param(mott_id, "shell_mass_kg", "壳体质量", "double", "8.0", true, "0.1",
            "", 1, "kg");
  add_param(mott_id, "frag_coeff", "破片系数", "double", "0.8", true, "0",
            "", 2);

  const QString point_source_id =
      add_algo("CAT_FIREBALL", "点源模型", "估算目标点热通量",
               "calc_point_source", blast_dll);
  add_param(point_source_id, "yield_kg", "当量", "double", "10.0", true, "0",
            "", 1, "kg");
  add_param(point_source_id, "distance_m", "距离", "double", "50.0", true,
            "0.1", "", 2, "m");

  const QString scaled_distance_py_id =
      add_algo("CAT_AIR", "当量距离（Python）",
               "按 Hopkinson-Cranz 计算当量距离",
               "calc_scaled_distance_py", python_script, "python");
  add_param(scaled_distance_py_id, "charge_kg", "装药量", "double", "8.0",
            true, "0.001", "", 1, "kg");
  add_param(scaled_distance_py_id, "distance_m", "距离", "double", "25.0",
            true, "0.1", "", 2, "m");

  const QString fragment_energy_py_id =
      add_algo("CAT_VEL", "破片动能（Python）",
               "按质量和速度估算破片动能",
               "calc_fragment_energy_py", python_script, "python");
  add_param(fragment_energy_py_id, "fragment_mass_kg", "破片质量", "double",
            "0.02", true, "0.0001", "", 1, "kg");
  add_param(fragment_energy_py_id, "velocity_m_s", "速度", "double", "1200.0",
            true, "0.1", "", 2, "m/s");

  const QString thermal_dose_py_id =
      add_algo("CAT_FIREBALL", "热剂量（Python）",
               "按热流和持续时间估算热剂量",
               "calc_thermal_dose_py", python_script, "python");
  add_param(thermal_dose_py_id, "heat_flux_kw_m2", "热流密度", "double",
            "35.0", true, "0.001", "", 1, "kW/m2");
  add_param(thermal_dose_py_id, "duration_s", "持续时间", "double", "4.0",
            true, "0.001", "", 2, "s");

  const QString blast_risk_py_id =
      add_algo("CAT_AIR", "冲击波风险评估（Python）",
               "用于展示多参数风险评估的动态参数面板",
               "calc_blast_risk_py", python_script, "python");
  add_param(blast_risk_py_id, "charge_kg", "装药量", "double", "12.0", true,
            "0.001", "", 1, "kg", "爆源 TNT 当量");
  add_param(blast_risk_py_id, "distance_m", "距离", "double", "28.0", true,
            "0.1", "", 2, "m", "目标点到爆源距离");
  add_param(blast_risk_py_id, "terrain_factor", "地形修正系数", "double",
            "1.15", true, "0.5", "3.0", 3, "", "开阔场地取 1.0，复杂场景可适当上调");
  add_param(blast_risk_py_id, "personnel_count", "暴露人数", "int", "12", true,
            "1", "500", 4, "人", "参与评估的目标人数");
  add_param(blast_risk_py_id, "indoor_scene", "室内场景", "bool", "false", true,
            "", "", 5, "", "室内环境通常会增大风险评分");
  add_param(blast_risk_py_id, "scene_label", "场景名称", "string", "厂房 A 区",
            true, "", "", 6, "", "仅用于输出标识");

  const QString fragment_screening_py_id =
      add_algo("CAT_DIST", "破片遮蔽评估（Python）",
               "用于展示布尔、整数、字符串混合参数的运行页",
               "calc_fragment_screening_py", python_script, "python");
  add_param(fragment_screening_py_id, "fragment_mass_kg", "单片质量", "double",
            "0.03", true, "0.0001", "", 1, "kg", "单个破片质量");
  add_param(fragment_screening_py_id, "velocity_m_s", "飞行速度", "double",
            "950.0", true, "0.1", "", 2, "m/s", "典型破片飞行速度");
  add_param(fragment_screening_py_id, "fragment_count", "破片数量", "int", "180",
            true, "1", "100000", 3, "片", "参与评估的破片数量");
  add_param(fragment_screening_py_id, "target_area_m2", "目标面积", "double",
            "12.0", true, "0.01", "", 4, "m2", "受击目标投影面积");
  add_param(fragment_screening_py_id, "use_barrier", "启用遮蔽", "bool", "true",
            true, "", "", 5, "", "是否考虑遮挡结构削弱作用");
  add_param(fragment_screening_py_id, "barrier_material", "遮蔽材料", "string",
            "steel panel", true, "", "", 6, "", "如 steel panel / concrete wall");

  const QString thermal_response_py_id =
      add_algo("CAT_FIREBALL", "热响应评估（Python）",
               "用于展示 6 参数热辐射响应模型的动态表单",
               "calc_thermal_response_py", python_script, "python");
  add_param(thermal_response_py_id, "heat_flux_kw_m2", "热流密度", "double",
            "32.0", true, "0.001", "", 1, "kW/m2", "目标表面热流密度");
  add_param(thermal_response_py_id, "duration_s", "作用时长", "double", "6.5",
            true, "0.001", "", 2, "s", "热流持续时间");
  add_param(thermal_response_py_id, "protective_factor", "防护系数", "double",
            "0.85", true, "0.1", "2.0", 3, "", "防护越强，该值越小");
  add_param(thermal_response_py_id, "people_count", "人员数量", "int", "6", true,
            "1", "1000", 4, "人", "群组暴露人数");
  add_param(thermal_response_py_id, "humidity_ratio", "湿度系数", "double",
            "0.35", true, "0.0", "1.0", 5, "", "0 到 1 之间");
  add_param(thermal_response_py_id, "has_shelter", "有无掩体", "bool", "false",
            true, "", "", 6, "", "有掩体时有效热剂量会下降");

  if (!db.commit()) {
    qDebug() << "Seed commit failed:" << db.lastError().text();
    db.rollback();
  }
}
