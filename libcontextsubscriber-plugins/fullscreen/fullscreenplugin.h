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

#ifndef FULLSCREENPLUGIN_H
#define FULLSCREENPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition
#include <QThread>

// Note: Including X11 headers needs to be done after including Qt
// headers: Qt has an enum CursorShape and libx #defines CursorShape to 0.
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

namespace ContextSubscriberFullScreen
{

/*!
  \class FullScreenPlugin

  \brief A libcontextsubscriber plugin for reading the full-screen
  status from window manager via the X protocol. Provides the context
  property Screen.FullScreen.

 */

class FullScreenPlugin : public IProviderPlugin, public QThread
{
    Q_OBJECT

public:
    explicit FullScreenPlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);
    void run();

private slots:
    void emitReady();
    void emitValueChanged(QString key, bool value);

private:
    void checkFullScreen();
    void cleanEventQueue();

    QString fullScreenKey;
    bool subscribed;

    ::Display* dpy;
    ::Atom clientListStackingAtom;
    ::Atom stateAtom;
    ::Atom fullScreenAtom;
    ::Atom windowTypeAtom;
    ::Atom windowTypeDesktopAtom;
    ::Atom windowTypeNotificationAtom;
};
}

#endif
