#include "TActivityBar.h"
#include "QalamTheme.h"
#include "Constants.h"
#include <QStyle>

TActivityBar::TActivityBar(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    applyStyles();
}

void TActivityBar::setupUi()
{
    using namespace Constants;
    
    setFixedWidth(Layout::ActivityBarWidth);
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // Top section - main view buttons
    QWidget *topSection = new QWidget();
    m_topLayout = new QVBoxLayout(topSection);
    m_topLayout->setContentsMargins(0, 4, 0, 0);
    m_topLayout->setSpacing(0);
    
    // Create main buttons
    auto explorerBtn = createButton(":/icons/resources/explorer.svg", ExplorerLabel, ViewType::Explorer);
    auto searchBtn = createButton(":/icons/resources/search.svg", SearchLabel, ViewType::Search);
    
    m_topLayout->addWidget(explorerBtn);
    m_topLayout->addWidget(searchBtn);
    
    // Bottom section - settings (pushed to bottom)
    QWidget *bottomSection = new QWidget();
    m_bottomLayout = new QVBoxLayout(bottomSection);
    m_bottomLayout->setContentsMargins(0, 0, 0, 4);
    m_bottomLayout->setSpacing(0);
    
    auto settingsBtn = createButton(":/icons/resources/settings.svg", SettingsLabel, ViewType::Settings);
    m_bottomLayout->addWidget(settingsBtn);
    
    // Assemble layout
    m_mainLayout->addWidget(topSection);
    m_mainLayout->addStretch(1);  // Push settings to bottom
    m_mainLayout->addWidget(bottomSection);
    
    // Start with no view selected (sidebar is hidden by default)
    m_currentView = ViewType::None;
    updateButtonStates();
}

QPushButton* TActivityBar::createButton(const QString &iconPath, const QString &tooltip, ViewType view)
{
    using namespace Constants;
    
    QPushButton *btn = new QPushButton(this);
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(Layout::ActivityBarIconSize, Layout::ActivityBarIconSize));
    btn->setFixedSize(Layout::ActivityBarButtonSize, Layout::ActivityBarButtonSize);
    btn->setToolTip(tooltip);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("viewType", QVariant::fromValue(view));
    
    connect(btn, &QPushButton::clicked, this, &TActivityBar::onButtonClicked);
    
    m_buttons[view] = btn;
    return btn;
}

void TActivityBar::onButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    
    ViewType clickedView = btn->property("viewType").value<ViewType>();
    
    // Settings is special - doesn't toggle sidebar, just emits signal
    if (clickedView == ViewType::Settings) {
        emit viewToggled(clickedView, true);
        return;
    }
    
    if (clickedView == m_currentView) {
        // Toggle sidebar visibility (clicking active button hides sidebar)
        emit viewToggled(clickedView, false);
        m_currentView = ViewType::None;
        updateButtonStates();
    } else {
        // Switch to new view
        setCurrentView(clickedView);
        emit viewToggled(clickedView, true);
    }
}

void TActivityBar::setCurrentView(ViewType view)
{
    if (m_currentView == view) return;
    
    ViewType oldView = m_currentView;
    m_currentView = view;
    updateButtonStates();
    
    if (view != ViewType::None) {
        emit viewChanged(view);
    }
}

void TActivityBar::updateButtonStates()
{
    for (auto it = m_buttons.begin(); it != m_buttons.end(); ++it) {
        it.value()->setChecked(it.key() == m_currentView);
    }
}

void TActivityBar::applyStyles()
{
    setStyleSheet(QalamTheme::activityBarStyleSheet());
}
