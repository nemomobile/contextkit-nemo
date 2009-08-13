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

#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <QDateTime>
#include <QFile>

/*!
    \page Logging
    
    \brief The library (and ContexKit in general) use a simple logging system designed 
    to unify the output and make the debugging easier.
    
    \section API
    
    Four types of log messages (presented here in the order of importance) are supported: 
    \b Test, \b Debug, \b Warning and \b Critical. 
    
    The first one, the \b Test message requires some attention. It's meant to be used 
    from tests and unit-tests to log various stages of the test execution. It'll make
    the test output more easily filterable.
    
    The log messages can be used like this:
    
    \code
    contextTest() << "This is some message";
    contextDebug() << "My value is:" << someVariable;
    contextWarning() << "Expecting key:" << something.getKey();
    contextCritical() << 5 << "is bigger than" << 4;
    \endcode

    Notice that the logging framework (very much like ie \b qDebug) automatically 
    ads whitespace. So:

    \code
    contextDebug() << "My value is" << 5 << "and should be 5";
    \endcode

    ...will actually print:

    \code
    My value is 5 and should be 5
    \endcode
    
    \section compilecontrol Compile-time verbosity control
    
    During the compile time certain defines can be used to turn-off debug messages. 
    Those defines are:
    
    \code
    CONTEXT_LOG_HIDE_TEST
    CONTEXT_LOG_HIDE_DEBUG
    CONTEXT_LOG_HIDE_WARNING
    CONTEXT_LOG_HIDE_CRITICAL
    \endcode
    
    A given define makes a respective macro message evaluate to an empty code. To be precise:
    it makes the macro message evaluate to an inline do-nothing class that is optimized by the
    compiler to do nothing.
    
    When ie. \c CONTEXT_LOG_HIDE_DEBUG define is used to turn off \c contextDebug()
    messages, the actual string content of the debug messages is \b not included in the binary 
    and during runtime the machine does not spend time evaluating it.
    
    Those compile-time control defines are integrated in the build/configure system.
    
    \section runtimecontrol Run-time verbosity control
    
    During run-time, the amount of debugging can be limited (filtered) but it can't be increased
    (expanded). In other words, if a package was compiled with warnings-only, it's not possible 
    to make it show debug messages at runtime. But it is possible to make it criticals-only. 
    
    The filtering happens via env variables. The major player is the \c CONTEXT_LOG_VERBOSITY variable
    which can be set to \c TEST, \c DEBUG, \c WARNING and \c CRITICAL. The \c CONTEXT_LOG_VERBOSITY 
    specifies the minimal level of the messages shown. Ie. \c CONTEXT_LOG_VERBOSITY set to 
    \c WARNING will show only warning and criticals. 
    
    The format of the output can be tweaked with \c CONTEXT_LOG_HIDE_TIMESTAMPS and \c CONTEXT_LOG_USE_COLOR. 
    The first one makes the messages shorter by skipping the timestamp info. The second one adds a 
    little bit of ANSI coloring to the messages.
    
    \c CONTEXT_LOG_SHOW_MODULE will filter-out (kill) all messages \b except the ones coming from the 
    specified module. Ie.:
    
    \code
    CONTEXT_LOG_SHOW_MODULE="subscriber" ./some-binary
    \endcode
    
    ...will run \c ./some-binary showing log messages \b only from \c subscriber module.
    
    Lastly, \c CONTEXT_LOG_HIDE_MODULE will hide log messages coming from the specified module. 
    All other messages will be show.

    \section modules Modules in logging

    In previous section we discussed and mentioned modules. For the purpose of logging, 
    a module is a piece of code (not neccesarily limited to one binary or shared object) that
    forms one component (feature-wise). Specyfying and naming the modules is used
    to set the origin of the logging messages. 

    The logging module is set using the \c CONTEXT_LOG_MODULE_NAME define. It should be
    (in most cases) defined in the build system and automatically applied to the whole source code. 
    Typically (with autotools) this can be achieved with something similar too:

    \code
    ...
    AM_CXXFLAGS = '-DCONTEXT_LOG_MODULE_NAME="libtest"'
    ...
    \endcode
    
    If \c CONTEXT_LOG_MODULE_NAME is undefined, the log messages will be marked as coming from an
    \b "Undefined" module.


    \section features Featues

    It's possible also to assign logging messages to feature groups and control the output 
    based on that. Features can be compared to tags - one message can belong to zero or more 
    features. To add to a feature to a log message:

    \code 
    contextDebug() << contextFeature("threads") << "Message goes here" << someVariable;
    contextDebug() << contextFeature("threads") << contextFeature("something") << "Message...";
    \endcode

    It doesn't matter where features are added to the message. There is no specific order required. 
    The following syntax is supported as well:

    \code
    contextDebug() << contextFeature("threads") << "Some message..." << contextFeature("another");
    \endcode

    There are two enviornment variables that control the output of messages vs. features: \b 
    CONTEXT_LOG_SHOW_FEATURES and \b CONTEXT_LOG_HIDE_FEATURES. Both take a comma-separated
    list of features. 

    If you specify CONTEXT_LOG_SHOW_FEATURES only messages with given features will be printed to
    the screen. If you specify \b CONTEXT_LOG_HIDE_FEATURES, messages with the specified features
    will be hidden (not displayed). For example: 

    \code
    CONTEXT_LOG_SHOW_FEATURES="threads,util" ./some-binary
    \endcode

    ...will make \b only the messages belonging to "threads" or "util" features displayed. 

    \code
    CONTEXT_LOG_HIDE_FEATURES="threads,util" ./some-binary
    \endcode

    ...will hide all logging messages belonging to "threads" and "util" feature groups.

    \section vanilla Vanilla

    If the default logging output is too much for you, it's possible to set a CONTEXT_LOG_VANILLA
    enviornment variable. This will simplify the logging output greatly -- no timestamps will be printed, 
    no module information will be printed, no line/function/class info will be printed. 
*/

/* ContextFeature */

/*!
    \class ContextFeature

    \brief This class represents a "feature" in the logging framework/system.

    A feature can be ie. "multithreading", "introspection", "dbus" or anything that makes sense
    in your setup. Using features you can later get more filtered debug output. You most likely
    want to use this class like this:
    
    
    \code
    ...
    contextDebug() << ContextFeature("introspection") << "Message";
    ...
    \endcode
    
    One message can belong to many features or to none.
*/

/// Constructor for a new feature.\a name is the feature name.
ContextFeature::ContextFeature(QString name) : featureName(name)
{
}

/// Returns the name of the feature.
QString ContextFeature::getName() const
{
    return featureName;
}

/* ContextRealLogger */

/*!
    \class ContextRealLogger

    \brief A real logging class.

    This is used by the actual macros to print messages.
*/

bool ContextRealLogger::showTest = true;
bool ContextRealLogger::showDebug = true;
bool ContextRealLogger::showWarning = true;
bool ContextRealLogger::showCritical = true;
bool ContextRealLogger::hideTimestamps = false;
bool ContextRealLogger::useColor = false;
char* ContextRealLogger::showModule = NULL;
char* ContextRealLogger::hideModule = NULL;
bool ContextRealLogger::initialized = false;
bool ContextRealLogger::vanilla = false;
QStringList ContextRealLogger::showFeatures = QStringList();
QStringList ContextRealLogger::hideFeatures = QStringList();

/// Initialize the class by checking the enviornment variables and setting 
/// the message output params. The log level is set from \c CONTEXT_LOG_VERBOSITY
/// and from this env var the showTest, showDebug, showWarning... are set. By default
/// everything is displayed at runtime. It's also possible to not show timestamps in 
/// messages and spice-up the output with some color.
void ContextRealLogger::initialize()
{
    if (getenv("CONTEXT_LOG_HIDE_TIMESTAMPS") != NULL)
        hideTimestamps = true;

    if (getenv("CONTEXT_LOG_USE_COLOR") != NULL)
        useColor = true;
    
    // Check feature enablers (showFeatures)
    const char *showFeaturesStr = getenv("CONTEXT_LOG_SHOW_FEATURES");
    if (showFeaturesStr) {
        foreach (QString f, QString(showFeaturesStr).split(',')) 
            showFeatures << f.trimmed();
    }
    
    // Check feature hide (hideFeatures)
    const char *hideFeaturesStr = getenv("CONTEXT_LOG_HIDE_FEATURES");
    if (hideFeaturesStr) {
        foreach (QString f, QString(hideFeaturesStr).split(',')) 
            hideFeatures << f.trimmed();
    }

    // Show/hide given module
    showModule = getenv("CONTEXT_LOG_SHOW_MODULE");
    hideModule = getenv("CONTEXT_LOG_HIDE_MODULE");

    // Vanilla
    if (getenv("CONTEXT_LOG_VANILLA")) 
        vanilla = true;

    // Check and do verbosity
    const char *verbosity = getenv("CONTEXT_LOG_VERBOSITY");
    if (! verbosity)
        return;
                
    if (strcmp(verbosity, "TEST") == 0) {
        // Do nothing, all left true
    } else if (strcmp(verbosity, "DEBUG") == 0) {
        showTest = false;
    } else if (strcmp(verbosity, "WARNING") == 0) {
        showTest = false;
        showDebug = false;
    } else if (strcmp(verbosity, "CRITICAL") == 0) {
        showTest = false;
        showDebug = false;
        showWarning = false;
    } else if (strcmp(verbosity, "NONE") == 0) {
        showDebug = false;
        showTest = false;
        showDebug = false;
        showWarning = false;
    }
    
    initialized = true;
}

/// Constructor. Called by the macros. \a func is the function name, \a file is
/// is the current source file and \a line specifies the line number.
ContextRealLogger::ContextRealLogger(int type, const char *module, const char *func, const char *file, int line) 
    : QTextStream(), msgType(type), moduleName(module)
{
    if (! initialized) {
        // This is not thread safe, but our initialization depends on 
        // non-mutable parts anyways, so we should be ok.
        initialize();
    }
    
    setString(&data);
    
    // Add timestamp
    if (! hideTimestamps && ! vanilla)
        *this << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // Module name
    if (! vanilla)
        *this << QString("[" + QString(module) + "]");
    
    // Message name
    switch(type) {
        case CONTEXT_LOG_MSG_TYPE_DEBUG:
            if (! vanilla)
                *this << "DEBUG";
            break;
        case CONTEXT_LOG_MSG_TYPE_WARNING:
            *this << ((useColor) ? "\033[103mWARNING\033[0m" : "WARNING");
            break;
        case CONTEXT_LOG_MSG_TYPE_CRITICAL:
            *this << ((useColor) ? "\033[101mCRITICAL\033[0m" : "CRITICAL");
            break;
        case CONTEXT_LOG_MSG_TYPE_TEST:
            *this << "TEST";
            break;
        default:
            *this << "UNKNOWN";
            break;
    }

    // File, line and function...
    
    if (! vanilla)
        *this << QString("[" + QString(file) + ":" + QString::number(line) + ":" + QString(func) + "]");
}

bool ContextRealLogger::shouldPrint()
{
    // First try to eliminated based on message type...
    if (msgType == CONTEXT_LOG_MSG_TYPE_DEBUG && ! showDebug)
        return false;
    else if (msgType == CONTEXT_LOG_MSG_TYPE_WARNING && ! showWarning) 
        return false;
    else if (msgType == CONTEXT_LOG_MSG_TYPE_TEST && ! showTest) 
        return false;
    else if (msgType == CONTEXT_LOG_MSG_TYPE_CRITICAL && ! showCritical) 
        return false;
        
    // Now try to eliminate based on module name...
    if (showModule && strcmp(showModule, moduleName) != 0) 
        return false;
    
    if (hideModule && strcmp(hideModule, moduleName) == 0) 
        return false;
        
    // Now try to eliminate by feature name
    foreach(QString feature, features) {
        if (hideFeatures.contains(feature))
            return false;
    }
    
    if (showFeatures.length() > 0) {
        foreach(QString feature, showFeatures) {
            if (features.contains(feature))
                return true;
            else
                return false;
        }
    }
            
    return true;
}

/// Append (print) all the features, separated with commas and wrapped in brackets.
void ContextRealLogger::appendFeatures()
{
    if (features.length() == 0)
        return;
    
    QTextStream::operator<<('[');
    int i;
    
    for (i = 0; i < features.length(); i++) {
        QTextStream::operator<<(QString ("#" + features.at(i)));
        if (i < features.length() - 1)
            QTextStream::operator<<(", ");
    }

    QTextStream::operator<<(']');
}

/// Operator for appending features.
ContextRealLogger& ContextRealLogger::operator<< (const ContextFeature &f)
{
    features << f.getName();
    return *this;
}

/// Destructor, prints \b end-of-line before going down.
ContextRealLogger::~ContextRealLogger()
{
    if (shouldPrint()) {
        appendFeatures();
        QTextStream::operator<<('\n');
        QTextStream(stderr) << data;
    }
    
    setDevice(NULL);
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (QChar v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (signed short v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (unsigned short v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (signed int v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (unsigned int v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (signed long v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (unsigned long v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (float v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (double v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (void *v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (const QString &v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (const char *v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

/// Standard QTextStream operators. Automatically adds whitespace.
ContextRealLogger& ContextRealLogger::operator<< (char v)
{
    QTextStream::operator<<(v);
    QTextStream::operator<<(' ');
    return *this;
}

