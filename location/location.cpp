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

#include "location.h"
#include "gypsy_interface.h"

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QXmlQuery>
#include <QDomDocument>
#include <QDBusPendingCallWatcher>

const QString LocationProvider::gypsyService("org.freedesktop.Gypsy");
const QString LocationProvider::satPositioningState("Location.SatPositioningState"); ///on, searching, off

const QString LocationProvider::coordinates("Location.Coordinates");
const QString LocationProvider::heading("Location.Heading");


IProviderPlugin* pluginFactory(const QString& constructionString)
{
	Q_UNUSED(constructionString)
	return new LocationProvider();
}

LocationProvider::LocationProvider() :
  gpsDevice(NULL), position(NULL), course(NULL)
{
	qDebug() << "LocationProvider " << "Initializing LocationProvider provider";

	QDBusInterface interface(gypsyService, "/org/freedesktop/Gypsy",
				 "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), this);

	QDBusReply<QString> reply = interface.call("Introspect");

	QString xml = reply.value();

	QString devicePath = parseOutNode(xml);

	qDebug()<<"device path: "<<devicePath;

	if(!devicePath.isEmpty())
	{
		 gpsDevice = new OrgFreedesktopGypsyDeviceInterface(gypsyService, devicePath,
							   QDBusConnection::systemBus(), this);
		 position = new OrgFreedesktopGypsyPositionInterface(gypsyService, devicePath, QDBusConnection::systemBus(), this);
		 
		 connect(gpsDevice,SIGNAL(ConnectionStatusChanged(bool)),this,SLOT(connectionStatusChanged(bool)));
		 connect(gpsDevice,SIGNAL(FixStatusChanged(int)),this, SLOT(fixStatusChanged(int)));
		 
		 connect(position,SIGNAL(PositionChanged(int,int,double,double,double)), this, SLOT(positionChanged(int,int,double,double,double)));
		 
		 course = new OrgFreedesktopGypsyCourseInterface(gypsyService, devicePath, QDBusConnection::systemBus(), this);
		 connect(course, SIGNAL(CourseChanged(int,int,double,double,double)),
			 this, SLOT(courseChanged(int,int,double,double,double)));
		 
	}
	QMetaObject::invokeMethod(this, "ready", Qt::QueuedConnection);
}

LocationProvider::~LocationProvider()
{

}

void LocationProvider::subscribe(QSet<QString> keys)
{
	qDebug() << "LocationProvider " << "subscribed to LocationProvider provider";

	if(!subscribedProps.count()) onFirstSubscriberAppeared();

	subscribedProps.unite(keys);

	if (subscribedProps.contains(coordinates)) {
	  getCoordinates();
	}
	if (subscribedProps.contains(heading)) {
	  getHeading();
	}

	QMetaObject::invokeMethod(this, "emitSubscribeFinished", Qt::QueuedConnection);
}

void LocationProvider::unsubscribe(QSet<QString> keys)
{
	subscribedProps.subtract(keys);
	if(!subscribedProps.count()) onLastSubscriberDisappeared();
}

void LocationProvider::onFirstSubscriberAppeared()
{
	qDebug("first subscriber appeared!");
	qDebug() << "LocationProvider " << "First subscriber appeared, connecting to Gypsy";

	QMetaObject::invokeMethod(this, "updateProperties", Qt::QueuedConnection);
}

void LocationProvider::onLastSubscriberDisappeared()
{
	qDebug() << "LocationProvider" << "Last subscriber gone, destroying LocationProvider connections";

}

void LocationProvider::updateProperties()
{
	///TODO: update properties

	if(!gpsDevice || !gpsDevice->isValid())
	{
		qDebug("gpsDevice is not valid");
		return;
	}
	bool isConnected = gpsDevice->GetConnectionStatus();
	int fixStatus = gpsDevice->GetFixStatus();

	qDebug()<<" connected? "<<isConnected<<" fix status: "<<fixStatus;

	if(!isConnected)
		Properties[satPositioningState] = "off";
	else
		Properties[satPositioningState] = fixStatus == 1 ? "searching":"on";

	foreach(QString key, subscribedProps)
	{
		emit valueChanged(key, Properties[key]);
	}
}

void LocationProvider::emitSubscribeFinished()
{
	foreach(QString key, subscribedProps)
	{
		emit subscribeFinished(key);
	}
}

QString LocationProvider::parseOutNode(QString xml)
{
	QXmlQuery query;
	query.setFocus(xml);
	query.setQuery("/node/node");

	if(!query.isValid())
		return 0;

	query.evaluateTo(&xml);

	if(xml.isEmpty() || xml.isNull())
		return "";

	QDomDocument doc;
	doc.setContent(xml);

	QDomNodeList nodes = doc.elementsByTagName("node");

	if(!nodes.size()) return "";

	QDomNode node = nodes.at(0);

	return "/org/freedesktop/Gypsy/"+node.toElement().attribute("name");
}

void LocationProvider::getCoordinates()
{
  if (!position->isValid()) {
    qDebug() << "position interface is invalid!";
    return;
  }
  QDBusPendingReply<int, int, double, double, double> reply = position->GetPosition();
  QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
  connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), 
	  this, SLOT(getPositionFinished(QDBusPendingCallWatcher*)));
}

void LocationProvider::getHeading()
{
  if (!course->isValid()) {
    qDebug() << "course interface is invalid!";
    return;
  }
  QDBusPendingReply<int, int, double, double, double> reply = course->GetCourse();
  QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
  connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), 
	  this, SLOT(getCourseFinished(QDBusPendingCallWatcher*)));
}

void LocationProvider::updateProperty(const QString& key, const QVariant& value)
{
    bool changed = true;
    if (Properties[key] == value) {
      changed = false;
    }
    
    Properties[key] = value;
    if (subscribedProps.contains(key) && changed) {
      emit valueChanged(key, value);
    }
}

void LocationProvider::fixStatusChanged(int)
{
	updateProperties();
}

void LocationProvider::positionChanged(int fields, int timestamp, double latitude, double longitude, double altitude)
{
  //FIXME: use fields correctly
    qDebug() << "LocationProvider:" << "New coordinate values:";
    qDebug() << "\tfields:" << fields << endl
             << "\ttimestamp:" << timestamp << endl
             << "\tlatitude:" << latitude << endl
             << "\tlongitude:" << longitude << endl
             << "\taltitude:" << altitude;

    QList<QVariant> coords;
    coords.append(QVariant(latitude));
    coords.append(QVariant(longitude));
    coords.append(QVariant(altitude));
    updateProperty("Location.Coordinates", coords);
}

void LocationProvider::courseChanged(int fields, int timestamp, double speed, double direction, double climb)
{
  //FIXME: use fields correctly
  qDebug() << "LocationProvider:" << "New course values:";
  qDebug() << "\tfields:" << fields << endl
	   << "\ttimestamp:" << timestamp << endl
	   << "\tspeed:" << speed << endl
	   << "\tdirection:" << direction << endl
	   << "\tclimb:" << climb << endl;
  updateProperty(heading, direction);
}


void LocationProvider::connectionStatusChanged(bool)
{
	updateProperties();
}

void LocationProvider::getPositionFinished(QDBusPendingCallWatcher *watcher)
{
  QDBusPendingReply<int, int, double, double, double> reply = *watcher;
  if (reply.isError()) {
    qDebug() << "GetPosition resulted in error!";
  } else {
    positionChanged(reply.argumentAt<0>(),
		    reply.argumentAt<1>(),
		    reply.argumentAt<2>(),
		    reply.argumentAt<3>(),
		    reply.argumentAt<4>());
  }
  watcher->deleteLater();
}

void LocationProvider::getCourseFinished(QDBusPendingCallWatcher* watcher)
{
  QDBusPendingReply<int, int, double, double, double> reply = *watcher;
  if (reply.isError()) {
    qDebug() << "GetCourse resulted in error!";
  } else {
    courseChanged(reply.argumentAt<0>(),
		  reply.argumentAt<1>(),
		  reply.argumentAt<2>(),
		  reply.argumentAt<3>(),
		  reply.argumentAt<4>());
  }
  watcher->deleteLater();
}
