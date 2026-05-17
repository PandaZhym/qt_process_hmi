#ifndef SCREEN_BUILDER_H
#define SCREEN_BUILDER_H

#include <QObject>

class SimDataManager;
class QWidget;

class ScreenBuilder : public QObject
{
    Q_OBJECT
public:
    explicit ScreenBuilder(SimDataManager *simData, QObject *parent = nullptr);

    QWidget *buildFromJson(const QString &jsonPath);

private:
    SimDataManager *m_simData = nullptr;
};

#endif
