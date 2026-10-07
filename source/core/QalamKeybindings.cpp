#include "QalamKeybindings.h"

#include <QAction>
#include <QSettings>
#include <QShortcut>

QalamKeybindings::QalamKeybindings(CommandRegistry *registry, QObject *parent)
    : QObject(parent), m_registry(registry)
{
    // Remember the shipped defaults: the registry entries are rewritten with
    // the effective keys, and a reset must still find the original.
    if (m_registry) {
        for (const CommandRegistry::Command &command : m_registry->commands())
            m_defaults.insert(command.id, command.shortcut);
    }
}

void QalamKeybindings::bind(const QString &commandId, QAction *action)
{
    if (action) m_actions.insert(commandId, action);
}

void QalamKeybindings::bind(const QString &commandId, QShortcut *shortcut)
{
    if (shortcut) m_shortcuts.insert(commandId, shortcut);
}

bool QalamKeybindings::isRebindable(const QString &commandId)
{
    return commandId != QLatin1String("code.expandSelection") and
           commandId != QLatin1String("code.shrinkSelection");
}

bool QalamKeybindings::isBound(const QString &commandId) const
{
    return m_actions.value(commandId) or m_shortcuts.value(commandId);
}

QString QalamKeybindings::settingsKey(const QString &commandId)
{
    return QStringLiteral("shortcuts/") + commandId;
}

QKeySequence QalamKeybindings::effectiveShortcut(const CommandRegistry::Command &command,
                                                 const QSettings &settings)
{
    const QString key = settingsKey(command.id);
    if (settings.contains(key))
        return QKeySequence::fromString(settings.value(key).toString(), QKeySequence::PortableText);
    return QKeySequence::fromString(command.shortcut, QKeySequence::PortableText);
}

QHash<QString, QStringList> QalamKeybindings::conflicts(
    const QHash<QString, QKeySequence> &bindings)
{
    QHash<QString, QStringList> byKey;
    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it) {
        if (it.value().isEmpty()) continue;
        byKey[it.value().toString(QKeySequence::PortableText)] << it.key();
    }
    QHash<QString, QStringList> result;
    for (auto it = byKey.cbegin(); it != byKey.cend(); ++it) {
        if (it.value().size() < 2) continue;
        QStringList ids = it.value();
        ids.sort();
        result.insert(it.key(), ids);
    }
    return result;
}

void QalamKeybindings::apply(const QSettings &settings)
{
    if (not m_registry) return;
    for (CommandRegistry::Command command : m_registry->commands()) {
        command.shortcut = m_defaults.value(command.id, command.shortcut);
        const QKeySequence sequence = effectiveShortcut(command, settings);
        if (QAction *action = m_actions.value(command.id)) action->setShortcut(sequence);
        if (QShortcut *shortcut = m_shortcuts.value(command.id)) shortcut->setKey(sequence);

        const QString text = sequence.toString(QKeySequence::PortableText);
        if (text != m_registry->command(command.id).shortcut) {
            command.shortcut = text;
            m_registry->registerCommand(command);
        }
    }
}
