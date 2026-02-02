#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @brief TStatusBar - VSCode-like status bar
 * 
 * Blue background (#007acc) with clickable status items.
 * RTL layout with items flowing right-to-left.
 * 
 * Left side (in LTR, right in RTL): Line/Column, Indentation, Encoding, Language
 * Right side (in LTR, left in RTL): Problems count, Branch name
 */
class TStatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit TStatusBar(QWidget* parent = nullptr);
    ~TStatusBar() override = default;

    /// Update cursor position display
    void setCursorPosition(int line, int column);
    
    /// Update encoding display (e.g., "UTF-8")
    void setEncoding(const QString& encoding);
    
    /// Update line ending display (e.g., "LF", "CRLF")
    void setLineEnding(const QString& ending);
    
    /// Update language/file type display (e.g., "Baa")
    void setLanguage(const QString& language);
    
    /// Update indentation display (e.g., "Spaces: 4")
    void setIndentation(const QString& indentation);
    
    /// Update problems count
    void setProblemsCount(int errors, int warnings);
    
    /// Update git branch name
    void setBranch(const QString& branch);
    
    /// Set whether a folder is currently open (changes bar color)
    void setFolderOpen(bool open);
    
    /// Show a temporary message (auto-hides after timeout)
    void showMessage(const QString& message, int timeoutMs = 3000);

signals:
    void cursorPositionClicked();
    void encodingClicked();
    void lineEndingClicked();
    void languageClicked();
    void indentationClicked();
    void problemsClicked();
    void branchClicked();

private:
    QPushButton* createStatusItem(const QString& text, const QString& tooltip = QString());
    void applyStyles();
    void updateBackgroundColor();

    QHBoxLayout* m_layout{nullptr};
    
    // Right side items (in RTL layout, these appear on the right)
    QPushButton* m_problemsBtn{nullptr};
    QPushButton* m_branchBtn{nullptr};
    
    // Left side items (in RTL layout, these appear on the left)
    QPushButton* m_cursorPosBtn{nullptr};
    QPushButton* m_indentationBtn{nullptr};
    QPushButton* m_encodingBtn{nullptr};
    QPushButton* m_lineEndingBtn{nullptr};
    QPushButton* m_languageBtn{nullptr};
    
    // Message display
    QLabel* m_messageLabel{nullptr};
    QTimer* m_messageTimer{nullptr};
    
    bool m_folderOpen{false};
    int m_errorCount{0};
    int m_warningCount{0};
};
