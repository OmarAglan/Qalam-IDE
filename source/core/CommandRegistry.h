#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class CommandRegistry : public QObject
{
    Q_OBJECT

public:
    struct Command {
        QString id;
        QString title;
        QString description;
        QString shortcut;
    };

    explicit CommandRegistry(QObject *parent = nullptr);

    void clear();
    void registerCommand(const Command &command);
    bool contains(const QString &id) const;
    Command command(const QString &id) const;
    QVector<Command> commands() const;

    static QVector<Command> defaultCommands();

signals:
    void commandsChanged();

private:
    QVector<Command> m_commands;
};
