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
        if (currentThemeFormats.contains(token.type)) {
            setFormat(token.start, token.length, currentThemeFormats[token.type]);
        }
    }

    setCurrentBlockState(lexer->getFinalState());
}
