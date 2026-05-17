#ifndef TAG_DISPLAY_H
#define TAG_DISPLAY_H

#include <QWidget>

// 极简数值显示 — 仅用于报警测试
class TagDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit TagDisplay(QWidget *parent = nullptr);
    void setTagName(const QString &name) { m_tagName = name; update(); }
    void setUnit(const QString &unit) { m_unit = unit; update(); }
    void setValue(double val);
    void setAlarmActive(bool active) { m_alarmActive = active; update(); }
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QString m_tagName;
    QString m_unit;
    double m_value = 0;
    bool m_alarmActive = false;
};
#endif
