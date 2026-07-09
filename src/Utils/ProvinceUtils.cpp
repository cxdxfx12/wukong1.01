#include "ProvinceUtils.h"

const QStringList &ProvinceUtils::getSuffixes()
{
    static const QStringList suffixes = {
        QStringLiteral("省"), QStringLiteral("市"),
        QStringLiteral("自治区"), QStringLiteral("特别行政区")
    };
    return suffixes;
}

QString ProvinceUtils::normalize(const QString &province)
{
    QString s = province.trimmed();
    const QStringList &suffixes = getSuffixes();
    for (const QString &sf : suffixes) {
        if (s.endsWith(sf)) {
            s.chop(sf.size());
            break;
        }
    }
    return s.trimmed();
}

QString ProvinceUtils::extractTwoCharPrefix(const QString &province)
{
    QString normalized = normalize(province);
    if (normalized.size() >= 2) {
        return normalized.left(2);
    }
    return normalized;
}

bool ProvinceUtils::matches(const QString &input, const QString &target)
{
    QString normInput = normalize(input);
    QString normTarget = normalize(target);

    // 精确匹配
    if (normInput == normTarget) {
        return true;
    }

    // contains 模糊匹配
    if (normInput.contains(normTarget) || normTarget.contains(normInput)) {
        return true;
    }

    // 前两字前缀匹配
    if (normInput.size() >= 2 && normTarget.size() >= 2 &&
        normInput.left(2) == normTarget.left(2)) {
        return true;
    }

    return false;
}
