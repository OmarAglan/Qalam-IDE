#include "AutoComplete.h"
#include <QRegularExpression>
#include <QSet>

// --- Keyword Strategy ---
KeywordStrategy::KeywordStrategy() {
    keywords = {
        // Types (§3.1)
        "صحيح", "نص", "منطقي",
        // Constants (§4)
        "ثابت",
        // Boolean literals (§3.1)
        "صواب", "خطأ",
        // Control flow (§7)
        "إذا", "وإلا", "طالما", "لكل",
        // Switch (§7.5)
        "اختر", "حالة", "افتراضي",
        // Loop control (§7.4)
        "توقف", "استمر",
        // Functions (§5)
        "إرجع",
        // Entry point
        "الرئيسية"
    };
}

QVector<CompletionItem> KeywordStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items;
    if (prefix.isEmpty()) return items;

    for (const auto &k : keywords) {
        if (k.startsWith(prefix, Qt::CaseInsensitive)) {
            items.push_back(CompletionItem(k, k, "كلمة محجوزة", CompletionType::Keyword));
        }
    }
    return items;
}

// --- Built-ins Strategy ---
BuiltinStrategy::BuiltinStrategy() {
    builtins = {
        "اطبع", "اقرأ"
    };
}

QVector<CompletionItem> BuiltinStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items{};
    if (prefix.isEmpty()) return items;

    for (const auto &b : builtins) {
        if (b.startsWith(prefix, Qt::CaseInsensitive)) {
            items.push_back(CompletionItem(b, b, "دالة ضمن لغة باء", CompletionType::Builtin));
        }
    }
    return items;
}

// --- Snippet Strategy ---
QVector<CompletionItem> SnippetStrategy::getSuggestions(const QString &prefix, const QString &) {
    QVector<CompletionItem> items;
    QString p = prefix.toLower();

    if (QString("الرئيسية").startsWith(p)) {
        items.push_back(CompletionItem("الرئيسية",
                         "صحيح الرئيسية() {\n\tإرجع ٠.\n}",
                         "الدالة الرئيسية للبرنامج",
                         CompletionType::Snippet));
    }
    if (QString("إذا").startsWith(p)) {
        items.push_back(CompletionItem("إذا",
                         "إذا (الشرط) {\n\t\n}",
                         "جملة شرطية",
                         CompletionType::Snippet));
    }
    if (QString("وإلا").startsWith(p)) {
        items.push_back(CompletionItem("وإلا",
                         "وإلا {\n\t\n}",
                         "تتمة الجملة الشرطية",
                         CompletionType::Snippet));
    }
    if (QString("لكل").startsWith(p)) {
        items.push_back(CompletionItem("لكل",
                         "لكل (صحيح س = ٠؛ س < ١٠؛ س++) {\n\t\n}",
                         "حلقة تكرارية (For)",
                         CompletionType::Snippet));
    }
    if (QString("طالما").startsWith(p)) {
        items.push_back(CompletionItem("طالما",
                         "طالما (الشرط) {\n\t\n}",
                         "حلقة تكرارية شرطية (While)",
                         CompletionType::Snippet));
    }
    if (QString("اختر").startsWith(p)) {
        items.push_back(CompletionItem("اختر",
                         "اختر (المتغير) {\n\tحالة القيمة:\n\t\t\n\tافتراضي:\n\t\t\n}",
                         "جملة الاختيار (Switch)",
                         CompletionType::Snippet));
    }
    if (QString("تضمين").startsWith(p)) {
        items.push_back(CompletionItem("تضمين (#تضمين)",
                         "#تضمين \"ملف.baahd\"",
                         "تضمين ملف خارجي",
                         CompletionType::Snippet));
    }
    if (QString("إذا_عرف").contains(p)) {
        items.push_back(CompletionItem("إذا_عرف (#إذا_عرف)",
                         "#إذا_عرف اسم_الماكرو\n\t\n#نهاية",
                         "شرط المعالجة القبلية (Ifdef)",
                         CompletionType::Snippet));
    }
    if (QString("تعريف").startsWith(p)) {
        items.push_back(CompletionItem("تعريف (#تعريف)",
                         "#تعريف الاسم القيمة",
                         "تعريف ماكرو (Define)",
                         CompletionType::Snippet));
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
