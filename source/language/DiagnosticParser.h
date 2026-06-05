#pragma once

#include "Diagnostic.h"

#include <QVector>
#include <QString>

class DiagnosticParser
{
public:
    static QVector<Diagnostic> parseCompilerOutput(const QString &text,
                                                   const QString &fallbackFile = QString(),
                                                   const QString &workspaceRoot = QString());

private:
    static QString normalizeFilePath(const QString &file,
                                     const QString &fallbackFile,
                                     const QString &workspaceRoot);
};
