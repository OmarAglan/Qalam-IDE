#pragma once

class QTextDocument;

// Finds the bracket that pairs with the one at the caret. Brackets inside Baa
// strings and comments are ignored by reusing the lexer, so a ")" in a string
// can never balance a "(" in code.
class QalamBracketMatcher {
public:
    struct Match {
        int bracket{-1};   // Position of the bracket at the caret, or -1.
        int partner{-1};   // Position of its partner, or -1 when unmatched.

        bool hasBracket() const { return bracket >= 0; }
        bool isMatched() const { return partner >= 0; }
    };

    // Looks at the character after the caret first, then the one before it.
    static Match find(const QTextDocument *document, int caretPosition);

    // Upper bound on characters scanned for a partner, keeping the caret
    // responsive in very large files. A partner beyond it is reported missing.
    static constexpr int ScanLimit = 200000;
};
