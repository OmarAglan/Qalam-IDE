#pragma once

#include "CommandRegistry.h"

#include <QHash>
#include <QKeySequence>
#include <QObject>
#include <QPointer>
#include <QStringList>

class QAction;
class QSettings;
class QShortcut;

// One owner for every workbench key binding. Each command id is bound to the
// QAction or QShortcut that carries its key; apply() resolves the registry
// default or the user's override from settings and updates both the binding
// and the registry, so menus, the command palette, and the keys agree.
class QalamKeybindings : public QObject
{
    Q_OBJECT

public:
    explicit QalamKeybindings(CommandRegistry *registry, QObject *parent = nullptr);

    void bind(const QString &commandId, QAction *action);
    void bind(const QString &commandId, QShortcut *shortcut);

    // Re-reads overrides and updates every bound action and shortcut.
    void apply(const QSettings &settings);

    static QString settingsKey(const QString &commandId);
    // False for keys the editor interprets itself (semantic selection), which
    // a window shortcut would steal from the text widget.
    static bool isRebindable(const QString &commandId);
    bool isBound(const QString &commandId) const;
    // An override is stored even when empty: an empty sequence unbinds.
    static QKeySequence effectiveShortcut(const CommandRegistry::Command &command,
                                          const QSettings &settings);
    // Commands that share a non-empty key, grouped by that key's text.
    static QHash<QString, QStringList> conflicts(const QHash<QString, QKeySequence> &bindings);

private:
    CommandRegistry *m_registry{};
    QHash<QString, QPointer<QAction>> m_actions;
    QHash<QString, QPointer<QShortcut>> m_shortcuts;
    QHash<QString, QString> m_defaults;
};
