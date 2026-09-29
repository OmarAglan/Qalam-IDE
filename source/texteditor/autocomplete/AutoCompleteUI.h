#pragma once

#include "AutoComplete.h"

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <QListView>
#include <QTextBrowser>

#include <vector>



// --- Custom Model ---
class CompletionModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit CompletionModel(QObject *parent = nullptr);
    void updateData(const std::vector<CompletionItem>& items);
    const CompletionItem *itemAt(int row) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    std::vector<CompletionItem> m_data{};
};

// --- Rich Popup View (The "Container" for List + Footer) ---
class QalamCompletionPopup : public QListView {
    Q_OBJECT
public:
    explicit QalamCompletionPopup(QWidget *parent = nullptr);
    void showDocumentation(const QJsonValue &documentation);

signals:
    void selectedCompletionChanged(const QModelIndex &index);
    void dismissed();
    void documentationLinkActivated(const QUrl &url);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void hideEvent(QHideEvent *event) override;

private:
    void updateFooterLayout();

    QTextBrowser *infoBrowser{};
    QString m_descriptionHeader;
    int footerHeight{};
};

// --- Modern Delegate ---
class QalamModernCompletionDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit QalamModernCompletionDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
