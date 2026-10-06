#include "QalamBracketMatcher.h"
#include "QalamLexer.h"

#include <QHash>
#include <QTextBlock>
#include <QTextDocument>
#include <QVector>

namespace {
QChar partnerOf(QChar bracket)
{
    switch (bracket.unicode()) {
    case '(': return QLatin1Char(')');
    case ')': return QLatin1Char('(');
    case '[': return QLatin1Char(']');
    case ']': return QLatin1Char('[');
    case '{': return QLatin1Char('}');
    case '}': return QLatin1Char('{');
    default: return QChar();
    }
}

bool isOpening(QChar bracket)
{
    return bracket == QLatin1Char('(') or bracket == QLatin1Char('[') or
           bracket == QLatin1Char('{');
}

// Per-block mask of characters that are code rather than string or comment.
class CodeMask {
public:
    bool isCode(const QTextBlock &block, int offset)
    {
        auto found = m_masks.find(block.blockNumber());
        if (found == m_masks.end()) found = m_masks.insert(block.blockNumber(), build(block));
        return offset >= 0 and offset < found->size() and found->at(offset);
    }

private:
    QVector<bool> build(const QTextBlock &block)
    {
        const QString text = block.text();
        QVector<bool> mask(text.size(), true);
        // The highlighter stores the lexer's end-of-line state; an unhighlighted
        // block (-1) is treated as starting in normal code.
        const QTextBlock previous = block.previous();
        const int state = previous.isValid() ? qMax(0, previous.userState()) : 0;
        QalamLexer lexer;
        for (const QalamToken &token : lexer.tokenize(text, state)) {
            if (token.type != TokenType::String and token.type != TokenType::Comment) continue;
            const int end = qMin(static_cast<int>(text.size()), token.start + token.length);
            for (int index = qMax(0, token.start); index < end; ++index) mask[index] = false;
        }
        return mask;
    }

    QHash<int, QVector<bool>> m_masks;
};
}

QalamBracketMatcher::Match QalamBracketMatcher::find(const QTextDocument *document,
                                                     int caretPosition)
{
    Match result;
    if (not document) return result;
    CodeMask mask;

    auto bracketAt = [&](int position) {
        if (position < 0 or position >= document->characterCount() - 1) return false;
        const QTextBlock block = document->findBlock(position);
        const QChar character = document->characterAt(position);
        return not partnerOf(character).isNull() and
               mask.isCode(block, position - block.position());
    };

    if (bracketAt(caretPosition)) {
        result.bracket = caretPosition;
    } else if (bracketAt(caretPosition - 1)) {
        result.bracket = caretPosition - 1;
    } else {
        return result;
    }

    const QChar bracket = document->characterAt(result.bracket);
    const QChar partner = partnerOf(bracket);
    const int step = isOpening(bracket) ? 1 : -1;
    QTextBlock block = document->findBlock(result.bracket);
    int offset = result.bracket - block.position() + step;
    int depth = 0;

    for (int scanned = 0; block.isValid() and scanned < ScanLimit; ++scanned) {
        const QString text = block.text();
        if (offset < 0 or offset >= text.size()) {
            block = step > 0 ? block.next() : block.previous();
            if (not block.isValid()) break;
            offset = step > 0 ? 0 : block.text().size() - 1;
            continue;
        }
        const QChar character = text.at(offset);
        if ((character == bracket or character == partner) and mask.isCode(block, offset)) {
            if (character == bracket) {
                ++depth;
            } else if (depth == 0) {
                result.partner = block.position() + offset;
                return result;
            } else {
                --depth;
            }
        }
        offset += step;
    }
    return result;
}
