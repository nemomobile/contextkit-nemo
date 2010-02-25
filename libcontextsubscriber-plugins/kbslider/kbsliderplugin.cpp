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

#define GPIO_FILE "/dev/input/gpio-keys"
#define KEYPAD_FILE "/dev/input/keypad"

#define SLIDE_EVENT_ID SW_KEYPAD_SLIDE
// Context keys
#define KEY_KB_PRESENT QString("/maemo/InternalKeyboard/Present")
#define KEY_KB_OPEN QString("/maemo/InternalKeyboard/Open")

#include <QDir>
#include <QSocketNotifier>

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

KbSliderPlugin::KbSliderPlugin():
    sn(0)
{
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

/// Reads the KEYPAD_FILE, and checks the events it offers. If it offers the
/// QWERTY key events, sets kbPresent to true, otherwise, to false.
void KbSliderPlugin::readKbPresent()
{
    static bool read = false;
    if (!read) {
        read = true;
        QFile file(KEYPAD_FILE);
        if (!file.exists()) return;
        int keypadFd = open(KEYPAD_FILE, O_RDONLY);
        if (keypadFd < 0) return;

        // Read what key events this input device provides.
        unsigned long keys[NBITS(KEY_MAX)] = {0};
        if (ioctl(keypadFd, EVIOCGBIT(EV_KEY, KEY_MAX), keys) < 0)
            return;

        if (test_bit(KEY_Q, keys) && test_bit(KEY_W, keys) &&
            test_bit(KEY_R, keys) && test_bit(KEY_T, keys) &&
            test_bit(KEY_Y, keys))
            kbPresent = QVariant(true);
        else
            kbPresent = QVariant(false);
        close(keypadFd);
    }
}

/// Emits the subscribeFinished or subscribeFailed signal for KEY_KB_PRESENT.
void KbSliderPlugin::emitFinishedKbPresent()
{
    if (kbPresent.isNull())
        emit subscribeFailed(KEY_KB_PRESENT, QString("Cannot read ") + KEYPAD_FILE);
    else
        emit subscribeFinished(KEY_KB_PRESENT, kbPresent);
}

void KbSliderPlugin::readSliderStatus()
{
    unsigned long bits[NBITS(KEY_MAX)] = {0};

    if (ioctl(eventFd, EVIOCGSW(KEY_MAX), bits) > 0)
        kbOpen = QVariant(test_bit(SLIDE_EVENT_ID, bits) ? false : true);

    emit subscribeFinished(KEY_KB_OPEN, kbOpen);
}

void KbSliderPlugin::onSliderEvent()
{
    // Something happened in the input file; read the events, decide whether
    // they're interesting, and update the context properties.
    struct input_event events[64];
    size_t rd = read(eventFd, events, sizeof(struct input_event)*64);
    for (size_t i = 0; i < rd/sizeof(struct input_event); ++i) {
        if (events[i].type == EV_SW && events[i].code == SLIDE_EVENT_ID)
            kbOpen = (events[i].value == 0);
    }
    emit valueChanged(KEY_KB_OPEN, kbOpen);
    // note: find out whether we get activated again if we didn't read all
    // that is available
}

/// Implementation of the IPropertyProvider::subscribe.
void KbSliderPlugin::subscribe(QSet<QString> keys)
{
    if (keys.contains(KEY_KB_PRESENT)) {
        // Only a one-time effort needed to subscribe to this key.
        QMetaObject::invokeMethod(this, "readKbPresent", Qt::QueuedConnection);
        QMetaObject::invokeMethod(this, "emitFinishedKbPresent", Qt::QueuedConnection);
    }
    if (keys.contains(KEY_KB_OPEN)) {
        // Start polling the event file
        eventFd = open(GPIO_FILE, O_RDONLY);
        if (eventFd < 0) {
            emit subscribeFailed(KEY_KB_OPEN, "Cannot open event file");
        }
        sn = new QSocketNotifier(eventFd, QSocketNotifier::Read);
        sconnect(sn, SIGNAL(activated(int)), this, SLOT(onSliderEvent()));

        // Read the initial status. This will also emit subscribeFinished /
        // subscribeFailed.
        QMetaObject::invokeMethod(this, "readSliderStatus", Qt::QueuedConnection);
    }
}

/// Implementation of the IPropertyProvider::unsubscribe.
void KbSliderPlugin::unsubscribe(QSet<QString> keys)
{
    if (keys.contains(KEY_KB_OPEN)) {
        // stop the listening activities
        delete sn;
        sn = 0;
        close(eventFd);
    }
}

} // end namespace

