/**
 * @file    main.cpp
 * @brief   ORM 模块 SQLite 示例
 *
 * 演示内容：
 *   1. 初始化 SQLite 连接（内存模式）
 *   2. 自动建表（Department / User）
 *   3. 基础 CRUD：insert / findById / update / remove
 *   4. 关联查询：ManyToOne / OneToMany 预加载
 *   5. 条件查询 QxSqlQuery（where / orderBy / limit）
 *   6. 事务（DbManager::transaction）
 *   7. 批量插入 batchInsert
 *   8. 统计行数 count
 */

#include <Orm.h>
#include "models/User.h"
#include "models/Department.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>

// ─────────────────────────────────────────────────────────────────────────────
// 工具宏：打印分隔标题
// ─────────────────────────────────────────────────────────────────────────────
#define SECTION(title) \
    qDebug() << "\n══════════════════════════════════════════"; \
    qDebug() << " " << title;                                   \
    qDebug() << "══════════════════════════════════════════"

// ─────────────────────────────────────────────────────────────────────────────
// 检查 QSqlError 辅助
// ─────────────────────────────────────────────────────────────────────────────
static bool checkErr(const QSqlError &err, const char *op)
{
    if (err.isValid()) {
        qCritical() << "[FAIL]" << op << "=>" << err.text();
        return false;
    }
    qDebug() << "[OK  ]" << op;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 函数
// ─────────────────────────────────────────────────────────────────────────────

static void demo_basic_crud()
{
    SECTION("1. 基础 CRUD");

    orm::Repository<Department> deptRepo;
    orm::Repository<User>       userRepo;

    // ── 插入部门 ─────────────────────────────────────────────────────────────
    Department rd("研发部", "RD");
    Department hr("人力资源部", "HR");
    checkErr(deptRepo.insert(rd), "insert department RD");
    checkErr(deptRepo.insert(hr), "insert department HR");
    qDebug() << "  RD.id =" << rd.id << "  HR.id =" << hr.id;

    // ── 插入用户 ─────────────────────────────────────────────────────────────
    User alice("Alice", "alice@example.com", 28);
    alice.department = std::make_shared<Department>(rd);
    alice.createdAt  = QDateTime::currentDateTime().toString(Qt::ISODate);

    User bob("Bob", "bob@example.com", 32);
    bob.department = std::make_shared<Department>(rd);
    bob.createdAt  = QDateTime::currentDateTime().toString(Qt::ISODate);

    User carol("Carol", "carol@example.com", 25);
    carol.department = std::make_shared<Department>(hr);
    carol.createdAt  = QDateTime::currentDateTime().toString(Qt::ISODate);

    checkErr(userRepo.insert(alice), "insert user Alice");
    checkErr(userRepo.insert(bob),   "insert user Bob");
    checkErr(userRepo.insert(carol), "insert user Carol");
    qDebug() << "  Alice.id=" << alice.id
             << "  Bob.id="   << bob.id
             << "  Carol.id=" << carol.id;

    // ── findById ─────────────────────────────────────────────────────────────
    User found;
    found.id = alice.id;
    checkErr(userRepo.findById(found), "findById Alice");
    qDebug() << "  found:" << found.name << found.email << "age=" << found.age;

    // ── update ───────────────────────────────────────────────────────────────
    found.age = 29;
    checkErr(userRepo.update(found, {"age"}), "update Alice.age=29");

    // ── 验证更新 ─────────────────────────────────────────────────────────────
    User verify; verify.id = alice.id;
    userRepo.findById(verify);
    qDebug() << "  after update: age=" << verify.age;

    // ── exists ───────────────────────────────────────────────────────────────
    qDebug() << "  Alice exists:" << userRepo.exists(found);

    // ── removeById ───────────────────────────────────────────────────────────
    checkErr(userRepo.removeById(carol), "removeById Carol");
    qDebug() << "  count after remove Carol:" << userRepo.count();
}

static void demo_relation_query()
{
    SECTION("2. 关联查询（ManyToOne / OneToMany）");

    orm::Repository<User>       userRepo;
    orm::Repository<Department> deptRepo;

    // ── 按主键查询 User，同时预加载 department ────────────────────────────────
    orm::Repository<User>::EntityPtrList users;
    checkErr(userRepo.fetchAll(users), "fetchAll users (no relation)");
    qDebug() << "  total users:" << users.count();

    // 预加载 ManyToOne：department（sKey 与注册时一致）
    orm::Repository<User>::EntityPtrList usersWithDept;
    checkErr(
        userRepo.fetchAllWithRelation(QStringList{"department"}, usersWithDept),
        "fetchAllWithRelation department"
    );
    for (long i = 0; i < usersWithDept.count(); ++i) {
        auto &u = *usersWithDept.getByIndex(i);
        QString deptName = (u.department ? u.department->name : "(none)");
        qDebug() << "  user:" << u.name
                 << "  dept:" << deptName;
    }

    // ── 按主键查询 Department，同时预加载 users ───────────────────────────────
    orm::Repository<Department>::EntityPtrList depts;
    checkErr(
        deptRepo.fetchAllWithRelation(QStringList{"users"}, depts),
        "fetchAllWithRelation users"
    );
    for (long i = 0; i < depts.count(); ++i) {
        auto &d = *depts.getByIndex(i);
        qDebug() << "  dept:" << d.name
                 << "  users count:" << d.users.size();
        for (auto &u : d.users)
            qDebug() << "    -" << u->name;
    }
}

static void demo_query_builder()
{
    SECTION("3. 条件查询（QxSqlQuery）");

    orm::Repository<User> userRepo;

    // ── WHERE age > 25 ────────────────────────────────────────────────────────
    qx::QxSqlQuery q;
    q.where("age").isGreaterThan(25);

    orm::Repository<User>::EntityPtrList result;
    checkErr(userRepo.fetchByQuery(q, result), "fetchByQuery age>25");
    qDebug() << "  users with age>25:";
    for (long i = 0; i < result.count(); ++i)
        qDebug() << "   " << result.getByIndex(i)->name
                 << "age=" << result.getByIndex(i)->age;

    // ── WHERE name LIKE 'A%' ─────────────────────────────────────────────────
    qx::QxSqlQuery q2;
    q2.where("name").like("A%");

    orm::Repository<User>::EntityPtrList result2;
    checkErr(userRepo.fetchByQuery(q2, result2), "fetchByQuery name LIKE A%");
    qDebug() << "  users name like A%:" << result2.count();

    // ── count ────────────────────────────────────────────────────────────────
    qx::QxSqlQuery qAll;
    long total = userRepo.count(qAll);
    qDebug() << "  total user count:" << total;
}

static void demo_transaction()
{
    SECTION("4. 事务（DbManager::transaction）");

    orm::Repository<User> userRepo;

    // ── 正常事务（提交）─────────────────────────────────────────────────────
    bool ok = orm::DbManager::instance().transaction(
        [&](qx::QxSession &session) -> bool {
            User u1("Dave", "dave@example.com", 40);
            User u2("Eve",  "eve@example.com",  35);
            session += qx::dao::insert(u1, session.database());
            session += qx::dao::insert(u2, session.database());
            qDebug() << "  [tx] inserted Dave.id=" << u1.id
                     << " Eve.id=" << u2.id;
            return true; // commit
        }
    );
    qDebug() << "  normal transaction:" << (ok ? "COMMIT" : "ROLLBACK");

    // ── 回滚事务──────────────────────────────────────────────────────────────
    bool ok2 = orm::DbManager::instance().transaction(
        [&](qx::QxSession &session) -> bool {
            User u3("Frank", "frank@example.com", 22);
            session += qx::dao::insert(u3, session.database());
            qDebug() << "  [tx] inserted Frank.id=" << u3.id << " (will rollback)";
            return false; // 触发 rollback
        }
    );
    qDebug() << "  rollback transaction:" << (ok2 ? "COMMIT" : "ROLLBACK");
    qDebug() << "  total after tx:" << userRepo.count();
}

static void demo_batch()
{
    SECTION("5. 批量操作（batchInsert）");

    orm::Repository<User> userRepo;
    long before = userRepo.count();

    // QxCollection 批量插入
    qx::QxCollection<long, std::shared_ptr<User>> batch;
    for (int i = 1; i <= 5; ++i) {
        auto u = std::make_shared<User>(
            QString("BatchUser%1").arg(i),
            QString("batch%1@example.com").arg(i),
            20 + i
        );
        batch.insert(i, u);
    }

    checkErr(userRepo.batchInsert(batch), "batchInsert 5 users");
    qDebug() << "  count before:" << before
             << "  after:" << userRepo.count();
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "╔══════════════════════════════════════════╗";
    qDebug() << "║    ORM Module — SQLite Example           ║";
    qDebug() << "╚══════════════════════════════════════════╝";

    // ── 1. 初始化数据库连接（内存模式，trace=true 输出 SQL）────────────────
    // auto cfg = orm::DbConfig::sqlite(":memory:", true);
    auto cfg = orm::DbConfig::sqlite("test.db", true);
    if (!orm::DbManager::instance().init(cfg)) {
        qCritical() << "DB init failed:" << orm::DbManager::instance().lastError();
        return 1;
    }
    qDebug() << "[main] DB init OK:" << cfg.toString();

    // ── 2. 建表 ──────────────────────────────────────────────────────────────
    orm::DbManager::instance().createTable<Department>();
    orm::DbManager::instance().createTable<User>();
    qDebug() << "[main] tables created.";

    // ── 3. 运行示例 ──────────────────────────────────────────────────────────
    demo_basic_crud();
    demo_relation_query();
    demo_query_builder();
    demo_transaction();
    demo_batch();

    qDebug() << "\n[main] SQLite example completed.";
    return 0;
}
