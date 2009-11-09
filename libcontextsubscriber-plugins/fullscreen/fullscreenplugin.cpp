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

#include "logging.h"
#include "fullscreenplugin.h"

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberFullScreen::FullScreenPlugin();
}

namespace ContextSubscriberFullScreen {

// How many fixed windows we always have on the top (possibly not
// visible currently).
#define FIXED_ON_TOP 0 // FIXME: update this to reflect the reality

FullScreenPlugin::FullScreenPlugin()
    : fullScreenKey("Screen.FullScreen"), subscribed(false)
{
    // Initialize the objects needed when communicating via X.
    dpy = XOpenDisplay(0);
    if (dpy == 0) {
        emit failed("Cannot open display");
        return;
    }

    clientListStackingAtom = XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
    stateAtom = XInternAtom(dpy, "_NET_WM_STATE", False);
    fullScreenAtom = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    windowTypeAtom = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    windowTypeDesktopAtom = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    windowTypeNotificationAtom = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);

    // Emitting ready() is not allowed inside the constructor. Thus,
    // queue it.

    // FIXME: Casting to IProviderPlugin* is ugly; FullScreenPlugin
    // inherits QObject twice...
    QMetaObject::invokeMethod((IProviderPlugin*)this, "emitReady", Qt::QueuedConnection);
}

void FullScreenPlugin::checkFullScreen()
{
    if (dpy == 0) {
        contextWarning() << "Display == 0";
        return;
    }

    Atom actualType;
    int actualFormat;
    unsigned long numWindows, bytesLeft;
    unsigned char* windowData = 0;

    // Get the stacking list of clients from the root window
    int result = XGetWindowProperty(dpy, DefaultRootWindow(dpy), clientListStackingAtom,
                                    0, 0x7fffffff, False, XA_WINDOW,
                                    &actualType, &actualFormat, &numWindows, &bytesLeft, &windowData);

    if (result != Success || windowData == None) {
        contextDebug() << "Cannot get win props for root";
        return;
    }

    bool interestingWindowsFound = false;
    // Start reading the windows from the top
    Window *wins = (Window *)windowData;
    for (long i = numWindows - 1 - FIXED_ON_TOP; i >= 0; --i) {
        contextDebug() << "Checking window" << i << wins[i];

        // FIXME: Is it enough to listen to the root window for
        // _NET_CLIENT_LIST_STACKING? Or should we listen to other windows
        // as well?

        // Check the window attributes. If the window is unmapped or
        // 0-sized, we're not interested in it.
        XWindowAttributes winAttr;
        if (XGetWindowAttributes(dpy, wins[i], &winAttr) == 0) continue;
        if (winAttr.width == 0 || winAttr.height == 0) continue;
        if (winAttr.c_class != InputOutput || winAttr.map_state != IsViewable) continue;

        // Check the window type. If it is a desktop window or a
        // notification window, we're not interested.
        // FIXME: Can we really have multiple types?
        bool interesting = true;
        unsigned char *data = 0;
        unsigned long count = 0;

        result = XGetWindowProperty(dpy, wins[i], windowTypeAtom,
                                    0L, 16L, False, XA_ATOM,
                                    &actualType, &actualFormat, &count, &bytesLeft, &data);
        if (result != Success) continue;

        Atom *types = (Atom *)data;
        for (unsigned int t = 0; t < count; ++t) {
            if (types[t] == windowTypeDesktopAtom || types[t] == windowTypeNotificationAtom) {
                interesting = false;
                break;
            }
        }

        XFree(data);
        data = 0;

        if (!interesting) continue;

        // Check the window state; whether it's fullscreen or not
        // FIXME: Can we really have multiple states?
        result = XGetWindowProperty (dpy, wins[i], stateAtom,
                                     0L, 16L, False, XA_ATOM,
                                     &actualType, &actualFormat, &count, &bytesLeft, &data);

        if (result != Success) continue;

        bool fullScreen = false;
        Atom *states = (Atom *)data;
        for (unsigned int s = 0; s < count; ++s) {
            if (states[s] == fullScreenAtom) {
                fullScreen = true;
            }
        }
        XFree(data);
        data = 0;

        // This window we're looking at is enough to determine whether
        // we're fullscreen or not.
        interestingWindowsFound = true;

        QMetaObject::invokeMethod((IProviderPlugin*)this, "emitValueChanged", Qt::QueuedConnection,
                                  Q_ARG(QString, fullScreenKey), Q_ARG(bool, fullScreen));

        // We have successfully checked at least one interesting
        // window; don't continue with the others.
        break;
    }

    if (!interestingWindowsFound) {
        // There were no windows except the desktop + notifications
        QMetaObject::invokeMethod((IProviderPlugin*)this, "emitValueChanged", Qt::QueuedConnection,
                                  Q_ARG(QString, fullScreenKey), Q_ARG(bool, false));
    }

    XFree(windowData);
}

void FullScreenPlugin::run()
{
    XEvent event;
    while (subscribed) {
        // This blocks until we get an event
        XNextEvent(dpy, &event);
        // It's possible that the property was unsubscribed while we
        // were waiting
        contextDebug() << "Got an event";

        if (!subscribed) break;

        if (event.type == PropertyNotify && event.xproperty.window == DefaultRootWindow(dpy)
            && event.xproperty.atom == clientListStackingAtom) {
            contextDebug() << "Interesting event";
            checkFullScreen();
            // We're anyway going to check the full screen property;
            // no need to check the other events we possibly have
            cleanEventQueue();
        }
    }
    contextDebug() << "Returning from run";
}

void FullScreenPlugin::subscribe(QSet<QString> keys)
{
    // Check for invalid keys
    foreach(const QString& key, keys) {
        if (key != fullScreenKey) {
            emit subscribeFailed(key, "Invalid key");
        }
    }

    if (keys.contains(fullScreenKey)) {

        checkFullScreen(); // This also signals valueChanged

        // Now the value is there; signal that the subscription is done.
        emit subscribeFinished(fullScreenKey);

        // Start listening to changes in the client list
        XSelectInput(dpy, DefaultRootWindow(dpy), PropertyChangeMask);
        // Start the thread
        subscribed = true;
        start();
    }
}

void FullScreenPlugin::unsubscribe(QSet<QString> keys)
{
    if (keys.contains(fullScreenKey)) {
        // Stop listening to changes in the client list
        XSelectInput(dpy, DefaultRootWindow(dpy), NoEventMask);

        // Stop the thread (will affect after the next event is read)
        subscribed = false;

        // Clean the event queue so that we don't have old events when
        // a new subscripton comes
        cleanEventQueue();
    }
}

void FullScreenPlugin::emitReady()
{
    emit ready();
}

void FullScreenPlugin::emitValueChanged(QString key, bool value)
{
    emit valueChanged(key, value);
}

void FullScreenPlugin::cleanEventQueue()
{
    int numEvents = XEventsQueued(dpy, QueuedAlready);
    contextDebug() << "Ignoring" << numEvents << "events";
    XEvent event;
    for (int i=0; i<numEvents; ++i) {
        XNextEvent(dpy, &event);
    }
}

} // end namespace

