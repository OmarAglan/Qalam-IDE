#include "TBracketHandler.h"

TBracketHandler::TBracketHandler(QPlainTextEdit *editor) : m_editor(editor) {}

bool TBracketHandler::handleAutoPairing(QKeyEvent *e) {
    QString text = e->text();

    if (!text.isEmpty()) {
        QChar typedChar = text.at(0);

        // Handle opening brackets
        if (typedChar == '(' || typedChar == '[' || typedChar == '{') {
            QChar closingBracket;
            if (typedChar == '(') closingBracket = ')';
            else if (typedChar == '[') closingBracket = ']';
            else closingBracket = '}';

            return handleBracketCompletion(typedChar, closingBracket);
        }
        // Handle quotes
        else if (typedChar == '\'' || typedChar == '"' || typedChar == '`') {
                return handleQuoteCompletion(typedChar);
        }
        // Handle closing brackets (skip over existing ones)
        else if (typedChar == ')' || typedChar == ']' || typedChar == '}' ||
                 typedChar == '\'' || typedChar == '"' || typedChar == '`') {
            return handleBracketSkip(typedChar);
        }
    }

    return false;
}

bool TBracketHandler::handleBracketCompletion(QChar openingBracket, QChar closingBracket) {
    QTextCursor cursor = m_editor->textCursor();

    // Check if there's a selection
    if (cursor.hasSelection()) {
        // Wrap selection with brackets
        QString selectedText = cursor.selectedText();
        cursor.insertText(openingBracket + selectedText + closingBracket);

        // Move cursor after the opening bracket to select the original text
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, selectedText.length() + 1);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, selectedText.length());
        m_editor->setTextCursor(cursor);
    } else {
        // Insert both brackets and place cursor between them
        cursor.insertText(QString(openingBracket) + closingBracket);
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        m_editor->setTextCursor(cursor);
    }

    return true;
}

bool TBracketHandler::handleQuoteCompletion(QChar quoteChar) {
    QTextCursor cursor = m_editor->textCursor();
    QTextDocument *doc = m_editor->document();

    // Get the character at cursor position
    int pos = cursor.position();

    // Check if there's a selection
    if (cursor.hasSelection()) {
        // Wrap selection with quotes
        QString selectedText = cursor.selectedText();
        cursor.insertText(quoteChar + selectedText + quoteChar);

        // Move cursor after the opening quote to select the original text
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, selectedText.length() + 1);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, selectedText.length());
        m_editor->setTextCursor(cursor);
        return true;
    }

    // Check if next character is the same quote (should skip)
    if (pos < doc->characterCount() - 1) {
        QChar nextChar = doc->characterAt(pos);
        if (nextChar == quoteChar) {
            // Just move cursor over the existing quote
            cursor.movePosition(QTextCursor::Right);
            m_editor->setTextCursor(cursor);
            return true;
        }
    }

    // Insert the quote pair
    cursor.insertText(QString(quoteChar) + quoteChar);
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    m_editor->setTextCursor(cursor);

    return true;
}

bool TBracketHandler::handleBracketSkip(QChar typedChar) {
    QTextCursor cursor = m_editor->textCursor();
    QTextDocument *doc = m_editor->document();
    int pos = cursor.position();

    // Check if the next character matches the typed closing bracket/quote
    if (pos < doc->characterCount() - 1) {
        QChar nextChar = doc->characterAt(pos);
        if (nextChar == typedChar) {
            // Just move the cursor over the existing bracket/quote
            cursor.movePosition(QTextCursor::Right);
            m_editor->setTextCursor(cursor);
            return true;
        }
    }

    return false;
}
