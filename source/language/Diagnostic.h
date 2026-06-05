#pragma once

#include <QString>
#include <Qt>

struct Diagnostic
{
    QString file;
    int line = 1;
    int column = 1;
    QString severity = "error";
    QString message;
    QString source = "compiler";

    QString key() const
    {
        return QString("%1|%2|%3|%4|%5|%6")
            .arg(file)
            .arg(line)
            .arg(column)
            .arg(severity, source, message);
    }

    bool isWarning() const { return severity.compare("warning", Qt::CaseInsensitive) == 0; }
    bool isError() const { return !isWarning(); }
};
