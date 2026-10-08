#include "QalamEditor.h"
#include "QalamDocumentModel.h"

#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QSettings>
#include <QScopeGuard>

class TestEditorSemanticRequests : public QObject
{
    Q_OBJECT

private slots:
    void requestsSignaturesFromArabicEditingTriggers();
    void requestsCompletionInsideIncludePaths();
    void keepsOrdinaryCompletionsOutOfIncludePaths();
    void keepsPairsAndSelectionsInLogicalDocumentOrder();
    void expandsAndShrinksCompilerOwnedSelections();
    void keepsLiteralBracesOutOfLocalFolding();
    void rendersInlayHintsWithoutChangingSourceText();
    void rejectsStaleCompletionDocumentation();
    void keepsCompletionVisibleDuringSemanticHighlighting();
};

void TestEditorSemanticRequests::keepsCompletionVisibleDuringSemanticHighlighting()
{
    QalamEditor editor;
    editor.setFilePath(QStringLiteral("رئيسي.baa"));
    editor.setPlainText(QStringLiteral("صحيح قيمة.\nاط"));
    editor.moveCursor(QTextCursor::End);
    editor.resize(800, 600);
    editor.show();
    QCoreApplication::processEvents();

    BaaCompletionItem item;
    item.label = QStringLiteral("اطبع");
    item.newText = item.label;
    item.startLine = 1;
    item.endLine = 1;
    item.endCharacter = 2;
    item.protocolItem = QJsonObject{{"label", item.label}};
    QSignalSpy requests(&editor, &QalamEditor::completionResolveRequested);
    QSignalSpy cancellations(&editor, &QalamEditor::completionResolveCancelled);
    editor.showLanguageCompletions({item}, 1, 2);
    QVERIFY(editor.hasVisibleCompletion());
    QVERIFY(not requests.isEmpty());
    const QString selectionId = requests.last().at(3).toString();
    const QString source = editor.toPlainText();
    const int revision = editor.documentModel()->sourceRevision();
    QSignalSpy sourceChanges(editor.documentModel(), &QalamDocumentModel::sourceTextChanged);
    editor.setSemanticTokens({BaaSemanticToken{0, 5, 4, QStringLiteral("variable"), {}}});
    QCoreApplication::processEvents();
    QCOMPARE(editor.toPlainText(), source);
    QCOMPARE(editor.documentModel()->sourceRevision(), revision);
    QCOMPARE(sourceChanges.size(), 0);
    QVERIFY(editor.hasVisibleCompletion());
    QCOMPARE(cancellations.size(), 0);
    editor.showCompletionDocumentation(selectionId, QStringLiteral("توثيق الدالة"));
    auto *completer = editor.findChild<QCompleter *>();
    auto *footer = completer->popup()->findChild<QTextBrowser *>(
        QStringLiteral("completionDocumentation"));
    QVERIFY(footer->toPlainText().contains(QStringLiteral("توثيق الدالة")));
    editor.clearSemanticTokens();
    QCoreApplication::processEvents();
    QVERIFY(editor.hasVisibleCompletion());
    QCOMPARE(editor.documentModel()->sourceRevision(), revision);
    QCOMPARE(sourceChanges.size(), 0);

    // Both editor groups share one source revision, including undo/redo.
    QalamEditor otherView(editor.documentModel());
    otherView.moveCursor(QTextCursor::End);
    otherView.insertPlainText(QStringLiteral("ب"));
    QCOMPARE(sourceChanges.size(), 1);
    QCOMPARE(editor.documentModel()->sourceRevision(), revision + 1);
    QVERIFY(not editor.hasVisibleCompletion());
    editor.showCompletionDocumentation(selectionId, QStringLiteral("رد قديم"));
    QVERIFY(not footer->toPlainText().contains(QStringLiteral("رد قديم")));
    otherView.undo();
    QCOMPARE(sourceChanges.size(), 2);
    QCOMPARE(editor.toPlainText(), source);
    QCOMPARE(editor.documentModel()->sourceRevision(), revision + 2);
    otherView.redo();
    QCOMPARE(sourceChanges.size(), 3);
    QCOMPARE(editor.documentModel()->sourceRevision(), revision + 3);
}

void TestEditorSemanticRequests::rejectsStaleCompletionDocumentation()
{
    QTemporaryDir settingsDirectory;
    const QSettings::Format previousFormat = QSettings::defaultFormat();
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
    const auto restoreSettings = qScopeGuard([previousFormat] {
        QSettings::setDefaultFormat(previousFormat);
    });
    QalamEditor editor;
    editor.setFilePath(QStringLiteral("رئيسي.baa"));
    editor.setPlainText(QStringLiteral("اط"));
    editor.moveCursor(QTextCursor::End);
    editor.resize(800, 600);
    editor.show();
    BaaCompletionItem first;
    first.label = QStringLiteral("اطبع");
    first.newText = first.label;
    first.protocolItem = QJsonObject{{"label", first.label}, {"data", QJsonObject{{"id", 1}}}};
    first.documentation = QStringLiteral("توثيق مباشر");
    BaaCompletionItem second = first;
    second.label = QStringLiteral("اطبع_سطر");
    second.newText = second.label;
    second.protocolItem = QJsonObject{{"label", second.label}, {"data", QJsonObject{{"id", 2}}}};
    QSignalSpy requests(&editor, &QalamEditor::completionResolveRequested);
    QSignalSpy cancellations(&editor, &QalamEditor::completionResolveCancelled);
    editor.showLanguageCompletions({first, second}, 0, 2);
    QCoreApplication::processEvents();
    QVERIFY(editor.hasVisibleCompletion());
    QVERIFY(not requests.isEmpty());
    auto *completer = editor.findChild<QCompleter *>();
    QVERIFY(completer);
    auto *popup = static_cast<QalamCompletionPopup *>(completer->popup());
    auto *footer = popup->findChild<QTextBrowser *>(QStringLiteral("completionDocumentation"));
    QVERIFY(footer);
    QVERIFY(footer->toPlainText().contains(QStringLiteral("توثيق مباشر")));
    const QString previousId = requests.last().at(3).toString();
    popup->setCurrentIndex(completer->completionModel()->index(1, 0));
    const QString selectedId = requests.last().at(3).toString();
    QVERIFY(previousId != selectedId);
    QVERIFY(not cancellations.isEmpty());
    editor.showCompletionDocumentation(selectedId, QStringLiteral("توثيق الاختيار الحالي"));
    editor.showCompletionDocumentation(previousId, QStringLiteral("توثيق قديم"));
    QVERIFY(footer->toPlainText().contains(QStringLiteral("توثيق الاختيار الحالي")));
    QVERIFY(not footer->toPlainText().contains(QStringLiteral("توثيق قديم")));
    QCOMPARE(editor.toPlainText(), QStringLiteral("اط"));
    popup->hide();
    editor.showCompletionDocumentation(selectedId, QStringLiteral("بعد الإغلاق"));
    QVERIFY(not footer->toPlainText().contains(QStringLiteral("بعد الإغلاق")));
    editor.showLanguageCompletions({first, second}, 0, 2);
    const QString editId = requests.last().at(3).toString();
    editor.insertPlainText(QStringLiteral("ب"));
    editor.showCompletionDocumentation(editId, QStringLiteral("بعد التعديل"));
    QVERIFY(not editor.hasVisibleCompletion());
    QVERIFY(not footer->toPlainText().contains(QStringLiteral("بعد التعديل")));
    first.endCharacter = 3;
    second.endCharacter = 3;
    editor.showLanguageCompletions({first, second}, 0, 3);
    const QString cursorId = requests.last().at(3).toString();
    editor.moveCursor(QTextCursor::Start);
    editor.moveCursor(QTextCursor::End);
    editor.showCompletionDocumentation(cursorId, QStringLiteral("بعد تحريك المؤشر"));
    QVERIFY(not editor.hasVisibleCompletion());

    editor.showLanguageCompletions({first, second}, 0, 3);
    QTest::keyClick(popup, Qt::Key_Down);
    const QString acceptId = requests.last().at(3).toString();
    editor.showCompletionDocumentation(acceptId, QStringLiteral("توثيق للقراءة فقط"));
    const QString selectedText = popup->currentIndex().data(Qt::EditRole).toString();
    QTest::keyClick(popup, Qt::Key_Return);
    QCOMPARE(editor.toPlainText(), selectedText);
    QVERIFY(not editor.hasVisibleCompletion());
}

void TestEditorSemanticRequests::requestsCompletionInsideIncludePaths()
{
    QalamEditor editor;
    editor.setFilePath(QStringLiteral("رئيسي.باء"));
    editor.setPlainText(QStringLiteral("#تضمين "));
    QTextCursor cursor = editor.textCursor();
    cursor.movePosition(QTextCursor::End);
    editor.setTextCursor(cursor);
    QSignalSpy requests(&editor, &QalamEditor::completionRequested);

    QTest::keyClick(&editor, Qt::Key_QuoteDbl);
    QVERIFY(not requests.isEmpty());
    const int afterQuote = requests.size();
    editor.insertPlainText(QStringLiteral("واجهات"));
    QTest::keyClick(&editor, Qt::Key_Slash);
    QVERIFY(requests.size() > afterQuote);
    QCOMPARE(requests.constLast().at(1).toInt(), 0);
    QCOMPARE(requests.constLast().at(2).toInt(),
             editor.textCursor().positionInBlock());
}

void TestEditorSemanticRequests::keepsOrdinaryCompletionsOutOfIncludePaths()
{
    QalamEditor editor;
    editor.setFilePath(QStringLiteral("رئيسي.باء"));
    editor.setPlainText(QStringLiteral("#تضمين \"\""));
    QTextCursor cursor = editor.textCursor();
    cursor.setPosition(QStringLiteral("#تضمين \"").size());
    editor.setTextCursor(cursor);

    BaaCompletionItem keyword;
    keyword.label = QStringLiteral("اختر");
    keyword.newText = keyword.label;
    keyword.detail = QStringLiteral("كلمة محجوزة");
    keyword.kind = 14;
    keyword.startLine = 0;
    keyword.startCharacter = cursor.positionInBlock();
    keyword.endLine = 0;
    keyword.endCharacter = cursor.positionInBlock();
    editor.showLanguageCompletions({keyword}, 0, cursor.positionInBlock());

    CompletionModel *model = editor.findChild<CompletionModel *>();
    QVERIFY(model != nullptr);
    QCOMPARE(model->rowCount(), 0);
    QVERIFY(not editor.hasVisibleCompletion());

    BaaCompletionItem header = keyword;
    header.label = QStringLiteral("المكتبة_القياسية.رأسباء");
    header.newText = header.label;
    header.detail = QStringLiteral("ملف يمكن تضمينه");
    header.kind = 17;
    editor.showLanguageCompletions({header}, 0, cursor.positionInBlock());
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->index(0, 0).data().toString(), header.label);
}

void TestEditorSemanticRequests::requestsSignaturesFromArabicEditingTriggers()
{
    QTemporaryDir workspace(QDir::temp().filePath(QStringLiteral("qalam-semantic-مسار-XXXXXX")));
    QVERIFY(workspace.isValid());

    QalamEditor editor;
    editor.setFilePath(workspace.filePath(QStringLiteral("رئيسي.baa")));
    editor.setPlainText(QStringLiteral("اجمع"));
    QTextCursor cursor = editor.textCursor();
    cursor.movePosition(QTextCursor::End);
    editor.setTextCursor(cursor);

    QSignalSpy requests(&editor, &QalamEditor::signatureHelpRequested);

    QTest::keyClicks(&editor, QStringLiteral("("));
    QCOMPARE(editor.toPlainText(), QStringLiteral("اجمع()"));
    QCOMPARE(requests.size(), 1);
    QCOMPARE(requests.constLast().at(1).toInt(), 0);
    QCOMPARE(requests.constLast().at(2).toInt(), 5);

    QKeyEvent arabicComma(QEvent::KeyPress, Qt::Key_Comma,
                          Qt::NoModifier, QString(QChar(0x060c)));
    QCoreApplication::sendEvent(&editor, &arabicComma);
    QCOMPARE(editor.toPlainText(), QStringLiteral("اجمع(،)"));
    QCOMPARE(requests.size(), 2);
    QCOMPARE(requests.constLast().at(2).toInt(), 6);

    const QString beforeShortcut = editor.toPlainText();
    QTest::keyClick(&editor, Qt::Key_Space,
                    Qt::ControlModifier | Qt::ShiftModifier);
    QCOMPARE(requests.size(), 3);
    QCOMPARE(editor.toPlainText(), beforeShortcut);
}

void TestEditorSemanticRequests::keepsPairsAndSelectionsInLogicalDocumentOrder()
{
    QalamEditor editor;
    editor.setPlainText(QStringLiteral("مرحبا"));
    QTextCursor cursor = editor.textCursor();
    cursor.setPosition(0);
    cursor.setPosition(5, QTextCursor::KeepAnchor);
    editor.setTextCursor(cursor);

    QTest::keyClicks(&editor, QStringLiteral("("));
    QCOMPARE(editor.toPlainText(), QStringLiteral("(مرحبا)"));
    QCOMPARE(editor.textCursor().selectedText(), QStringLiteral("مرحبا"));

    editor.setPlainText(QStringLiteral("اجمع()"));
    cursor = editor.textCursor();
    cursor.setPosition(5);
    editor.setTextCursor(cursor);
    QTest::keyClicks(&editor, QStringLiteral(")"));
    QCOMPARE(editor.toPlainText(), QStringLiteral("اجمع()"));
    QCOMPARE(editor.textCursor().position(), 6);

    editor.setPlainText(QStringLiteral("باء"));
    cursor = editor.textCursor();
    cursor.setPosition(0);
    cursor.setPosition(3, QTextCursor::KeepAnchor);
    editor.setTextCursor(cursor);
    QTest::keyClicks(&editor, QStringLiteral("\""));
    QCOMPARE(editor.toPlainText(), QStringLiteral("\"باء\""));
    QCOMPARE(editor.textCursor().selectedText(), QStringLiteral("باء"));
}

void TestEditorSemanticRequests::expandsAndShrinksCompilerOwnedSelections()
{
    QalamEditor editor;
    editor.setFilePath(QStringLiteral("رئيسي.baa"));
    const QString source =
        QStringLiteral("صحيح الرئيسية() {\n    صحيح س = ١.\n}\n");
    editor.setPlainText(source);
    const QString declarationLine = source.split('\n').at(1);
    const int character = declarationLine.indexOf(QStringLiteral("س"));
    QTextCursor cursor(editor.document()->findBlockByNumber(1));
    cursor.setPosition(cursor.block().position() + character);
    editor.setTextCursor(cursor);

    QSignalSpy requests(&editor, &QalamEditor::selectionRangeRequested);
    QTest::keyClick(&editor, Qt::Key_Right,
                    Qt::ShiftModifier | Qt::AltModifier);
    QCOMPARE(requests.size(), 1);
    QCOMPARE(requests.first().at(1).toInt(), 1);
    QCOMPARE(requests.first().at(2).toInt(), character);

    editor.applySemanticSelectionRanges({
        {1, character, 1, character + 1},
        {1, 4, 1, static_cast<int>(declarationLine.size())},
        {0, 16, 2, 1},
        {0, 0, 3, 0}
    }, 1, character);
    QCOMPARE(editor.textCursor().selectedText(), QStringLiteral("س"));

    QTest::keyClick(&editor, Qt::Key_Right,
                    Qt::ShiftModifier | Qt::AltModifier);
    QCOMPARE(editor.textCursor().selectedText(), declarationLine.mid(4));
    QCOMPARE(requests.size(), 1);

    QTest::keyClick(&editor, Qt::Key_Left,
                    Qt::ShiftModifier | Qt::AltModifier);
    QCOMPARE(editor.textCursor().selectedText(), QStringLiteral("س"));
    QTest::keyClick(&editor, Qt::Key_Left,
                    Qt::ShiftModifier | Qt::AltModifier);
    QVERIFY(not editor.textCursor().hasSelection());

    QTest::keyClick(&editor, Qt::Key_Right,
                    Qt::ShiftModifier | Qt::AltModifier);
    QTest::keyClick(&editor, Qt::Key_Right,
                    Qt::ShiftModifier | Qt::AltModifier);
    QTest::keyClick(&editor, Qt::Key_Right,
                    Qt::ShiftModifier | Qt::AltModifier);
    QTest::keyClick(&editor, Qt::Key_Right,
                    Qt::ShiftModifier | Qt::AltModifier);
    QCOMPARE(editor.textCursor().selectionStart(), 0);
    QCOMPARE(editor.textCursor().selectionEnd(), static_cast<int>(source.size()));

    QTest::keyClick(&editor, Qt::Key_Right,
                    Qt::ShiftModifier | Qt::AltModifier);
    QCOMPARE(requests.size(), 1);
}

void TestEditorSemanticRequests::keepsLiteralBracesOutOfLocalFolding()
{
    QalamEditor editor;
    editor.setFilePath(QStringLiteral("رئيسي.baa"));
    editor.setPlainText(QStringLiteral(
        "صحيح الرئيسية() {\n"
        "    نص قيمة = \"{\nليس نطاقا}\".\n"
        "}\n"));
    editor.useLocalFoldingRanges();
    QCOMPARE(editor.foldingRangeCount(), 1);

    editor.setFoldingRanges({
        {0, 16, 3, 1, QStringLiteral("region")}
    });
    QCOMPARE(editor.foldingRangeCount(), 1);
}

void TestEditorSemanticRequests::rendersInlayHintsWithoutChangingSourceText()
{
    QalamEditor editor;
    editor.resize(640, 240);
    const QString source = QStringLiteral(
        "صحيح اجمع(صحيح أول، صحيح ثان) { إرجع أول + ثان. }\n"
        "صحيح الرئيسية() { إرجع اجمع(١، ٢). }\n");
    editor.setPlainText(source);

    QImage withoutHints(editor.size(), QImage::Format_ARGB32_Premultiplied);
    withoutHints.fill(Qt::transparent);
    editor.render(&withoutHints);

    editor.setInlayHints({
        {1, 27, QStringLiteral("أول:"), QStringLiteral("أول"), true, true},
        {1, 30, QStringLiteral("ثان:"), QStringLiteral("ثان"), true, true}
    });
    QCOMPARE(editor.inlayHintCount(), 2);
    QCOMPARE(editor.property("qalam.inlayHintCount").toInt(), 2);
    QVERIFY(editor.accessibleDescription().contains(QStringLiteral("2")));
    QCOMPARE(editor.toPlainText(), source);

    QImage rendered(editor.size(), QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::transparent);
    editor.render(&rendered);
    QVERIFY(not rendered.isNull());
    QVERIFY(rendered != withoutHints);
    QCOMPARE(editor.toPlainText(), source);

    editor.clearInlayHints();
    QCOMPARE(editor.inlayHintCount(), 0);
    QCOMPARE(editor.property("qalam.inlayHintCount").toInt(), 0);
    QVERIFY(editor.accessibleDescription().isEmpty());
}

QTEST_MAIN(TestEditorSemanticRequests)
#include "TestEditorSemanticRequests.moc"
