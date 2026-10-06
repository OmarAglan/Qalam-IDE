#include "QalamBracketMatcher.h"
#include "QalamDocumentModel.h"
#include "QalamEditor.h"

#include <QTest>
#include <QTextDocument>

class TestBracketMatcher : public QObject
{
    Q_OBJECT
private slots:
    void matchesNestedBracketsInBothDirections();
    void ignoresBracketsInsideStringsAndComments();
    void matchesAcrossLines();
    void reportsAnUnmatchedBracket();
    void ignoresCaretsAwayFromCodeBrackets();
    void editorDecoratesBothBrackets();
};

void TestBracketMatcher::matchesNestedBracketsInBothDirections()
{
    // ا0 ط1 ب2 ع3 (4 س5 [6 ١7 ]8 ،9 _10 {11 ص12 }13 )14 .15
    QTextDocument document(QStringLiteral("اطبع(س[١]، {ص})."));
    auto match = QalamBracketMatcher::find(&document, 4);
    QCOMPARE(match.bracket, 4);
    QCOMPARE(match.partner, 14);

    // The caret after a closing bracket finds it through the previous character.
    match = QalamBracketMatcher::find(&document, 15);
    QCOMPARE(match.bracket, 14);
    QCOMPARE(match.partner, 4);

    match = QalamBracketMatcher::find(&document, 11);
    QCOMPARE(match.partner, 13);
    match = QalamBracketMatcher::find(&document, 8);
    QCOMPARE(match.bracket, 8);
    QCOMPARE(match.partner, 6);
}

void TestBracketMatcher::ignoresBracketsInsideStringsAndComments()
{
    const QString text = QStringLiteral("اطبع(\")\" // )\n).");
    QTextDocument document(text);
    const auto match = QalamBracketMatcher::find(&document, 4);
    QCOMPARE(match.bracket, 4);
    QCOMPARE(match.partner, text.lastIndexOf(QLatin1Char(')')));
}

void TestBracketMatcher::matchesAcrossLines()
{
    const QString text = QStringLiteral("صحيح الرئيسية() {\n    إرجع ٠.\n}");
    QTextDocument document(text);
    const int open = text.indexOf(QLatin1Char('{'));
    const auto match = QalamBracketMatcher::find(&document, open);
    QCOMPARE(match.partner, text.size() - 1);
}

void TestBracketMatcher::reportsAnUnmatchedBracket()
{
    QTextDocument document(QStringLiteral("اطبع(س."));
    const auto match = QalamBracketMatcher::find(&document, 4);
    QVERIFY(match.hasBracket());
    QVERIFY(not match.isMatched());
}

void TestBracketMatcher::ignoresCaretsAwayFromCodeBrackets()
{
    QTextDocument document(QStringLiteral("س = \"(\"."));
    QVERIFY(not QalamBracketMatcher::find(&document, 1).hasBracket());
    // The only bracket is inside the string literal.
    QVERIFY(not QalamBracketMatcher::find(&document, 5).hasBracket());
    QVERIFY(not QalamBracketMatcher::find(nullptr, 0).hasBracket());
}

void TestBracketMatcher::editorDecoratesBothBrackets()
{
    QalamDocumentModel model;
    QalamEditor editor(&model);
    editor.setPlainText(QStringLiteral("اطبع(س)."));
    QTextCursor cursor = editor.textCursor();
    cursor.setPosition(4);
    editor.setTextCursor(cursor);

    QList<int> decorated;
    for (const QTextEdit::ExtraSelection &selection : editor.extraSelections()) {
        if (selection.format.property(QTextFormat::UserProperty).toString() ==
            QStringLiteral("qalam.bracket"))
            decorated << selection.cursor.selectionStart();
    }
    std::sort(decorated.begin(), decorated.end());
    QCOMPARE(decorated, (QList<int>{4, 6}));
    editor.document()->setModified(false);
}

QTEST_MAIN(TestBracketMatcher)
#include "TestBracketMatcher.moc"
