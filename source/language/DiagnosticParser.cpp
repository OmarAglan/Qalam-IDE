#include "DiagnosticParser.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>

namespace {
QString normalizedSeverity(const QString &raw)
{
    return raw.contains("warning", Qt::CaseInsensitive) || raw.contains("تحذير")
        ? QStringLiteral("warning")
        : QStringLiteral("error");
}

int normalizedInt(const QString &text, int fallback = 1)
{
    bool ok = false;
    const int value = text.toInt(&ok);
    return ok ? qMax(1, value) : fallback;
}
}

QVector<Diagnostic> DiagnosticParser::parseCompilerOutput(const QString &text,
                                                          const QString &fallbackFile,
                                                          const QString &workspaceRoot)
{
    QVector<Diagnostic> diagnostics;
    QSet<QString> seen;

    const QList<QRegularExpression> patterns = {
        QRegularExpression(R"(([^:\n]+):(\d+):(\d+):\s*(error|warning|خطأ|تحذير)[:：]?\s*(.*))",
                           QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression(R"(([^:\n]+):(\d+):\s*(error|warning|خطأ|تحذير)[:：]?\s*(.*))",
                           QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption),
        QRegularExpression(R"((?:line|السطر)\s+(\d+)(?:[,،]\s*(?:column|العمود)\s+(\d+))?.*(error|warning|خطأ|تحذير)[:：]?\s*(.*))",
                           QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption)
    };

    for (const QString &rawLine : text.split('\n')) {
        const QString lineText = rawLine.trimmed();
        if (lineText.isEmpty()) continue;

        Diagnostic diagnostic;
        diagnostic.file = fallbackFile;
        diagnostic.source = "compiler";
        bool matched = false;

        QRegularExpressionMatch match = patterns[0].match(lineText);
        if (match.hasMatch()) {
            diagnostic.file = match.captured(1).trimmed();
            diagnostic.line = normalizedInt(match.captured(2));
            diagnostic.column = normalizedInt(match.captured(3));
            diagnostic.severity = normalizedSeverity(match.captured(4));
            diagnostic.message = match.captured(5).trimmed();
            matched = true;
        }

        if (!matched) {
            match = patterns[1].match(lineText);
            if (match.hasMatch()) {
                diagnostic.file = match.captured(1).trimmed();
                diagnostic.line = normalizedInt(match.captured(2));
                diagnostic.column = 1;
                diagnostic.severity = normalizedSeverity(match.captured(3));
                diagnostic.message = match.captured(4).trimmed();
                matched = true;
            }
        }

        if (!matched) {
            match = patterns[2].match(lineText);
            if (match.hasMatch()) {
                diagnostic.file = fallbackFile;
                diagnostic.line = normalizedInt(match.captured(1));
                diagnostic.column = normalizedInt(match.captured(2));
                diagnostic.severity = normalizedSeverity(match.captured(3));
                diagnostic.message = match.captured(4).trimmed();
                matched = true;
            }
        }

        if (!matched) continue;
        if (diagnostic.message.isEmpty()) diagnostic.message = lineText;
        diagnostic.file = normalizeFilePath(diagnostic.file, fallbackFile, workspaceRoot);

        const QString key = diagnostic.key();
        if (seen.contains(key)) continue;
        seen.insert(key);
        diagnostics.push_back(diagnostic);
    }

    return diagnostics;
}

QString DiagnosticParser::normalizeFilePath(const QString &file,
                                            const QString &fallbackFile,
                                            const QString &workspaceRoot)
{
    QString result = file.trimmed();
    if (result.isEmpty()) result = fallbackFile;
    if (result.isEmpty()) return QString();

    QFileInfo info(result);
    if (info.isRelative()) {
        if (!fallbackFile.isEmpty()) {
            result = QFileInfo(fallbackFile).absoluteDir().filePath(result);
        } else if (!workspaceRoot.isEmpty()) {
            result = QDir(workspaceRoot).filePath(result);
        }
    }
    return QDir::cleanPath(result);
}
