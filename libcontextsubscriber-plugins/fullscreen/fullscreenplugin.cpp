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
#include "sconnect.h"
#include <QSocketNotifier>

// How many fixed windows we always have on the top (possibly not
// visible currently).
#define FIXED_ON_TOP 0 // FIXME: update this to reflect the reality

/// The factory method for constructing the IPropertyProvider instance.
IProviderPlugin* pluginFactory(const QString& /*constructionString*/)
{
    // Note: it's the caller's responsibility to delete the plugin if
    // needed.
    return new ContextSubscriberFullScreen::FullScreenPlugin();
}

namespace ContextSubscriberFullScreen {

/// Typedef for Xlib error handling functions
typedef int (*xerrfunc)(Display*, XErrorEvent*);

/// Handler for X errors. All errors are ignored: we cannot do any
/// sophisticated handling, but we don't want the client process to
/// exit in case of errors.
int onXError(Display* eDpy, XErrorEvent* error)
{
    contextWarning() << "X error occured";
    return 0;
}


/// Constructor. Opens the X display and creates the atoms. When this
/// is done, the "ready" signal is scheduled to be emitted (we cannot
/// emit it in the constructor).
FullScreenPlugin::FullScreenPlugin()
    : fullScreenKey("Screen.FullScreen")
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

    // Fetch the fd used for communicating with X and start listening
    // to it
    xNotifier = new QSocketNotifier(ConnectionNumber(dpy), QSocketNotifier::Read, this);
    sconnect(xNotifier, SIGNAL(activated(int)), this, SLOT(onXEvent()));

    // Emitting ready() is not allowed inside the constructor. Thus,
    // queue it.

    QMetaObject::invokeMethod(this, "emitReady", Qt::QueuedConnection);
}

/// Destructor.
FullScreenPlugin::~FullScreenPlugin()
{
    XCloseDisplay(dpy);
    dpy = 0;
}

/// Check whether the top-most window is in a fullscreen state. We
/// ignore a specified amount of windows which are always fixed on
/// top, see #define's. Schedules a valueChanged signal to be emitted
/// if the fullscreen status if found out.
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

    // Set the error handler. A common situation is that we get a
    // stacking client list from X, and while we are processing that
    // list, a client has already exited. This will result in an
    // BadWindow error.
    xerrfunc oldHandler = XSetErrorHandler(onXError);

    // Get the stacking list of clients from the root window
    int result = XGetWindowProperty(dpy, DefaultRootWindow(dpy), clientListStackingAtom,
                                    0, 0x7fffffff, False, XA_WINDOW,
                                    &actualType, &actualFormat, &numWindows, &bytesLeft, &windowData);

    if (result != Success || windowData == None) {
        contextDebug() << "Cannot get win props for root";
        // Set back the old error handler
        XSync(dpy, False);
        XSetErrorHandler(oldHandler);
        return;
    }

    bool interestingWindowsFound = false;
    // Start reading the windows from the top
    Window *wins = (Window *)windowData;
    for (long i = numWindows - 1 - FIXED_ON_TOP; i >= 0; --i) {
        contextDebug() << "Checking window" << i << wins[i];

        // TODO: Is it enough to listen to the root window for
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

        // The valueChanged is emitted in a delayed way, since this
        // function is called from subscribe, and emitting
        // valueChanged there makes libcontextsubscriber block.
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection,
                                  Q_ARG(QString, fullScreenKey), Q_ARG(bool, fullScreen));

        // We have successfully checked at least one interesting
        // window; don't continue with the others.
        break;
    }

    if (!interestingWindowsFound) {
        // There were no windows except the desktop + notifications
        QMetaObject::invokeMethod(this, "emitValueChanged", Qt::QueuedConnection,
                                  Q_ARG(QString, fullScreenKey), Q_ARG(bool, false));
    }

    XFree(windowData);

    // Set back the old error handler
    XSync(dpy, False);
    XSetErrorHandler(oldHandler);
}

/// Check whether Xlib has an event for us. If so, process it: check
/// whether the stacking order of the root window has changed, and if
/// so, check the fullscreen status again.
void FullScreenPlugin::onXEvent()
{
    XEvent event;
    int numEvents = XEventsQueued(dpy, QueuedAfterReading);
    contextDebug() << "Can read from X, no of events" << numEvents;
    if (numEvents <= 0) {
        contextDebug() << "No events in the queue";
        return;
    }

    for (int i=0; i < numEvents; ++i) {
        // This blocks until we get an event, but now there should be one
        contextDebug() << "Trying to read an event from X (blocking)";
        XNextEvent(dpy, &event);
        contextDebug() << "Got an event";

        if (event.type == PropertyNotify && event.xproperty.window == DefaultRootWindow(dpy)
            && event.xproperty.atom == clientListStackingAtom) {
            contextDebug() << "Interesting event";
            // We're anyway going to check the full screen property;
            // no need to check the other events we possibly have
            cleanEventQueue();
            checkFullScreen();
            break;
        }
    }
}

/// Implementation of the IProviderPlugin::subscribe function. If the
/// fullscreen property was subscribed to, initiate the needed X
/// things (e.g., start listening to root window events) and emit
/// subscribeFinished.
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
        XFlush(dpy);
    }
}

/// Implementation of the IProviderPlugin::unsubscribe. If the
/// fullscreen property was unsubscribed from, stop listening to X
/// events.
void FullScreenPlugin::unsubscribe(QSet<QString> keys)
{
    if (keys.contains(fullScreenKey)) {

        // Stop listening to changes in the client list
        XSelectInput(dpy, DefaultRootWindow(dpy), NoEventMask);
        XFlush(dpy);

        // Clean the event queue so that we don't have old events when
        // a new subscripton comes
        cleanEventQueue();
    }
}

/// For emitting the ready() signal in a delayed way.
void FullScreenPlugin::emitReady()
{
    emit ready();
}

/// For emitting the ready() signal in a delayed way.
void FullScreenPlugin::emitValueChanged(QString key, bool value)
{
    emit valueChanged(key, value);
}

/// Ignore X events which have already arrived. This is to prevent
/// checking the fullscreen status multiple times when there are
/// multiple events in the event queue. Also, when the fullscreen
/// property is unsubscribed, the event queue is cleared, so that the
/// events don't get processed if the property is re-subscribed.
void FullScreenPlugin::cleanEventQueue()
{
    int numEvents = XEventsQueued(dpy, QueuedAfterReading);
    contextDebug() << "Ignoring" << numEvents << "events";
    XEvent event;
    for (int i=0; i<numEvents; ++i) {
        XNextEvent(dpy, &event);
    }
}

} // end namespace

