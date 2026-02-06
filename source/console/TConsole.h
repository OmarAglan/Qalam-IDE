#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QProcess>
#include <QTimer>
#include <QMutex>
#include <QVector>

class TConsole : public QWidget {
    Q_OBJECT
public:
    explicit TConsole(QWidget *parent = nullptr);
    ~TConsole() override;

    // actions
    void startCmd();                    // start cmd.exe (Windows)
    void stopCmd();                     // stop process
    void clear();                       // clear output
    void setConsoleRTL();               // force RTL on widgets
    void appendPlainTextThreadSafe(const QString &text); // thread-safe append

signals:
    void commandEntered(const QString &cmd); // emitted when user enters command

private slots:
    void processStdout();
    void processStderr();
    void processFinished(int code, QProcess::ExitStatus status);
    void onInputReturn();
    void flushPending();

private:
    QPlainTextEdit *m_output{};
    QLineEdit *m_input{};
    QProcess *m_process{};
    QTimer *m_flushTimer{};

    QMutex m_pendingMutex{};
    QStringList m_pending{}; // staging lines

    // history
    QVector<QString> m_history{};
    int m_historyIndex{}; // -1 means not browsing

    // autoscroll
    bool m_autoscroll{};

    // helpers
    void appendOutput(const QString &text); // needs to run in GUI thread
    QString ansiToHtmlFragment(const QString &chunk); // simple ansi -> html/text formatting
    bool eventFilter(QObject *obj, QEvent *ev) override;

    QStringList m_buffer{};          // تخزين جميع الأسطر
    int m_maxLines = 2000;         // آخر 2000 سطر
};

