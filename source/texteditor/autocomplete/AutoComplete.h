#pragma once

#include <QString>
#include <QJsonObject>

enum CompletionType {
    Keyword,
    Snippet,
    Function,
    Variable,
    Type,
    Value,
    Preprocessor,
    File,
    Folder
};

struct CompletionItem {
    QJsonObject protocolItem;
    QJsonValue documentation;
    QString label;
    QString completion;
    QString description;
    QString serverSortText;
    QString context;
    QString stableKey;
    CompletionType type{CompletionType::Value};
    bool snippet{};
    int startLine{};
    int startCharacter{};
    int endLine{};
    int endCharacter{};
};

