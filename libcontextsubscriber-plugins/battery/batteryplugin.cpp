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

#include "batteryplugin.h"
#include "logging.h"
#include "sconnect.h"
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <QSocketNotifier>
#include <QFileSystemWatcher>
#include <QVarLengthArray>
#include <QFile>
#include <QStringList>
#include <QSet>

Q_DECLARE_METATYPE(QSet<QString>);

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


/// Constructor.
BatteryPlugin::BatteryPlugin():inotifyFd(-1), sn(NULL)
{
    // Plugin is ready to take subscriptions in
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}


/// Implementation of the IPropertyProvider::subscribe.
/// The provider source of the battery properties is initialised only on the first subscription.
/// Otherwise, the new properties to subscribe to are tracked in the cache.
void BatteryPlugin::subscribe(QSet<QString> keys)
{
    if (subscribedProperties.isEmpty()) {
        qRegisterMetaType<QSet <QString> >("QSet<QString>");
        QMetaObject::invokeMethod(this, "readInitialValues", Qt::QueuedConnection, Q_ARG(QSet <QString>, keys));
        initProviderSource();
    } else
        subscribedProperties.unite(keys);
}

/// Implementation of the IPropertyProvider::unsubscribe.
/// Properties to unsubscribe are removed from the cache.
/// The provider source is closed and cleaned up when no properties remain in the cache.
void BatteryPlugin::unsubscribe(QSet<QString> keys)
{
    subscribedProperties.subtract(keys);
    if (subscribedProperties.isEmpty())
        cleanProviderSource();
}

/// Start to watch the provider source BMEIPC_EVENT on first subscription or when the source has been deleted or moved
bool BatteryPlugin::initProviderSource()
{
    if (!QFile::exists(BMEIPC_EVENT))
        if (bmeipc_epush(0) < 0) {
            QMetaObject::invokeMethod(this, "emitFailed", Qt::QueuedConnection,Q_ARG(QString,"Battery plugin failed to create file BMEIPC_EVENT"));
            return false;
        }

    if (inotifyFd < 0) {

        inotifyFd = bmeipc_eopen(-1); // Mask is thrown away

        if (inotifyFd < 0) {
            QMetaObject::invokeMethod(this, "emitFailed", Qt::QueuedConnection,Q_ARG(QString,"Battery plugin failed to retrieve file descriptor for BMEIPC_EVENT"));
            return false;
        }

        else {
            fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
            int wd = inotify_add_watch(inotifyFd, BMEIPC_EVENT, (IN_DELETE_SELF | IN_MOVE_SELF));

            if (wd < 0) {
                QMetaObject::invokeMethod(this, "emitFailed", Qt::QueuedConnection,Q_ARG(QString,"Battery plugin failed to add watcher on BMEIPC_EVENT"));
                return false;
            }
            else {
                sn = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
                connect(sn, SIGNAL(activated(int)), this, SLOT(onBMEEvent()));
            }
        }
    }
    return true;
}

/// When the provicer source BMEIPC_EVENT is deleted, moved or when all properties have been unsubscribed
/// watcher is removed
void BatteryPlugin::cleanProviderSource()
{
    delete sn;
    sn = NULL;
    // No need to remove watchers. All  associated  watches are
    // automatically freed when file descriptor is close
    bmeipc_close(inotifyFd);
    inotifyFd = -1;
}

/// Called when provider source has been modified and new values are available
bool BatteryPlugin::readBatteryValues()
{
    bmestat_t st;
    int sd = -1;

    propertyCache[CHARGE_PERCENT] = QVariant(20);

    if ((sd = bmeipc_open()) < 0){
        contextDebug() << "Cannot open socket connected to BME server";
        return false;
    }

    if (bmeipc_stat(sd, &st) < 0) {
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

/// Values of battery properties are emitted only if we were able to read BME statistics
void BatteryPlugin::emitBatteryValues()
{
    foreach(const QString& key, subscribedProperties) {
        contextDebug() << "emit ValueChanged" << key << "," << propertyCache[key];
        emit valueChanged(key, propertyCache[key]);
    }
}


/// When provider source has been modified, we emit ValueChanged signal
/// Otherwise, we handle gracefully the deletion of the source and try to recover
void BatteryPlugin::onBMEEvent()
{

    // Empty buffer. Should not blog, since we got signal from QSocketNotifier
    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(inotifyFd, buffer.data(), buffSize);
    inotify_event *ev = reinterpret_cast<inotify_event *>(buffer.data());

    if ((ev->mask & IN_DELETE_SELF) || (ev->mask & IN_MOVE_SELF)) {
        cleanProviderSource();
        emit failed ("Provider source of battery plugin was deleted or moved");
        if (initProviderSource())
            QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
    } else if ((ev->mask & IN_IGNORED)) {
        contextDebug() << "Do nothing because this event means that watcher has been removed automatically";
    } else {
        if (readBatteryValues())
            emitBatteryValues();
        else
            emit failed ("Battery plugin could not read values from BME");
    }
}
/// Called only on first subscription
void BatteryPlugin::readInitialValues(QSet <QString> keys)
{
    if (readBatteryValues()) {
        foreach(const QString& key, keys) {
            emit subscribeFinished(key, propertyCache[key]);
        }
        subscribedProperties.unite(keys);
    }

    else
        emit failed ("Battery plugin could not read values from BME");
}

/// For emitting the failed() signal in a delayed way.
void BatteryPlugin::emitFailed(const QString &msg)
{
    emit failed(msg);
}


} // end namespace

