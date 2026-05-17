#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QVector>

class IProtocol;
class TagManager;
class ValueDisplay;

class TestWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TestWindow(QWidget *parent = nullptr);
    ~TestWindow() override;

private slots:
    void onProtocolChanged(int index);
    void onConnect();
    void onDisconnect();

    void onConnected();
    void onDisconnected();
    void onError(const QString &msg);
    void onTagValues(const QHash<QString, QVariant> &values);
    void onTagChanged(const QString &name, const QVariant &value, bool valid);

    void appendLog(const QString &msg);

private:
    void setupUi();
    void setupProtocol();
    void setupMappings(IProtocol *protocol);

    IProtocol    *m_protocol   = nullptr;
    TagManager   *m_tagManager = nullptr;

    // UI
    QComboBox    *m_comboProtocol = nullptr;
    QPushButton  *m_btnConnect    = nullptr;
    QPushButton  *m_btnDisconnect = nullptr;
    QLabel       *m_statusLed     = nullptr;
    QLabel       *m_statusText    = nullptr;
    QTextEdit    *m_logOutput     = nullptr;

    QVector<ValueDisplay *> m_displays;
};

#endif
