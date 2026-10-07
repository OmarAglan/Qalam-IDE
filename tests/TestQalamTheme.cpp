#include "QalamTheme.h"

#include <QPushButton>
#include <QTest>

class TestQalamTheme : public QObject
{
    Q_OBJECT
private slots:
    void cleanup();
    void focusRingIsAlwaysPresent();
    void highContrastAddsOverlayOnlyWhenSelected();
    void applyFollowsSystemContrast();
};

void TestQalamTheme::cleanup()
{
    QalamTheme::instance().setType(QalamTheme::Type::Dark);
}

void TestQalamTheme::focusRingIsAlwaysPresent()
{
    const QString sheet = QalamTheme::instance().globalStyleSheet();
    QVERIFY(sheet.contains(QStringLiteral("QPushButton:focus")));
    QVERIFY(sheet.contains(QStringLiteral("QAbstractItemView:focus")));
}

void TestQalamTheme::highContrastAddsOverlayOnlyWhenSelected()
{
    QalamTheme &theme = QalamTheme::instance();
    theme.setType(QalamTheme::Type::Dark);
    QVERIFY(not theme.globalStyleSheet().contains(QStringLiteral("#ffd700")));

    theme.setType(QalamTheme::Type::HighContrast);
    const QString sheet = theme.globalStyleSheet();
    QVERIFY(sheet.contains(QStringLiteral("border: 2px solid #ffd700")));
    // The overlay comes last so it wins over the dark palette it repeats.
    QVERIFY(sheet.lastIndexOf(QStringLiteral("#ffd700"))
            > sheet.lastIndexOf(QStringLiteral("QPushButton:focus {")));
}

void TestQalamTheme::applyFollowsSystemContrast()
{
    auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    QVERIFY(app);
    QalamTheme::instance().apply(app);
    const QalamTheme::Type expected = QalamTheme::systemPrefersHighContrast()
        ? QalamTheme::Type::HighContrast : QalamTheme::Type::Dark;
    QCOMPARE(QalamTheme::instance().currentType(), expected);
    QCOMPARE(app->styleSheet(), QalamTheme::instance().globalStyleSheet());
}

QTEST_MAIN(TestQalamTheme)
#include "TestQalamTheme.moc"
