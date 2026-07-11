#pragma once

#include <QString>
#include <Qt>

struct Diagnostic
{
    QString file;
    int line = 1;
    int column = 1;
    int endLine = 1;
    int endColumn = 1;
    QString severity = "error";
    QString code;
    QString category;
    QString message;
    QString hint;
    QString source = "compiler";

    QString key() const
    {
        return QString("%1|%2|%3|%4|%5|%6|%7")
            .arg(file)
            .arg(line)
            .arg(column)
            .arg(severity, source, code, message);
    }

    bool isWarning() const { return severity.compare("warning", Qt::CaseInsensitive) == 0; }
    bool isError() const { return severity.compare("error", Qt::CaseInsensitive) == 0; }

    QString displayMessage() const
    {
        QString result = code.isEmpty() ? message : QString("[%1] %2").arg(code, message);
        if (!hint.isEmpty()) result += QString(" — %1").arg(hint);
        return result;
    }
};
