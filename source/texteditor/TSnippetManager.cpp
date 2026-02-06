#include "TSnippetManager.h"
#include <QTextBlock>
#include <QTextDocument>

TSnippetManager::TSnippetManager(QPlainTextEdit *editor) : m_editor(editor) {}

void TSnippetManager::insertSnippet(const QString &snippet, QTextCursor &tc) {
    QString textToInsert = snippet;

    // Calculate indentation from the current line
    QTextBlock block = tc.block();
    QString lineText = block.text();
    QString baseIndentation{};
    for (const QChar &ch : lineText) {
        if (ch.isSpace()) baseIndentation.append(ch);
        else break;
    }

    // Apply indentation to multi-line snippets
    if (textToInsert.contains('\n')) {
        QStringList lines = textToInsert.split('\n');
        // Start from index 1 because index 0 is appended to the current line
        // (which already has indentation on the left).
        // Subsequent lines need the base indentation explicitly added.
        for (int i = 1; i < lines.size(); ++i) {
            lines[i] = baseIndentation + lines[i];
        }
        textToInsert = lines.join('\n');
    }

    // Perform the insertion
    tc.insertText(textToInsert);
    m_editor->setTextCursor(tc);

    // Reset snippet targets
    m_targets.clear();

    // Setup snippet navigation based on snippet content
    if (snippet.contains("اسم_الدالة")) {
        // Function snippet: "صحيح اسم_الدالة(صحيح معامل) {...}"
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("اسم_الدالة", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "معامل" << "مرر";
    }
    else if (snippet.startsWith("صنف")) {
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("اسم", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
    }
    else if (snippet.startsWith("إذا")) {
        // If snippet: "إذا (الشرط) {...}"
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("الشرط", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
    }
    else if (snippet.startsWith("لكل")) {
        // For loop snippet: "لكل (صحيح س = ٠؛ س < ١٠؛ س++) {...}"
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("س", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
    }
    else if (snippet.startsWith("طالما")) {
        // While snippet: "طالما (الشرط) {...}"
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("الشرط", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
    }
    else if (snippet.startsWith("حاول")) {
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("مرر", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
    }
}

bool TSnippetManager::processSnippetNavigation() {
    if (m_targets.isEmpty()) return false;
    QString nextTarget = m_targets.first();
    QTextCursor tc = m_editor->textCursor();
    QTextCursor found = m_editor->document()->find(nextTarget, tc);
    if (!found.isNull()) {
        m_editor->setTextCursor(found);
        m_targets.removeFirst();
        return true;
    }
    m_targets.clear();
    return false;
}

bool TSnippetManager::hasActiveSnippet() const {
    return !m_targets.isEmpty();
}
