#pragma once

#include <QString>

// ==================== Constants & Enums ====================

namespace StateMasks {
const int TypeMask = 0xFF;
const int DelimMask = 0xFF00;

enum StateType {
    Normal = 0,
    String = 1,
    FString = 2,
    TripleString = 3,
    Interpolation = 4,
    FunctionDef = 5,
    ClassDef = 6
};

enum DelimiterType {
    None = 0,
    Single = 0x100,      // '
    Double = 0x200,      // "
    TriSingle = 0x300,   // '''
    TriDouble = 0x400    // """
};
}

enum class TokenType {
    None = 0,
    Keyword,
    BuiltinFunc,
    MagicMethod,
    Decorator,
    Number,
    String,
    FString,
    Comment,
    Function,
    ClassDef,
    Operator,
    Identifier,
    Whitespace,
    MultiLineString,
    Self,
    Error
};

struct TToken {
    TokenType type;
    int start;
    int length;
    QString value;

    TToken(TokenType t = TokenType::None, int s = 0, int l = 0, const QString& v = "")
        : type(t), start(s), length(l), value(v) {}
};
