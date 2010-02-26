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

#include <QSocketNotifier>

#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>

#define GPIO_FILE "/dev/input/gpio-keys"
#define KEYPAD_FILE "/dev/input/keypad"

// Context keys
static const QString KEY_KB_PRESENT("/maemo/InternalKeyboard/Present");
static const QString KEY_KB_OPEN("/maemo/InternalKeyboard/Open");

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    return new ContextSubscriberKbSlider::KbSliderPlugin();
}

namespace ContextSubscriberKbSlider {

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define LONG(x) ((x)/BITS_PER_LONG)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

KbSliderPlugin::KbSliderPlugin():
    sn(0), eventFd(-1)
{
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

/// Reads the KEYPAD_FILE, and checks the events it offers. If it offers the
/// QWERTY key events, sets kbPresent to true, otherwise, to false.
void KbSliderPlugin::readKbPresent()
{
    static bool read = false;
    unsigned long keys[NBITS(KEY_MAX)] = {0};

    if (read)
        return;
    read = true;

    int keypadFd = open(KEYPAD_FILE, O_RDONLY);
    if (keypadFd < 0)
        goto out;

    // Read what key events this input device provides.
    if (ioctl(keypadFd, EVIOCGBIT(EV_KEY, KEY_MAX), keys) < 0)
        goto out;

    // cunning.
    kbPresent = test_bit(KEY_Q, keys) && test_bit(KEY_W, keys) &&
        test_bit(KEY_E, keys) && test_bit(KEY_R, keys) &&
        test_bit(KEY_T, keys) && test_bit(KEY_Y, keys);
out:
    close(keypadFd);
}

/// Emits the subscribeFinished or subscribeFailed signal for KEY_KB_PRESENT.
void KbSliderPlugin::emitFinishedKbPresent()
{
    if (kbPresent.isNull())
        emit subscribeFailed(KEY_KB_PRESENT, QString("Cannot read " KEYPAD_FILE));
    else
        emit subscribeFinished(KEY_KB_PRESENT, kbPresent);
}

void KbSliderPlugin::readSliderStatus()
{
    unsigned long bits[NBITS(KEY_MAX)] = {0};

    if (ioctl(eventFd, EVIOCGSW(KEY_MAX), bits) > 0)
        kbOpen = QVariant(test_bit(SW_KEYPAD_SLIDE, bits) == 0);

    if (!kbPresent.isNull() && kbPresent == false) {
        // But if the keyboard is not present, it cannot be open. Also stop
        // watching the open/closed status.
        kbOpen = QVariant();
        unsubscribe(QSet<QString>() << KEY_KB_OPEN);
    }

    emit subscribeFinished(KEY_KB_OPEN, kbOpen);
}

void KbSliderPlugin::onSliderEvent()
{
    // Something happened in the input file; read the events, decide whether
    // they're interesting, and update the context properties.
    struct input_event event;
    size_t rd = read(eventFd, &event, sizeof(event));
    if (event.type == EV_SW && event.code == SW_KEYPAD_SLIDE) {
        kbOpen = (event.value == 0);
    }
    emit valueChanged(KEY_KB_OPEN, kbOpen);
}

/// Implementation of the IPropertyProvider::subscribe.
void KbSliderPlugin::subscribe(QSet<QString> keys)
{
    // Always read the "keyboard present" info, we need that for both provided
    // context properties.
    QMetaObject::invokeMethod(this, "readKbPresent", Qt::QueuedConnection);

    if (keys.contains(KEY_KB_PRESENT)) {
        QMetaObject::invokeMethod(this, "emitFinishedKbPresent", Qt::QueuedConnection);
    }
    if (keys.contains(KEY_KB_OPEN)) {
        // Start polling the event file
        eventFd = open(GPIO_FILE, O_RDONLY);
        if (eventFd < 0) {
            emit subscribeFailed(KEY_KB_OPEN, "Cannot open " GPIO_FILE);
            return;
        }
        sn = new QSocketNotifier(eventFd, QSocketNotifier::Read, this);
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
        if (eventFd >= 0) close(eventFd);
        eventFd = -1;
    }
}

} // end namespace

