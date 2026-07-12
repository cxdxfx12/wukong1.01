#include "CalculationRule.h"
#include <cmath>

CalculationRule::Mode CalculationRule::modeFromString(const QString &str) {
    if (str == QStringLiteral("实际重量")) return Mode::ActualWeight;
    if (str == QStringLiteral("拉均重"))   return Mode::AverageWeight;
    if (str == QStringLiteral("百克续"))   return Mode::HundredGram;
    if (str == QStringLiteral("全续"))     return Mode::FullAdditional;
    return Mode::ActualWeight;
}

QString CalculationRule::modeToString(CalculationRule::Mode mode) {
    switch (mode) {
    case Mode::ActualWeight:     return QStringLiteral("实际重量");
    case Mode::AverageWeight:    return QStringLiteral("拉均重");
    case Mode::HundredGram:      return QStringLiteral("百克续");
    case Mode::FullAdditional:   return QStringLiteral("全续");
    }
    return QStringLiteral("实际重量");
}

QStringList CalculationRule::allModes() {
    return {QStringLiteral("实际重量"), QStringLiteral("拉均重"),
            QStringLiteral("百克续"), QStringLiteral("全续")};
}

double CalculationRule::getEffectiveWeight(double actualWeight, double volumetricWeight, Mode mode) {
    double maxWeight = std::max(actualWeight, volumetricWeight);
    switch (mode) {
    case Mode::ActualWeight:     return maxWeight;
    case Mode::AverageWeight:    return maxWeight;
    case Mode::HundredGram:      return maxWeight;
    case Mode::FullAdditional:   return maxWeight;
    }
    return maxWeight;
}

bool CalculationRule::isWeightInRange(double weight, const WeightSegment &segment) {
    // 左闭右闭区间：[minWeight, maxWeight]
    if (weight < segment.minWeight) return false;
    if (segment.maxWeight < 0) return true;
    return weight <= segment.maxWeight;
}

double CalculationRule::calculateStandard(double weight, const WeightSegment &segment) {
    double firstWeightPrice = segment.firstWeightPrice;
    double additionalPrice = segment.additionalPrice;
    double firstWeight = segment.minWeight;

    if (segment.maxWeight < 0 && segment.minWeight >= 30) {
        firstWeight = 3.0;
    }

    double extraWeight = weight - firstWeight;
    if (extraWeight < 0) extraWeight = 0;
    return firstWeightPrice + extraWeight * additionalPrice;
}

double CalculationRule::calculateStandard(double weight, const QList<WeightSegment> &segments) {
    if (segments.isEmpty()) return 0.0;

    const WeightSegment* matchedSeg = nullptr;
    double maxFirstWeightPrice = 0.0;

    for (const WeightSegment& seg : segments) {
        if (seg.firstWeightPrice > maxFirstWeightPrice) {
            maxFirstWeightPrice = seg.firstWeightPrice;
        }
        if (isWeightInRange(weight, seg)) {
            matchedSeg = &seg;
            break;
        }
    }

    if (!matchedSeg) return 0.0;

    double result = calculateStandard(weight, *matchedSeg);

    if (result < maxFirstWeightPrice) {
        result = maxFirstWeightPrice;
    }

    return result;
}

double CalculationRule::calculateHundredGram(double weight, const WeightSegment &segment) {
    double minWeight = segment.minWeight;
    double firstWeightPrice = segment.firstWeightPrice;
    double additionalPrice = segment.additionalPrice;

    double extraWeight = weight - minWeight;
    int hundredGramUnits = static_cast<int>(std::ceil(extraWeight * 10));
    if (hundredGramUnits < 0) hundredGramUnits = 0;
    return firstWeightPrice + hundredGramUnits * (additionalPrice / 10.0);
}

double CalculationRule::calculateHundredGram(double weight, const QList<WeightSegment> &segments) {
    if (segments.isEmpty()) return 0.0;

    const WeightSegment* matchedSeg = nullptr;
    double maxFirstWeightPrice = 0.0;

    for (const WeightSegment& seg : segments) {
        if (seg.firstWeightPrice > maxFirstWeightPrice) {
            maxFirstWeightPrice = seg.firstWeightPrice;
        }
        if (isWeightInRange(weight, seg)) {
            matchedSeg = &seg;
            break;
        }
    }

    if (!matchedSeg) return 0.0;

    double result = calculateHundredGram(weight, *matchedSeg);

    if (result < maxFirstWeightPrice) {
        result = maxFirstWeightPrice;
    }

    return result;
}

double CalculationRule::calculateFullAdditional(double weight, const WeightSegment &segment) {
    double fullWeight = std::ceil(weight);
    return calculateStandard(fullWeight, segment);
}

double CalculationRule::calculateFullAdditional(double weight, const QList<WeightSegment> &segments) {
    if (segments.isEmpty()) return 0.0;

    double fullWeight = std::ceil(weight);

    const WeightSegment* matchedSeg = nullptr;
    double maxFirstWeightPrice = 0.0;

    for (const WeightSegment& seg : segments) {
        if (seg.firstWeightPrice > maxFirstWeightPrice) {
            maxFirstWeightPrice = seg.firstWeightPrice;
        }
        if (isWeightInRange(fullWeight, seg)) {
            matchedSeg = &seg;
            break;
        }
    }

    if (!matchedSeg) return 0.0;

    double result = calculateStandard(fullWeight, *matchedSeg);

    if (result < maxFirstWeightPrice) {
        result = maxFirstWeightPrice;
    }

    return result;
}

double CalculationRule::applyPriceIncreases(double freight, double weight,
                                              const QList<PriceIncreaseRule> &rules)
{
    double result = freight;
    for (const PriceIncreaseRule &rule : rules) {
        if (!rule.isActive) continue;
        switch (rule.mode) {
        case IncreaseMode::PerTicketFixed:
            result += rule.amount;
            break;
        case IncreaseMode::PerTicketPercent:
            result *= (1.0 + rule.amount);
            break;
        case IncreaseMode::PerKg:
            result += weight * rule.amount;
            break;
        }
    }
    return result;
}

double CalculationRule::applyPriceIncreases(double freight, double weight,
                                              const QList<PriceIncreaseRule> &rules,
                                              const QDateTime &timeFilter)
{
    double result = freight;
    for (const PriceIncreaseRule &rule : rules) {
        if (!rule.isActive) continue;
        if (!rule.isTimeInRange(timeFilter)) continue;
        switch (rule.mode) {
        case IncreaseMode::PerTicketFixed:
            result += rule.amount;
            break;
        case IncreaseMode::PerTicketPercent:
            result *= (1.0 + rule.amount);
            break;
        case IncreaseMode::PerKg:
            result += weight * rule.amount;
            break;
        }
    }
    return result;
}
