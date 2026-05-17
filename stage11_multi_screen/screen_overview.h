#ifndef SCREEN_OVERVIEW_H
#define SCREEN_OVERVIEW_H

#include "hmi_screen.h"
#include <QVector>
#include <QLabel>

class OverviewCard;

class ScreenOverview : public HmiScreen
{
    Q_OBJECT
public:
    explicit ScreenOverview(QWidget *parent = nullptr);

    void onEnter() override;

private:
    void setupCards();

    QVector<OverviewCard *> m_cards;
};

// ── OverviewCard: self-painted quick-glance card ──────────────

class OverviewCard : public QWidget
{
    Q_OBJECT
public:
    explicit OverviewCard(const QString &tagName, const QString &unit,
                          QWidget *parent = nullptr);

    void setValue(double v);
    void setWarning(bool on);
    void setAlarm(bool on);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_tagName;
    QString m_unit;
    double  m_value    = 0;
    bool    m_warning  = false;
    bool    m_alarm    = false;
};

#endif
