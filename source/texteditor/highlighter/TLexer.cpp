#include "TLexer.h"



// ==================== Helpers ====================

static QPair<int, QString> checkStringStart(QStringView text, int pos) {
    if (pos >= text.length()) return {-1, ""};
    QChar ch = text[pos];
    if (ch == '"' || ch == '\'') {
        return {0, QString(ch)};
    }
    return {-1, ""};
}

// ==================== Normal State ====================

TToken NormalState::readToken(QStringView text, int& pos, const LanguageDefinition& langDef) {
    if (pos >= text.length()) return TToken(TokenType::None, pos, 0);

    QChar ch = text[pos];

    // 1. Whitespace
    if (ch.isSpace()) {
        int start = pos;
        while (pos < text.length() && text[pos].isSpace()) pos++;
        return TToken(TokenType::Whitespace, start, pos - start);
    }

    // 2. Comments (Baa uses // for single-line comments)
    if (pos + 1 < text.length() && ch == '/' && text[pos + 1] == '/') {
        int start = pos;
        pos = text.length();
        return TToken(TokenType::Comment, start, pos - start);
    }

    // 3. Preprocessor directives (start with #)
    if (ch == '#') {
        int start = pos;
        pos++; // Skip the #

        // Read the directive name (Arabic letters and underscores after #)
        while (pos < text.length() && (text[pos].isLetter() || text[pos] == '_')) {
            pos++;
        }

        QString directive = text.mid(start, pos - start).toString();

        if (langDef.preprocessorSet.contains(directive)) {
            return TToken(TokenType::Preprocessor, start, pos - start, directive);
        }

        return TToken(TokenType::Operator, start, pos - start, directive);
    }

    // 4. Strings
    auto strCheck = checkStringStart(text, pos);
    if (strCheck.first != -1) {
        int start = pos;
        QString quote = strCheck.second;
        
        pos += quote.length(); 

        int delimId = (quote == "\"") ? StateMasks::Double : StateMasks::Single;
        pendingState = std::make_unique<StringState>(quote, delimId);
        return TToken(TokenType::String, start, pos - start);
    }

    // 5. Identifiers & Keywords
    if (ch.isLetter() || ch == '_') {
        int start = pos;
        while (pos < text.length() && (text[pos].isLetterOrNumber() || text[pos] == '_')) pos++;
        QStringView wordView = text.mid(start, pos - start);
        QString word = wordView.toString();

        if (word == "صواب" || word == "خطأ") return TToken(TokenType::BooleanLiteral, start, pos - start, word);
        if (langDef.keywordSet.contains(word)) return TToken(TokenType::Keyword, start, pos - start, word);
        if (langDef.builtinSet.contains(word)) return TToken(TokenType::BuiltinFunc, start, pos - start, word);
        if (langDef.preprocessorSet.contains(word)) return TToken(TokenType::Preprocessor, start, pos - start, word);

        // Check for function pattern 'func('
        int next = pos;
        while(next < text.length() && text[next].isSpace()) next++;
        if (next < text.length() && text[next] == '(') {
            return TToken(TokenType::Function, start, pos - start, word);
        }

        return TToken(TokenType::Identifier, start, pos - start, word);
    }

    // 6. Numbers (Integers Only (§3.1))
    if (ch.isDigit() || ch == u'٠' || ch == u'١' || ch == u'٢' || ch == u'٣' || ch == u'٤' || ch == u'٥' || ch == u'٦' || ch == u'٧' || ch == u'٨' || ch == u'٩') {
        int start = pos;
        if (ch == '0' && pos + 1 < text.length() && text.mid(pos, 2).compare(u"0x", Qt::CaseInsensitive) == 0) {
            // Use matchView to avoid QString copy (Qt 6.1+)
            auto m = langDef.hexPattern.matchView(text, start, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
            if (m.hasMatch()) { pos += m.capturedLength(); return TToken(TokenType::Number, start, m.capturedLength()); }
        }
        // Use matchView to avoid QString copy (Qt 6.1+)
        auto m = langDef.numberPattern.matchView(text, start, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
        if (m.hasMatch()) { pos += m.capturedLength(); return TToken(TokenType::Number, start, m.capturedLength()); }
        pos++; return TToken(TokenType::Number, start, 1);
    }

    // 7. Separators
    if (ch == '.' || ch == u'؛') {
        pos++;
        return TToken(TokenType::Separator, pos - 1, 1, QString(ch));
    }

    // 8. Operators
    pos++;
    return TToken(TokenType::Operator, pos - 1, 1, QString(ch));
}

std::unique_ptr<LexerState> NormalState::nextState() const {
    if (pendingState) return std::move(pendingState);
    return std::make_unique<NormalState>();
}

std::unique_ptr<LexerState> NormalState::clone() const {
    auto c = std::make_unique<NormalState>();
    if (pendingState) c->pendingState = pendingState->clone();
    return c;
}

// ==================== String States ====================

StringState::StringState(const QString& delim, int id) : delimiter(delim), delimId(id) {}

TToken StringState::readToken(QStringView text, int& pos, const LanguageDefinition&) {
    int start = pos;
    while (pos < text.length()) {
        if (text[pos] == '\\') { pos = qMin(pos + 2, static_cast<int>(text.length())); continue; }
        if (text.mid(pos).startsWith(delimiter)) {
            pos += delimiter.length();
            m_terminated = true;
            return TToken(TokenType::String, start, pos - start);
        }
        pos++;
    }
    m_terminated = false;
    return TToken(TokenType::String, start, pos - start);
}

std::unique_ptr<LexerState> StringState::nextState() const {
    if (m_terminated) {
        return std::make_unique<NormalState>();
    }
    return std::make_unique<StringState>(delimiter, delimId);
}

std::unique_ptr<LexerState> StringState::clone() const {
    return std::make_unique<StringState>(delimiter, delimId);
}

// ==================== Lexer ====================

TLexer::TLexer() { finalState = StateMasks::Normal; }

QVector<TToken> TLexer::tokenize(QStringView text, int initialState) {
    QVector<TToken> tokens;
    int pos = 0;

    std::unique_ptr<LexerState> currentState;

    int type = initialState & StateMasks::TypeMask;
    int dType = initialState & StateMasks::DelimMask;

    QString delim = "\"";
    if (dType == StateMasks::Single) delim = "'";

    if (type == StateMasks::String) {
        currentState = std::make_unique<StringState>(delim, dType);
    } else {
        currentState = std::make_unique<NormalState>();
    }

    while (pos < text.length()) {
        TToken token = currentState->readToken(text, pos, langDef);

        if (token.length > 0) tokens.append(token);
        else if (pos < text.length()) pos++;

        currentState = currentState->nextState();
    }

    finalState = currentState->getStateId();
    return tokens;
}
