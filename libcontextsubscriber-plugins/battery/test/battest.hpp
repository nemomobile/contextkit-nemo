#ifndef _ENERGY_TEST_HPP_
#define _ENERGY_TEST_HPP_

#include <QObject>

#include <contextproperty.h>
#include <functional>


typedef void (*prop_fn_t)(QString const &, QVariant const &);

class Test : public QObject
{
        Q_OBJECT;
public:
        Test(QString name, prop_fn_t fn);
        virtual ~Test();

private slots:
        void onValueChanged();

private:
        QString name;
        ContextProperty prop;
        prop_fn_t fn;
};

#endif // _ENERGY_TEST_HPP_
