#pragma once
#include "TSyntaxThemes.h"
#include <QVector>
#include <memory>

class ThemeManager {
public:
    static QVector<std::shared_ptr<SyntaxTheme>> getAvailableThemes() {
        QVector<std::shared_ptr<SyntaxTheme>> themes;
        themes.append(std::make_shared<VSCodeDarkTheme>());
        themes.append(std::make_shared<MonokaiTheme>());
        themes.append(std::make_shared<OceanicTheme>());
        themes.append(std::make_shared<QalamGlowTheme>());
        return themes;
    }
    
    static std::shared_ptr<SyntaxTheme> getThemeByIndex(int index) {
        auto themes = getAvailableThemes();
        if (index >= 0 && index < themes.size()) {
            return themes.at(index);
        }
        return themes.at(0); // Default theme
    }
};
