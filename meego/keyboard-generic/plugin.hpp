#ifndef _CONTEXTKIT_PLUGIN_KEYBOARD_GENERIC_HPP_
#define _CONTEXTKIT_PLUGIN_KEYBOARD_GENERIC_HPP_
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

#include <iproviderplugin.h>

#include <functional>

#include <QObject>
#include <QMap>
#include <QSet>

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString&);
}

namespace ContextKitNemo
{

class KeyboardGeneric : public IProviderPlugin
{
    Q_OBJECT;
public:
    KeyboardGeneric();
    virtual ~KeyboardGeneric() {}

    virtual void subscribe(QSet<QString>);
    virtual void unsubscribe(QSet<QString>);
    virtual void blockUntilReady();
    virtual void blockUntilSubscribed(const QString&);

private:
    void setup();
    void emitChanged(QString const &, QVariant const &);

    QMap<QString, std::function<void ()> > props;

    bool is_setup;
    bool is_kbd_available;
};

}

#endif //_CONTEXTKIT_PLUGIN_KEYBOARD_GENERIC_HPP_
