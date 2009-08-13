/*
 * Copyright (C) 2008, 2009 Nokia Corporation.
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
 
#ifndef LOGGING_H
#define LOGGING_H

#include <QIODevice>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QBuffer>

#define CONTEXT_LOG_MSG_TYPE_TEST       1
#define CONTEXT_LOG_MSG_TYPE_DEBUG      2
#define CONTEXT_LOG_MSG_TYPE_WARNING    3
#define CONTEXT_LOG_MSG_TYPE_CRITICAL   4

#ifndef CONTEXT_LOG_MODULE_NAME
#define CONTEXT_LOG_MODULE_NAME "unknown"
#endif

class ContextFeature
{
public:
    ContextFeature(QString name);
    QString getName() const;
    
private:
    QString featureName;
};

class ContextRealLogger : public QTextStream
{
public:
    ContextRealLogger(int msgType, const char *module, const char *func, const char *file, int line);
    ~ContextRealLogger();
    
    static bool showTest; ///< Test messages enabled at runtime
    static bool showDebug; ///< Debug messages enabled at runtime
    static bool showWarning; ///< Warning messages enabled at runtime
    static bool showCritical; ///< Critical messages enabled at runtime
    static bool initialized; ///< Class initialized/env vars parsed
    static bool hideTimestamps; ///< Don't print timestamps
    static bool useColor; ///< Use simple colors for output (yellow for warnings, red for criticals)
    static char *showModule; ///< Show messages \b only from the specified module
    static char *hideModule; ///< Hide messages from the specified module
    static QStringList showFeatures; ///< Show messages with \b only the specified features
    static QStringList hideFeatures; ///< Hide messages with the specified features
    static bool vanilla; ///< Use vanilla (stripped-down) logging
    
    static void initialize();
    
    ContextRealLogger &operator<< (const ContextFeature&);

    virtual ContextRealLogger &operator<< (QChar);
    virtual ContextRealLogger &operator<< (signed short);
    virtual ContextRealLogger &operator<< (unsigned short);
    virtual ContextRealLogger &operator<< (signed int);
    virtual ContextRealLogger &operator<< (unsigned int);
    virtual ContextRealLogger &operator<< (signed long);
    virtual ContextRealLogger &operator<< (unsigned long);
    virtual ContextRealLogger &operator<< (float);
    virtual ContextRealLogger &operator<< (double);
    virtual ContextRealLogger &operator<< (void *);
    virtual ContextRealLogger &operator<< (const QString&);
    virtual ContextRealLogger &operator<< (const char *);
    virtual ContextRealLogger &operator<< (char);
 
private:
    
    bool shouldPrint();
    void appendFeatures();

    int msgType; ///< Type of message we're representing.
    const char* moduleName; ///< The module name.
    QString data; ///< Holds the stream data.
    QStringList features;
};

/*!
    \class ContextZeroLogger

    \brief A fake logging class.

    When a certain debug message is disabled at a compile-time the debug macros expand to 
    this class. It has all functions declared as \b inline and fundamentally kills all input
    targeted at it. The compiler optimizes the \b inline by not calling the functions at all and
    not storing the strings at all.
*/

class ContextZeroLogger
{
public:
    /// Constructor. Does nothing.
    inline ContextZeroLogger() {}
    
    /* Stubby ops */
    inline ContextZeroLogger &operator<< (QChar) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (char) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (signed short) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (unsigned short) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (signed int) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (unsigned int) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (signed long) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (unsigned long) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (float) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (double) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (const char *) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (const QString&) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (void *) { return *this;} ///< Does nothing.
    inline ContextZeroLogger &operator<< (const ContextFeature&) { return *this;} ///< Does nothing.
};

/* Macro defs */

#define contextFeature(name) (ContextFeature(name))

#ifdef CONTEXT_LOG_HIDE_TEST
#define contextTest() (ContextZeroLogger())
#else
#define contextTest() (ContextRealLogger(CONTEXT_LOG_MSG_TYPE_TEST, CONTEXT_LOG_MODULE_NAME, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif

#ifdef CONTEXT_LOG_HIDE_DEBUG
#define contextDebug() (ContextZeroLogger())
#else
#define contextDebug() (ContextRealLogger(CONTEXT_LOG_MSG_TYPE_DEBUG, CONTEXT_LOG_MODULE_NAME, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif

#ifdef CONTEXT_LOG_HIDE_WARNING
#define contextWarning() (ContextZeroLogger())
#else
#define contextWarning() (ContextRealLogger(CONTEXT_LOG_MSG_TYPE_WARNING, CONTEXT_LOG_MODULE_NAME, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif

#ifdef CONTEXT_LOG_HIDE_CRITICAL
#define contextCritical() (ContextZeroLogger())
#else
#define contextCritical() (ContextRealLogger(CONTEXT_LOG_MSG_TYPE_CRITICAL, CONTEXT_LOG_MODULE_NAME, __PRETTY_FUNCTION__, __FILE__, __LINE__))
#endif

#endif // LOGGING_H
