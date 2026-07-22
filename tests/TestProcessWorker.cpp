#include "ProcessWorker.h"

#include <QCoreApplication>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>

class TestProcessWorker : public QObject
{
    Q_OBJECT

private slots:
    void tailsCompleteAndFinalJsonLines();
    void reportsRequestedCancellation();
};

void TestProcessWorker::tailsCompleteAndFinalJsonLines()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString eventPath = temporary.filePath("أحداث جزئية.jsonl");
    ProcessWorker worker(
        QCoreApplication::applicationFilePath(),
        {"--helper-events", eventPath},
        temporary.path(),
        eventPath);
    QSignalSpy events(&worker, &ProcessWorker::eventLineReady);
    QSignalSpy finished(&worker, &ProcessWorker::finished);

    worker.start();
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QCOMPARE(finished.first().first().toInt(), 0);
    QCOMPARE(events.size(), 2);
    QCOMPARE(events[0].first().toByteArray(), QByteArray("{\"sequence\":1}"));
    QCOMPARE(events[1].first().toByteArray(), QByteArray("{\"sequence\":2}"));
}

void TestProcessWorker::reportsRequestedCancellation()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    ProcessWorker worker(
        QCoreApplication::applicationFilePath(),
        {"--helper-wait"},
        temporary.path());
    QSignalSpy finished(&worker, &ProcessWorker::finished);

    worker.start();
    QTest::qWait(150);
    worker.stop();
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QCOMPARE(finished.first().first().toInt(), -2);
}

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    const QStringList arguments = application.arguments();
    const int eventHelper = arguments.indexOf("--helper-events");
    if (eventHelper >= 0 and eventHelper + 1 < arguments.size()) {
        QFile file(arguments[eventHelper + 1]);
        if (not file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return 20;
        file.write("{\"sequence\":1}\n");
        file.flush();
        QThread::msleep(100);
        file.write("{\"sequence\":2}");
        file.flush();
        return 0;
    }
    if (arguments.contains("--helper-wait")) {
        QThread::sleep(30);
        return 0;
    }

    TestProcessWorker test;
    return QTest::qExec(&test, argc, argv);
}

#include "TestProcessWorker.moc"
