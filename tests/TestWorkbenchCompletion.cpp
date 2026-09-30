#include "Qalam.h"
#include "BaaLanguageClient.h"
#include "Constants.h"

#include <QFile>
#include <QKeyEvent>
#include <QScopeGuard>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>

class TestWorkbenchCompletion : public QObject
{
    Q_OBJECT
private slots:
    void completesArabicWords_data();
    void completesArabicWords();
};

void TestWorkbenchCompletion::completesArabicWords_data()
{
    QTest::addColumn<QString>("before");
    QTest::addColumn<QString>("word");
    QTest::addColumn<QString>("after");
    QTest::newRow("type") << QString() << QStringLiteral("صحيح") << QString();
    QTest::newRow("print") << QStringLiteral("صحيح الرئيسية() {\n    ")
        << QStringLiteral("اطبع") << QStringLiteral("\n    إرجع ٠.\n}");
    QTest::newRow("return") << QStringLiteral("صحيح الرئيسية() {\n    ")
        << QStringLiteral("إرجع") << QStringLiteral(" ٠.\n}");
}

void TestWorkbenchCompletion::completesArabicWords()
{
    QFETCH(QString, before);
    QFETCH(QString, word);
    QFETCH(QString, after);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto previousFormat = QSettings::defaultFormat();
    const auto restore = qScopeGuard([previousFormat]() {
        QSettings::setDefaultFormat(previousFormat);
    });
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, directory.path());
    QSettings settings(Constants::OrgName, Constants::AppName);
    settings.setValue(Constants::SettingsKeyCompilerPath, QString::fromUtf8(QALAM_TEST_BAA_COMPILER));
    settings.setValue(Constants::SettingsKeyLanguageServerPath, QString::fromUtf8(QALAM_TEST_BAA_LSP));
    settings.sync();

    const QString path = directory.filePath(QStringLiteral("رئيسي.باء"));
    QFile source(path);
    QVERIFY(source.open(QIODevice::WriteOnly));
    source.write((before + after).toUtf8());
    source.close();

    Qalam window(path, nullptr, directory.filePath(QStringLiteral("session.ini")));
    window.resize(1000, 700);
    window.show();
    auto *editor = window.findChild<QalamEditor *>();
    auto *client = window.findChild<BaaLanguageClient *>();
    QVERIFY(editor);
    QVERIFY(client);
    connect(client, &BaaLanguageClient::logMessage, this, [](const QString &message, int) {
        qInfo().noquote() << message;
    });
    QTRY_COMPARE_WITH_TIMEOUT(client->state(), BaaLanguageClient::State::Ready, 10000);
    editor->setFocus();
    QTextCursor cursor = editor->textCursor();
    cursor.setPosition(before.size());
    editor->setTextCursor(cursor);
    auto *completer = editor->findChild<QCompleter *>();
    QVERIFY(completer);
    QSignalSpy completions(client, &BaaLanguageClient::completionPublished);
    QSignalSpy tokens(client, &BaaLanguageClient::semanticTokensPublished);

    for (const QChar character : word) {
        completions.clear();
        QKeyEvent press(QEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, QString(character));
        QWidget *receiver = editor->hasVisibleCompletion()
            ? static_cast<QWidget *>(completer->popup()) : editor;
        QApplication::sendEvent(receiver, &press);
        QTRY_VERIFY_WITH_TIMEOUT(not completions.isEmpty(), 10000);
        QTRY_VERIFY_WITH_TIMEOUT(editor->hasVisibleCompletion(), 5000);
        bool found = false;
        for (int row = 0; row < completer->completionModel()->rowCount(); ++row) {
            if (completer->completionModel()->index(row, 0).data().toString() == word) {
                found = true;
                break;
            }
        }
        QVERIFY2(found, qPrintable(word));
    }
    QCOMPARE(editor->toPlainText(), before + word + after);
    // Let the real asynchronous analysis/formatting responses settle.
    QTRY_VERIFY_WITH_TIMEOUT(not tokens.isEmpty(), 10000);
    QTest::qWait(750);
    QVERIFY(editor->hasVisibleCompletion());
    if (const QString screenshots = qEnvironmentVariable("QALAM_TEST_SCREENSHOTS"); not screenshots.isEmpty()) {
        QVERIFY(window.grab().save(screenshots + "/workbench-" + QTest::currentDataTag() + ".png"));
        QVERIFY(completer->popup()->grab().save(screenshots + "/completion-" + QTest::currentDataTag() + ".png"));
    }
    completer->popup()->hide();
    editor->document()->setModified(false);
}

QTEST_MAIN(TestWorkbenchCompletion)
#include "TestWorkbenchCompletion.moc"
