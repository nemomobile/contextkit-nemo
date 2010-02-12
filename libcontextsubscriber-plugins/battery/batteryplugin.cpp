/*
 * Copyright (C) 2009 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include "logging.h"
#include "batteryplugin.h"
#include "sconnect.h"
#include <QSocketNotifier>
#include <QFileSystemWatcher>
#include <QVarLengthArray>
#include <QFile>

extern "C" {
    #include "bme/bmeipc.h"
}

#define ON_BATTERY       "Battery.OnBattery"
#define LOW_BATTERY      "Battery.LowBattery"
#define CHARGE_PERCENT   "Battery.ChargePercentage"
#define TIME_UNTIL_LOW   "Battery.TimeUntilLow"
#define TIME_UNTIL_FULL  "Battery.TimeUntilFull"
#define IS_CHARGING      "Battery.IsCharging"
#define BMEIPC_EVENT	 "/tmp/.bmeevt"
#define NANOSECS_PER_MIN 60000000

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberBattery::BatteryPlugin();
}

namespace ContextSubscriberBattery {


/// Constructor. Try to connect to Battery right away.
BatteryPlugin::BatteryPlugin()
{
    // try out: replace emitReady by ready and kill emitReady slot
    QMetaObject::invokeMethod(this, "readInitialValues", Qt::QueuedConnection);
}


/// Destructor.
BatteryPlugin::~BatteryPlugin()
{
}

/// Implementation of the IPropertyProvider::subscribe. We don't need
/// any extra work for subscribing to keys, thus subscribe is finished
/// right away.
void BatteryPlugin::subscribe(QSet<QString> keys)
{

    if (subscribedProperties.isEmpty()) {

        inotifyFd = bmeipc_eopen(-1); // Mask is thrown away

        if (0 > inotifyFd)

            QMetaObject::invokeMethod(this, "emitFailed", Qt::QueuedConnection,Q_ARG(QString,"Battery plugin failed to add watcher for BME events"));

        else {

            if (!QFile::exists(BMEIPC_EVENT))
                bmeipc_epush(0);

            fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
            int wd = inotify_add_watch(inotifyFd, BMEIPC_EVENT, (IN_CLOSE_WRITE | IN_DELETE_SELF));

            if (0 > wd)
                QMetaObject::invokeMethod(this, "emitFailed", Qt::QueuedConnection,Q_ARG(QString,"Battery plugin failed to add watcher for BME events"));
            else {
                QSocketNotifier *sn = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
                connect(sn, SIGNAL(activated(int)), this, SLOT(onBMEEvent()));
            }
        }
    }

    subscribedProperties.unite(keys);
    // assume that we already have gotten the value for each property
    foreach(const QString& key, keys)
        emit subscribeFinished(key, propertyCache[key]);
}

/// Implementation of the IPropertyProvider::unsubscribe. We're not
/// keeping track on subscriptions, so we don't need to do anything.
void BatteryPlugin::unsubscribe(QSet<QString> keys)
{
    //TODO: remove the keys from subscribedProperties
    // if empty close inotify

}

bool BatteryPlugin::readBatteryValues()
{
    bmestat_t st;
    int sd = -1;

    if (0 > (sd = bmeipc_open())){
        contextDebug() << "Cannot open BME file descriptor";
        return false;
    }

    if (0 > bmeipc_stat(sd, &st)) {
        contextDebug() << "Cannot get BME statistics";
        return false;
    }

    propertyCache[IS_CHARGING] = (st[CHARGING_STATE] == CHARGING_STATE_STARTED);

    propertyCache[ON_BATTERY] =  (st[CHARGER_STATE] != CHARGER_STATE_CONNECTED);

    propertyCache[LOW_BATTERY] = (st[BATTERY_STATE] == BATTERY_STATE_LOW);

    if (st[BATTERY_CAPA_MAX] != 0)
        propertyCache[CHARGE_PERCENT] = st[BATTERY_CAPA_NOW] * 100 / st[BATTERY_CAPA_MAX];
    else
        propertyCache[CHARGE_PERCENT] = QVariant();

    propertyCache[TIME_UNTIL_FULL] = (quint64)st[CHARGING_TIME] * NANOSECS_PER_MIN;

    propertyCache[TIME_UNTIL_LOW] = (quint64)st[BATTERY_TIME_LEFT] * NANOSECS_PER_MIN;

    bmeipc_close(sd);

    return true;

}


void BatteryPlugin::emitBatteryValues()
{
    foreach(const QString& key, subscribedProperties)
        emit valueChanged(key, propertyCache[key]);
}


/// A BMEEvent is received
void BatteryPlugin::onBMEEvent()
{
    // Empty buffer. Should not blog, since we got signal from QSocketNotifier
    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(inotifyFd, buffer.data(), buffSize);

    if (readBatteryValues())
        emitBatteryValues();
    else
        emit failed ("Battery plugin could not read values from BME");

}

void BatteryPlugin::readInitialValues()
{
    if (readBatteryValues())
        emit ready();
    else
        emit failed ("Battery plugin could not read values from BME");
}

/// For emitting the failed() signal in a delayed way.
void BatteryPlugin::emitFailed(const QString &msg)
{
    emit failed(msg);
}


} // end namespace

