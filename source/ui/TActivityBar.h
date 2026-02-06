#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMap>

/**
 * @brief Activity Bar - Vertical icon bar for switching views (like VSCode)
 * 
 * In RTL mode, this appears on the RIGHT side of the window.
 * Contains icon buttons for: Explorer, Search, Settings
 * Settings button is pushed to the bottom.
 */
class TActivityBar : public QWidget
{
    Q_OBJECT

public:
    enum class ViewType {
        Explorer,
        Search,
        Settings,
        None
    };

    explicit TActivityBar(QWidget *parent = nullptr);
    ~TActivityBar() = default;

    ViewType currentView() const { return m_currentView; }
    void setCurrentView(ViewType view);

signals:
    void viewChanged(TActivityBar::ViewType view);
    void viewToggled(TActivityBar::ViewType view, bool visible);

private slots:
    void onButtonClicked();

private:
    void setupUi();
    QPushButton* createButton(const QString &iconPath, const QString &tooltip, ViewType view);
    void updateButtonStates();
    void applyStyles();

    ViewType m_currentView = ViewType::None;
    QMap<ViewType, QPushButton*> m_buttons;
    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_topLayout = nullptr;
    QVBoxLayout *m_bottomLayout = nullptr;
};

Q_DECLARE_METATYPE(TActivityBar::ViewType)
