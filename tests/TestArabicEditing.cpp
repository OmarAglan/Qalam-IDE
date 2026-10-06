#include "QalamDocumentModel.h"
#include "QalamEditor.h"

#include <QApplication>
#include <QTest>

namespace {
// U+1D538: one supplementary-plane character, two UTF-16 code units.
QString supplementary()
{
    return QString::fromUcs4(U"\U0001D538");
}

void place(QalamEditor *editor, int position)
{
    QTextCursor cursor = editor->textCursor();
    cursor.setPosition(position);
    editor->setTextCursor(cursor);
}

void press(QalamEditor *editor, Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    QTest::keyClick(editor, key, modifiers);
}
}

// Keyboard-only Arabic editing contract. Lines are right-to-left, so the arrow
// keys move visually: Left advances logically and Right moves back.
class TestArabicEditing : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void arrowsMoveVisuallyInRightToLeftLines();
    void wordMovesStopAtArabicPunctuationAndDigits();
    void homeEndAndSelectionAreLogical();
    void backspaceRemovesOneMarkAndDeleteRemovesTheCluster();
    void caretMovesNeverSplitClustersOrSurrogates();
    void diagnosticRangesAreExactAfterSupplementaryCharacters();
    void emptyDiagnosticRangesStillCoverOneCharacter();

private:
    QalamDocumentModel *m_model{};
    QalamEditor *m_editor{};
};

void TestArabicEditing::init()
{
    m_model = new QalamDocumentModel(this);
    m_editor = new QalamEditor(m_model);
    m_editor->resize(600, 300);
    m_editor->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_editor));
}

void TestArabicEditing::cleanup()
{
    m_editor->document()->setModified(false);
    delete m_editor;
    m_editor = nullptr;
    delete m_model;
    m_model = nullptr;
}

void TestArabicEditing::arrowsMoveVisuallyInRightToLeftLines()
{
    m_editor->setPlainText(QStringLiteral("متغير س."));
    place(m_editor, 0);
    press(m_editor, Qt::Key_Left);
    QCOMPARE(m_editor->textCursor().position(), 1);
    press(m_editor, Qt::Key_Right);
    QCOMPARE(m_editor->textCursor().position(), 0);
    // Moving "back" from the logical start stays put.
    press(m_editor, Qt::Key_Right);
    QCOMPARE(m_editor->textCursor().position(), 0);
}

void TestArabicEditing::wordMovesStopAtArabicPunctuationAndDigits()
{
    // م0 ت1 غ2 ي3 ر4 _5 س6 _7 =8 _9 ١10 ٢11 ٣12 ،13 _14 ص15 .16
    m_editor->setPlainText(QStringLiteral("متغير س = ١٢٣، ص."));
    place(m_editor, 0);
    const QList<int> stops{6, 8, 10, 15, 16};
    for (const int stop : stops) {
        press(m_editor, Qt::Key_Left, Qt::ControlModifier);
        QCOMPARE(m_editor->textCursor().position(), stop);
    }
}

void TestArabicEditing::homeEndAndSelectionAreLogical()
{
    m_editor->setPlainText(QStringLiteral("اطبع(\"مرحبا\").\nصحيح ع = ٥."));
    place(m_editor, 0);
    press(m_editor, Qt::Key_End);
    QCOMPARE(m_editor->textCursor().position(), 14);
    press(m_editor, Qt::Key_Home);
    QCOMPARE(m_editor->textCursor().position(), 0);
    press(m_editor, Qt::Key_Down);
    QCOMPARE(m_editor->textCursor().position(), 15);
    press(m_editor, Qt::Key_End, Qt::ShiftModifier);
    QCOMPARE(m_editor->textCursor().selectedText(), QStringLiteral("صحيح ع = ٥."));
}

void TestArabicEditing::backspaceRemovesOneMarkAndDeleteRemovesTheCluster()
{
    // Baa with shadda and fatha: one grapheme cluster of three code points.
    const QString cluster = QStringLiteral("بَّ");
    m_editor->setPlainText(cluster + QStringLiteral("ت"));
    place(m_editor, cluster.size());
    press(m_editor, Qt::Key_Backspace);
    QCOMPARE(m_editor->toPlainText(), cluster.chopped(1) + QStringLiteral("ت"));

    m_editor->setPlainText(cluster + QStringLiteral("ت"));
    place(m_editor, 0);
    press(m_editor, Qt::Key_Delete);
    QCOMPARE(m_editor->toPlainText(), QStringLiteral("ت"));

    m_editor->setPlainText(QStringLiteral("س") + supplementary() + QStringLiteral("ع"));
    place(m_editor, 3);
    press(m_editor, Qt::Key_Backspace);
    QCOMPARE(m_editor->toPlainText(), QStringLiteral("سع"));
}

void TestArabicEditing::caretMovesNeverSplitClustersOrSurrogates()
{
    const QString cluster = QStringLiteral("بَّ");
    m_editor->setPlainText(cluster + QStringLiteral("ت"));
    place(m_editor, 0);
    press(m_editor, Qt::Key_Left);
    QCOMPARE(m_editor->textCursor().position(), cluster.size());

    m_editor->setPlainText(QStringLiteral("س") + supplementary() + QStringLiteral("ع"));
    place(m_editor, 1);
    press(m_editor, Qt::Key_Left);
    QCOMPARE(m_editor->textCursor().position(), 3);
}

void TestArabicEditing::diagnosticRangesAreExactAfterSupplementaryCharacters()
{
    // Line 2 is "    𝔸 متغير." and the diagnostic covers exactly "متغير":
    // one-based UTF-16 columns 8 through 13 (exclusive end).
    m_editor->setPlainText(QStringLiteral("صحيح.\n    ") + supplementary() + QStringLiteral(" متغير."));
    QalamEditor::Diagnostic diagnostic;
    diagnostic.line = 2;
    diagnostic.column = 8;
    diagnostic.endLine = 2;
    diagnostic.endColumn = 13;
    diagnostic.message = QStringLiteral("رمز غير معرف");
    const QPair<int, int> range = m_editor->diagnosticRange(diagnostic);
    QTextCursor cursor(m_editor->document());
    cursor.setPosition(range.first);
    cursor.setPosition(range.second, QTextCursor::KeepAnchor);
    QCOMPARE(cursor.selectedText(), QStringLiteral("متغير"));
}

void TestArabicEditing::emptyDiagnosticRangesStillCoverOneCharacter()
{
    m_editor->setPlainText(QStringLiteral("س = ") + supplementary() + QStringLiteral("."));
    QalamEditor::Diagnostic diagnostic;
    diagnostic.line = 1;
    diagnostic.column = 5;
    diagnostic.endLine = 1;
    diagnostic.endColumn = 5;
    QPair<int, int> range = m_editor->diagnosticRange(diagnostic);
    QCOMPARE(range.first, 4);
    // The single visible character is the whole surrogate pair.
    QCOMPARE(range.second, 6);

    // At the end of a line the previous character is used instead.
    diagnostic.column = 8;
    diagnostic.endColumn = 0;
    range = m_editor->diagnosticRange(diagnostic);
    QCOMPARE(range.first, 6);
    QCOMPARE(range.second, 7);
}

QTEST_MAIN(TestArabicEditing)
#include "TestArabicEditing.moc"
