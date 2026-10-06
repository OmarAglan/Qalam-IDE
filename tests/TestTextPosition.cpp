#include "QalamTextPosition.h"

#include <QTest>
#include <QTextDocument>

namespace {
// U+1D538 MATHEMATICAL DOUBLE-STRUCK CAPITAL A: four UTF-8 bytes, two UTF-16 units.
QString supplementary()
{
    return QString::fromUcs4(U"\U0001D538");
}
}

class TestTextPosition : public QObject
{
    Q_OBJECT
private slots:
    void convertsArabicAndSupplementaryBytes();
    void rejectsOffsetsInsideSequencesAndMalformedUtf8();
    void countsLinesForEveryLineEnding();
    void mapsBytesToLineAndCharacter();
    void clampsDocumentPositionsWithoutSplittingSurrogates();
    void roundTripsDocumentPositions();
};

void TestTextPosition::convertsArabicAndSupplementaryBytes()
{
    // "س" is two bytes, the supplementary character four, "ع" two.
    const QByteArray utf8 = (QStringLiteral("س") + supplementary() + QStringLiteral("ع")).toUtf8();
    QCOMPARE(utf8.size(), 8);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, 0), 0);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, 2), 1);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, 6), 3);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, 8), 4);
}

void TestTextPosition::rejectsOffsetsInsideSequencesAndMalformedUtf8()
{
    const QByteArray utf8 = (QStringLiteral("س") + supplementary()).toUtf8();
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, 1), -1);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, 4), -1);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, -1), -1);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(utf8, utf8.size() + 1), -1);

    const QByteArray truncated = QByteArray("\xD8", 1);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(truncated, 1), -1);
    const QByteArray overlong = QByteArray("\xC0\x80", 2);
    QCOMPARE(QalamTextPosition::utf16OffsetForUtf8Byte(overlong, 2), -1);
}

void TestTextPosition::countsLinesForEveryLineEnding()
{
    const QString text = QStringLiteral("أ\r\nب\rج\nد");
    auto position = QalamTextPosition::lineCharacterForUtf16Offset(text, text.size());
    QCOMPARE(position.line, 3);
    QCOMPARE(position.character, 1);

    // Between CR and LF still belongs to the end of the first line.
    position = QalamTextPosition::lineCharacterForUtf16Offset(text, 2);
    QCOMPARE(position.line, 0);
    QCOMPARE(position.character, 1);

    position = QalamTextPosition::lineCharacterForUtf16Offset(text, 3);
    QCOMPARE(position.line, 1);
    QCOMPARE(position.character, 0);

    QVERIFY(not QalamTextPosition::lineCharacterForUtf16Offset(text, -1).isValid());
}

void TestTextPosition::mapsBytesToLineAndCharacter()
{
    const QString line2 = supplementary() + QStringLiteral(" متغير");
    const QByteArray utf8 = (QStringLiteral("صحيح.\r\n") + line2).toUtf8();
    // The identifier starts after the supplementary character and a space.
    const qsizetype byte = utf8.indexOf(QStringLiteral("متغير").toUtf8());
    const auto position = QalamTextPosition::lineCharacterForUtf8Byte(utf8, byte);
    QCOMPARE(position.line, 1);
    QCOMPARE(position.character, 3);
    QVERIFY(not QalamTextPosition::lineCharacterForUtf8Byte(utf8, byte + 1).isValid());
}

void TestTextPosition::clampsDocumentPositionsWithoutSplittingSurrogates()
{
    QTextDocument document(QStringLiteral("س") + supplementary() + QStringLiteral("\nع"));
    QCOMPARE(QalamTextPosition::documentPosition(&document, 0, 1), 1);
    // Character 2 is between the two surrogates; it moves to the pair start.
    QCOMPARE(QalamTextPosition::documentPosition(&document, 0, 2), 1);
    QCOMPARE(QalamTextPosition::documentPosition(&document, 0, 3), 3);
    QCOMPARE(QalamTextPosition::documentPosition(&document, 0, 99), 3);
    QCOMPARE(QalamTextPosition::documentPosition(&document, 1, 1), 5);
    QCOMPARE(QalamTextPosition::documentPosition(&document, 9, 0), 4);
    QCOMPARE(QalamTextPosition::documentPosition(&document, -3, 0), 0);
    QCOMPARE(QalamTextPosition::documentPosition(nullptr, 0, 0), 0);
}

void TestTextPosition::roundTripsDocumentPositions()
{
    QTextDocument document(QStringLiteral("دالة\n    ") + supplementary() + QStringLiteral("س."));
    for (int position = 0; position < document.characterCount() - 1; ++position) {
        const auto lineCharacter = QalamTextPosition::lineCharacterForPosition(&document, position);
        QVERIFY(lineCharacter.isValid());
        const int back = QalamTextPosition::documentPosition(
            &document, lineCharacter.line, lineCharacter.character);
        // The only lossy positions are inside a surrogate pair.
        if (document.characterAt(position).isLowSurrogate())
            QCOMPARE(back, position - 1);
        else
            QCOMPARE(back, position);
    }
}

QTEST_MAIN(TestTextPosition)
#include "TestTextPosition.moc"
