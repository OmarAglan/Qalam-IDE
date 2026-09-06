#include "QalamWindow.h"

#include <QEvent>

#if defined(Q_OS_WIN)
#include <QWKCore/windowagentbase.h>
#include <QWKWidgets/widgetwindowagent.h>
#endif

QalamWindow::QalamWindow(QWidget *parent) : QMainWindow(parent)
{
    Qt::WindowFlags flags = Qt::Window
        | Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowMaximizeButtonHint
        | Qt::WindowCloseButtonHint;
#if not defined(Q_OS_WIN)
    flags |= Qt::FramelessWindowHint;
#endif
    setWindowFlags(flags);

#if defined(Q_OS_WIN)
    // QWindowKit maps Qalam's caption widgets to their native roles so
    // Windows can provide resize, Snap Layouts and window animations without
    // painting a second maximize/restore glyph.
    m_windowAgent = new QWK::WidgetWindowAgent(this);
    m_windowAgent->setup(this);
#endif

    m_titleBar = new QalamTitleBar(this);
    setMenuWidget(m_titleBar);

#if defined(Q_OS_WIN)
    m_windowAgent->setTitleBar(m_titleBar);
    m_windowAgent->setSystemButton(
        QWK::WindowAgentBase::Minimize, m_titleBar->minimizeButton());
    m_windowAgent->setSystemButton(
        QWK::WindowAgentBase::Maximize, m_titleBar->maximizeButton());
    m_windowAgent->setSystemButton(
        QWK::WindowAgentBase::Close, m_titleBar->closeButton());
    m_windowAgent->setHitTestVisible(
        m_titleBar->commandCenterButton(), true);
#endif

    connect(m_titleBar, &QalamTitleBar::minimizeClicked,
            this, &QMainWindow::showMinimized);
    connect(m_titleBar, &QalamTitleBar::maximizeRestoreClicked,
            this, [this]() {
                if (isMaximized()) showNormal();
                else showMaximized();
            });
    connect(m_titleBar, &QalamTitleBar::closeClicked,
            this, &QMainWindow::close);
    connect(m_titleBar, &QalamTitleBar::commandCenterClicked,
            this, &QalamWindow::commandCenterClicked);
    connect(this, &QWidget::windowTitleChanged,
            m_titleBar, &QalamTitleBar::setTitle);
    m_titleBar->setMaximizedState(isMaximized());
}

void QalamWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange and m_titleBar)
        m_titleBar->setMaximizedState(isMaximized());
}

void QalamWindow::setCustomMenuBar(QWidget *menu)
{
    if (not m_titleBar) return;

    m_titleBar->addMenuBar(menu);
#if defined(Q_OS_WIN)
    if (m_windowAgent and menu)
        m_windowAgent->setHitTestVisible(menu, true);
#endif
}
