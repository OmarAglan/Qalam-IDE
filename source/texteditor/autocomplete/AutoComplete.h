#pragma once

#include <QString>
#include <QVector>
#include <QStringList>
#include <QSet>

enum CompletionType {
    Keyword,
    Snippet,
    Builtin,
    Preprocessor,
    DynamicWord
};

// Identifies which snippet template is being inserted,
// so TSnippetManager can set up the correct placeholder navigation.
enum class SnippetId {
    None = 0,
    Function,
    Class,
    If,
    IfElse,
    Else,
    ElseIf,
    ForLoop,
    WhileLoop,
    Switch,
    Array,
    Constant,
    Main
};

struct CompletionItem {
    QString label;
    QString completion;
    QString description; // New field for the UI
    CompletionType type;
    SnippetId snippetId{SnippetId::None};

    CompletionItem(QString l = "", QString c = "", QString d = "",
                   CompletionType t = CompletionType::Keyword,
                   SnippetId sid = SnippetId::None)
        : label(l), completion(c), description(d), type(t), snippetId(sid) {}
};

// Abstract Strategy Interface
class ICompletionStrategy {
public:
    virtual ~ICompletionStrategy() = default;
    virtual QVector<CompletionItem> getSuggestions(const QString &prefix, const QString &fullText) = 0;
};

// --- Concrete Strategies ---

class KeywordStrategy : public ICompletionStrategy {
public:
    QVector<CompletionItem> getSuggestions(const QString &prefix, const QString &text) override;
};

class BuiltinStrategy : public ICompletionStrategy {
public:
    QVector<CompletionItem> getSuggestions(const QString &prefix, const QString &text) override;
};

class SnippetStrategy : public ICompletionStrategy {
public:
    QVector<CompletionItem> getSuggestions(const QString &prefix, const QString &text) override;
};

class PreprocessorStrategy : public ICompletionStrategy {
public:
    QVector<CompletionItem> getSuggestions(const QString &prefix, const QString &text) override;
};

class DynamicWordStrategy : public ICompletionStrategy {
    QSet<QString> wordIndex;
public:
    QVector<CompletionItem> getSuggestions(const QString &prefix, const QString &fullText) override;
    void rebuildIndex(const QString &fullText);
    void updateIndex(const QString &text); // Incremental update (optional for now)
};

