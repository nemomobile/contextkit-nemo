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

#ifndef KBSLIDERPLUGIN_H
#define KBSLIDERPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition
#include <QSet>
#include <QString>

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

class QSocketNotifier;

namespace ContextSubscriberKbSlider
{

/*!
  \class KbSliderPlugin

  \brief A libcontextsubscriber plugin for reading the keyboard slider
  status. Provides context properties /maemo/InternalKeyboard/Present and
  /maemo/InternalKeyboard/Open.

 */

class KbSliderPlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit KbSliderPlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);

private Q_SLOTS:
    void onSliderEvent();
    void readSliderStatus();
    void readKbPresent();
    void emitFinishedKbPresent();

private:
    QVariant kbOpen; // current value of the "keyboard open" key
    QVariant kbPresent; // current value of the "keyboard present" key
    QSocketNotifier* sn;
    int eventFd;
};
}

#endif
