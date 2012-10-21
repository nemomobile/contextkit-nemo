/*
 * Generic keyboard properties provider
 *
 * Copyright (C) 2012 Jolla Ltd.
 * Contact: Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 */

#include <contextkit_props/internal_keyboard.hpp>
#include <cor/udev.hpp>
#include "plugin.hpp"

#include <linux/input.h>

#include <QString>
#include <QStringList>
#include <QVector>


IProviderPlugin* pluginFactory(const QString&)
{
    return new ContextKitNemo::KeyboardGeneric();
}

namespace ContextKitNemo
{

namespace ckit = contextkit::internal_keyboard;

static bool isKeyboardDevice(cor::udev::Device const &dev)
{
    QString key(dev.attr("capabilities/key"));
    QStringList caps_strs(key.split(' ', QString::SkipEmptyParts));
    if (caps_strs.isEmpty())
        return false;
    QVector<unsigned long> caps;
    foreach(QString const &s, caps_strs) {
        unsigned long v;
        bool is_ok = false;
        v = s.toULong(&is_ok, 16);
        if (!is_ok)
            return false;
        caps.push_back(v);
    }
    size_t count = 0;
    for (unsigned i = KEY_Q; i <= KEY_P; ++i) {
        int pos = caps.size() - (i / sizeof(unsigned long));
        if (pos < 0)
            break;
        size_t bit = i % sizeof(unsigned long);
        if ((caps[pos] >> bit) & 1)
            ++count;
    }
    return (count == KEY_P - KEY_Q);
}

static bool isKeyboardAvailable()
{
    using namespace cor::udev;
    Root udev;
    if (!udev)
        return false;

    Enumerate input(udev);
    if (!input)
        return false;

    input.subsystem_add("input");
    ListEntry devs(input);

    bool is_kbd_found = false;
    auto find_kbd = [&is_kbd_found, &udev](ListEntry const &e) -> bool {
        Device d(udev, e.path());
        is_kbd_found = isKeyboardDevice(d);
        return !is_kbd_found;
    };
    devs.for_each(find_kbd);
    return is_kbd_found;
}

KeyboardGeneric::KeyboardGeneric()
{
    props[ckit::is_present] = [&]() {
        emitChanged(ckit::is_present, is_kbd_available);
    };
    props[ckit::is_open] = [&]() {
        emitChanged(ckit::is_open, is_kbd_available);
    };
    QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

void KeyboardGeneric::setup()
{
    if (!is_setup) {
        is_kbd_available = isKeyboardAvailable();
        is_setup = true;
    }
}

void KeyboardGeneric::emitChanged(QString const &name, QVariant const &value)
{
    emit valueChanged(name, value);
}

/// Implementation of the IPropertyProvider::subscribe.
void KeyboardGeneric::subscribe(QSet<QString> keys)
{
    setup();
    foreach(QString const &k, keys) {
        if (props.contains(k))
            props[k]();
    }
}

/// Implementation of the IPropertyProvider::unsubscribe.
void KeyboardGeneric::unsubscribe(QSet<QString>)
{
}

void KeyboardGeneric::blockUntilReady()
{
    emit ready();
}

void KeyboardGeneric::blockUntilSubscribed(const QString &)
{
}

} // namespace
