#include "BreakpointModel.h"

#include <QDir>
#include <QtGlobal>

BreakpointModel::BreakpointModel(QObject *parent)
    : QObject(parent)
{
}

QString BreakpointModel::key(const QString &file, int line) const
{
    return QDir::cleanPath(file) + ":" + QString::number(qMax(1, line));
}

void BreakpointModel::toggleBreakpoint(const QString &file, int line)
{
    const QString target = key(file, line);
    for (int i = 0; i < m_breakpoints.size(); ++i) {
        if (key(m_breakpoints[i].file, m_breakpoints[i].line) == target) {
            m_breakpoints.removeAt(i);
            emit breakpointsChanged();
            return;
        }
    }

    m_breakpoints.push_back({QDir::cleanPath(file), qMax(1, line), true});
    emit breakpointsChanged();
}

void BreakpointModel::clear()
{
    if (m_breakpoints.isEmpty()) return;
    m_breakpoints.clear();
    emit breakpointsChanged();
}

QVector<BreakpointModel::Breakpoint> BreakpointModel::breakpoints() const
{
    return m_breakpoints;
}

bool BreakpointModel::hasBreakpoint(const QString &file, int line) const
{
    const QString target = key(file, line);
    for (const Breakpoint &breakpoint : m_breakpoints) {
        if (key(breakpoint.file, breakpoint.line) == target) return true;
    }
    return false;
}
