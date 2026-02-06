#include "TSyntaxHighlighter.h"

// ==================== Syntax Highlighter ====================

TSyntaxHighlighter::TSyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    lexer = std::make_unique<TLexer>();
}

void TSyntaxHighlighter::setTheme(const std::shared_ptr<SyntaxTheme>& theme) {
    currentThemeFormats.clear();
    if (theme) {
        theme->apply(currentThemeFormats);
    }
    rehighlight();
}

void TSyntaxHighlighter::highlightBlock(const QString& text) {
    int startState = previousBlockState();
    if (startState == -1) startState = StateMasks::Normal;

    QVector<TToken> tokens = lexer->tokenize(text, startState);

    for (const TToken& token : tokens) {
        auto it = currentThemeFormats.find(token.type);
        if (it != currentThemeFormats.end()) {
            setFormat(token.start, token.length, *it);
        }
    }

    setCurrentBlockState(lexer->getFinalState());
}
