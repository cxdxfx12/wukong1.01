#ifndef PROVINCEUTILS_H
#define PROVINCEUTILS_H

#include <QString>
#include <QStringList>

/**
 * @brief 省份名称处理工具类
 * 
 * 统一处理省份名称的标准化、匹配和模糊搜索
 * 消除代码中重复的省份处理逻辑
 */
class ProvinceUtils {
public:
    /**
     * @brief 标准化省份名称
     * @param province 原始省份名称
     * @return 标准化后的省份名称（去掉省/市/自治区等后缀）
     */
    static QString normalize(const QString &province);

    /**
     * @brief 提取省份前两字（用于模糊匹配）
     * @param province 省份名称
     * @return 前两字，如果长度不足2则返回原字符串
     */
    static QString extractTwoCharPrefix(const QString &province);

    /**
     * @brief 判断两个省份名称是否匹配
     * @param input 输入省份
     * @param target 目标省份
     * @return true 如果匹配
     */
    static bool matches(const QString &input, const QString &target);

    /**
     * @brief 获取后缀列表
     * @return 省份后缀列表（省、市、自治区等）
     */
    static const QStringList &getSuffixes();
};

#endif // PROVINCEUTILS_H
