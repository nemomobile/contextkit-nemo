/*
 * Copyright (C) 2008 Nokia Corporation.
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

#ifndef PROPERTY_H
#define PROPERTY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDBusConnection>
#include <QHash>
#include <QVariant>

namespace ContextProvider {

class Property : public QObject
{
    Q_OBJECT

public:
    explicit Property(const QString &key, QObject *parent = 0);
    ~Property();
        
    void setValue(const QVariant &v);
    void unsetValue();
    QVariant get();
    void fakeFirst();
    void fakeLast();
    QString getKey();

private:
    QString key;

signals:
    void firstSubscriberAppeared(const QString &key); 
    void lastSubscriberDisappeared(const QString &key);
};

}

#endif
