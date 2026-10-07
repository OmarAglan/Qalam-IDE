#include <QCoreApplication>
#include <QSettings>
#include <QTemporaryDir>

// Linked into every test executable. Qalam opens its settings through
// Constants::settings(), which follows QSettings::defaultFormat(); switching
// that to INI under a throwaway directory before any test runs keeps the
// suite away from the developer's real Qalam settings (the registry on
// Windows). Tests that need their own directory still call setPath() later.
namespace {

QTemporaryDir *settingsDirectory{};

void removeSettingsDirectory()
{
    delete settingsDirectory;
    settingsDirectory = nullptr;
}

void isolateSettings()
{
    settingsDirectory = new QTemporaryDir;
    if (not settingsDirectory->isValid()) qFatal("Cannot create a test settings directory");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory->path());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settingsDirectory->path());
    qAddPostRoutine(removeSettingsDirectory);
}

} // namespace

Q_COREAPP_STARTUP_FUNCTION(isolateSettings)
