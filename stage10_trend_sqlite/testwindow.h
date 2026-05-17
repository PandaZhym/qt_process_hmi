#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QElapsedTimer>
#include <QtMath>

class TrendChart;
class DataLogger;

class TestWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = nullptr);
    ~TestWindow() override;

private slots:
    void onSimTick();
    void onQueryHistory();
    void onRealtimeMode();
    void onLoggerError(const QString &msg);

private:
    void setupUi();
    void setupSimulation();

    TrendChart  *m_chart   = nullptr;
    DataLogger  *m_logger  = nullptr;
    QTimer      *m_simTimer = nullptr;
    QElapsedTimer m_elapsed;

    QLabel *m_lblTemp  = nullptr;
    QLabel *m_lblPress = nullptr;
    QLabel *m_lblTank  = nullptr;
    QLabel *m_lblFlow  = nullptr;
    QLabel *m_lblPump  = nullptr;
    QLabel *m_lblValve = nullptr;

    QPushButton *m_btnHistory  = nullptr;
    QPushButton *m_btnRealtime = nullptr;

    double m_tempVal  = 0;
    double m_pressVal = 0;
    double m_tankVal  = 0;
    double m_flowVal  = 0;
    double m_pumpVal  = 0;
    double m_valveVal = 0;
};

#endif
