#pragma once

#include <QPlainTextEdit>
#include <QStringList>

// Handles snippet insertion (with indentation) and Tab/Enter navigation
// through snippet placeholders. Extracted from TEditor.
class TSnippetManager {
public:
    explicit TSnippetManager(QPlainTextEdit *editor);

    // Insert a snippet at the given cursor position and set up placeholder navigation.
    void insertSnippet(const QString &snippet, QTextCursor &tc);

    // Try to navigate to the next snippet placeholder.
    // Returns true if navigation occurred (key event consumed).
    bool processSnippetNavigation();

    // Whether there are remaining snippet placeholders to navigate to.
    bool hasActiveSnippet() const;

private:
    QPlainTextEdit *m_editor{};
    QStringList m_targets{};
};
