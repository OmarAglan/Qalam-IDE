#pragma once

#include <QObject>
#include <QStringList>

class WorkspaceIndexer : public QObject
{
    Q_OBJECT

public:
    struct SymbolLocation {
        QString symbol;
        QString file;
        int line = 1;
        int column = 1;
    };

    explicit WorkspaceIndexer(QObject *parent = nullptr);

    void setRootPath(const QString &rootPath);
    QString rootPath() const;
    void refresh();

    QStringList files() const;
    QStringList quickOpenFiles() const;
    bool isIgnoredPath(const QString &filePath) const;

    bool findDefinition(const QString &symbol, SymbolLocation *location) const;
    QVector<SymbolLocation> findReferences(const QString &symbol) const;

signals:
    void indexUpdated();

private:
    bool isAllowedFile(const QString &filePath) const;
    QStringList candidateFilesForSearch() const;

    QString m_rootPath;
    QStringList m_files;
};
