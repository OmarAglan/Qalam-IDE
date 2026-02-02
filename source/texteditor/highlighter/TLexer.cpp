#include "TLexer.h"



// ==================== Helpers ====================

static QPair<int, QString> checkStringStart(QStringView text, int pos) {
    int idx = pos;
    while (idx < text.length() && idx < pos + 2 && text[idx].isLetter()) {
        idx++;
    }
    for (int end = idx; end >= pos; --end) {
        if (end >= text.length()) continue;
        QChar ch = text[end];
        if (ch == '"' || ch == '\'') {
            QString prefix = text.mid(pos, end - pos).toString().toLower();
            bool valid = true;
            for(QChar c : prefix) if(!QString("م").contains(c)) valid = false;
            if(valid) return {end - pos, QString(ch)};
        }
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

    // 2. Comments (Baa uses // for single-line comments, NOT #)
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
            // It's a valid preprocessor directive - highlight the directive part
            return TToken(TokenType::Preprocessor, start, pos - start, directive);
        }

        // Unknown directive starting with # - treat as error/operator
        return TToken(TokenType::Operator, start, pos - start, directive);
    }

    // 3. Strings
    auto strCheck = checkStringStart(text, pos);
    if (strCheck.first != -1) {
        int start = pos;
        int prefixLen = strCheck.first;
        QString quote = strCheck.second;
        
        pos += prefixLen;
        pos += quote.length(); // Single or Double quote (no triple in Baa)

        int delimId = (quote == "\"") ? StateMasks::Double : StateMasks::Single;
        pendingState = std::make_unique<StringState>(quote, delimId);
        return TToken(TokenType::String, start, pos - start);
    }

    // 4. Identifiers
    if (ch.isLetter() || ch == '_') {
        int start = pos;
        while (pos < text.length() && (text[pos].isLetterOrNumber() || text[pos] == '_')) pos++;
        QStringView wordView = text.mid(start, pos - start);
        QString word = wordView.toString();

        if (word == "صواب" || word == "خطأ") return TToken(TokenType::BooleanLiteral, start, pos - start, word);
        if (langDef.keywordSet.contains(word)) return TToken(TokenType::Keyword, start, pos - start, word);
        if (langDef.builtinSet.contains(word)) return TToken(TokenType::BuiltinFunc, start, pos - start, word);
        if (langDef.preprocessorSet.contains(word)) return TToken(TokenType::Preprocessor, start, pos - start, word);

        // Check for function call pattern 'func('
        int next = pos;
        while(next < text.length() && text[next].isSpace()) next++;
        if (next < text.length() && text[next] == '(') {
            return TToken(TokenType::Function, start, pos - start, word);
        }

        return TToken(TokenType::Identifier, start, pos - start, word);
    }

    // 5. Numbers
    if (ch.isDigit()) {
        int start = pos;
        if (ch == '0' && pos + 1 < text.length() && text.mid(pos, 2).compare(u"0x", Qt::CaseInsensitive) == 0) {
            auto m = langDef.hexPattern.match(text.toString(), start, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
            if (m.hasMatch()) { pos += m.capturedLength(); return TToken(TokenType::Number, start, m.capturedLength()); }
        }
        auto m = langDef.numberPattern.match(text.toString(), start, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
        if (m.hasMatch()) { pos += m.capturedLength(); return TToken(TokenType::Number, start, m.capturedLength()); }
        pos++; return TToken(TokenType::Number, start, 1);
    }

    if (ch == '.' || ch == u'؛') {
        pos++;
        return TToken(TokenType::Separator, pos - 1, 1, QString(ch));
    }

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

// ==================== Function Definition State ====================
TToken FunctionDefState::readToken(QStringView text, int& pos, const LanguageDefinition& langDef) {
    if (pos >= text.length()) return TToken(TokenType::None, pos, 0);

    // Skip spaces
    if (text[pos].isSpace()) {
        int start = pos;
        while(pos < text.length() && text[pos].isSpace()) pos++;
        return TToken(TokenType::Whitespace, start, pos - start);
    }

    // Capture Name
    if (text[pos].isLetter() || text[pos] == '_') {
        int start = pos;
        while(pos < text.length() && (text[pos].isLetterOrNumber() || text[pos] == '_')) pos++;

        QString name = text.mid(start, pos - start).toString();

        // Done with definition, go back to normal
        pendingState = std::make_unique<NormalState>();
        return TToken(TokenType::Function, start, pos - start, name);
    }

    // Abort if we hit something unexpected
    pendingState = std::make_unique<NormalState>();
    pos++;
    return TToken(TokenType::Operator, pos-1, 1, QString(text[pos-1]));
}

std::unique_ptr<LexerState> FunctionDefState::nextState() const {
    if(pendingState) return std::move(pendingState);
    return std::make_unique<FunctionDefState>();
}

std::unique_ptr<LexerState> FunctionDefState::clone() const {
    auto c = std::make_unique<FunctionDefState>();
    if(pendingState) c->pendingState = pendingState->clone();
    return c;
}

// ==================== String States ====================

StringState::StringState(const QString& delim, int id) : delimiter(delim), delimId(id) {}

TToken StringState::readToken(QStringView text, int& pos, const LanguageDefinition&) {
    int start = pos;
    while (pos < text.length()) {
        if (text[pos] == '\\') { pos += 2; continue; }
        if (text.mid(pos).startsWith(delimiter)) {
            pos += delimiter.length();
            return TToken(TokenType::String, start, pos - start);
        }
        pos++;
    }
    return TToken(TokenType::String, start, pos - start);
}

std::unique_ptr<LexerState> StringState::nextState() const {
    return std::make_unique<NormalState>();
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

    switch(type) {
    case StateMasks::String: currentState = std::make_unique<StringState>(delim, dType); break;
    case StateMasks::FunctionDef: currentState = std::make_unique<FunctionDefState>(); break;
    default: currentState = std::make_unique<NormalState>(); break;
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
