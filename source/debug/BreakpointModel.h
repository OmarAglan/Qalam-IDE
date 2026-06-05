#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

class BreakpointModel : public QObject
{
    Q_OBJECT

public:
    struct Breakpoint {
        QString file;
        int line = 1;
        bool enabled = true;
    };

    explicit BreakpointModel(QObject *parent = nullptr);

    void toggleBreakpoint(const QString &file, int line);
    void clear();
    QVector<Breakpoint> breakpoints() const;
    bool hasBreakpoint(const QString &file, int line) const;

signals:
    void breakpointsChanged();

private:
    QString key(const QString &file, int line) const;
    QVector<Breakpoint> m_breakpoints;
};
