#include "QalamWindow.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QWindow>
#include <QMenuBar>
#include <QPushButton>
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")

QalamWindow::QalamWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. Setup Title Bar
    m_titleBar = new TTitleBar(this);
    
    connect(m_titleBar, &TTitleBar::minimizeClicked, this, &QMainWindow::showMinimized);
    connect(m_titleBar, &TTitleBar::maximizeRestoreClicked, [this]() {
        if (isMaximized()) showNormal();
        else showMaximized();
    });
    connect(m_titleBar, &TTitleBar::closeClicked, this, &QMainWindow::close);
    
    // Set menu widget to title bar (QMainWindow feature) or add to layout?
    // QMainWindow::setMenuWidget() puts it at top. Perfect.
    setMenuWidget(m_titleBar);
    
    // 2. Window Flags for Custom Frame
    // We do NOT use FramelessWindowHint because we want native snap/shadow.
    // We rely on WM_NCCALCSIZE to hide the frame.
    // However, Qt sometimes fights this.
    // A stable cross-platform way + Native Snap in Qt 6:
    // Use WS_THICKFRAME | WS_CAPTION but handle NCCALCSIZE.
    
    // For simplicity and stability in this iteration, IF we are on MinGW/Qt6,
    // we can try the pure Qt approach first:
    // setWindowFlags(Qt::FramelessWindowHint);
    // But then we lose shadows and snap (Win+Arrows).
    
    // Enabling Native Events for Snap/Shadow:
    // We keep standard flags.
}

void QalamWindow::setCustomMenuBar(QWidget *menu) {
    if (m_titleBar) {
        m_titleBar->addMenuBar(menu);
    }
}

bool QalamWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        HWND hwnd = msg->hwnd;

        switch (msg->message) {
            case WM_NCCALCSIZE: {
                // Return 0 to indicate we handle the client area (remove title bar/borders visuals)
                // but keep the logic of thick frame for resizing.
                if (msg->wParam == TRUE) {
                    *result = 0;
                    return true;
                }
                break;
            }
            case WM_NCHITTEST: {
                // Handle resizing zones
                long x = GET_X_LPARAM(msg->lParam);
                long y = GET_Y_LPARAM(msg->lParam);
                
                POINT pt = {x, y};
                ScreenToClient(hwnd, &pt);

                const int borderWidth = 8; // Resize border width
                
                RECT rw;
                GetClientRect(hwnd, &rw);
                
                // Check borders
                bool left = pt.x < borderWidth;
                bool right = pt.x >= rw.right - borderWidth;
                bool top = pt.y < borderWidth;
                bool bottom = pt.y >= rw.bottom - borderWidth;
                
                if (top && left) { *result = HTTOPLEFT; return true; }
                if (top && right) { *result = HTTOPRIGHT; return true; }
                if (bottom && left) { *result = HTBOTTOMLEFT; return true; }
                if (bottom && right) { *result = HTBOTTOMRIGHT; return true; }
                if (left) { *result = HTLEFT; return true; }
                if (right) { *result = HTRIGHT; return true; }
                if (bottom) { *result = HTBOTTOM; return true; }
                if (top) { *result = HTTOP; return true; }
                
                // Title Bar Dragging
                // Check if point is within title bar bounds
                // Note: setMenuWidget puts titlebar at (0,0) usually.
                if (m_titleBar && m_titleBar->geometry().contains(QPoint(pt.x, pt.y))) {
                    // Exclude buttons and menu bar from drag
                    QWidget *child = m_titleBar->childAt(pt.x, pt.y);
                    if (child) {
                        // If the child is a button or is part of a menu bar, let it handle the event
                        bool isButton = qobject_cast<QPushButton*>(child);
                        bool isMenuBar = qobject_cast<QMenuBar*>(child) || (child->parent() && qobject_cast<QMenuBar*>(child->parentWidget()));
                        
                        if (isButton || isMenuBar) {
                            return false; // Let Qt handle it (HTCLIENT result will be returned by default)
                        }
                    }
                    
                    // Otherwise, it's a draggable title bar area
                    *result = HTCAPTION;
                    return true;
                }
                
                break;
            }
            case WM_SIZE: {
                // Update maximize button state
                if (m_titleBar) {
                    m_titleBar->setMaximizedState(windowState() & Qt::WindowMaximized);
                }
                break;
            }
        }
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}
