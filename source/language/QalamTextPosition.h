#pragma once

#include <QByteArray>
#include <QString>

class QTextDocument;

// The single conversion point between the position units Qalam receives and
// the positions Qt edits. Baa contracts report exact UTF-8 byte offsets, LSP
// reports zero-based UTF-16 line/character pairs, and QTextDocument stores
// UTF-16 code units with one paragraph separator per line. Every conversion
// is logical: it never depends on the visual direction of a right-to-left line.
namespace QalamTextPosition {

struct LineCharacter {
    int line{-1};
    int character{-1};

    bool isValid() const { return line >= 0 and character >= 0; }
};

// UTF-16 offset of an exact UTF-8 byte offset in utf8, or -1 when the offset
// is outside the text or inside a multi-byte sequence. Malformed UTF-8 also
// yields -1 because no exact UTF-16 position exists for it.
int utf16OffsetForUtf8Byte(const QByteArray &utf8, qsizetype byteOffset);

// Zero-based line and UTF-16 character for a UTF-16 offset into raw file
// text. CRLF, CR, and LF each end one line, matching LSP and QTextDocument.
LineCharacter lineCharacterForUtf16Offset(const QString &text, int offset);

// Exact line/character for a UTF-8 byte offset into raw file bytes.
LineCharacter lineCharacterForUtf8Byte(const QByteArray &utf8, qsizetype byteOffset);

// Document position for a zero-based line and UTF-16 character. The line and
// character are clamped to the document, and a character that would split a
// surrogate pair is moved to the start of that pair.
int documentPosition(const QTextDocument *document, int line, int character);

// Zero-based line and UTF-16 character for a document position.
LineCharacter lineCharacterForPosition(const QTextDocument *document, int position);

} // namespace QalamTextPosition
