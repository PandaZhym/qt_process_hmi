#ifndef NAV_BAR_H
#define NAV_BAR_H

#include <QWidget>
#include <QVector>
#include <QRectF>
#include <QTimer>
#include <QTime>

class NavBar : public QWidget
{
    Q_OBJECT
public:
    struct ButtonDef {
        QString label;
        QString icon;   // Unicode glyph
    };

    explicit NavBar(QWidget *parent = nullptr);

    void addButton(const ButtonDef &btn);
    void setActiveIndex(int index);
    int  activeIndex() const { return m_activeIndex; }
    int  buttonCount() const { return m_buttons.size(); }

    void setAlarmCount(int active, int unacked);
    void refreshClock();

signals:
    void screenSelected(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct NavButton {
        ButtonDef def;
        QRectF    bounds;
    };
    QVector<NavButton> m_buttons;
    int m_activeIndex  = -1;
    int m_hoveredIndex = -1;

    QTimer *m_clockTimer = nullptr;
    QTime   m_clockNow;

    int m_alarmActive  = 0;
    int m_alarmUnacked = 0;

    static constexpr int BTN_WIDTH  = 96;
    static constexpr int BTN_HEIGHT = 38;
    static constexpr int BTN_TOP    = 5;
    static constexpr int BAR_HEIGHT = 48;

    QRectF buttonRect(int index) const;
    void   recalcLayout();
};

#endif
