#include "WorkspaceIndexer.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QtGlobal>
#include <QStringConverter>

WorkspaceIndexer::WorkspaceIndexer(QObject *parent)
    : QObject(parent)
{
}

void WorkspaceIndexer::setRootPath(const QString &rootPath)
{
    const QString clean = QDir::cleanPath(rootPath);
    if (m_rootPath == clean) return;
    m_rootPath = clean;
    refresh();
}

QString WorkspaceIndexer::rootPath() const
{
    return m_rootPath;
}

void WorkspaceIndexer::refresh()
{
    m_files.clear();
    if (m_rootPath.isEmpty() || !QDir(m_rootPath).exists()) {
        emit indexUpdated();
        return;
    }

    QDirIterator it(m_rootPath, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = QDir::cleanPath(it.next());
        if (isIgnoredPath(path)) continue;
        if (!isAllowedFile(path)) continue;
        m_files << path;
    }
    m_files.sort(Qt::CaseInsensitive);
    emit indexUpdated();
}

QStringList WorkspaceIndexer::files() const
{
    return m_files;
}

QStringList WorkspaceIndexer::quickOpenFiles() const
{
    return m_files;
}

bool WorkspaceIndexer::isIgnoredPath(const QString &filePath) const
{
    const QString normalized = QDir::fromNativeSeparators(QDir::cleanPath(filePath));
    const QStringList ignored = {
        "/.git/", "/.hg/", "/.svn/", "/build/", "/dist/", "/out/",
        "/node_modules/", "/.cache/", "/CMakeFiles/", "/.vs/"
    };
    for (const QString &segment : ignored) {
        if (normalized.contains(segment, Qt::CaseInsensitive)) return true;
    }
    return false;
}

bool WorkspaceIndexer::isAllowedFile(const QString &filePath) const
{
    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) return false;
    if (info.size() > 5 * 1024 * 1024) return false;

    const QString suffix = info.suffix().toLower();
    const QStringList allowed = {"baa", "baahd", "txt", "md", "json", "cmake", "cpp", "c", "h", "hpp"};
    return allowed.contains(suffix) || info.fileName().compare("CMakeLists.txt", Qt::CaseInsensitive) == 0;
}

QStringList WorkspaceIndexer::candidateFilesForSearch() const
{
    return m_files;
}

bool WorkspaceIndexer::findDefinition(const QString &symbol, SymbolLocation *location) const
{
    if (symbol.trimmed().isEmpty()) return false;

    const QString escaped = QRegularExpression::escape(symbol.trimmed());
    const QRegularExpression definitionPattern(
        QStringLiteral(R"((?:^|\s)(?:دالة|صنف|ثابت|صحيح|عدد|نص|منطقي|حرف|متغير)\s+%1(?:\s|\(|=|\.|$))").arg(escaped),
        QRegularExpression::UseUnicodePropertiesOption);

    for (const QString &filePath : candidateFilesForSearch()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        int lineNumber = 0;
        while (!stream.atEnd()) {
            const QString lineText = stream.readLine();
            ++lineNumber;
            const QRegularExpressionMatch match = definitionPattern.match(lineText);
            if (!match.hasMatch()) continue;

            if (location) {
                location->symbol = symbol;
                location->file = filePath;
                location->line = lineNumber;
                location->column = qMax(1, lineText.indexOf(symbol) + 1);
            }
            return true;
        }
    }
    return false;
}

QVector<WorkspaceIndexer::SymbolLocation> WorkspaceIndexer::findReferences(const QString &symbol) const
{
    QVector<SymbolLocation> locations;
    if (symbol.trimmed().isEmpty()) return locations;

    const QString escaped = QRegularExpression::escape(symbol.trimmed());
    const QRegularExpression referencePattern(
        QStringLiteral(R"((?<![\p{L}\p{N}_])%1(?![\p{L}\p{N}_]))").arg(escaped),
        QRegularExpression::UseUnicodePropertiesOption);

    for (const QString &filePath : candidateFilesForSearch()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        int lineNumber = 0;
        while (!stream.atEnd()) {
            const QString lineText = stream.readLine();
            ++lineNumber;
            auto it = referencePattern.globalMatch(lineText);
            while (it.hasNext()) {
                const QRegularExpressionMatch match = it.next();
                if (!match.hasMatch()) continue;
                locations.push_back({symbol, filePath, lineNumber, match.capturedStart() + 1});
            }
        }
    }

    return locations;
}
