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
    QMetaObject::invokeMethod(this, "emitReady", Qt::QueuedConnection);
}

void KbSliderPlugin::readInitialValues()
{
    // TODO: really read the initial values
    foreach (const QString& key, pendingSubscriptions)
        emit subscribeFinished(key, QVariant(0)/*some value we got*/);
    pendingSubscriptions.clear();
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

// For emitting the ready() signal in a delayed way
void KbSliderPlugin::emitReady()
{
    emit ready();
}

} // end namespace

