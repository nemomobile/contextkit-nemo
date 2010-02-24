extern "C" {
    #include "bmeipc.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QFile>
#include <batterywatcher.h>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDebug>
#include <QVarLengthArray>

#define BMEIPC_EVENT	"/tmp/.bmeevt"

BatteryWatcher::BatteryWatcher()
{
    bmeipc_epush(0);

/*  Alternative version
    QFileSystemWatcher *fsw = new QFileSystemWatcher(this);
    fsw->addPath(QString(BMEIPC_EVENT));
    connect(fsw, SIGNAL(fileChanged(QString)), SLOT(readFromInotify()));
*/

    inotifyFd = bmeipc_eopen(-1); // Mask is thrown away
    fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
    int wd = inotify_add_watch(inotifyFd, BMEIPC_EVENT, (IN_CLOSE_WRITE | IN_DELETE_SELF));
    if (0 > wd) {
        qDebug() << "inotify_add_watch failed";
        exit(-1);
    }
    QSocketNotifier *sn = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
    connect(sn, SIGNAL(activated(int)), this, SLOT(readFromInotify()));
}

void pullStats()
{
    int32_t _sd = -1;
    bmestat_t st;

    if (0 > (_sd = bmeipc_open())) {
        qDebug() << "Failed to connect BME server, " <<  strerror(errno);
        exit(errno);
    }

    if (0 > bmeipc_stat(_sd, &st)) {
        qDebug() << "Failed to get statistics, " << strerror(errno);
    }

    switch (st[CHARGING_STATE]) {
    case CHARGING_STATE_STOPPED:
        if (BATTERY_STATE_FULL != st[BATTERY_STATE]) {
            qDebug() << "off";
        } else {
            qDebug() << "full";
        }
        break;

    case CHARGING_STATE_STARTED:
      qDebug() << "Charging state:" << "charging started";
        break;

    case CHARGING_STATE_SPECIAL:
        qDebug() << "Charging state:" << "special";
        break;
    case CHARGING_STATE_ERROR:
        qDebug() << "Charging state:" << "error";
        break;
    default:
        qDebug() << "Charging state:" << "unknown";

    }

    switch (st[BATTERY_STATE]) {
    case BATTERY_STATE_EMPTY:
        qDebug() << "Battery state:" << "empty";
        break;
    case BATTERY_STATE_LOW:
        qDebug() << "Battery state:" << "low";
        break;
    case BATTERY_STATE_OK:
        qDebug() << "Battery state:" << "ok";
        break;
    case BATTERY_STATE_FULL:
        qDebug() << "Battery state:" << "full";
        break;
    case BATTERY_STATE_ERROR:
    default:
        qDebug() << "Battery state:" << "unknown";
        break;
    }

    qDebug() << "level:" << st[BATTERY_LEVEL_NOW] << "bars:" << st[BATTERY_LEVEL_MAX] 
	     << "capacity:" << st[BATTERY_CAPA_NOW] << "/" << st[BATTERY_CAPA_MAX];
}

void BatteryWatcher::readFromInotify()
{
    // Empty buffer. Should not blog, since we got signal from QSocketNotifier
    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(inotifyFd, buffer.data(), buffSize);

    pullStats();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    pullStats();

    BatteryWatcher bt;
    
    return app.exec();
}
