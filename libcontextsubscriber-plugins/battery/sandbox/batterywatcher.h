#ifndef BATTERYWATCHER_H
#define BATTERYWATCHER_H

#include <QObject>

class BatteryWatcher : public QObject
{
    Q_OBJECT

public:
    BatteryWatcher();
    
public slots:
    void readFromInotify();

private:
    int inotifyFd;
};

#endif
