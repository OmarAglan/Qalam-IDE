#include "QalamArabicSearch.h"

#include <QRegularExpression>

namespace QalamArabicSearch {

namespace {
const QString OptionalMarks = QStringLiteral("[\\x{064B}-\\x{0652}\\x{0670}\\x{0640}]*");

QString escapedCodePoint(char32_t codePoint)
{
    return QRegularExpression::escape(QString::fromUcs4(&codePoint, 1));
}
}

bool isIgnorableMark(char32_t codePoint)
{
    return (codePoint >= 0x064B and codePoint <= 0x0652) or
           codePoint == 0x0670 or codePoint == 0x0640;
}

QString diacriticInsensitivePattern(const QString &literal)
{
    QString pattern;
    // NFC first so a decomposed query letter is one alternative group below.
    const QList<uint> codePoints =
        literal.normalized(QString::NormalizationForm_C).toUcs4();
    for (const uint codePoint : codePoints) {
        if (isIgnorableMark(codePoint)) continue;

        // The decomposed spelling keeps its significant marks but may carry
        // vowel marks between them, exactly as canonical ordering allows.
        const QString single = QString::fromUcs4(
            reinterpret_cast<const char32_t *>(&codePoint), 1);
        const QList<uint> decomposed =
            single.normalized(QString::NormalizationForm_D).toUcs4();
        QString alternative;
        if (decomposed.size() > 1) {
            alternative = escapedCodePoint(decomposed.first());
            for (qsizetype index = 1; index < decomposed.size(); ++index) {
                if (isIgnorableMark(decomposed.at(index))) continue;
                alternative += OptionalMarks + escapedCodePoint(decomposed.at(index));
            }
        }

        const QString composed = escapedCodePoint(codePoint);
        if (alternative.isEmpty() or alternative == composed)
            pattern += composed;
        else
            pattern += QStringLiteral("(?:%1|%2)").arg(composed, alternative);
        pattern += OptionalMarks;
    }
    // Trailing marks stay in the match: they belong to the last letter, so a
    // replacement never leaves an orphaned vowel behind.
    return pattern;
}

} // namespace QalamArabicSearch
