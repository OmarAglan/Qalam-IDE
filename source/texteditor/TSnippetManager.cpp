#include "TSnippetManager.h"
#include <QTextBlock>
#include <QTextDocument>

TSnippetManager::TSnippetManager(QPlainTextEdit *editor) : m_editor(editor) {}

void TSnippetManager::insertSnippet(const QString &snippet, QTextCursor &tc, SnippetId snippetId) {
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

    // Set up placeholder navigation based on the snippet ID
    setupNavigation(snippetId);
}

void TSnippetManager::setupNavigation(SnippetId snippetId) {
    m_targets.clear();

    switch (snippetId) {
    case SnippetId::Function: {
        // "صحيح اسم_الدالة(صحيح معامل) {...}" — select function name first, then parameter
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("اسم_الدالة", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "معامل" << "مرر";
        break;
    }
    case SnippetId::Class: {
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("اسم", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
        break;
    }
    case SnippetId::If:
    case SnippetId::IfElse:
    case SnippetId::ElseIf: {
        // "إذا (الشرط) {...}" — select the condition placeholder
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("الشرط", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
        break;
    }
    case SnippetId::ForLoop: {
        // "لكل (صحيح س = ٠؛ ...) {...}" — select loop variable
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("س", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
        break;
    }
    case SnippetId::WhileLoop: {
        // "طالما (الشرط) {...}" — select condition
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("الشرط", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
        break;
    }
    case SnippetId::Switch: {
        // "اختر (المتغير) {...}" — select the variable placeholder
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("المتغير", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "مرر";
        break;
    }
    case SnippetId::Array: {
        // "صحيح المصفوفة[١٠]." — select array name
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("المصفوفة", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        break;
    }
    case SnippetId::Constant: {
        // "ثابت صحيح الاسم = القيمة." — select constant name, then value
        QTextCursor finder = m_editor->textCursor();
        finder = m_editor->document()->find("الاسم", finder, QTextDocument::FindBackward);
        if (!finder.isNull()) m_editor->setTextCursor(finder);
        m_targets << "القيمة";
        break;
    }
    case SnippetId::Main:
    case SnippetId::Else:
    case SnippetId::None:
        // No placeholder navigation needed
        break;
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
