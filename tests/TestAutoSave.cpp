#include "QalamAutoSave.h"
#include "Constants.h"

#include <QDir>
#include <QFile>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class TestAutoSave : public QObject
{
    Q_OBJECT
private slots:
    void writesBackupForModifiedDocument();
    void skipsUnmodifiedDocument();
    void reportsFailureOnceAndRecovery();
};

void TestAutoSave::writesBackupForModifiedDocument()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QPlainTextEdit editor;
    QalamAutoSave autoSave(&editor);
    autoSave.filePath = directory.filePath(QStringLiteral("رئيسي.باء"));
    editor.setPlainText(QStringLiteral("صحيح س = ١."));
    editor.document()->setModified(true);

    QVERIFY(autoSave.performAutoSave());
    QFile backup(autoSave.filePath + Constants::BackupExtension);
    QVERIFY(backup.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(backup.readAll()), QStringLiteral("صحيح س = ١."));
}

void TestAutoSave::skipsUnmodifiedDocument()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QPlainTextEdit editor;
    QalamAutoSave autoSave(&editor);
    autoSave.filePath = directory.filePath(QStringLiteral("ملف.باء"));
    editor.setPlainText(QStringLiteral("نص"));
    editor.document()->setModified(false);

    QVERIFY(autoSave.performAutoSave());
    QVERIFY(not QFile::exists(autoSave.filePath + Constants::BackupExtension));
}

void TestAutoSave::reportsFailureOnceAndRecovery()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QPlainTextEdit editor;
    QalamAutoSave autoSave(&editor);
    QSignalSpy failed(&autoSave, &QalamAutoSave::backupFailed);
    QSignalSpy restored(&autoSave, &QalamAutoSave::backupRestored);
    editor.setPlainText(QStringLiteral("نص"));
    editor.document()->setModified(true);

    // A missing parent directory makes every backup write fail.
    const QString folder = directory.filePath(QStringLiteral("مجلد-محذوف"));
    autoSave.filePath = folder + QStringLiteral("/ملف.باء");
    QVERIFY(not autoSave.performAutoSave());
    QVERIFY(not autoSave.performAutoSave());
    QCOMPARE(failed.count(), 1);
    QCOMPARE(failed.first().at(0).toString(), autoSave.filePath + Constants::BackupExtension);
    QVERIFY(not failed.first().at(1).toString().isEmpty());
    QCOMPARE(restored.count(), 0);

    QVERIFY(QDir().mkpath(folder));
    QVERIFY(autoSave.performAutoSave());
    QCOMPARE(restored.count(), 1);
    QCOMPARE(failed.count(), 1);
}

QTEST_MAIN(TestAutoSave)
#include "TestAutoSave.moc"
