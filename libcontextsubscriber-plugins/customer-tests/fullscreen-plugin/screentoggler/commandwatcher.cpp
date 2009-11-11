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

#include "commandwatcher.h"
#include <QSocketNotifier>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QWidget>
#include <QCoreApplication>
#include <fcntl.h>

CommandWatcher::CommandWatcher(int commandfd, QWidget* window, QObject *parent) :
    QObject(parent), commandfd(commandfd), window(window)
{
    commandNotifier = new QSocketNotifier(commandfd, QSocketNotifier::Read, this);
    connect(commandNotifier, SIGNAL(activated(int)), this, SLOT(onActivated()));
}

void CommandWatcher::onActivated()
{
    // read all available input to commandBuffer
    static QByteArray commandBuffer = "";
    static char buf[1024];
    int readSize;
    fcntl(commandfd, F_SETFL, O_NONBLOCK);
    while ((readSize = read(commandfd, &buf, 1024)) > 0)
        commandBuffer += QByteArray(buf, readSize);

    // handle all available whole input lines as commands
    int nextSeparator;
    while ((nextSeparator = commandBuffer.indexOf('\n')) != -1) {
        // split lines to separate commands by semicolons
        QStringList commands = QString::fromUtf8(commandBuffer.constData()).left(nextSeparator).split(";");
        foreach (QString command, commands)
            interpret(command.trimmed());
        commandBuffer.remove(0, nextSeparator + 1);
    }

    if (readSize == 0) // EOF
        QCoreApplication::exit(0);
}

void CommandWatcher::interpret(const QString& command) const
{
    QTextStream out(stdout);
    QTextStream err(stderr);
    if (command == "") {
        err << "Invalid command, use full or normal." << endl;
    } else {
        QStringList args = command.split(" ");
        QString commandName = args[0];
        args.pop_front();

        if (QString("full").startsWith(commandName)) {
            window->setWindowState(Qt::WindowFullScreen);
        } else if (QString("normal").startsWith(commandName)) {
            window->setWindowState(Qt::WindowNoState);
        }
    }
}
