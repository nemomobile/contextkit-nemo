#ifndef _CONTEXTKIT_TEST_HPP_
#define _CONTEXTKIT_TEST_HPP_

#include <QObject>
#include <QDebug>

#include <contextproperty.h>
#include <functional>


typedef void (*prop_fn_t)(QString const &, QVariant const &);

class CKitProperty : public QObject
{
    Q_OBJECT;
public:
    CKitProperty(QString name, prop_fn_t fn)
        : QObject(0)
        , name(name)
        , prop(name)
        , fn(fn)
    {
        connect(&prop, SIGNAL(valueChanged()),
                this, SLOT(onValueChanged()));
        prop.subscribe();
    }
    virtual ~CKitProperty()
    {
        prop.unsubscribe();
        disconnect(&prop, SIGNAL(valueChanged()),
                   this, SLOT(onValueChanged()));
    }

private slots:
    void onValueChanged()
    {
        fn(name, prop.value());
    }

private:
    QString name;
    ContextProperty prop;
    prop_fn_t fn;
};

static void print_int
(QString const &name, QVariant const &value)
{
    qDebug() << "Got " << name << "=" << value.toInt() << "\n";
}

static void
print_ulonglong(QString const &name, QVariant const &value)
{
    qDebug() << "Got " << name << "=" << value.toULongLong() << "\n";
}

static void
print_bool(QString const &name, QVariant const &value)
{
    qDebug() << "Got " << name << "=" << value.toBool() << "\n";
}

static void
print_string(QString const &name, QVariant const &value)
{
    qDebug() << "Got " << name << "=" << value.toString() << "\n";
}


#endif // _CONTEXTKIT_TEST_HPP_
