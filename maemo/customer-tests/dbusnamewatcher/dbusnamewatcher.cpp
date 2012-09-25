#include "dbusnamewatcher.h"
#include <QtDebug>
#include <QStringList>
#include <QTimer>
#include <QCoreApplication>

DbusNameWatcher::DbusNameWatcher(QDBusConnection connection, QString bn, int timeout, QObject* parent):QObject(parent), busName(bn)
{
    connect((QObject*) connection.interface(),
            SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this,SLOT(OnServiceOwnerChanged(QString,QString,QString)));

    QTimer *timer = new QTimer(this);
    connect (timer, SIGNAL(timeout()),this,SLOT(timerDone()));
    timer->start(timeout*1000);
}


void DbusNameWatcher::OnServiceOwnerChanged(const QString& name, const QString& oldName, const QString& newName)
{
    if (name == busName && oldName == "" && newName != "" ) {
        QCoreApplication::exit(0);
    }
}

void DbusNameWatcher::timerDone()
{
    QCoreApplication::exit(1);
}

void usage()
{
    QTextStream out(stdout);
    out << "Usage: dbusNameWatcher [--session | --system] [BUSNAME] [TIMEOUT]\n";
    out << "Default bus type is system\n";
}


int main( int argc, char** argv )
{
    QCoreApplication app( argc, argv );
    QStringList args = app.arguments();
    bool ok;
    int timeOut;
    QString busName;
    QDBusConnection connection = QDBusConnection::systemBus();
    DbusNameWatcher* watcher;
    if (args.contains("--help") || args.contains("-h")) {
        usage();
        return 0;
    }
    // session/system
    if (args.contains("--session")) {
        connection = QDBusConnection::sessionBus();
        args.removeAll("--session");
    }

    if (args.contains("--system")) {
        connection = QDBusConnection::systemBus();
        args.removeAll("--system");
    }

    if (args.count() == 3) {
        busName = args.at(1);
        timeOut = args.at(2).toInt(&ok, 10);
        watcher = new DbusNameWatcher(connection, busName, timeOut);
    }
    else{
        usage();
        return 0;
    }
    return app.exec();
}
