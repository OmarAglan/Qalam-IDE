#pragma once

#include <QMainWindow>
#include "TTitleBar.h"

class QalamWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit QalamWindow(QWidget *parent = nullptr);
    void setCustomMenuBar(QWidget *menu);

protected:
    // Native event handling for Windows
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    
    // Custom painting/layout if needed (usually handled by central widget layout)
    // We need to insert TitleBar into the layout.

private:
    TTitleBar *m_titleBar;
    
    // Helper to setup the layout with title bar
    void initFrameless();
};
