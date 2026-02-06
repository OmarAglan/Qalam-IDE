#include "AutoComplete.h"
#include "../highlighter/TSyntaxDefinition.h"
#include <QRegularExpression>
#include <QSet>

// --- Keyword Strategy ---
// Reads from the single source of truth: LanguageDefinition::instance()

QVector<CompletionItem> KeywordStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items;
    if (prefix.isEmpty()) return items;

    const auto &keywords = LanguageDefinition::instance().keywordList;
    for (const auto &k : keywords) {
        if (k.startsWith(prefix, Qt::CaseInsensitive)) {
            items.push_back(CompletionItem(k, k, "كلمة محجوزة", CompletionType::Keyword));
        }
    }
    return items;
}

// --- Built-ins Strategy ---
// Reads from the single source of truth: LanguageDefinition::instance()

QVector<CompletionItem> BuiltinStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items{};
    if (prefix.isEmpty()) return items;

    const auto &builtins = LanguageDefinition::instance().builtinList;
    for (const auto &b : builtins) {
        if (b.startsWith(prefix, Qt::CaseInsensitive)) {
            items.push_back(CompletionItem(b, b, "دالة ضمن لغة باء", CompletionType::Builtin));
        }
    }
    return items;
}

// --- Snippet Strategy ---
// Code templates for common Baa constructs
QVector<CompletionItem> SnippetStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items;
    QString p = prefix.toLower();

    // Main function template (§5.4)
    if (QString("الرئيسية").startsWith(p) || QString("main").startsWith(p)) {
        items.push_back(CompletionItem("الرئيسية (دالة)",
                         "صحيح الرئيسية() {\n\t\n\tإرجع ٠.\n}",
                         "الدالة الرئيسية - نقطة بداية البرنامج",
                         CompletionType::Snippet, SnippetId::Main));
    }

    // Function template (§5.1)
    if (QString("دالة").startsWith(p) || QString("function").startsWith(p)) {
        items.push_back(CompletionItem("دالة جديدة",
                         "صحيح اسم_الدالة(صحيح معامل) {\n\t\n\tإرجع ٠.\n}",
                         "قالب دالة جديدة",
                         CompletionType::Snippet, SnippetId::Function));
    }

    // If statement (§7.1)
    if (QString("إذا").startsWith(p)) {
        items.push_back(CompletionItem("إذا (شرط)",
                         "إذا (الشرط) {\n\t\n}",
                         "جملة شرطية - تنفذ إذا تحقق الشرط",
                         CompletionType::Snippet, SnippetId::If));
    }

    // If-else statement (§7.1)
    if (QString("إذا_وإلا").contains(p) || QString("ifelse").startsWith(p)) {
        items.push_back(CompletionItem("إذا-وإلا",
                         "إذا (الشرط) {\n\t\n} وإلا {\n\t\n}",
                         "جملة شرطية مع بديل",
                         CompletionType::Snippet, SnippetId::IfElse));
    }

    // Else clause (§7.1)
    if (QString("وإلا").startsWith(p)) {
        items.push_back(CompletionItem("وإلا",
                         "وإلا {\n\t\n}",
                         "تتمة الجملة الشرطية - تنفذ إذا لم يتحقق الشرط",
                         CompletionType::Snippet, SnippetId::Else));
    }

    // Else-if clause (§7.1)
    if (QString("وإلا_إذا").contains(p)) {
        items.push_back(CompletionItem("وإلا إذا",
                         "وإلا إذا (الشرط) {\n\t\n}",
                         "شرط إضافي في الجملة الشرطية",
                         CompletionType::Snippet, SnippetId::ElseIf));
    }

    // For loop (§7.3) - note: uses Arabic semicolon ؛
    if (QString("لكل").startsWith(p) || QString("for").startsWith(p)) {
        items.push_back(CompletionItem("لكل (حلقة)",
                         "لكل (صحيح س = ٠؛ س < ١٠؛ س++) {\n\t\n}",
                         "حلقة تكرارية محددة العدد (For)",
                         CompletionType::Snippet, SnippetId::ForLoop));
    }

    // While loop (§7.2)
    if (QString("طالما").startsWith(p) || QString("while").startsWith(p)) {
        items.push_back(CompletionItem("طالما (حلقة)",
                         "طالما (الشرط) {\n\t\n}",
                         "حلقة تكرارية شرطية (While)",
                         CompletionType::Snippet, SnippetId::WhileLoop));
    }

    // Switch statement (§7.5)
    if (QString("اختر").startsWith(p) || QString("switch").startsWith(p)) {
        items.push_back(CompletionItem("اختر (تحويل)",
                         "اختر (المتغير) {\n\tحالة ١:\n\t\t\n\t\tتوقف.\n\tحالة ٢:\n\t\t\n\t\tتوقف.\n\tافتراضي:\n\t\t\n\t\tتوقف.\n}",
                         "جملة الاختيار المتعدد (Switch)",
                         CompletionType::Snippet, SnippetId::Switch));
    }

    // Array declaration (§3.3)
    if (QString("مصفوفة").startsWith(p) || QString("array").startsWith(p)) {
        items.push_back(CompletionItem("مصفوفة",
                         "صحيح المصفوفة[١٠].",
                         "تعريف مصفوفة ثابتة الحجم",
                         CompletionType::Snippet, SnippetId::Array));
    }

    // Constant declaration (§4)
    if (QString("ثابت").startsWith(p) || QString("const").startsWith(p)) {
        items.push_back(CompletionItem("ثابت (متغير)",
                         "ثابت صحيح الاسم = القيمة.",
                         "تعريف ثابت لا يمكن تغيير قيمته",
                         CompletionType::Snippet, SnippetId::Constant));
    }

    return items;
}

// --- Preprocessor Strategy ---
// Reads from the single source of truth: LanguageDefinition::instance()

QVector<CompletionItem> PreprocessorStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items;
    if (prefix.isEmpty()) return items;

    // Check if prefix starts with # or matches directive name without #
    bool startsWithHash = prefix.startsWith('#') || prefix.startsWith(QString("#"));

    const auto &directives = LanguageDefinition::instance().preprocessorList;
    for (const auto &d : directives) {
        bool matches = false;

        if (startsWithHash) {
            // Match with the # prefix
            matches = d.startsWith(prefix, Qt::CaseInsensitive);
        } else {
            // Match without # (user typing just the Arabic part)
            QString withoutHash = d.mid(1); // Remove the #
            matches = withoutHash.startsWith(prefix, Qt::CaseInsensitive);
        }

        if (matches) {
            QString desc;
            QString completion = d;

            if (d == "#تضمين") {
                desc = "تضمين ملف خارجي (Include)";
                completion = "#تضمين \"ملف.baahd\"";
            } else if (d == "#تعريف") {
                desc = "تعريف ماكرو ثابت (Define)";
                completion = "#تعريف الاسم القيمة";
            } else if (d == "#إذا_عرف") {
                desc = "شرط المعالجة القبلية (Ifdef)";
                completion = "#إذا_عرف الاسم\n\t\n#نهاية";
            } else if (d == "#وإلا") {
                desc = "فرع بديل في شرط المعالجة (Else)";
            } else if (d == "#نهاية") {
                desc = "إنهاء شرط المعالجة القبلية (Endif)";
            } else if (d == "#الغاء_تعريف") {
                desc = "إلغاء تعريف ماكرو (Undef)";
                completion = "#الغاء_تعريف الاسم";
            }

            items.push_back(CompletionItem(d, completion, desc, CompletionType::Preprocessor));
        }
    }

    return items;
}

// --- Dynamic Word Strategy ---
QVector<CompletionItem> DynamicWordStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items;
    if (prefix.length() < 2) return items;

    // Query the pre-built index instead of regex scanning everything
    for (const auto &word : wordIndex) {
        if (word != prefix && word.startsWith(prefix, Qt::CaseInsensitive)) {
            items.push_back(CompletionItem(word, word, "نص ضمن الملف الحالي", CompletionType::DynamicWord));
        }
    }

    return items;
}

void DynamicWordStrategy::rebuildIndex(const QString &fullText) {
    wordIndex.clear();
    static QRegularExpression re("[a-zA-Z0-9_\u0600-\u06FF_0-9]+");
    QRegularExpressionMatchIterator i = re.globalMatch(fullText);

    while (i.hasNext()) {
        QString word = i.next().captured(0);
        if (word.length() >= 2) {
            wordIndex.insert(word);
        }
    }
}

void DynamicWordStrategy::updateIndex(const QString &text) {
    // Basic implementation: just add new words from the changed text
    static QRegularExpression re("[a-zA-Z0-9_\u0600-\u06FF_0-9]+");
    QRegularExpressionMatchIterator i = re.globalMatch(text);

    while (i.hasNext()) {
        QString word = i.next().captured(0);
        if (word.length() >= 2) {
            wordIndex.insert(word);
        }
    }
}
