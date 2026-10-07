#include "QalamTakweenProjectView.h"
#include "Constants.h"

#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

constexpr int PathRole = Qt::UserRole;
constexpr int CustomProfileRole = Qt::UserRole + 1;

QIcon resourceIcon(const QString &name)
{
    return QIcon(QStringLiteral(":/icons/resources/%1.svg").arg(name));
}

QString targetKindLabel(const QString &kind)
{
    if (kind == "executable") return QStringLiteral("تنفيذي");
    if (kind == "library") return QStringLiteral("مكتبة");
    if (kind == "test") return QStringLiteral("اختبار");
    return kind;
}

QString targetStatusLabel(const QString &status)
{
    if (status == "ready") return QStringLiteral("جاهز");
    if (status == "unsupported") return QStringLiteral("غير مدعوم");
    return status;
}

bool selectableTarget(const TakweenTarget &target)
{
    return target.buildable or target.test;
}

} // namespace

QalamTakweenProjectView::QalamTakweenProjectView(QWidget *parent)
    : QWidget(parent)
{
    using namespace Constants;
    setObjectName(QStringLiteral("takweenProjectView"));
    setAttribute(Qt::WA_StyledBackground, true);
    setLayoutDirection(Qt::RightToLeft);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 8);
    layout->setSpacing(8);

    auto *heading = new QHBoxLayout;
    heading->setSpacing(6);
    auto *titles = new QVBoxLayout;
    titles->setSpacing(2);
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("takweenProjectTitle"));
    m_rootLabel = new QLabel(this);
    m_rootLabel->setObjectName(QStringLiteral("takweenProjectRoot"));
    m_rootLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    titles->addWidget(m_titleLabel);
    titles->addWidget(m_rootLabel);
    heading->addLayout(titles, 1);

    m_refreshButton = new QToolButton(this);
    m_refreshButton->setObjectName(QStringLiteral("takweenProjectRefresh"));
    m_refreshButton->setIcon(resourceIcon(QStringLiteral("restart")));
    m_refreshButton->setToolTip(QStringLiteral("تحديث بيانات المشروع من تكوين"));
    m_refreshButton->setAutoRaise(true);
    connect(m_refreshButton, &QToolButton::clicked,
            this, &QalamTakweenProjectView::refreshRequested);
    heading->addWidget(m_refreshButton, 0, Qt::AlignTop);
    layout->addLayout(heading);

    auto *form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setHorizontalSpacing(8);
    form->setVerticalSpacing(6);
    m_targetCombo = new QComboBox(this);
    m_targetCombo->setObjectName(QStringLiteral("takweenTargetCombo"));
    m_targetCombo->setToolTip(QStringLiteral("الهدف الذي يبنيه ويشغله قلم ويحلله خادم اللغة"));
    m_profileCombo = new QComboBox(this);
    m_profileCombo->setObjectName(QStringLiteral("takweenProfileCombo"));
    m_profileCombo->setToolTip(QStringLiteral("نمط البناء الممرر إلى تكوين عبر --نمط"));
    form->addRow(QStringLiteral("الهدف"), m_targetCombo);
    form->addRow(QStringLiteral("النمط"), m_profileCombo);
    layout->addLayout(form);

    m_messageLabel = new QLabel(this);
    m_messageLabel->setObjectName(QStringLiteral("takweenProjectMessage"));
    m_messageLabel->setWordWrap(true);
    m_messageLabel->hide();
    layout->addWidget(m_messageLabel);

    m_tree = new QTreeWidget(this);
    m_tree->setObjectName(QStringLiteral("takweenProjectTree"));
    m_tree->setColumnCount(2);
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(true);
    m_tree->setIconSize(QSize(16, 16));
    m_tree->header()->setStretchLastSection(false);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    connect(m_tree, &QTreeWidget::itemActivated, this,
            [this](QTreeWidgetItem *item) {
        const QString path = item ? item->data(0, PathRole).toString() : QString();
        if (not path.isEmpty() and QFileInfo(path).isFile()) emit fileActivated(path);
    });
    layout->addWidget(m_tree, 1);

    connect(m_targetCombo, &QComboBox::activated, this, [this]() { emitSelection(); });
    connect(m_profileCombo, &QComboBox::activated, this, [this](int index) {
        if (m_profileCombo->itemData(index, CustomProfileRole).toBool()) {
            bool accepted{};
            const QString name = QInputDialog::getText(
                this, QStringLiteral("نمط مخصص"),
                QStringLiteral("اسم النمط كما في [الأنماط.<اسم>]:"),
                QLineEdit::Normal, m_snapshot.activeProfile, &accepted).trimmed();
            rebuildSelectors();
            if (accepted and not name.isEmpty() and name != m_snapshot.activeProfile) {
                emit selectionChangeRequested(m_targetCombo->currentData().toString(), name);
            }
            return;
        }
        emitSelection();
    });

    setStyleSheet(QString(R"(
        #takweenProjectView { background-color: %1; }
        #takweenProjectTitle { color: %2; font-weight: 600; font-size: %5px; }
        #takweenProjectRoot { color: %3; }
        #takweenProjectTree { background-color: %1; border: none; }
        #takweenProjectTree::item { padding: 2px 4px; height: 22px; }
        #takweenProjectTree::item:hover { background-color: %4; }
    )")
        .arg(Colors::SidebarBackground, Colors::TextPrimary, Colors::TextMuted,
             Colors::ListHoverBackground)
        .arg(Fonts::UISize + 1));

    showMessage(QStringLiteral("افتح ملفًا داخل مشروع تكوين لعرض أهدافه وخطة بنائه."));
}

void QalamTakweenProjectView::showMessage(const QString &message)
{
    m_snapshot = Snapshot{};
    m_titleLabel->setText(QStringLiteral("لا يوجد مشروع تكوين"));
    m_rootLabel->clear();
    m_rootLabel->hide();
    m_targetCombo->clear();
    m_profileCombo->clear();
    m_targetCombo->setEnabled(false);
    m_profileCombo->setEnabled(false);
    m_refreshButton->setEnabled(false);
    m_tree->clear();
    m_messageLabel->setStyleSheet(QString("color: %1;").arg(Constants::Colors::TextMuted));
    m_messageLabel->setText(message);
    m_messageLabel->show();
}

void QalamTakweenProjectView::setSnapshot(const Snapshot &snapshot)
{
    m_snapshot = snapshot;
    const QString name = snapshot.hasPlan
        ? snapshot.plan.project : QFileInfo(snapshot.projectRoot).fileName();
    m_titleLabel->setText(QStringLiteral("مشروع %1").arg(name));
    m_rootLabel->setText(QDir::toNativeSeparators(snapshot.projectRoot));
    m_rootLabel->setToolTip(m_rootLabel->text());
    m_rootLabel->show();
    m_refreshButton->setEnabled(true);

    QString problem = snapshot.targetsError;
    if (problem.isEmpty()) problem = snapshot.planError;
    if (problem.isEmpty()) {
        m_messageLabel->hide();
    } else {
        m_messageLabel->setStyleSheet(
            QString("color: %1;").arg(Constants::Colors::WarningForeground));
        m_messageLabel->setText(QStringLiteral("⚠ ") + problem);
        m_messageLabel->show();
    }

    rebuildSelectors();
    rebuildTree();
}

void QalamTakweenProjectView::rebuildSelectors()
{
    m_updating = true;
    m_targetCombo->clear();
    m_targetCombo->addItem(QStringLiteral("افتراضي البيان"), QString());
    int targetIndex = 0;
    for (const TakweenTarget &target : m_snapshot.targets) {
        if (not selectableTarget(target)) continue;
        m_targetCombo->addItem(
            QStringLiteral("%1 — %2").arg(target.name, targetKindLabel(target.kind)),
            target.name);
        if (target.name == m_snapshot.activeTarget) targetIndex = m_targetCombo->count() - 1;
    }
    // Keep a remembered target visible even when Takween no longer lists it.
    if (targetIndex == 0 and not m_snapshot.activeTarget.isEmpty()) {
        m_targetCombo->addItem(
            QStringLiteral("%1 (غير موجود)").arg(m_snapshot.activeTarget),
            m_snapshot.activeTarget);
        targetIndex = m_targetCombo->count() - 1;
    }
    m_targetCombo->setCurrentIndex(targetIndex);

    m_profileCombo->clear();
    m_profileCombo->addItem(QStringLiteral("افتراضي البيان"), QString());
    int profileIndex = 0;
    const QStringList builtIn = {QStringLiteral("تطوير"), QStringLiteral("إصدار")};
    for (const QString &profile : builtIn) {
        m_profileCombo->addItem(profile, profile);
        if (profile == m_snapshot.activeProfile) profileIndex = m_profileCombo->count() - 1;
    }
    if (profileIndex == 0 and not m_snapshot.activeProfile.isEmpty()) {
        m_profileCombo->addItem(m_snapshot.activeProfile, m_snapshot.activeProfile);
        profileIndex = m_profileCombo->count() - 1;
    }
    m_profileCombo->addItem(QStringLiteral("نمط مخصص…"), QString());
    m_profileCombo->setItemData(m_profileCombo->count() - 1, true, CustomProfileRole);
    m_profileCombo->setCurrentIndex(profileIndex);

    const bool usable = m_snapshot.targetsError.isEmpty();
    m_targetCombo->setEnabled(usable);
    m_profileCombo->setEnabled(usable);
    m_updating = false;
}

void QalamTakweenProjectView::emitSelection()
{
    if (m_updating) return;
    const QString target = m_targetCombo->currentData().toString();
    const QString profile = m_profileCombo->currentData().toString();
    if (target == m_snapshot.activeTarget and profile == m_snapshot.activeProfile) return;
    emit selectionChangeRequested(target, profile);
}

QTreeWidgetItem *QalamTakweenProjectView::addSection(const QString &title,
                                                     const QString &iconName)
{
    auto *section = new QTreeWidgetItem(m_tree, {title});
    section->setIcon(0, resourceIcon(iconName));
    QFont font = section->font(0);
    font.setBold(true);
    section->setFont(0, font);
    section->setFlags(Qt::ItemIsEnabled);
    section->setExpanded(true);
    return section;
}

void QalamTakweenProjectView::rebuildTree()
{
    using namespace Constants;
    m_tree->clear();
    const QString root = m_snapshot.projectRoot;

    auto *targets = addSection(QStringLiteral("الأهداف"), QStringLiteral("build"));
    const QString plannedTarget = m_snapshot.hasPlan ? m_snapshot.plan.target : QString();
    for (const TakweenTarget &target : m_snapshot.targets) {
        auto *item = new QTreeWidgetItem(targets, {target.name, targetKindLabel(target.kind)});
        const bool ready = target.status == "ready";
        QStringList abilities;
        if (target.buildable) abilities << QStringLiteral("قابل للبناء");
        if (target.runnable) abilities << QStringLiteral("قابل للتشغيل");
        if (target.test) abilities << QStringLiteral("اختبار");
        item->setToolTip(0, QStringLiteral("%1 — %2%3")
                                .arg(target.name, targetStatusLabel(target.status),
                                     abilities.isEmpty()
                                         ? QString()
                                         : QStringLiteral(" (") + abilities.join(QStringLiteral("، ")) + ")"));
        item->setIcon(0, resourceIcon(target.name == plannedTarget
                                          ? QStringLiteral("run")
                                          : (ready ? QStringLiteral("file-baa")
                                                   : QStringLiteral("warning"))));
        if (not ready) {
            item->setText(1, targetStatusLabel(target.status));
            item->setForeground(1, QColor(Colors::WarningForeground));
        }
        if (target.name == plannedTarget) {
            QFont font = item->font(0);
            font.setBold(true);
            item->setFont(0, font);
        }
    }

    if (not m_snapshot.hasPlan) return;
    const TakweenBuildPlan &plan = m_snapshot.plan;

    auto *profile = addSection(QStringLiteral("النمط"), QStringLiteral("settings"));
    new QTreeWidgetItem(profile, {plan.profileName.isEmpty()
                                      ? QStringLiteral("افتراضي البيان") : plan.profileName,
                                  // Isolate the flag so RTL layout keeps it as "-O2".
                                  QStringLiteral("⁦-O%1⁩%2")
                                      .arg(plan.optimization)
                                      .arg(plan.verify ? QStringLiteral(" تحقق") : QString())});

    auto *order = addSection(QStringLiteral("ترتيب البناء"), QStringLiteral("references"));
    for (int index = 0; index < plan.targetOrder.size(); ++index) {
        new QTreeWidgetItem(order, {QStringLiteral("%1. %2")
                                        .arg(index + 1)
                                        .arg(plan.targetOrder.at(index))});
    }

    auto *sources = addSection(QStringLiteral("ملفات المصدر"), QStringLiteral("file"));
    for (const QString &source : plan.sourceFiles) {
        const QString path = resolvePlanPath(plan, root, source);
        auto *item = new QTreeWidgetItem(sources, {QDir(root).relativeFilePath(path)});
        item->setData(0, PathRole, path);
        item->setToolTip(0, QDir::toNativeSeparators(path));
        item->setIcon(0, resourceIcon(path.endsWith(QStringLiteral(".نظم"))
                                          ? QStringLiteral("file-nazm")
                                          : QStringLiteral("file-baa")));
    }

    if (not plan.includePaths.isEmpty()) {
        auto *includes = addSection(QStringLiteral("مسارات التضمين"), QStringLiteral("folder"));
        for (const QString &include : plan.includePaths) {
            const QString path = resolvePlanPath(plan, root, include);
            auto *item = new QTreeWidgetItem(includes, {QDir(root).relativeFilePath(path)});
            item->setToolTip(0, QDir::toNativeSeparators(path));
        }
    }

    auto *state = addSection(QStringLiteral("حالة المخرجات"), QStringLiteral("save"));
    const QString manifest = planOutputPath(plan, root, QStringLiteral("--emit-build-manifest"));
    const QList<QPair<QString, QString>> files = {
        {QStringLiteral("قفل الاعتماديات"), QDir(root).filePath(QStringLiteral("تكوين.قفل"))},
        {QStringLiteral("الناتج"), planOutputPath(plan, root, QStringLiteral("-o"))},
        {QStringLiteral("بيان البناء"), manifest},
        {QStringLiteral("كاش المحتوى"), manifest.isEmpty()
             ? QString()
             : QFileInfo(manifest).dir().filePath(QStringLiteral("build-cache.json"))},
    };
    for (const auto &[label, path] : files) {
        if (path.isEmpty()) continue;
        const bool present = QFileInfo::exists(path);
        auto *item = new QTreeWidgetItem(state, {label, present ? QStringLiteral("موجود")
                                                                : QStringLiteral("غير موجود")});
        item->setIcon(0, resourceIcon(present ? QStringLiteral("success")
                                              : QStringLiteral("info")));
        item->setForeground(1, QColor(present ? Colors::SuccessForeground : Colors::TextMuted));
        item->setToolTip(0, QDir::toNativeSeparators(path));
        if (present and path.endsWith(QStringLiteral(".json"))) item->setData(0, PathRole, path);
    }
}

QString QalamTakweenProjectView::resolvePlanPath(const TakweenBuildPlan &plan,
                                                 const QString &projectRoot,
                                                 const QString &path)
{
    if (QFileInfo(path).isAbsolute()) return QDir::cleanPath(path);
    const QString base = QDir(projectRoot).filePath(plan.workingDirectory);
    return QDir::cleanPath(QDir(base).filePath(path));
}

QString QalamTakweenProjectView::planOutputPath(const TakweenBuildPlan &plan,
                                                const QString &projectRoot,
                                                const QString &option)
{
    const qsizetype index = plan.argv.indexOf(option);
    if (index < 0 or index + 1 >= plan.argv.size()) return QString();
    return resolvePlanPath(plan, projectRoot, plan.argv.at(index + 1));
}
