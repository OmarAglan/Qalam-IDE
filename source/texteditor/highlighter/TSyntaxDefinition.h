#pragma once

#include <QString>
#include <QRegularExpression>

class LanguageDefinition {
public:
    QSet<QString> keywordSet{};
    QSet<QString> builtinSet{};
    QSet<QString> magicSet{};
    QRegularExpression numberPattern{};
    QRegularExpression hexPattern{};

    LanguageDefinition();
};


