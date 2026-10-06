#include "QalamTextPosition.h"

#include <QTextBlock>
#include <QTextDocument>

namespace QalamTextPosition {

namespace {
// Length of the UTF-8 sequence introduced by lead, or 0 for a byte that
// cannot start a sequence.
int utf8SequenceLength(unsigned char lead)
{
    if (lead < 0x80) return 1;
    if (lead >= 0xC2 and lead <= 0xDF) return 2;
    if (lead >= 0xE0 and lead <= 0xEF) return 3;
    if (lead >= 0xF0 and lead <= 0xF4) return 4;
    return 0;
}
}

int utf16OffsetForUtf8Byte(const QByteArray &utf8, qsizetype byteOffset)
{
    if (byteOffset < 0 or byteOffset > utf8.size()) return -1;

    int utf16 = 0;
    qsizetype index = 0;
    while (index < byteOffset) {
        const int length = utf8SequenceLength(static_cast<unsigned char>(utf8.at(index)));
        if (length == 0 or index + length > utf8.size()) return -1;
        for (int continuation = 1; continuation < length; ++continuation) {
            if ((static_cast<unsigned char>(utf8.at(index + continuation)) & 0xC0) != 0x80)
                return -1;
        }
        index += length;
        // Four-byte sequences are supplementary-plane scalars: two UTF-16 units.
        utf16 += length == 4 ? 2 : 1;
    }
    // An offset that lands inside a sequence has no exact UTF-16 position.
    return index == byteOffset ? utf16 : -1;
}

LineCharacter lineCharacterForUtf16Offset(const QString &text, int offset)
{
    if (offset < 0 or offset > text.size()) return {};

    LineCharacter result{0, 0};
    int lineStart = 0;
    for (int index = 0; index < offset; ++index) {
        const QChar character = text.at(index);
        if (character == QLatin1Char('\r')) {
            // CRLF is one line ending; an offset between CR and LF belongs to
            // the line the pair terminates.
            if (index + 1 < text.size() and text.at(index + 1) == QLatin1Char('\n')) {
                if (index + 1 == offset) {
                    result.character = index - lineStart;
                    return result;
                }
                ++index;
            }
            ++result.line;
            lineStart = index + 1;
        } else if (character == QLatin1Char('\n') or
                   character == QChar::ParagraphSeparator or
                   character == QChar::LineSeparator) {
            ++result.line;
            lineStart = index + 1;
        }
    }
    result.character = offset - lineStart;
    return result;
}

LineCharacter lineCharacterForUtf8Byte(const QByteArray &utf8, qsizetype byteOffset)
{
    const int offset = utf16OffsetForUtf8Byte(utf8, byteOffset);
    if (offset < 0) return {};
    return lineCharacterForUtf16Offset(QString::fromUtf8(utf8), offset);
}

int documentPosition(const QTextDocument *document, int line, int character)
{
    if (not document) return 0;
    const int lastLine = qMax(0, document->blockCount() - 1);
    const QTextBlock block = document->findBlockByNumber(qBound(0, line, lastLine));
    if (not block.isValid()) return 0;

    const QString text = block.text();
    int bounded = qBound(0, character, static_cast<int>(text.size()));
    if (bounded > 0 and bounded < text.size() and
        text.at(bounded - 1).isHighSurrogate() and text.at(bounded).isLowSurrogate()) {
        --bounded;
    }
    return block.position() + bounded;
}

LineCharacter lineCharacterForPosition(const QTextDocument *document, int position)
{
    if (not document) return {};
    const int bounded = qBound(0, position, qMax(0, document->characterCount() - 1));
    const QTextBlock block = document->findBlock(bounded);
    if (not block.isValid()) return {};
    return {block.blockNumber(), bounded - block.position()};
}

} // namespace QalamTextPosition
