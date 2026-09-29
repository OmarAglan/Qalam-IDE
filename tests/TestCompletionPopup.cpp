#include "texteditor/autocomplete/AutoCompleteUI.h"

#include <QTextBrowser>
#include <QSignalSpy>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTest>

class TestCompletionPopup : public QObject
{
    Q_OBJECT

private slots:
    void showsTheSelectedCompletionDescription();
    void rendersDocumentationAndOpensLinksOnlyOnClick();
};

void TestCompletionPopup::showsTheSelectedCompletionDescription()
{
    CompletionModel model;
    CompletionItem item;
    item.label = QStringLiteral("اطبع");
    item.completion = item.label;
    item.description = QStringLiteral("يطبع النص أو القيمة في نافذة المخرجات.");
    item.type = Function;
    model.updateData({item});

    QalamCompletionPopup popup;
    popup.resize(340, 190);
    popup.setModel(&model);
    popup.setCurrentIndex(model.index(0, 0));
    QCoreApplication::processEvents();

    QTextBrowser *footer = popup.findChild<QTextBrowser *>(
        QStringLiteral("completionDocumentation"));
    QVERIFY(footer != nullptr);
    QVERIFY(footer->toPlainText().contains(item.description));
    QVERIFY(footer->height() >= 72);
    QVERIFY(footer->geometry().top() >= popup.contentsRect().top());
    QVERIFY(footer->geometry().bottom() <= popup.contentsRect().bottom());
    QCOMPARE(popup.property("qalam.completionFooterHeight").toInt(),
             footer->height());
}

void TestCompletionPopup::rendersDocumentationAndOpensLinksOnlyOnClick()
{
    CompletionModel model;
    CompletionItem item;
    item.label = QStringLiteral("اطبع");
    item.completion = item.label;
    item.description = QStringLiteral("دالة ← عدم");
    item.type = Function;
    item.documentation = QJsonObject{{"kind", "markdown"}, {"value", QStringLiteral(
        "يطبع **النص أو القيمة** في نافذة المخرجات.\n\n"
        "[دليل باء](https://example.com/baa)\n\n"
        "```baa\nاطبع(\"مرحبا بالعالم\").\n```\n\n"
        "لا يغيّر النص الأصلي في المحرر.\n\n") +
        QStringLiteral("شرح إضافي للمعاملات والقيم المعادة.\n\n").repeated(12)}};
    model.updateData({item});
    QalamCompletionPopup popup;
    popup.setItemDelegate(new QalamModernCompletionDelegate(&popup));
    popup.resize(420, 330);
    popup.setModel(&model);
    popup.setCurrentIndex(model.index(0, 0));
    QSignalSpy links(&popup, &QalamCompletionPopup::documentationLinkActivated);
    popup.show();
    QCoreApplication::processEvents();
    auto *footer = popup.findChild<QTextBrowser *>(QStringLiteral("completionDocumentation"));
    QVERIFY(footer);
    QVERIFY(footer->toPlainText().contains(QStringLiteral("النص أو القيمة")));
    QVERIFY(not footer->toPlainText().contains(QStringLiteral("**")));
    QCOMPARE(footer->layoutDirection(), Qt::RightToLeft);
    QVERIFY(footer->verticalScrollBar()->maximum() > 0);
    QVERIFY(popup.viewport()->height() >= 96);
    QVERIFY(not footer->openLinks());
    QVERIFY(not footer->openExternalLinks());
    QCOMPARE(links.size(), 0);
    const QTextCursor link = footer->document()->find(QStringLiteral("دليل باء"));
    QVERIFY(not link.isNull());
    footer->setTextCursor(link);
    footer->ensureCursorVisible();
    QRect linkRect = footer->cursorRect(link);
    QPoint linkPoint = linkRect.center();
    // Locate the actual shaped Arabic anchor, independent of glyph direction.
    bool found = false;
    for (int x = 0; x < footer->viewport()->width(); ++x) {
        const QPoint point(x, linkPoint.y());
        if (not footer->anchorAt(point).isEmpty()) {
            linkPoint = point;
            found = true;
            break;
        }
    }
    QVERIFY(found);
    QTest::mouseClick(footer->viewport(), Qt::LeftButton, Qt::NoModifier, linkPoint);
    QCOMPARE(links.size(), 1);
    QCOMPARE(links.first().first().toUrl(), QUrl(QStringLiteral("https://example.com/baa")));
    QVERIFY(footer->source().isEmpty());
    footer->anchorClicked(QUrl(QStringLiteral("file:///private.txt")));
    footer->anchorClicked(QUrl(QStringLiteral("command:run")));
    QCOMPARE(links.size(), 1);

    const QString screenshot = qEnvironmentVariable("QALAM_COMPLETION_SCREENSHOT");
    if (not screenshot.isEmpty()) {
        footer->moveCursor(QTextCursor::Start);
        footer->verticalScrollBar()->setValue(0);
        QVERIFY(popup.grab().save(screenshot));
    }
    popup.showDocumentation(QStringLiteral("<b>نص حرفي</b>"));
    QVERIFY(footer->toPlainText().contains(QStringLiteral("<b>نص حرفي</b>")));
    popup.showDocumentation(QJsonObject{{"kind", "plaintext"}, {"value", "**literal**"}});
    QVERIFY(footer->toPlainText().contains(QStringLiteral("**literal**")));
    const QVariant resource = footer->document()->resource(
        QTextDocument::ImageResource, QUrl(QStringLiteral("file:///private.png")));
    QCOMPARE(resource.metaType().id(), QMetaType::QByteArray);
    QVERIFY(resource.toByteArray().isEmpty());
}

QTEST_MAIN(TestCompletionPopup)
#include "TestCompletionPopup.moc"
