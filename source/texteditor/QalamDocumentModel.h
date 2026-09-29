#pragma once

#include <QObject>
#include <QString>

class QTextDocument;
class QalamSyntaxHighlighter;

class QalamDocumentModel final : public QObject
{
    Q_OBJECT

public:
    explicit QalamDocumentModel(QObject *parent = nullptr);

    QTextDocument *document() const { return m_document; }
    QalamSyntaxHighlighter *highlighter() const { return m_highlighter; }
    QString filePath() const { return m_filePath; }
    int sourceRevision() const { return m_sourceRevision; }
    void setFilePath(const QString &path);

signals:
    void filePathChanged(const QString &path);
    void sourceTextChanged();

private:
    QTextDocument *m_document{};
    QalamSyntaxHighlighter *m_highlighter{};
    QString m_filePath;
    QString m_sourceText;
    int m_sourceRevision{};
};
