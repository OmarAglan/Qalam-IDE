#pragma once

#include "TakweenProtocol.h"

#include <QWidget>

class QComboBox;
class QLabel;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * @brief Sidebar view of the Takween project that owns the current file.
 *
 * Everything shown comes from Takween's own contracts (takween-targets-v1 and
 * takween-build-plan-v1); the view never reads مشروع.تكوين. Qalam runs the
 * queries and hands the result over as a Snapshot.
 */
class QalamTakweenProjectView : public QWidget
{
    Q_OBJECT

public:
    struct Snapshot {
        QString projectRoot;
        QVector<TakweenTarget> targets;
        QString targetsError;
        QString activeTarget;
        QString activeProfile;
        bool hasPlan{};
        TakweenBuildPlan plan;
        QString planError;
    };

    explicit QalamTakweenProjectView(QWidget *parent = nullptr);

    void setSnapshot(const Snapshot &snapshot);
    void showMessage(const QString &message);
    QString projectRoot() const { return m_snapshot.projectRoot; }

    /// Output files a plan names, resolved against the project root.
    static QString planOutputPath(const TakweenBuildPlan &plan,
                                  const QString &projectRoot,
                                  const QString &option);
    static QString resolvePlanPath(const TakweenBuildPlan &plan,
                                   const QString &projectRoot,
                                   const QString &path);

signals:
    void selectionChangeRequested(const QString &target, const QString &profile);
    void refreshRequested();
    void fileActivated(const QString &filePath);

private:
    void rebuildSelectors();
    void rebuildTree();
    void emitSelection();
    QTreeWidgetItem *addSection(const QString &title, const QString &iconName);

    Snapshot m_snapshot;
    bool m_updating{};
    QLabel *m_titleLabel{};
    QLabel *m_rootLabel{};
    QLabel *m_messageLabel{};
    QComboBox *m_targetCombo{};
    QComboBox *m_profileCombo{};
    QToolButton *m_refreshButton{};
    QTreeWidget *m_tree{};
};
