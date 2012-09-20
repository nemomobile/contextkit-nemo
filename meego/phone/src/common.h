/*  -*- Mode: C++ -*-
 *
 * contextkit-meego
 * Copyright © 2010, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef COMMON_H
#define COMMON_H
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtDBus/QtDBus>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>

#ifndef WANT_DEBUG
#define TRACE
#else
#include <QDebug>
#define TRACE qDebug()<<QString("[%1] %2(): %3").arg(__FILE__).arg(__func__).arg(__LINE__);
#endif

/*
 * Commonly used QRegExp expressions
 */
#include <QRegExp>
#define MATCH_ANY(p) QRegExp(p,Qt::CaseInsensitive,QRegExp::FixedString)
#define MATCH_ALL QRegExp()

QString stripLineID(QString lineid);

struct OfonoPathProperties
{
    QDBusObjectPath path;
    QVariantMap     properties;
};

Q_DECLARE_METATYPE ( OfonoPathProperties )

QDBusArgument &operator<<(QDBusArgument &argument,
                          const OfonoPathProperties &mystruct);

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                OfonoPathProperties &mystruct);

typedef QList< OfonoPathProperties > QArrayOfPathProperties;

Q_DECLARE_METATYPE ( QArrayOfPathProperties )

inline void registerContextDataTypes() {
    qDBusRegisterMetaType< OfonoPathProperties >();
    qDBusRegisterMetaType< QArrayOfPathProperties >();
}
#endif // COMMON_H
