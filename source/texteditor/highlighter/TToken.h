#pragma once

#include <QString>

// ==================== Constants & Enums ====================

namespace StateMasks {
const int TypeMask = 0xFF;
const int DelimMask = 0xFF00;

enum StateType {
    Normal = 0,
    String = 1,
    FunctionDef = 5
};

enum DelimiterType {
    None = 0,
    Single = 0x100,      // '
    Double = 0x200       // "
};
}

enum class TokenType {
    None = 0,
    Keyword,
    BuiltinFunc,
    Number,
    String,
    Comment,
    Function,
    Operator,
    Identifier,
    Whitespace,
    Error,
    Preprocessor,
    Separator,
    BooleanLiteral
};

struct TToken {
    TokenType type;
    int start;
    int length;
    QString value;

    TToken(TokenType t = TokenType::None, int s = 0, int l = 0, const QString& v = "")
        : type(t), start(s), length(l), value(v) {}
};
