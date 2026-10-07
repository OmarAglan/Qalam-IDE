#pragma once

#include "ThemeManager.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QCloseEvent>
#include <QStackedWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidget>

enum class QalamToolKind;

class QalamSettings : public QWidget {
    Q_OBJECT
public:
    explicit QalamSettings(QWidget* parent = nullptr);

    QVector<std::shared_ptr<SyntaxTheme>> getAvailableThemes() const { return ThemeManager::getAvailableThemes(); }

    QComboBox *getThemeCombo() const;

protected:
    void closeEvent(QCloseEvent* event) override;

signals:
    void fontSizeChanged(int size);
    void fontTypeChanged(QString font);
    void highlighterThemeChanged(int themeIdx);
    void toolPathsChanged();
    void analysisDelayChanged(int milliseconds);
    void shortcutsChanged();


private:
    void createCategory(const QString &name, const QString &iconName,
                        const QString &description);
    void createAppearancePage(QVBoxLayout*);
    void createToolsPage(QVBoxLayout*);
    void createShortcutsPage(QVBoxLayout*);
    void refreshShortcutConflicts();
    void saveShortcut(int row);
    void filterShortcuts(const QString &text);
    void refreshLanguageServerHealth();
    void chooseToolPath(QalamToolKind kind, QLineEdit *editor);
    void refreshToolHealth();
    void saveToolPaths();

    QListWidget* categoryList{};
    QStackedWidget* stackedWidget{};

    QSpinBox* fontSpin{};
    QComboBox* fontCombo{};
    QComboBox* themeCombo{};
    QSpinBox* analysisDelaySpin{};
    QTableWidget* shortcutTable{};
    QLabel* shortcutConflictLabel{};
    QLineEdit* baaPathEdit{};
    QLineEdit* takweenPathEdit{};
    QLineEdit* nazmPathEdit{};
    QLineEdit* languageServerPathEdit{};
    QLabel* baaStatusLabel{};
    QLabel* takweenStatusLabel{};
    QLabel* nazmStatusLabel{};
    QLabel* languageServerStatusLabel{};

};
