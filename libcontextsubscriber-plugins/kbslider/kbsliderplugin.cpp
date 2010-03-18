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

#include <QFile>
#include <QSocketNotifier>

#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>

extern "C" {
#include <libudev.h>
}

#define GPIO_FILE "/dev/input/gpio-keys"

static const QString KeypadFile("/dev/input/keypad");
static const QString InputDir("/dev/input/");
static const QString ClassEventFile("class/input/");

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

/// Finds the device, corresponding to keypad, under /sys.  E.g., if
/// KEYPAD_FILE is a symlink to /dev/input/eventX, this will return
/// class/input/eventX
QString KbSliderPlugin::findKeypadDevice()
{
    // KEYPAD_FILE is a symlink to /dev/input/eventX, check what X is.
    QString eventFile = QFile::symLinkTarget(KeypadFile);
    qDebug() << "event file is" << eventFile;
    if (!eventFile.startsWith("/dev"))
        return "";
    return eventFile.replace("/dev", "class");
}

/// Reads the KEYPAD_FILE, and checks the events it offers. If it offers the
/// QWERTY key events, sets kbPresent to true, otherwise, to false.
void KbSliderPlugin::readKbPresent()
{
    static bool read = false;

    if (read)
        return;
    read = true;

    struct udev* udev = udev_new();
    if (udev == NULL) {
        qDebug() << "udev is null";
        return;
    }

    QString devicePath = QString("%1/%2").arg(udev_get_sys_path(udev)).arg(findKeypadDevice());
    qDebug() << "dev path is" << devicePath;

    struct udev_device* dev = udev_device_new_from_syspath(udev, devicePath.toAscii().constData());

    const char* capabilities = 0;
    QStringList split;
    long lowBits[1] = {0};

    if (dev == NULL) {
        goto out_udev;
    }

    qDebug() << "dev path is also" << udev_device_get_devpath(dev);

    // Walk to the parent until we get a device with "capabilities/key". E.g.,
    // this device is /devices/platform/something/input/inputX/eventX but we
    // need to go to the parent /devices/platform/something/input/inputX
    while (dev != NULL && udev_device_get_sysattr_value(dev, "capabilities/key") == NULL) {
        dev = udev_device_get_parent_with_subsystem_devtype(dev, "input", NULL);
        // We don't need to decrement the ref count, since the returned device
        // is not referenced, and will be cleaned up when the child device is
        // cleaned up.
    }

    // We can assume that the input device in question has events of type
    // KEY. The only thing to check is which key events are supported.

    // Get the key bitmask. The returned string contains hexadecimal numbers
    // separated by spaces. The actual key bitmask is these numbers in reverse
    // order. We're only interested in the low bits, so we check only the last
    // entry.
    capabilities = udev_device_get_sysattr_value(dev, "capabilities/key");
    if (capabilities == 0)
        goto out_device;

    split = QString(capabilities).split(' ', QString::SkipEmptyParts);
    if (split.isEmpty())
        goto out_device;

    lowBits[0] = split.last().toLong(0, 16);

    // Check the bits KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T and KEY_Y.
    kbPresent = test_bit(KEY_Q, lowBits) && test_bit(KEY_W, lowBits) &&
        test_bit(KEY_E, lowBits) && test_bit(KEY_R, lowBits) &&
        test_bit(KEY_T, lowBits) && test_bit(KEY_Y, lowBits);

out_device:
    udev_device_unref(dev);
out_udev:
    udev_unref(udev);
}

/// Emits the subscribeFinished or subscribeFailed signal for KEY_KB_PRESENT.
void KbSliderPlugin::emitFinishedKbPresent()
{
    if (kbPresent.isNull())
        emit subscribeFailed(KEY_KB_PRESENT, QString("Cannot read keypad information"));
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

