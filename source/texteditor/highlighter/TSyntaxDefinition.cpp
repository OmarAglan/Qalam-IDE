#include "TSyntaxDefinition.h"

// ==================== Language Definition ====================

LanguageDefinition::LanguageDefinition() {
    QStringList keywords = {
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

    QStringList builtins = {
        // I/O (§6)
        "اطبع", "اقرأ"
    };

    QStringList magics = {
        // Reserved for future use or specific magic methods if any
    };

    QStringList preprocessors = {
        "#تضمين",      // Include (§2.1)
        "#تعريف",      // Define (§2.2)
        "#الغاء_تعريف", // Undefine (§2.4)
        "#إذا_عرف",    // Ifdef (§2.3)
        "#وإلا",       // Else (§2.3)
        "#نهاية"       // Endif (§2.3)
    };

    keywordSet = QSet<QString>(keywords.begin(), keywords.end());
    builtinSet = QSet<QString>(builtins.begin(), builtins.end());
    magicSet = QSet<QString>(magics.begin(), magics.end());
    preprocessorSet = QSet<QString>(preprocessors.begin(), preprocessors.end());

    hexPattern = QRegularExpression(R"(\b0[xX][0-9a-fA-F]+\b)");
    // Enhanced number pattern to support Arabic-Indic numerals ٠-٩
    numberPattern = QRegularExpression(R"(\b[\d٠-٩]+(\.[\d٠-٩]+)?([eE][+-]?[\d٠-٩]+)?\b)");
}
