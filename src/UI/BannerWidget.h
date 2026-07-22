#ifndef BANNERWIDGET_H
#define BANNERWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QString>
#include <QColor>
#include <QPainter>

class BannerWidget : public QWidget {
    Q_OBJECT
public:
    explicit BannerWidget(QWidget *parent = nullptr);
    void setInterval(int ms);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private slots:
    void showNext();

private:
    void drawSlide(QPainter &p, int index);

    QTimer *m_timer;
    int m_current = 0;
    int m_interval = 4000;
    float m_fadeProgress = 1.0f; // unused for now, simple swap
};

#endif
