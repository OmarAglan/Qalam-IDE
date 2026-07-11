#include "DiagnosticParser.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(text.trimmed().toUtf8(), &jsonError);
    if (jsonError.error == QJsonParseError::NoError and document.isObject()) {
        const QJsonObject root = document.object();
        if (root.value("schema_version").toString() == "diagnostics-json-v1") {
            const QJsonArray items = root.value("diagnostics").toArray();
            diagnostics.reserve(items.size());
            for (const QJsonValue &value : items) {
                if (!value.isObject()) continue;

                const QJsonObject item = value.toObject();
                Diagnostic diagnostic;
                diagnostic.file = normalizeFilePath(item.value("file").toString(),
                                                    fallbackFile, workspaceRoot);
                diagnostic.severity = item.value("severity").toString("error").toLower();
                diagnostic.code = item.value("code").toString();
                diagnostic.category = item.value("category").toString();
                diagnostic.message = item.value("message").toString().trimmed();
                diagnostic.hint = item.value("hint").toString().trimmed();
                diagnostic.source = "baa-json";

                const QJsonObject span = item.value("span").toObject();
                const QJsonObject start = span.value("start").toObject();
                const QJsonObject end = span.value("end").toObject();
                diagnostic.line = qMax(1, start.value("line").toInt(item.value("line").toInt(1)));
                diagnostic.column = qMax(1, start.value("column").toInt(item.value("column").toInt(1)));
                diagnostic.endLine = qMax(diagnostic.line, end.value("line").toInt(diagnostic.line));
                diagnostic.endColumn = qMax(1, end.value("column").toInt(diagnostic.column));

                if (diagnostic.message.isEmpty()) continue;
                const QString key = diagnostic.key();
                if (seen.contains(key)) continue;
                seen.insert(key);
                diagnostics.push_back(diagnostic);
            }
            return diagnostics;
        }
    }

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
        diagnostic.endLine = diagnostic.line;
        diagnostic.endColumn = diagnostic.column;
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
