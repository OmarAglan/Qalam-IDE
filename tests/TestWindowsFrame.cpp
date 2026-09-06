#include "QalamTitleBar.h"
#include "QalamWindow.h"
#include "Constants.h"

#include <QApplication>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include <windows.h>
#include <windowsx.h>

namespace {

LPARAM nativePosition(HWND hwnd, QWidget *window, const QPoint &clientPoint)
{
    const qreal scale = window->devicePixelRatioF();
    POINT point{qRound(clientPoint.x() * scale), qRound(clientPoint.y() * scale)};
    ClientToScreen(hwnd, &point);
    return MAKELPARAM(point.x, point.y);
}

LRESULT hitTest(HWND hwnd, QWidget *window, const QPoint &clientPoint)
{
    return SendMessageW(hwnd, WM_NCHITTEST, 0,
                        nativePosition(hwnd, window, clientPoint));
}

void showFrame(QalamWindow &window)
{
    window.show();
    // CMake's Windows process launcher can request SW_HIDE for the first
    // window. This test needs a real, exposed HWND, including under CTest.
    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    if (not IsWindowVisible(hwnd)) {
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        GetStartupInfoW(&startup);
        qInfo() << "Exposing frame for native test; startup flags:"
                << startup.dwFlags << "show command:" << startup.wShowWindow;
        ShowWindow(hwnd, SW_SHOW);
    }
}

}

class TestWindowsFrame final : public QObject
{
    Q_OBJECT

private slots:
    void exposesNativeMoveResizeAndSnapContracts();
    void captionButtonTogglesWindowState();
    void nativeSystemCommandsUpdateCaption();
};

void TestWindowsFrame::exposesNativeMoveResizeAndSnapContracts()
{
    QalamWindow window;
    window.setMinimumSize(Constants::Layout::WindowMinWidth,
                          Constants::Layout::WindowMinHeight);
    window.resize(1100, 720);
    showFrame(window);
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd);
    const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    QVERIFY2(style & WS_THICKFRAME,
             "Qalam must retain the native resizable frame on Windows");
    // QWindowKit retains WS_CAPTION for native animations. Verify that the
    // client area replaces the native title bar, rather than testing a flag
    // that does not establish whether Windows actually paints a caption.
    RECT frame{};
    QVERIFY(GetWindowRect(hwnd, &frame));
    POINT clientOrigin{};
    QVERIFY(ClientToScreen(hwnd, &clientOrigin));
    QVERIFY(clientOrigin.y - frame.top
            < GetSystemMetricsForDpi(SM_CYCAPTION, GetDpiForWindow(hwnd)));
    QVERIFY2(style & WS_MAXIMIZEBOX,
             "Qalam must retain the native maximize/Snap contract");
    QVERIFY(style & WS_MINIMIZEBOX);
    QVERIFY2(not (style & WS_SYSMENU),
             "DWM must not paint native caption glyphs over Qalam's buttons");

    MINMAXINFO limits{};
    SendMessageW(hwnd, WM_GETMINMAXINFO, 0,
                 reinterpret_cast<LPARAM>(&limits));
    const UINT dpi = qMax<UINT>(GetDpiForWindow(hwnd), 96);
    QVERIFY(limits.ptMinTrackSize.x
            <= MulDiv(500, static_cast<int>(dpi), 96));
    RECT client{};
    QVERIFY(GetClientRect(hwnd, &client));
    const LONG horizontalFrame = (frame.right - frame.left) - client.right;
    QCOMPARE(limits.ptMinTrackSize.x,
             static_cast<LONG>(MulDiv(Constants::Layout::WindowMinWidth,
                                      static_cast<int>(dpi), 96)) + horizontalFrame);

    auto *titleBar = window.findChild<QalamTitleBar*>();
    QVERIFY(titleBar);
    auto *commandCenter = titleBar->findChild<QPushButton*>(
        QStringLiteral("commandCenterButton"));
    auto *maximizeButton = titleBar->findChild<QPushButton*>(
        QStringLiteral("maximizeButton"));
    QVERIFY(commandCenter);
    QVERIFY(maximizeButton);

    // The native side border lies outside the Qt client area.
    QCOMPARE(SendMessageW(hwnd, WM_NCHITTEST, 0,
                          MAKELPARAM(frame.left + 1, (frame.top + frame.bottom) / 2)),
             static_cast<LRESULT>(HTLEFT));
    QCOMPARE(hitTest(hwnd, &window,
                     titleBar->mapTo(&window,
                                     QPoint(120, titleBar->height() / 2))),
             static_cast<LRESULT>(HTCAPTION));
    QCOMPARE(hitTest(hwnd, &window,
                     maximizeButton->mapTo(&window,
                                           maximizeButton->rect().center())),
             static_cast<LRESULT>(HTMAXBUTTON));

    // A command-centre click remains a normal client click; its custom button
    // switches to QWindow::startSystemMove only after the drag threshold.
    const LRESULT commandHit = hitTest(
        hwnd, &window,
        commandCenter->mapTo(&window, commandCenter->rect().center()));
    QVERIFY(commandHit != HTCAPTION);
    QVERIFY(commandHit != HTLEFT);
    QVERIFY(commandHit != HTRIGHT);
}

void TestWindowsFrame::captionButtonTogglesWindowState()
{
    QalamWindow window;
    window.setMinimumSize(Constants::Layout::WindowMinWidth,
                          Constants::Layout::WindowMinHeight);
    window.resize(1000, 680);
    showFrame(window);
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd);

    auto *titleBar = window.findChild<QalamTitleBar*>();
    QVERIFY(titleBar);
    auto *maximizeButton = titleBar->maximizeButton();
    QVERIFY(maximizeButton);
    QSignalSpy clicked(maximizeButton, &QPushButton::clicked);
    QCOMPARE(maximizeButton->toolTip(), QStringLiteral("تكبير"));

    // QWindowKit reads GetAsyncKeyState when forwarding non-client input.
    // SendMessage alone cannot emulate a physical press. Check the Qt action
    // here, native hit regions above, and real pointer routing in visual QA.
    QTest::mouseClick(maximizeButton, Qt::LeftButton);
    QTRY_VERIFY(window.isMaximized());
    QCOMPARE(clicked.count(), 1);
    QCOMPARE(maximizeButton->toolTip(), QStringLiteral("استعادة"));

    QTest::mouseClick(maximizeButton, Qt::LeftButton);
    QTRY_VERIFY(not window.isMaximized());
    QCOMPARE(clicked.count(), 2);
    QCOMPARE(maximizeButton->toolTip(), QStringLiteral("تكبير"));
    QVERIFY(not (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_SYSMENU));
}

void TestWindowsFrame::nativeSystemCommandsUpdateCaption()
{
    QalamWindow window;
    window.resize(1000, 680);
    showFrame(window);
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    auto *titleBar = window.findChild<QalamTitleBar*>();
    QVERIFY(titleBar);

    // Snap, double-click and the system menu can change state without a Qt
    // caption-button click. The icon must follow those changes as well.
    for (int i = 0; i < 3; ++i) {
        SendMessageW(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        QTRY_VERIFY(window.isMaximized());
        QCOMPARE(titleBar->maximizeButton()->toolTip(), QStringLiteral("استعادة"));
        SendMessageW(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
        QTRY_VERIFY(not window.isMaximized());
        QCOMPARE(titleBar->maximizeButton()->toolTip(), QStringLiteral("تكبير"));
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication app(argc, argv);
    TestWindowsFrame test;
    return QTest::qExec(&test, argc, argv);
}

#include "TestWindowsFrame.moc"
