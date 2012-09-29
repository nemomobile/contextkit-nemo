#ifndef _COR_UDEV_HPP_
#define _COR_UDEV_HPP_
/*
 * Tiny libudev wrapper
 *
 * Copyright (C) 2012 Jolla Ltd.
 * Contact: Denis Zalevskiy <denis.zalevskiy@jollamobile.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <libudev.h>
#include <stddef.h>

namespace cor {

namespace udev {

class Root
{
public:
    Root() : p(udev_new()) {}
    ~Root()
    {
        if (p)
            udev_unref(p);
    }

    operator bool() const { return (p != 0); }

    udev_device *mk_device(char const *path)
    {
        return udev_device_new_from_syspath(p, path);
    }

    struct udev_enumerate *mk_enumerate()
    {
        return udev_enumerate_new(p);
    }

private:
    struct udev *p;
};


class Enumerate
{
public:
    Enumerate(Root &root)
        : p(root.mk_enumerate())
    {}

    ~Enumerate()
    {
        if (p)
            udev_enumerate_unref(p);
    }

    operator bool() const { return (p != 0); }

    void subsystem_add(char const *name)
    {
        udev_enumerate_add_match_subsystem(p, name);
    }

    struct udev_list_entry *devices()
    {
        udev_enumerate_scan_devices(p);
        return udev_enumerate_get_list_entry(p);
    }

private:
    struct udev_enumerate *p;
};

class ListEntry
{
public:
    ListEntry(Enumerate &e)
        : p(e.devices()) {}

    ListEntry(struct udev_list_entry *p)
        : p(p) {}

    template <typename T>
    void for_each(T const &fn)
    {
        struct udev_list_entry *entry;
        udev_list_entry_foreach(entry, p) {
            if (!fn(ListEntry(entry)))
                break;
        }
    }

    char const *path() const
    {
        return udev_list_entry_get_name(p);
    }
private:
    struct udev_list_entry *p;
};


class Device
{
public:
    Device(Root &root, char const *path)
        : p(root.mk_device(path))
    { }

    operator bool() const { return (p != 0); }

    ~Device()
    {
        if (!p)
            udev_device_unref(p);
    }

    char const *attr(char const *name) const
    {
        return udev_device_get_sysattr_value(p, name);
    }

private:
    struct udev_device *p;
};

} // namespace udev

} // namespace cor

#endif // _COR_UDEV_HPP_
