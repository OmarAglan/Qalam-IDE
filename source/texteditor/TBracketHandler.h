#pragma once

#include <QPlainTextEdit>
#include <QKeyEvent>

// Handles bracket/quote auto-pairing and skip-over for TEditor.
// Extracted from TEditor to reduce its size and isolate bracket logic.
class TBracketHandler {
public:
    explicit TBracketHandler(QPlainTextEdit *editor);

    // Returns true if the key event was consumed (bracket/quote handled).
    bool handleAutoPairing(QKeyEvent *e);

private:
    bool handleBracketCompletion(QChar openingBracket, QChar closingBracket);
    bool handleQuoteCompletion(QChar quoteChar);
    bool handleBracketSkip(QChar typedChar);

    QPlainTextEdit *m_editor{};
};
