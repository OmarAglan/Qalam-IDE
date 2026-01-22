#pragma once

#include "TSyntaxDefinition.h"

#include <QString>
#include <QSet>
#include <QTextCharFormat>
#include <memory>

#include "TToken.h"

// ==================== State Machine Interfaces ====================

class LexerState {
public:
    virtual ~LexerState() = default;
    virtual TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) = 0;
    virtual std::unique_ptr<LexerState> nextState() const = 0;
    // Added clone for robust state copying (prototype pattern)
    virtual std::unique_ptr<LexerState> clone() const = 0;
    virtual int getStateId() const { return StateMasks::Normal; }
};

// 1. Normal Code State
class NormalState : public LexerState {
public:
    TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) override;
    std::unique_ptr<LexerState> nextState() const override;
    std::unique_ptr<LexerState> clone() const override;

    mutable std::unique_ptr<LexerState> pendingState;
    int getStateId() const override { return StateMasks::Normal; }
};

// 2. String State (Single or Double)
class StringState : public LexerState {
    QString delimiter;
    int delimId;
public:
    StringState(const QString& delim, int id);
    TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) override;
    std::unique_ptr<LexerState> nextState() const override;
    std::unique_ptr<LexerState> clone() const override;
    int getStateId() const override { return StateMasks::String | delimId; }
};

// 3. Triple String State
class TripleStringState : public LexerState {
    QString delimiter;
    int delimId;
public:
    TripleStringState(const QString& delim, int id);
    TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) override;
    std::unique_ptr<LexerState> nextState() const override;
    std::unique_ptr<LexerState> clone() const override;
    mutable std::unique_ptr<LexerState> pendingState;
    int getStateId() const override { return StateMasks::TripleString | delimId; }
};

// 4. F-String State
class FStringState : public LexerState {
    QString delimiter;
    int delimId;
public:
    FStringState(const QString& delim, int id);
    TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) override;
    std::unique_ptr<LexerState> nextState() const override;
    std::unique_ptr<LexerState> clone() const override;

    mutable std::unique_ptr<LexerState> pendingState;
    int getStateId() const override { return StateMasks::FString | delimId; }
};

// 5. Interpolation State { ... }
class InterpolationState : public LexerState {
    QString parentDelimiter;
    int parentDelimId;
    int braceBalance;
    // Holds the state of the code *inside* the interpolation (e.g., Normal, String)
    std::unique_ptr<LexerState> innerState;
public:
    InterpolationState(const QString& parentDelim, int parentId, int balance = 1, std::unique_ptr<LexerState> inner = nullptr);
    TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) override;
    std::unique_ptr<LexerState> nextState() const override;
    std::unique_ptr<LexerState> clone() const override;

    mutable std::unique_ptr<LexerState> pendingState;
    int getStateId() const override { return StateMasks::Interpolation | parentDelimId; }
};

// 6. Function Definition State (captures name after 'دالة')
class FunctionDefState : public LexerState {
public:
    TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) override;
    std::unique_ptr<LexerState> nextState() const override;
    std::unique_ptr<LexerState> clone() const override;

    mutable std::unique_ptr<LexerState> pendingState;
    int getStateId() const override { return StateMasks::FunctionDef; }
};

// 7. Class Definition State (captures name after 'class')
class ClassDefState : public LexerState {
public:
    TToken readToken(const QString& text, int& pos, const LanguageDefinition& langDef) override;
    std::unique_ptr<LexerState> nextState() const override;
    std::unique_ptr<LexerState> clone() const override;

    mutable std::unique_ptr<LexerState> pendingState;
    int getStateId() const override { return StateMasks::ClassDef; }
};

// ==================== Lexer ====================

class TLexer {
public:
    TLexer();
    QVector<TToken> tokenize(const QString& text, int initialState);
    int getFinalState() const { return finalState; }

private:
    LanguageDefinition langDef{};
    int finalState{};
};
