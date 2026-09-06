#pragma once

#include <QMainWindow>
#include "QalamTitleBar.h"

#if defined(Q_OS_WIN)
namespace QWK {
class WidgetWindowAgent;
}
#endif

class QalamWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit QalamWindow(QWidget *parent = nullptr);
    void setCustomMenuBar(QWidget *menu);

signals:
    void commandCenterClicked();

protected:
    void changeEvent(QEvent *event) override;

private:
    QalamTitleBar *m_titleBar{};
#if defined(Q_OS_WIN)
    QWK::WidgetWindowAgent *m_windowAgent{};
#endif
};
