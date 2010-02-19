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
#define EVENT_KEY KEY_CAMERA // FIXME: change this to the slider
// Context keys
#define KEY_KB_PRESENT QString("maemo/InternalKeyboard/Present")
#define KEY_KB_OPEN QString("/maemo/InternalKeyboard/Open")

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

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)
#define LONG(x) ((x)/BITS_PER_LONG)
#define OFF(x)  ((x)%BITS_PER_LONG)

KbSliderPlugin::KbSliderPlugin()
{
    // Try to find a correct input device for us
    if (findInputDevice().isEmpty())
        QMetaObject::invokeMethod(this, "failed", Qt::QueuedConnection);
    else
        QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void KbSliderPlugin::readInitialValues()
{
    qDebug() << "Read initial values";
    unsigned long bits[NBITS(KEY_MAX)] = {0};

    int fd = open(findInputDevice().toAscii().constData(), O_RDONLY);

    // FIXME: perhaps change this to EVIOCGSW too
    if (ioctl(fd, EVIOCGKEY(KEY_MAX), bits) > 0) {
        if (test_bit(EVENT_KEY, bits))
            kbOpen = QVariant(true);
        else
            kbOpen = QVariant(false);
    }

    // TODO: where to read the "kb present" info?
    if (pendingSubscriptions.contains(KEY_KB_OPEN))
        emit subscribeFinished(KEY_KB_OPEN, kbOpen);
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
        // We're already up to date
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

