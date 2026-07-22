#include "TakweenProtocol.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>

#include <cmath>
#include <limits>

namespace {
void setError(QString *error, const QString &message)
{
    if (error) *error = message;
}

bool requiredString(const QJsonObject &object,
                    const QString &key,
                    QString *value,
                    QString *error)
{
    const QJsonValue field = object.value(key);
    if (not field.isString() or field.toString().isEmpty()) {
        setError(error, QString("الحقل المطلوب %1 يجب أن يكون نصا غير فارغ.").arg(key));
        return false;
    }
    *value = field.toString();
    return true;
}

bool requiredBool(const QJsonObject &object,
                  const QString &key,
                  bool *value,
                  QString *error)
{
    const QJsonValue field = object.value(key);
    if (not field.isBool()) {
        setError(error, QString("الحقل المطلوب %1 يجب أن يكون منطقيا.").arg(key));
        return false;
    }
    *value = field.toBool();
    return true;
}

bool requiredInteger(const QJsonObject &object,
                     const QString &key,
                     qint64 minimum,
                     qint64 maximum,
                     qint64 *value,
                     QString *error)
{
    const QJsonValue field = object.value(key);
    if (not field.isDouble()) {
        setError(error, QString("الحقل المطلوب %1 يجب أن يكون عددا صحيحا.").arg(key));
        return false;
    }
    const double number = field.toDouble();
    if (not std::isfinite(number) or std::floor(number) != number or
        number < static_cast<double>(minimum) or number > static_cast<double>(maximum)) {
        setError(error, QString("الحقل %1 خارج مجال العدد الصحيح المقبول.").arg(key));
        return false;
    }
    *value = static_cast<qint64>(number);
    return true;
}

bool parseObject(const QByteArray &json, QJsonObject *object, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError or not document.isObject()) {
        setError(error, QString("JSON غير صالح: %1").arg(parseError.errorString()));
        return false;
    }
    *object = document.object();
    return true;
}

QString operationName(const QString &operation)
{
    if (operation == "build") return "البناء";
    if (operation == "check") return "الفحص";
    if (operation == "run") return "التشغيل";
    if (operation == "test") return "الاختبار";
    if (operation == "clean") return "التنظيف";
    return operation;
}

QString phaseName(const QString &phase)
{
    if (phase == "plan") return "التخطيط";
    if (phase == "prepare_output") return "تجهيز المخرجات";
    if (phase == "compiler") return "الترجمة";
    if (phase == "compiler_check") return "فحص المصرّف";
    if (phase == "cache_receipt") return "تثبيت الكاش";
    if (phase == "build") return "البناء";
    if (phase == "program") return "البرنامج";
    if (phase == "clean_output") return "تنظيف المخرجات";
    return phase;
}
}

bool TakweenProtocol::parseTargets(const QByteArray &json,
                                   QVector<TakweenTarget> *targets,
                                   QString *error)
{
    if (not targets) {
        setError(error, "وجهة الأهداف غير موجودة.");
        return false;
    }
    targets->clear();

    QJsonObject root;
    if (not parseObject(json, &root, error)) return false;
    if (root.value("schema_version").toString() != "takween-targets-v1") {
        setError(error, "إصدار عقد أهداف تكوين غير مدعوم.");
        return false;
    }
    if (not root.value("targets").isArray()) {
        setError(error, "حقل targets المطلوب ليس مصفوفة.");
        return false;
    }

    const QJsonArray records = root.value("targets").toArray();
    QSet<QString> names;
    QVector<TakweenTarget> parsed;
    parsed.reserve(records.size());
    for (const QJsonValue &record : records) {
        if (not record.isObject()) {
            setError(error, "كل سجل هدف يجب أن يكون كائن JSON.");
            return false;
        }
        const QJsonObject object = record.toObject();
        TakweenTarget target;
        if (not requiredString(object, "name", &target.name, error) or
            not requiredString(object, "kind", &target.kind, error) or
            not requiredString(object, "status", &target.status, error) or
            not requiredBool(object, "buildable", &target.buildable, error) or
            not requiredBool(object, "runnable", &target.runnable, error) or
            not requiredBool(object, "test", &target.test, error)) {
            return false;
        }
        if (names.contains(target.name)) {
            setError(error, "عقد أهداف تكوين يحتوي اسما مكررا: " + target.name);
            return false;
        }
        names.insert(target.name);
        parsed.push_back(target);
    }

    *targets = parsed;
    return true;
}

bool TakweenProtocol::parseBuildEvent(const QByteArray &line,
                                      TakweenBuildEvent *event,
                                      QString *error)
{
    if (not event) {
        setError(error, "وجهة حدث البناء غير موجودة.");
        return false;
    }
    *event = TakweenBuildEvent{};

    QJsonObject root;
    if (not parseObject(line, &root, error)) return false;
    if (root.value("schema_version").toString() != "takween-build-events-v1") {
        setError(error, "إصدار عقد أحداث تكوين غير مدعوم.");
        return false;
    }

    event->schemaVersion = "takween-build-events-v1";
    if (not requiredInteger(root, "sequence", 1, std::numeric_limits<int>::max(),
                         &event->sequence, error) or
        not requiredString(root, "event", &event->event, error) or
        not requiredString(root, "operation", &event->operation, error)) {
        return false;
    }

    const QSet<QString> operations = {"build", "check", "run", "test", "clean"};
    const QSet<QString> eventTypes = {
        "operation_started", "operation_finished", "phase_started", "phase_finished",
        "target_started", "target_finished", "package_started", "package_finished",
        "artifact"
    };
    if (not operations.contains(event->operation) or not eventTypes.contains(event->event)) {
        setError(error, "اسم العملية أو نوع الحدث غير معروف في v1.");
        return false;
    }

    const bool isFinished = event->event.endsWith("_finished");
    const bool isStarted = event->event.endsWith("_started");
    if (isStarted or isFinished) {
        if (not requiredString(root, "status", &event->status, error)) return false;
        if ((isStarted and event->status != "started") or
            (isFinished and event->status != "succeeded" and event->status != "failed")) {
            setError(error, "حالة حدث البداية أو النهاية غير متوافقة مع نوعه.");
            return false;
        }
    }
    if (isFinished) {
        qint64 exitCode = 0;
        if (not requiredInteger(root, "exit_code", std::numeric_limits<int>::min(),
                             std::numeric_limits<int>::max(), &exitCode, error)) {
            return false;
        }
        event->hasExitCode = true;
        event->exitCode = static_cast<int>(exitCode);
    }

    if (event->event.startsWith("operation_")) {
        if (not requiredString(root, "phase", &event->phase, error) or
            event->phase != "operation") {
            setError(error, "حدث العملية يجب أن يحمل phase=operation.");
            return false;
        }
    } else if (event->event.startsWith("phase_")) {
        if (not requiredString(root, "phase", &event->phase, error)) return false;
    } else if (event->event.startsWith("target_")) {
        if (not requiredString(root, "target", &event->target, error)) return false;
    } else if (event->event.startsWith("package_")) {
        if (not requiredString(root, "package", &event->package, error)) return false;
    } else if (event->event == "artifact") {
        if (not root.value("artifact").isObject()) {
            setError(error, "حدث artifact يحتاج كائن artifact.");
            return false;
        }
        const QJsonObject artifact = root.value("artifact").toObject();
        if (not requiredString(artifact, "kind", &event->artifactKind, error) or
            not requiredString(artifact, "path", &event->artifactPath, error)) {
            return false;
        }
    }

    return true;
}

bool TakweenProtocol::validateTransition(const TakweenBuildEvent &event,
                                         const QString &expectedOperation,
                                         qint64 previousSequence,
                                         bool terminalSeen,
                                         QString *error)
{
    if (terminalSeen) {
        setError(error, "وصل حدث بعد operation_finished.");
        return false;
    }
    if (event.operation != expectedOperation) {
        setError(error, "اسم العملية لا يطابق الأمر الجاري.");
        return false;
    }
    if (event.sequence != previousSequence + 1) {
        setError(error, "تسلسل الأحداث غير متصل.");
        return false;
    }
    if (event.sequence == 1 and event.event != "operation_started") {
        setError(error, "الحدث الأول ليس operation_started.");
        return false;
    }
    return true;
}

bool TakweenProtocol::validateCompletion(int processExitCode,
                                         bool cancellationRequested,
                                         bool protocolFailed,
                                         bool terminalSeen,
                                         int terminalExitCode,
                                         QString *error)
{
    if (cancellationRequested) return true;
    if (protocolFailed) {
        setError(error, "فشل التحقق من أحداث تكوين.");
        return false;
    }
    if (not terminalSeen) {
        setError(error, "انتهت العملية دون operation_finished.");
        return false;
    }
    if (terminalExitCode != processExitCode) {
        setError(error, "كود الحدث النهائي لا يطابق كود العملية.");
        return false;
    }
    return true;
}

QString TakweenProtocol::progressText(const TakweenBuildEvent &event)
{
    if (event.event == "operation_started") {
        return "بدأ " + operationName(event.operation) + "…";
    }
    if (event.event == "operation_finished") {
        return event.status == "succeeded"
            ? "اكتمل " + operationName(event.operation) + " بنجاح."
            : "فشل " + operationName(event.operation) + ".";
    }
    if (event.event == "phase_started") return phaseName(event.phase) + "…";
    if (event.event == "phase_finished" and event.status == "failed") {
        return "فشلت مرحلة " + phaseName(event.phase) + ".";
    }
    if (event.event == "target_started") return "اختبار الهدف " + event.target + "…";
    if (event.event == "package_started") return "معالجة الحزمة " + event.package + "…";
    if (event.event == "artifact") return "أنتج تكوين: " + event.artifactPath;
    return QString();
}
