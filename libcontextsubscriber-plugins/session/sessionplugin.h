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

#ifndef SESSIONSTATEPLUGIN_H
#define SESSIONSTATEPLUGIN_H

#include <iproviderplugin.h> // For IProviderPlugin definition

#include <contextproperty.h>
#include <QThread>

// Note: Including X11 headers needs to be done after including Qt
// headers: Qt has an enum CursorShape and Xlib #defines CursorShape to 0.
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using ContextSubscriber::IProviderPlugin;

extern "C" {
    IProviderPlugin* pluginFactory(const QString& constructionString);
}

class QSocketNotifier;

namespace ContextSubscriberSessionState
{

/*!
  \class SessionStatePlugin

  \brief A libcontextsubscriber plugin for providing the Session.State context property.

  SessionStatePlugin reads the full-screen status from window manager
  via the X protocol. It also listens to the screen blanking signal.

 */

class SessionStatePlugin : public IProviderPlugin
{
    Q_OBJECT

public:
    explicit SessionStatePlugin();
    ~SessionStatePlugin();
    virtual void subscribe(QSet<QString> keys);
    virtual void unsubscribe(QSet<QString> keys);
    virtual void blockUntilReady();
    virtual void blockUntilSubscribed(const QString& key);

private slots:
    void emitValueChanged();
    void onXEvent();

private:
    void checkFullScreen();
    void cleanXEventQueue();

    QString sessionStateKey; ///< Key of the fullscreen context property
    QSocketNotifier* xNotifier; ///< For listening to the file descriptor used for communicating with X
    bool fullscreen; ///< Whether we're currently in the fullscreen mode

    ::Display* dpy; ///< Pointer to the X display the plugin opens and uses
    ::Atom clientListStackingAtom; ///< X atom for querying the stacking client list
    ::Atom windowTypeAtom; ///< X atom for querying the window type
    ::Atom windowTypeDesktopAtom; ///< X atom for the desktop window type
    ::Atom windowTypeNotificationAtom; ///< X atom for the notification window type
    ::Atom stateAtom; ///< X atom for querying the window state
    ::Atom fullScreenAtom; ///< X atom for the fullscreen window state

    ContextProperty screenBlanked; ///< Context property for listening to the screen blanking status
};
}

#endif
