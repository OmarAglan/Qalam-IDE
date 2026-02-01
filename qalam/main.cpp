#include "Qalam.h"
#include "TWelcomeWindow.h"
#include "QalamTheme.h"

#include <QApplication>
#include <QMessageBox>
#include <QDebug>

#include <QFileDialog>
#include <QLockFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Alif");
    QCoreApplication::setApplicationName("Qalam");
    app.setLayoutDirection(Qt::RightToLeft);


    QString lockPath = QDir::tempPath() + "/qalam_ide.lock";
    QLockFile lockFile(lockPath);

    if (!lockFile.tryLock(100)) {
        QMessageBox::warning(nullptr, "قلم",
                             "البرنامج يعمل بالفعل!\nلا يمكن تشغيل أكثر من نسخة في نفس الوقت.");
        return 0;
    }

    int fontId1 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/Tajawal/Tajawal-Regular.ttf");
    int fontId2 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/KawkabMono-Regular.ttf");
    int fontId3 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/NotoKufiArabic-Regular.ttf");
    if(fontId1 == -1 or fontId2 == -1 or fontId3 == -1) {
        qWarning() << "لم يستطع تحميل الخط";
    } else {
        QString tajawal = QFontDatabase::applicationFontFamilies(fontId1).at(0);
        QString kawkabMono = QFontDatabase::applicationFontFamilies(fontId2).at(0);
        QString notoKufi = QFontDatabase::applicationFontFamilies(fontId3).at(0);
        QFont font{};
        QStringList fontFamilies{};
        fontFamilies << notoKufi << tajawal << kawkabMono;
        font.setFamilies(fontFamilies);
        font.setPixelSize(14);
        font.setWeight(QFont::Weight::Normal);
        app.setFont(font);
    }

    // Apply Qalam Theme
    QalamTheme::instance().apply(&app);

    // لتشغيل ملف باء بإستخدام محرر قلم عند إختيار المحرر ك برنامج للتشغيل
    QString filePath{};
    if (app.arguments().count() > 2) {
        int ret = QMessageBox::warning(nullptr, "قلم",
                                       "لا يمكن تمرير أكثر من معامل واحد",
                                       QMessageBox::Close);
        return ret;
    }

    if (app.arguments().count() == 2) {
        filePath = app.arguments().at(1);
    }

    app.setQuitOnLastWindowClosed(true);

    if (!filePath.isEmpty()) {
        Qalam *editor = new Qalam(filePath);
        editor->show();
    } else {
        WelcomeWindow *w = new WelcomeWindow();
        w->show();
    }

    return app.exec();
}
