#include "DiagnosticParser.h"

#include <QtTest/QtTest>

class TestDiagnosticParser : public QObject
{
    Q_OBJECT

private slots:
    void parsesColonError();
    void parsesArabicLineError();
    void deduplicatesRepeatedDiagnostics();
};

void TestDiagnosticParser::parsesColonError()
{
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(
        "main.baa:12:4: error: متغير غير معرف", QString(), "/tmp/project");

    QCOMPARE(diagnostics.size(), 1);
    QCOMPARE(diagnostics[0].line, 12);
    QCOMPARE(diagnostics[0].column, 4);
    QCOMPARE(diagnostics[0].severity, QString("error"));
    QVERIFY(diagnostics[0].message.contains("متغير"));
}

void TestDiagnosticParser::parsesArabicLineError()
{
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(
        "السطر 9، العمود 2 خطأ: فاصلة مفقودة", "/tmp/project/main.baa", "/tmp/project");

    QCOMPARE(diagnostics.size(), 1);
    QCOMPARE(diagnostics[0].file, QString("/tmp/project/main.baa"));
    QCOMPARE(diagnostics[0].line, 9);
    QCOMPARE(diagnostics[0].column, 2);
    QCOMPARE(diagnostics[0].severity, QString("error"));
}

void TestDiagnosticParser::deduplicatesRepeatedDiagnostics()
{
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(
        "main.baa:1:1: warning: تنبيه\nmain.baa:1:1: warning: تنبيه", QString(), "/tmp/project");

    QCOMPARE(diagnostics.size(), 1);
    QCOMPARE(diagnostics[0].severity, QString("warning"));
}

QTEST_MAIN(TestDiagnosticParser)
#include "TestDiagnosticParser.moc"
