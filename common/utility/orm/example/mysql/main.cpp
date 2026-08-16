/**
 * @file    main.cpp
 * @brief   ORM 模块 MySQL 示例
 *
 * 演示内容：
 *   1. 通过命令行参数 / 环境变量配置 MySQL 连接
 *   2. 建表 CREATE TABLE（仅开发测试阶段，生产请用迁移工具）
 *   3. 带关联的 CRUD（Category ←→ Product）
 *   4. QxSqlQuery 高级条件查询
 *   5. 事务与批量操作
 *   6. 原生 SQL 执行
 *
 * 运行前请确保：
 *   - MySQL 服务已启动
 *   - 数据库 ess_test 已创建：  CREATE DATABASE ess_test CHARACTER SET utf8mb4;
 *   - QxOrm 依赖的 MySQL 驱动 DLL 已拷贝到可执行文件目录
 *     （位于 common/3rdparty/QxOrm/dep/mysql/）
 *
 * 使用命令行参数覆盖默认连接参数：
 *   orm_mysql_example --host 192.168.1.100 --port 3306
 *                     --db ess_test --user root --pass 123456
 */

#include <Orm.h>
#include "models/Category.h"
#include "models/Product.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

// ─────────────────────────────────────────────────────────────────────────────
// 工具宏
// ─────────────────────────────────────────────────────────────────────────────
#define SECTION(title) \
    qDebug() << "\n══════════════════════════════════════════"; \
    qDebug() << " " << title;                                   \
    qDebug() << "══════════════════════════════════════════"

static bool checkErr(const QSqlError &err, const char *op)
{
    if (err.isValid()) {
        qCritical() << "[FAIL]" << op << "=>" << err.text();
        return false;
    }
    qDebug() << "[OK  ]" << op;
    return true;
}

static bool execDdl(QSqlDatabase &db, const QString &sql, const char *op)
{
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qCritical() << "[FAIL]" << op << "=>" << query.lastError().text();
        qCritical() << "SQL:" << sql;
        return false;
    }
    qDebug() << "[OK  ]" << op;
    return true;
}

static bool ensureMySqlSchema()
{
    QSqlDatabase db = orm::DbManager::instance().database();
    if (!db.isValid() || !db.isOpen()) {
        qCritical() << "[FAIL] ensureMySqlSchema => invalid database connection";
        return false;
    }

    // Keep names/columns aligned with register.cpp for Category/Product mapping.
    const QString ddlCategory =
        "CREATE TABLE IF NOT EXISTS `category` ("
        "  `id` BIGINT NOT NULL AUTO_INCREMENT,"
        "  `name` VARCHAR(255) NOT NULL,"
        "  `description` VARCHAR(1024) NULL,"
        "  `sort_order` INT NOT NULL DEFAULT 0,"
        "  PRIMARY KEY (`id`)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

    const QString ddlProduct =
        "CREATE TABLE IF NOT EXISTS `product` ("
        "  `id` BIGINT NOT NULL AUTO_INCREMENT,"
        "  `name` VARCHAR(255) NOT NULL,"
        "  `sku` VARCHAR(128) NOT NULL,"
        "  `price` DOUBLE NOT NULL DEFAULT 0,"
        "  `stock` INT NOT NULL DEFAULT 0,"
        "  `enabled` TINYINT(1) NOT NULL DEFAULT 1,"
        "  `category_id` BIGINT NULL,"
        "  PRIMARY KEY (`id`),"
        "  UNIQUE KEY `uk_product_sku` (`sku`),"
        "  KEY `idx_product_category_id` (`category_id`),"
        "  CONSTRAINT `fk_product_category` FOREIGN KEY (`category_id`) "
        "    REFERENCES `category`(`id`) ON DELETE SET NULL ON UPDATE CASCADE"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

    if (!execDdl(db, ddlCategory, "create table category")) { return false; }
    if (!execDdl(db, ddlProduct, "create table product")) { return false; }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 函数
// ─────────────────────────────────────────────────────────────────────────────

static void demo_crud()
{
    SECTION("1. 基础 CRUD（Category / Product）");

    orm::Repository<Category> catRepo;
    orm::Repository<Product>  prodRepo;

    // 清空旧数据（幂等运行）
    prodRepo.removeAll();
    catRepo.removeAll();

    // ── 插入分类 ─────────────────────────────────────────────────────────────
    Category electronics("电子产品", "各类电子消费品", 1);
    Category furniture ("家具",     "桌椅沙发等",     2);
    checkErr(catRepo.insert(electronics), "insert category electronics");
    checkErr(catRepo.insert(furniture),   "insert category furniture");
    qDebug() << "  electronics.id=" << electronics.id
             << "  furniture.id="   << furniture.id;

    // ── 插入产品 ─────────────────────────────────────────────────────────────
    Product laptop("笔记本电脑", "SKU-001", 6999.0, 50);
    laptop.category = std::make_shared<Category>(electronics);

    Product phone("智能手机", "SKU-002", 3999.0, 200);
    phone.category = std::make_shared<Category>(electronics);

    Product desk("办公桌", "SKU-003", 1299.0, 30);
    desk.category = std::make_shared<Category>(furniture);

    checkErr(prodRepo.insert(laptop), "insert laptop");
    checkErr(prodRepo.insert(phone),  "insert phone");
    checkErr(prodRepo.insert(desk),   "insert desk");

    // ── findById ─────────────────────────────────────────────────────────────
    Product found; found.id = laptop.id;
    checkErr(prodRepo.findById(found), "findById laptop");
    qDebug() << "  found:" << found.name << "price=" << found.price;

    // ── update（只更新 price / stock 列）─────────────────────────────────────
    found.price = 6599.0;
    found.stock = 45;
    checkErr(prodRepo.update(found, {"price", "stock"}), "update laptop price+stock");

    // ── save（insert-or-update）───────────────────────────────────────────────
    Product tablet("平板电脑", "SKU-004", 2999.0, 80);
    tablet.category = std::make_shared<Category>(electronics);
    checkErr(prodRepo.save(tablet), "save tablet (insert)");
    tablet.price = 2799.0;
    checkErr(prodRepo.save(tablet), "save tablet (update)");

    qDebug() << "  total products:" << prodRepo.count();
}

static void demo_relation()
{
    SECTION("2. 关联查询");

    orm::Repository<Category> catRepo;
    orm::Repository<Product>  prodRepo;

    // 查询所有分类 + 关联产品列表
    orm::Repository<Category>::EntityPtrList cats;
    checkErr(
        catRepo.fetchAllWithRelation({"products"}, cats),
        "fetchAllWithRelation products"
    );
    for (long i = 0; i < cats.count(); ++i) {
        auto &c = *cats.getByIndex(i);
        qDebug() << "  category:" << c.name
                 << "  products:" << c.products.size();
        for (auto &p : c.products)
            qDebug() << "    -" << p->name << "￥" << p->price;
    }

    // 查询产品 + 关联分类
    orm::Repository<Product>::EntityPtrList prods;
    checkErr(
        prodRepo.fetchAllWithRelation({"category_id"}, prods),
        "fetchAllWithRelation category_id"
    );
    for (long i = 0; i < prods.count(); ++i) {
        auto &p = *prods.getByIndex(i);
        QString catName = p.category ? p.category->name : "(none)";
        qDebug() << "  product:" << p.name
                 << "  category:" << catName;
    }
}

static void demo_query()
{
    SECTION("3. 条件查询（QxSqlQuery）");

    orm::Repository<Product> prodRepo;

    // price > 3000
    {
        qx::QxSqlQuery q;
        q.where("price").isGreaterThan(3000.0);

        orm::Repository<Product>::EntityPtrList result;
        checkErr(prodRepo.fetchByQuery(q, result), "price > 3000");
        qDebug() << "  products price>3000: count=" << result.count();
        for (long i = 0; i < result.count(); ++i)
            qDebug() << "   " << result.getByIndex(i)->name
                     << result.getByIndex(i)->price;
    }

    // enabled = true AND stock > 40 ORDER BY price DESC
    {
        qx::QxSqlQuery q;
        q.where("enabled").isEqualTo(true)
         .and_("stock").isGreaterThan(40);

        orm::Repository<Product>::EntityPtrList result;
        checkErr(prodRepo.fetchByQuery(q, result), "enabled AND stock>40");
        qDebug() << "  enabled & stock>40: count=" << result.count();
    }

    // 原生 SQL 统计
    {
        qx::QxSqlQuery q("SELECT COUNT(*) as cnt, SUM(stock) as total_stock "
                         "FROM product WHERE enabled = :enabled");
        q.bind(":enabled", true);

        orm::Repository<Product>::EntityPtrList dummy;
        checkErr(prodRepo.executeQuery(q, dummy), "raw SQL aggregate");
    }
}

static void demo_transaction()
{
    SECTION("4. 事务");

    orm::Repository<Product> prodRepo;
    long before = prodRepo.count();

    // 正常提交
    bool ok = orm::DbManager::instance().transaction(
        [](qx::QxSession &session) -> bool {
            Product p1("测试品A", "TST-001", 100.0, 10);
            Product p2("测试品B", "TST-002", 200.0, 20);
            session += qx::dao::insert(p1, session.database());
            session += qx::dao::insert(p2, session.database());
            qDebug() << "  [tx] p1.id=" << p1.id << " p2.id=" << p2.id;
            return session.isValid();
        }
    );
    qDebug() << "  commit tx:" << (ok ? "OK" : "FAIL");
    qDebug() << "  count: before=" << before << " after=" << prodRepo.count();

    // 强制回滚
    bool ok2 = orm::DbManager::instance().transaction(
        [](qx::QxSession &session) -> bool {
            Product p3("回滚品", "RBK-001", 1.0, 1);
            session += qx::dao::insert(p3, session.database());
            qDebug() << "  [tx] p3.id=" << p3.id << " (will rollback)";
            return false; // 回滚
        }
    );
    qDebug() << "  rollback tx:" << (!ok2 ? "ROLLBACK OK" : "UNEXPECTED COMMIT");
    qDebug() << "  count after rollback=" << prodRepo.count();
}

static void demo_batch()
{
    SECTION("5. 批量插入");

    orm::Repository<Product> prodRepo;
    long before = prodRepo.count();

    qx::QxCollection<long, std::shared_ptr<Product>> batch;
    for (int i = 1; i <= 10; ++i) {
        auto p = std::make_shared<Product>(
            QString("批量产品%1").arg(i),
            QString("BATCH-%1").arg(i, 3, 10, QChar('0')),
            100.0 * i,
            i * 5
        );
        batch.insert(i, p);
    }

    checkErr(prodRepo.batchInsert(batch), "batchInsert 10 products");
    qDebug() << "  before=" << before << "  after=" << prodRepo.count();
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("orm_mysql_example");

    // ── 命令行参数解析 ────────────────────────────────────────────────────────
    QCommandLineParser parser;
    parser.setApplicationDescription("ORM MySQL Example");
    parser.addHelpOption();
    parser.addOption({"host", "MySQL host",     "host", "127.0.0.1"});
    parser.addOption({"port", "MySQL port",     "port", "3306"});
    parser.addOption({"db",   "Database name",  "db",   "ess_test"});
    parser.addOption({"user", "Username",        "user", "root"});
    parser.addOption({"pass", "Password",        "pass", "123456"});
    parser.addOption({"trace","Enable SQL trace (1/0)", "trace", "1"});
    parser.process(app);

    bool trace = (parser.value("trace") != "0");

    auto cfg = orm::DbConfig::mysql(
        parser.value("db"),
        parser.value("host"),
        parser.value("port").toInt(),
        parser.value("user"),
        parser.value("pass"),
        trace
    );

    qDebug() << "╔══════════════════════════════════════════╗";
    qDebug() << "║    ORM Module — MySQL Example            ║";
    qDebug() << "╚══════════════════════════════════════════╝";
    qDebug() << "[main] connecting:" << cfg.toString();

    // ── 初始化连接 ────────────────────────────────────────────────────────────
    if (!orm::DbManager::instance().init(cfg)) {
        qCritical() << "[main] DB init failed:"
                    << orm::DbManager::instance().lastError();
        qCritical() << "[hint] Please ensure MySQL is running and database"
                    << cfg.database << "exists.";
        qCritical() << "       CREATE DATABASE ess_test CHARACTER SET utf8mb4;";
        return 1;
    }
    qDebug() << "[main] DB init OK";

    // ── 建表（MySQL 推荐使用迁移工具；示例中用显式 DDL）──────────────────────
    if (!ensureMySqlSchema()) {
        qCritical() << "[main] failed to ensure MySQL schema.";
        return 1;
    }
    qDebug() << "[main] tables ensured by explicit DDL.";

    // ── 运行示例 ──────────────────────────────────────────────────────────────
    demo_crud();
    demo_relation();
    demo_query();
    demo_transaction();
    demo_batch();

    qDebug() << "\n[main] MySQL example completed.";
    return 0;
}
