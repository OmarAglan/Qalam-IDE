#pragma once

#include <QString>

// Search-friendly Arabic matching that never rewrites source text. The query
// is turned into a regular expression over the original document, so match
// offsets stay exact and replacements touch only what the user saw.
namespace QalamArabicSearch {

// True for the short vowels, tanween, shadda, sukun, superscript alef, and
// tatweel. Hamza and madda are letter-forming marks and stay significant.
bool isIgnorableMark(char32_t codePoint);

// Escaped pattern for a literal query that matches canonically equivalent
// spellings (composed or decomposed) with or without vowel marks and tatweel.
QString diacriticInsensitivePattern(const QString &literal);

} // namespace QalamArabicSearch
