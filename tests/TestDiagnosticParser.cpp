#include "DiagnosticParser.h"
#include "DiagnosticsModel.h"

#include <QtTest/QtTest>

class TestDiagnosticParser : public QObject
{
    Q_OBJECT

private slots:
    void parsesColonError();
    void parsesArabicLineError();
    void parsesBaaJsonDiagnostics();
    void convertsExactByteSpansToUtf16Columns();
    void keepsDisplayColumnsWhenTheSourceChanged();
    void ignoresUnknownJsonSchema();
    void deduplicatesRepeatedDiagnostics();
    void replacesOnlyOneAnalysisSource();
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

void TestDiagnosticParser::parsesBaaJsonDiagnostics()
{
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(R"json(
        {
          "schema_version": "diagnostics-json-v1",
          "diagnostics": [
            {
              "code": "B1000",
              "severity": "error",
              "category": "semantic",
              "message": "متغير غير معرف",
              "file": "source/main.baa",
              "line": 4,
              "column": 7,
              "span": {
                "start": {"line": 4, "column": 7, "byte": 20},
                "end": {"line": 4, "column": 10, "byte": 23}
              },
              "hint": "عرّف المتغير قبل استخدامه",
              "hints": ["عرّف المتغير قبل استخدامه"]
            }
          ]
        }
    )json", QString(), "/tmp/project");

    QCOMPARE(diagnostics.size(), 1);
    QCOMPARE(diagnostics[0].file, QString("/tmp/project/source/main.baa"));
    QCOMPARE(diagnostics[0].line, 4);
    QCOMPARE(diagnostics[0].column, 7);
    QCOMPARE(diagnostics[0].endColumn, 10);
    QCOMPARE(diagnostics[0].code, QString("B1000"));
    QCOMPARE(diagnostics[0].category, QString("semantic"));
    QCOMPARE(diagnostics[0].source, QString("baa-json"));
    QVERIFY(diagnostics[0].displayMessage().contains("B1000"));
    QVERIFY(diagnostics[0].displayMessage().contains("عرّف"));
}

namespace {
QString byteSpanJson(int line, int column, qsizetype startByte, qsizetype endByte)
{
    return QString(R"json({"schema_version": "diagnostics-json-v1", "diagnostics": [
        {"code": "B2001", "severity": "error", "message": "رمز غير معرف",
         "file": "رئيسي.باء",
         "span": {"start": {"line": %1, "column": %2, "byte": %3},
                  "end": {"line": %1, "column": %2, "byte": %4}}}]})json")
        .arg(line).arg(column).arg(startByte).arg(endByte);
}
}

void TestDiagnosticParser::convertsExactByteSpansToUtf16Columns()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    // The supplementary character before the identifier is one display
    // column but two UTF-16 units, so Baa's display column is off by one.
    const QByteArray source = (QStringLiteral("صحيح.\r\n    ") +
        QString::fromUcs4(U"\U0001D538") + QStringLiteral(" متغير.")).toUtf8();
    QFile file(directory.filePath(QStringLiteral("رئيسي.باء")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(source);
    file.close();

    const QByteArray identifier = QStringLiteral("متغير").toUtf8();
    const qsizetype start = source.indexOf(identifier);
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(
        byteSpanJson(2, 7, start, start + identifier.size()),
        QString(), directory.path());

    QCOMPARE(diagnostics.size(), 1);
    QCOMPARE(diagnostics[0].line, 2);
    QCOMPARE(diagnostics[0].column, 8);
    QCOMPARE(diagnostics[0].endLine, 2);
    QCOMPARE(diagnostics[0].endColumn, 13);
}

void TestDiagnosticParser::keepsDisplayColumnsWhenTheSourceChanged()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile file(directory.filePath(QStringLiteral("رئيسي.باء")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QStringLiteral("سطر واحد فقط.").toUtf8());
    file.close();

    // The byte offset now points into line 1, not the reported line 3.
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(
        byteSpanJson(3, 5, 4, 6), QString(), directory.path());
    QCOMPARE(diagnostics.size(), 1);
    QCOMPARE(diagnostics[0].line, 3);
    QCOMPARE(diagnostics[0].column, 5);
}

void TestDiagnosticParser::ignoresUnknownJsonSchema()
{
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(
        R"json({"schema_version":"diagnostics-json-v2","diagnostics":[{"message":"bad"}]})json");
    QVERIFY(diagnostics.isEmpty());
}

void TestDiagnosticParser::deduplicatesRepeatedDiagnostics()
{
    const auto diagnostics = DiagnosticParser::parseCompilerOutput(
        "main.baa:1:1: warning: تنبيه\nmain.baa:1:1: warning: تنبيه", QString(), "/tmp/project");

    QCOMPARE(diagnostics.size(), 1);
    QCOMPARE(diagnostics[0].severity, QString("warning"));
}

void TestDiagnosticParser::replacesOnlyOneAnalysisSource()
{
    DiagnosticsModel model;
    Diagnostic first;
    first.file = "/tmp/first.baa";
    first.message = "قديم";
    first.source = "baa-lsp:/tmp/first.baa";
    Diagnostic second;
    second.file = "/tmp/second.baa";
    second.message = "يبقى";
    second.source = "baa-lsp:/tmp/second.baa";
    model.setDiagnostics({first, second});

    Diagnostic replacement;
    replacement.file = "/tmp/first.baa";
    replacement.message = "جديد";
    model.replaceDiagnosticsFromSource("baa-lsp:/tmp/first.baa", {replacement});

    QCOMPARE(model.count(), 2);
    QCOMPARE(model.diagnosticsForFile("/tmp/first.baa").first().message, QString("جديد"));
    QCOMPARE(model.diagnosticsForFile("/tmp/second.baa").first().message, QString("يبقى"));
    QCOMPARE(model.diagnosticsForFile("/tmp/first.baa").first().source,
             QString("baa-lsp:/tmp/first.baa"));
}

QTEST_MAIN(TestDiagnosticParser)
#include "TestDiagnosticParser.moc"
