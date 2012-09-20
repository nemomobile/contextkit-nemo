#ifndef DBUSNAMEWATCHER_H
#define DBUSNAMEWATCHER_H

#include <QObject>
#include <QDBusConnection>
#include <QTimer>
#include <QString>

class DbusNameWatcher : public QObject
{
    Q_OBJECT

public:
    DbusNameWatcher(QDBusConnection connection, QString busName, int timeout, QObject * parent = 0);

private:
    QString busName;

private slots:
    void OnServiceOwnerChanged(const QString&, const QString&, const QString&);
    void timerDone();
};

#endif
