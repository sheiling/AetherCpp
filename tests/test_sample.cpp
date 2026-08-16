// 简单占位测试：验证 Qt Test 与构建链路正常
#include <QtTest>
#include <QString>

class SampleTest : public QObject
{
    Q_OBJECT

private slots:
    void string_compare_ok()
    {
        QCOMPARE(QStringLiteral("ok"), QStringLiteral("ok"));
    }

    void arithmetic_ok()
    {
        QVERIFY(2 + 2 == 4);
    }
};

QTEST_GUILESS_MAIN(SampleTest)
#include "test_sample.moc"
