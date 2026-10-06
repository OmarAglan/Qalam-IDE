#include "ProjectSearchService.h"
#include "QalamArabicSearch.h"
#include "QalamSearchPanel.h"
#include "QalamDocumentModel.h"
#include "QalamEditor.h"

#include <QCheckBox>
#include <QFile>
#include <QLineEdit>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

namespace {
QStringList matches(const QString &query, const QString &text)
{
    const QRegularExpression expression(
        QalamArabicSearch::diacriticInsensitivePattern(query),
        QRegularExpression::UseUnicodePropertiesOption);
    QStringList found;
    auto iterator = expression.globalMatch(text);
    while (iterator.hasNext()) found << iterator.next().captured();
    return found;
}
}

class TestArabicSearch : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void ignoresVowelMarksAndTatweelInSourceAndQuery();
    void matchesComposedAndDecomposedSpellings();
    void keepsHamzaAndMaddaSignificant();
    void escapesRegularExpressionCharacters();
    void projectSearchReportsExactOriginalRanges();
    void findPanelHighlightsMarkedSpellings();
};

void TestArabicSearch::initTestCase()
{
    qRegisterMetaType<ProjectSearchResult>();
}

void TestArabicSearch::ignoresVowelMarksAndTatweelInSourceAndQuery()
{
    QCOMPARE(matches(QStringLiteral("كتب"), QStringLiteral("كَتَبَ كتب كـتـب")),
             (QStringList{QStringLiteral("كَتَبَ"), QStringLiteral("كتب"),
                          QStringLiteral("كـتـب")}));
    // Marks typed in the query are ignored too.
    QCOMPARE(matches(QStringLiteral("كُتِب"), QStringLiteral("كتب")),
             QStringList{QStringLiteral("كتب")});
}

void TestArabicSearch::matchesComposedAndDecomposedSpellings()
{
    const QString composed = QStringLiteral("آمن");
    const QString decomposed = QStringLiteral("آمن");
    QCOMPARE(matches(composed, decomposed), QStringList{decomposed});
    QCOMPARE(matches(decomposed, composed), QStringList{composed});
    // A vowel mark may sit between the base letter and the madda.
    const QString marked = QStringLiteral("آَمن");
    QCOMPARE(matches(composed, marked), QStringList{marked});
}

void TestArabicSearch::keepsHamzaAndMaddaSignificant()
{
    QVERIFY(matches(QStringLiteral("امن"), QStringLiteral("أمن آمن")).isEmpty());
    QVERIFY(matches(QStringLiteral("أمن"), QStringLiteral("امن")).isEmpty());
}

void TestArabicSearch::escapesRegularExpressionCharacters()
{
    QCOMPARE(matches(QStringLiteral("س.ص("), QStringLiteral("سصص( س.ص(")),
             QStringList{QStringLiteral("س.ص(")});
}

void TestArabicSearch::projectSearchReportsExactOriginalRanges()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString filePath = directory.filePath(QStringLiteral("رئيسي.baa"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QStringLiteral("صحيح عَدَد = ١. عدد = عددين.\n").toUtf8());
    file.close();

    ProjectSearchService service;
    QSignalSpy finished(&service, &ProjectSearchService::searchFinished);
    ProjectSearchRequest request;
    request.rootPath = directory.path();
    request.filePaths = {filePath};
    request.query = QStringLiteral("عدد");
    request.wholeWord = true;
    request.ignoreDiacritics = true;
    service.search(request);
    QTRY_COMPARE(finished.count(), 1);

    const auto result = qvariant_cast<ProjectSearchResult>(finished.takeFirst().at(0));
    QCOMPARE(result.matches.size(), 2);
    QCOMPARE(result.matches[0].character, 5);
    QCOMPARE(result.matches[0].matchedText, QStringLiteral("عَدَد"));
    QCOMPARE(result.matches[1].matchedText, QStringLiteral("عدد"));

    // The option is part of the cache identity: turning it off finds one.
    request.ignoreDiacritics = false;
    service.search(request);
    QTRY_COMPARE(finished.count(), 1);
    QCOMPARE(qvariant_cast<ProjectSearchResult>(finished.takeFirst().at(0)).matches.size(), 1);
}

void TestArabicSearch::findPanelHighlightsMarkedSpellings()
{
    QalamDocumentModel model;
    QalamEditor editor(&model);
    editor.setPlainText(QStringLiteral("مُتَغَيِّر متغير"));
    QalamSearchPanel panel;
    panel.setEditor(&editor);
    panel.show();

    auto *input = panel.findChild<QLineEdit *>(QStringLiteral("searchInput"));
    auto *diacritics = panel.findChild<QCheckBox *>(QStringLiteral("searchIgnoreDiacritics"));
    auto *regex = panel.findChild<QCheckBox *>(QStringLiteral("searchRegex"));
    QVERIFY(input);
    QVERIFY(diacritics);
    QVERIFY(regex);

    input->setText(QStringLiteral("متغير"));
    QCOMPARE(editor.searchHighlightCount(), 1);

    diacritics->setChecked(true);
    QCOMPARE(editor.searchHighlightCount(), 2);

    // Regular expressions are literal, so the option is disabled with them.
    regex->setChecked(true);
    QVERIFY(not diacritics->isEnabled());
    editor.document()->setModified(false);
}

QTEST_MAIN(TestArabicSearch)
#include "TestArabicSearch.moc"
