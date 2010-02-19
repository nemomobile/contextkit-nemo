/*
 * Copyright (C) 2010 Nokia Corporation.
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

#include "kbsliderplugin.h"
#include "sconnect.h"

#include "logging.h"

#define EVENT_NAME QString("gpio-keys")
#define EVENT_DIR "/dev/input"

#include <QDir>
#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberKbSlider::KbSliderPlugin();
}

namespace ContextSubscriberKbSlider {

KbSliderPlugin::KbSliderPlugin()
{
    findInputDevice();
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void KbSliderPlugin::readInitialValues()
{
    // TODO: really read the initial values
    foreach (const QString& key, pendingSubscriptions)
        emit subscribeFinished(key, QVariant(0)/*some value we got*/);
    pendingSubscriptions.clear();
}

/// Searches the /dev/input/event0... files, queries the names, and stops when
/// it finds the name EVENT_NAME
QString KbSliderPlugin::findInputDevice()
{
    static bool checked = false; // Whether we've tried to find the device
    static QString foundDevice = ""; // The device name we've found (empty if none)

    if (!checked) {
        checked = true;
        QDir dir(EVENT_DIR);
        // Loop through the files in that directory, read the input name from each
        int fd;
        char inputName[256];
        foreach (const QString& fileName,
                 dir.entryList(QStringList() << "event*", QDir::System)) {
            QString filePath = dir.filePath(fileName);
            fd = open(filePath.toLocal8Bit().constData(), O_RDONLY);
            if (fd < 0) continue;
            ioctl(fd, EVIOCGNAME(sizeof(inputName)), inputName);
            close(fd);
            if (QString(inputName) == EVENT_NAME) {
                foundDevice = filePath;
                break;
            }
        }
    }
    // Now we have tried to find the device file, but haven't necessarily
    // succeeded
    return foundDevice;
}

void KbSliderPlugin::onKbEvent()
{
    // TODO: update the values
}

/// Implementation of the IPropertyProvider::subscribe.
void KbSliderPlugin::subscribe(QSet<QString> keys)
{
    if (!wantedSubscriptions.isEmpty()) {
        foreach (const QString& key, keys)
            emit subscribeFinished(key);
    }
    else {
        pendingSubscriptions.unite(keys);
        QMetaObject::invokeMethod(this, "readInitialValues", Qt::QueuedConnection);
    }
    wantedSubscriptions.unite(keys);
}

/// Implementation of the IPropertyProvider::unsubscribe.
void KbSliderPlugin::unsubscribe(QSet<QString> keys)
{
    wantedSubscriptions.subtract(keys);
    if (wantedSubscriptions.isEmpty()) {
        // TODO: stop all our activities
    }
}

} // end namespace

