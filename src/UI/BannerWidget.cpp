#include "BannerWidget.h"
#include <QPainterPath>
#include <QFont>
#include <QTimer>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QtMath>

BannerWidget::BannerWidget(QWidget *parent)
    : QWidget(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &BannerWidget::showNext);
    m_timer->start(m_interval);
    setMinimumHeight(60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
}

void BannerWidget::setInterval(int ms) { m_interval = ms; }

void BannerWidget::showNext() { m_current = (m_current + 1) % 4; update(); }

void BannerWidget::mousePressEvent(QMouseEvent *)
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.hbdxm.com/")));
}

void BannerWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    drawSlide(p, m_current);
}

static void drawDecorCircle(QPainter &p, qreal cx, qreal cy, qreal r, QColor c, int a)
{
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(c.red(),c.green(),c.blue(),a), 2));
    p.drawEllipse(QPointF(cx,cy), r, r);
}

void BannerWidget::drawSlide(QPainter &p, int index)
{
    QRectF r = rect();
    int W = r.width(), H = height();
    if (W < 100) return;

    QFont font(QStringLiteral("Microsoft YaHei"), H > 70 ? 16 : 13, QFont::Bold);

    switch (index) {
    case 0: {
        QLinearGradient bg(0,0,W,0);
        bg.setColorAt(0.0,QColor("#0b1a3b")); bg.setColorAt(1.0,QColor("#152a55"));
        p.setBrush(bg); p.setPen(Qt::NoPen); p.drawRoundedRect(r,8,8);
        drawDecorCircle(p, W-70, H/2, 44, QColor("#5b9bd5"), 40);
        drawDecorCircle(p, W-70, H/2, 28, QColor("#5b9bd5"), 80);
        p.setFont(font);
        p.setPen(QColor("#ffb347"));
        p.drawText(r, Qt::AlignCenter, QStringLiteral("买计费软件 送公司整体运维     杭州喵喵至家网络有限公司"));
        break;
    }
    case 1: {
        QLinearGradient bg(0,0,W,0);
        bg.setColorAt(0.0,QColor("#0a2e2e")); bg.setColorAt(1.0,QColor("#134e4a"));
        p.setBrush(bg); p.setPen(Qt::NoPen); p.drawRoundedRect(r,8,8);
        p.setPen(QPen(QColor(255,255,255,12), 1));
        for (int i=0;i<6;i++) p.drawLine(QPointF(W-180, H*0.15+i*H*0.12), QPointF(W-40, H*0.15+i*H*0.12));
        p.setFont(font);
        p.setPen(QColor("#5eead4"));
        p.drawText(r, Qt::AlignCenter, QStringLiteral("百万行数据秒级处理        SAX流式解析 + SIMD加速 + 多线程并行"));
        break;
    }
    case 2: {
        QLinearGradient bg(0,0,W,0);
        bg.setColorAt(0.0,QColor("#1e0a3b")); bg.setColorAt(1.0,QColor("#3b1f6e"));
        p.setBrush(bg); p.setPen(Qt::NoPen); p.drawRoundedRect(r,8,8);
        QPainterPath dia; qreal dx=W-80, dy=H/2, ds=28;
        dia.moveTo(dx,dy-ds); dia.lineTo(dx+ds,dy); dia.lineTo(dx,dy+ds); dia.lineTo(dx-ds,dy); dia.closeSubpath();
        p.setPen(QPen(QColor(255,255,255,20), 2)); p.setBrush(Qt::NoBrush); p.drawPath(dia);
        p.setFont(font);
        p.setPen(QColor("#c4b5fd"));
        p.drawText(r, Qt::AlignCenter, QStringLiteral("三通一达 + 顺丰 + 京东全覆盖        6套报价模板 一键切换"));
        break;
    }
    case 3: {
        QLinearGradient bg(0,0,W,0);
        bg.setColorAt(0.0,QColor("#052e16")); bg.setColorAt(1.0,QColor("#14532d"));
        p.setBrush(bg); p.setPen(Qt::NoPen); p.drawRoundedRect(r,8,8);
        p.setPen(QPen(QColor(255,255,255,10), 1));
        qreal gx=W-150, gy=H*0.25, gs=14, gap=5;
        for(int row=0;row<3;row++) for(int col=0;col<4;col++)
            p.drawRect(QRectF(gx+col*(gs+gap), gy+row*(gs+gap), gs, gs));
        p.setFont(font);
        p.setPen(QColor("#4ade80"));
        p.drawText(r, Qt::AlignCenter, QStringLiteral("智能报价精准计费        4种计算模式 + 3种加价策略 + 快速测试"));
        break;
    }
    }
}
