#pragma once

#include <QTimer>
#include <QScrollBar>
#include <QPlainTextEdit>
#include <QCompleter>
#include <memory>

#include "TSettings.h"
#include "TSyntaxHighlighter.h"
#include "AutoComplete.h"
#include "AutoCompleteUI.h"
#include "TBracketHandler.h"
#include "TAutoSave.h"
#include "TSnippetManager.h"
#include "Constants.h"


class LineNumberArea;


class TEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    TEditor(QWidget* parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth() const;
    QString filePath{};

    QString getCurrentLineIndentation(const QTextCursor &cursor) const;
    void cursorIndentation();

    void setCompleter(QCompleter *completer);

    void startAutoSave();
    void stopAutoSave();
    void removeBackupFile();

public slots:
    void updateFontSize(int);
    void updateFontType(QString font);
    void toggleComment();
    void duplicateLine();
    void moveLineUp();
    void moveLineDown();
    void updateHighlighterTheme(std::shared_ptr<SyntaxTheme>);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    void keyPressEvent(QKeyEvent *e) override;
    // We override focusOutEvent to close the popup if the user clicks away
    void focusOutEvent(QFocusEvent *e) override;

private:
    TSyntaxHighlighter* highlighter{};

    LineNumberArea* lineNumberArea{};

    struct FoldRegion {
        int startBlockNumber;
        int endBlockNumber;
        bool folded = false;
    };
    QVector<FoldRegion> foldRegions;

    void updateFoldRegions();
    void toggleFold(int blockNum);

    // Extracted helpers
    TBracketHandler m_bracketHandler;
    TAutoSave *m_autoSave{};
    TSnippetManager m_snippetManager;

    QTimer *m_indexRebuildTimer{};

    friend class LineNumberArea;

    QCompleter* c{};
    CompletionModel *model{};
    std::vector<std::unique_ptr<ICompletionStrategy>> strategies{};
    DynamicWordStrategy* dynamicStrategy{};
    QString textUnderCursor() const;
    void performCompletion();
    void setupAutoComplete();
    void insertWord(const QString& completion, QTextCursor& tc);
    void insertBuiltinFunction(const QString& functionName, QTextCursor& tc);

private slots:
    void updateLineNumberAreaWidth();
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void insertCompletion(const QString &completion, CompletionType type);
signals:
    void openRequest(QString filePath);
};


class LineNumberArea : public QWidget {
public:
    LineNumberArea(TEditor* editor) : QWidget(editor), tEditor(editor) {
        this->setStyleSheet(QString(
            "QWidget {"
            "   border-left: 1px solid %1;"
            "   border-top-left-radius: 9px;"
            "   border-bottom-left-radius: 9px;"
            "}").arg(Constants::Colors::LineNumberBorder));
    }

    QSize sizeHint() const override {
        return QSize(tEditor->lineNumberAreaWidth(), 0);
    }

    void mousePressEvent(QMouseEvent* event) override {
        int y = event->position().y();
        QTextBlock block = tEditor->firstVisibleBlock();
        int top = qRound(tEditor->blockBoundingGeometry(block).translated(tEditor->contentOffset()).top());
        int height = qRound(tEditor->blockBoundingRect(block).height());

        while (block.isValid() && top <= y) {
            if (y >= top && y < top + height) {
                int blockNum = block.blockNumber();
                tEditor->toggleFold(blockNum);
                return;
            }
            block = block.next();
            top += height;
            height = qRound(tEditor->blockBoundingRect(block).height());
        }
    }


protected:
    void paintEvent(QPaintEvent* event) override {
        tEditor->lineNumberAreaPaintEvent(event);
    }


private:
    TEditor* tEditor{};
};
