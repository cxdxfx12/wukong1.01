#include "UI/ChartDialog.h"
#include "UI/ChartWidgets.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QPixmap>
#include <QApplication>
#include <QDateTime>
#include <algorithm>

ChartDialog::ChartDialog(const QList<OrderData> &orders,
                         const QString &calcMode,
                         QWidget *parent)
    : QDialog(parent)
    , m_calcMode(calcMode)
{
    // 计算统计数据
    m_stats = computeCustomerStats(orders);

    // 按运费降序排序
    std::sort(m_stats.begin(), m_stats.end(), CustomerStats::sortByFreight);

    setupUi();
}

void ChartDialog::setupUi()
{
    setWindowTitle(QStringLiteral("计算统计图表"));
    resize(960, 680);
    setMinimumSize(800, 560);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    // 标题
    auto *titleLabel = new QLabel(this);
    QString titleText = QStringLiteral("运费结算统计");
    if (!m_calcMode.isEmpty())
        titleText += QStringLiteral(" — %1模式").arg(m_calcMode);
    titleLabel->setText(titleText);
    titleLabel->setStyleSheet(
        QStringLiteral("font-size: 16px; font-weight: bold; color: #4a90d9;"));
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 概要统计卡片
    auto *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(10);

    int totalCount = 0;
    double totalFreight = 0.0;
    double totalWeight = 0.0;
    for (const CustomerStats &s : m_stats) {
        totalCount += s.shipmentCount;
        totalFreight += s.totalFreight;
        totalWeight += s.totalWeight;
    }
    double avgWeightAll = totalCount > 0 ? totalWeight / totalCount : 0.0;

    m_cardTotal = new StatCardWidget(
        QStringLiteral("总件数"), QString::number(totalCount),
        QColor("#4a90d9"), this);
    m_cardFreight = new StatCardWidget(
        QStringLiteral("总运费"), QStringLiteral("¥%1").arg(totalFreight, 0, 'f', 2),
        QColor("#27ae60"), this);
    m_cardAvgWeight = new StatCardWidget(
        QStringLiteral("平均计费重量"), QStringLiteral("%1kg").arg(avgWeightAll, 0, 'f', 2),
        QColor("#e8a838"), this);
    m_cardCustomers = new StatCardWidget(
        QStringLiteral("客户数"), QString::number(m_stats.size()),
        QColor("#8e44ad"), this);

    cardsLayout->addWidget(m_cardTotal);
    cardsLayout->addWidget(m_cardFreight);
    cardsLayout->addWidget(m_cardAvgWeight);
    cardsLayout->addWidget(m_cardCustomers);
    mainLayout->addLayout(cardsLayout);

    // 标签页
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(
        QStringLiteral("QTabWidget::pane { border: 1px solid #d0d0d0; border-radius: 6px; "
                       "background: white; }"
                       "QTabBar::tab { padding: 8px 20px; font-size: 12px; "
                       "border: 1px solid #d0d0d0; border-bottom: none; "
                       "border-top-left-radius: 6px; border-top-right-radius: 6px; "
                       "margin-right: 2px; background: #f5f6fa; color: #666; }"
                       "QTabBar::tab:selected { background: white; color: #4a90d9; "
                       "font-weight: bold; border-bottom: 2px solid #4a90d9; }"
                       "QTabBar::tab:hover { background: #e8ecf5; }"));

    m_tabWidget->addTab(createOverviewTab(), QStringLiteral("📊 综合概览"));
    m_tabWidget->addTab(createCountTab(), QStringLiteral("📦 件数分布"));
    m_tabWidget->addTab(createFreightTab(), QStringLiteral("💰 运费统计"));
    m_tabWidget->addTab(createAverageTab(), QStringLiteral("📈 平均数据"));
    mainLayout->addWidget(m_tabWidget, 1);

    // 底部按钮
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    auto *exportBtn = new QPushButton(QStringLiteral("导出图表"), this);
    exportBtn->setMinimumHeight(32);
    exportBtn->setMinimumWidth(100);
    exportBtn->setStyleSheet(
        QStringLiteral("QPushButton { background-color: #27ae60; color: white; border: none; "
                       "border-radius: 4px; padding: 6px 20px; font-size: 12px; }"
                       "QPushButton:hover { background-color: #219a52; }"));

    btnLayout->addWidget(exportBtn);
    btnLayout->addStretch();

    auto *closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    closeBtn->setMinimumHeight(32);
    closeBtn->setMinimumWidth(100);
    closeBtn->setStyleSheet(
        QStringLiteral("QPushButton { background-color: #4a90d9; color: white; border: none; "
                       "border-radius: 4px; padding: 6px 20px; font-size: 12px; }"
                       "QPushButton:hover { background-color: #3a7bc8; }"));
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);

    connect(exportBtn, &QPushButton::clicked, this, &ChartDialog::onExportImage);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

// ============================================================================
// 综合概览 Tab — 概览卡片 + Top 10 客户条形图
// ============================================================================
QWidget* ChartDialog::createOverviewTab()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(10);
    layout->setContentsMargins(12, 12, 12, 12);

    // 取前10个客户做展示
    int topN = qMin(10, m_stats.size());
    QStringList categories;
    QList<double> freightValues;
    for (int i = 0; i < topN; ++i) {
        categories << m_stats[i].customerName;
        freightValues << m_stats[i].totalFreight;
    }

    auto *barChart = new BarChartWidget(page);
    barChart->setTitle(QStringLiteral("Top %1 客户运费排名").arg(topN));
    barChart->setData(categories, freightValues, QStringLiteral(" 元"),
                      BarChartWidget::Horizontal, BarChartWidget::ShowValue);
    barChart->setMinimumHeight(qMax(250, topN * 36 + 40));
    layout->addWidget(barChart);

    // 底部提示
    int totalC = 0;
    double totalF = 0.0;
    for (const CustomerStats &s : m_stats) {
        totalC += s.shipmentCount;
        totalF += s.totalFreight;
    }
    auto *hint = new QLabel(
        QStringLiteral("共 %1 个客户 | 总件数 %2 | 总运费 ¥%3")
            .arg(m_stats.size())
            .arg(totalC)
            .arg(totalF, 0, 'f', 2),
        page);
    hint->setStyleSheet(QStringLiteral("color: #999; font-size: 10px;"));
    hint->setAlignment(Qt::AlignCenter);
    layout->addWidget(hint);

    return page;
}

// ============================================================================
// 件数分布 Tab — 环形图 + 条形图
// ============================================================================
QWidget* ChartDialog::createCountTab()
{
    auto *page = new QWidget(this);
    auto *layout = new QHBoxLayout(page);
    layout->setSpacing(10);
    layout->setContentsMargins(12, 12, 12, 12);

    // 环形图数据（取前 10 个 + "其他"）
    QList<QPair<QString, double>> pieData;
    int totalCount = 0;
    for (const CustomerStats &s : m_stats)
        totalCount += s.shipmentCount;

    int topN = qMin(9, m_stats.size());
    int shownCount = 0;
    for (int i = 0; i < topN; ++i) {
        pieData.append({m_stats[i].customerName, (double)m_stats[i].shipmentCount});
        shownCount += m_stats[i].shipmentCount;
    }
    int otherCount = totalCount - shownCount;
    if (otherCount > 0 && m_stats.size() > topN) {
        pieData.append({QStringLiteral("其他"), (double)otherCount});
    }

    auto *donut = new DonutChartWidget(page);
    donut->setData(pieData, QStringLiteral("总件数\n%1").arg(totalCount));
    donut->setMinimumWidth(420);
    layout->addWidget(donut);

    // 右侧条形图
    auto *rightPanel = new QWidget(page);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);

    QStringList categories;
    QList<double> countValues;
    int showN = qMin(10, m_stats.size());
    for (int i = 0; i < showN; ++i) {
        categories << m_stats[i].customerName;
        countValues << (double)m_stats[i].shipmentCount;
    }

    auto *barChart = new BarChartWidget(rightPanel);
    barChart->setTitle(QStringLiteral("客户件数排名"));
    barChart->setData(categories, countValues, QStringLiteral(" 件"),
                      BarChartWidget::Horizontal, BarChartWidget::ShowValue);
    barChart->setMinimumHeight(qMax(220, showN * 36 + 40));
    rightLayout->addWidget(barChart);

    layout->addWidget(rightPanel);

    return page;
}

// ============================================================================
// 运费统计 Tab — 环形图 + 柱形图
// ============================================================================
QWidget* ChartDialog::createFreightTab()
{
    auto *page = new QWidget(this);
    auto *layout = new QHBoxLayout(page);
    layout->setSpacing(10);
    layout->setContentsMargins(12, 12, 12, 12);

    // 环形图
    double totalFreight = 0.0;
    for (const CustomerStats &s : m_stats)
        totalFreight += s.totalFreight;

    QList<QPair<QString, double>> pieData;
    int topN = qMin(9, m_stats.size());
    double shownFreight = 0.0;
    for (int i = 0; i < topN; ++i) {
        pieData.append({m_stats[i].customerName, m_stats[i].totalFreight});
        shownFreight += m_stats[i].totalFreight;
    }
    double otherFreight = totalFreight - shownFreight;
    if (otherFreight > 0.01 && m_stats.size() > topN) {
        pieData.append({QStringLiteral("其他"), otherFreight});
    }

    auto *donut = new DonutChartWidget(page);
    donut->setData(pieData, QStringLiteral("总运费\n¥%1").arg(totalFreight, 0, 'f', 0));
    donut->setMinimumWidth(420);
    layout->addWidget(donut);

    // 右侧柱形图
    auto *rightPanel = new QWidget(page);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);

    QStringList categories;
    QList<double> freightValues;
    int showN = qMin(10, m_stats.size());
    for (int i = 0; i < showN; ++i) {
        categories << m_stats[i].customerName;
        freightValues << m_stats[i].totalFreight;
    }

    auto *barChart = new BarChartWidget(rightPanel);
    barChart->setTitle(QStringLiteral("客户运费排名"));
    barChart->setData(categories, freightValues, QStringLiteral(" 元"),
                      BarChartWidget::Horizontal, BarChartWidget::ShowValue);
    barChart->setMinimumHeight(qMax(220, showN * 36 + 40));
    rightLayout->addWidget(barChart);

    // 平均运费
    double avgFreight = 0.0;
    int totalCount = 0;
    for (const CustomerStats &s : m_stats) {
        avgFreight += s.totalFreight;
        totalCount += s.shipmentCount;
    }
    avgFreight = totalCount > 0 ? avgFreight / totalCount : 0.0;

    auto *hint = new QLabel(
        QStringLiteral("平均单件运费: ¥%1").arg(avgFreight, 0, 'f', 2), rightPanel);
    hint->setStyleSheet(QStringLiteral("color: #888; font-size: 10px;"));
    hint->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(hint);

    layout->addWidget(rightPanel);

    return page;
}

// ============================================================================
// 平均数据 Tab — 平均重量 + 平均运费柱形图
// ============================================================================
QWidget* ChartDialog::createAverageTab()
{
    auto *page = new QWidget(this);
    auto *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *inner = new QWidget();
    auto *layout = new QVBoxLayout(inner);
    layout->setSpacing(16);
    layout->setContentsMargins(12, 12, 12, 12);

    int showN = qMin(15, m_stats.size());

    // 平均重量柱形图
    {
        QStringList categories;
        QList<double> avgWeights;
        for (int i = 0; i < showN; ++i) {
            categories << m_stats[i].customerName;
            avgWeights << m_stats[i].avgWeight();
        }

        auto *chart = new BarChartWidget(inner);
        chart->setTitle(QStringLiteral("客户平均计费重量 (kg)"));
        chart->setData(categories, avgWeights, QStringLiteral(" kg"),
                       BarChartWidget::Vertical, BarChartWidget::ShowValue);
        chart->setMinimumHeight(280);
        layout->addWidget(chart);
    }

    // 平均运费柱形图
    {
        QStringList categories;
        QList<double> avgFreights;
        for (int i = 0; i < showN; ++i) {
            categories << m_stats[i].customerName;
            avgFreights << m_stats[i].avgFreight();
        }

        auto *chart = new BarChartWidget(inner);
        chart->setTitle(QStringLiteral("客户平均单件运费 (元)"));
        chart->setData(categories, avgFreights, QStringLiteral(" 元"),
                       BarChartWidget::Vertical, BarChartWidget::ShowValue);
        chart->setMinimumHeight(280);
        layout->addWidget(chart);
    }

    // 平均实际重量 vs 平均体积重
    {
        QStringList categories;
        QList<double> avgActWeights, avgVolWeights;
        for (int i = 0; i < showN; ++i) {
            categories << m_stats[i].customerName;
            avgActWeights << m_stats[i].avgActualWeight();
            avgVolWeights << (m_stats[i].totalVolumetricWeight / qMax(m_stats[i].shipmentCount, 1));
        }

        auto *chart = new BarChartWidget(inner);
        chart->setTitle(QStringLiteral("客户平均实际重量 (kg)"));
        chart->setData(categories, avgActWeights, QStringLiteral(" kg"),
                       BarChartWidget::Vertical, BarChartWidget::ShowValue);
        chart->setMinimumHeight(280);
        layout->addWidget(chart);
    }

    layout->addStretch();

    auto *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);

    return page;
}

// ============================================================================
// 导出为图片
// ============================================================================
void ChartDialog::onExportImage()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("导出图表"),
        QStringLiteral("运费统计图表_%1.png")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        QStringLiteral("PNG图片 (*.png);;JPEG图片 (*.jpg)"));

    if (filePath.isEmpty())
        return;

    // 渲染整个对话框为图片
    QPixmap pixmap(size());
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    render(&pixmap);

    if (pixmap.save(filePath)) {
        QMessageBox::information(this, QStringLiteral("导出成功"),
                                 QStringLiteral("图表已保存到:\n%1").arg(filePath));
    } else {
        QMessageBox::warning(this, QStringLiteral("导出失败"),
                             QStringLiteral("无法保存图片，请检查路径和权限。"));
    }
}
